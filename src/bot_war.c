/*
 * bot_war.c - Botların kabal yaşamı: liderlik ve üyelik (gerçek 'induct'
 *             komutuyla), kabal savaşı (kim listesi + bölge devriyesi ile
 *             hedef bulma), yardım çağrısı, karargâh baskını ve savunma.
 *
 * Kural: bot yalnızca bir oyuncunun bilebileceğini bilir. Kim çevrimiçi ve
 * hangi kabalda ('kim'), aynı bölgede kim var ('nerede'), kabal eşyası
 * nerede ('cabal_scan') ve haritadaki oda/bölge yerleşimi.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "db.h"
#include "bot.h"

#define BOT_CABAL_LEVEL   20      /* kabala girmek için asgari seviye (botların kendi kuralı) */
#define BOT_RAID_LEVEL    25      /* baskına katılmak için asgari seviye */

static int raid_next_pulse[MAX_CABAL];     /* kabal başına baskın soğuması */

/* ---------------------------------------------------------------------
 * yardımcılar
 * ------------------------------------------------------------------ */
static int pct( int cur, int max )
{
    return max > 0 ? cur * 100 / max : 100;
}

static bool pk_capable( CHAR_DATA *ch )
{
    return ch != NULL && !IS_NPC(ch) && ch->cabal != CABAL_NONE && ch->pcdata->oyuncu_katli == 1
        && !IS_SET( ch->act, PLR_GHOST ) && !IS_IMMORTAL(ch);
}

/* 'kim' listesinde görünen bir oyuncu mu (ölümsüz görünmezliği hariç) */
static bool visible_in_who( CHAR_DATA *viewer, CHAR_DATA *wch )
{
    if ( wch == NULL || IS_NPC(wch) || wch->in_room == NULL )
        return FALSE;
    if ( wch->invis_level > get_trust( viewer ) )
        return FALSE;
    if ( !IS_BOT(wch) && ( wch->desc == NULL || wch->desc->connected != CON_PLAYING ) )
        return FALSE;
    return TRUE;
}

/* 'nerede <isim>' gibi: aynı bölgede ve görülebilir */
static bool in_my_area( CHAR_DATA *ch, CHAR_DATA *victim )
{
    return victim != NULL && victim->in_room != NULL && ch->in_room != NULL
        && victim->in_room->area == ch->in_room->area && can_see( ch, victim );
}

static CHAR_DATA *online_pc_by_name( const char *name )
{
    CHAR_DATA *wch;

    for ( wch = char_list; wch != NULL; wch = wch->next )
        if ( !IS_NPC(wch) && !str_cmp( wch->name, name ) )
            return wch;
    return NULL;
}

/* görülen düşmanları hatırla (odaya girince) */
static void note_seen( BOT_DATA *bot, CHAR_DATA *pc )
{
    int i, oldest = 0;

    for ( i = 0; i < BOT_SEENPC_MAX; i++ )
    {
        if ( bot->seen_id[i] == pc->id )
        {
            bot->seen_vnum[i]  = pc->in_room->vnum;
            bot->seen_pulse[i] = bot_pulse;
            return;
        }
        if ( bot->seen_pulse[i] < bot->seen_pulse[oldest] )
            oldest = i;
    }
    bot->seen_id[oldest]    = pc->id;
    bot->seen_vnum[oldest]  = pc->in_room->vnum;
    bot->seen_pulse[oldest] = bot_pulse;
}

static ROOM_INDEX_DATA *last_seen_room( BOT_DATA *bot, long id, int max_age )
{
    int i;

    for ( i = 0; i < BOT_SEENPC_MAX; i++ )
        if ( bot->seen_id[i] == id && bot_pulse - bot->seen_pulse[i] <= max_age )
            return get_room_index( bot->seen_vnum[i] );
    return NULL;
}

void bot_war_note_room( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch, *rch;

    if ( ch == NULL || ch->in_room == NULL || ch->cabal == CABAL_NONE )
        return;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
        if ( rch != ch && !IS_NPC(rch) && rch->cabal != CABAL_NONE && rch->cabal != ch->cabal && can_see( ch, rch ) )
            note_seen( bot, rch );
}

/* ---------------------------------------------------------------------
 * kabal uygunluğu ve tercih
 * ------------------------------------------------------------------ */
static bool is_caster_class( CHAR_DATA *ch )
{
    switch ( ch->iclass )
    {
    case CLASS_INVOKER: case CLASS_CLERIC: case CLASS_TRANSMUTER:
    case CLASS_NECROMANCER: case CLASS_ELEMENTALIST: case CLASS_VAMPIRE:
        return TRUE;
    }
    return FALSE;
}

/* oyunun 'induct' kuralları + kabal karakteri (yönelim/etik) */
bool bot_cabal_fits( CHAR_DATA *ch, int cabal )
{
    if ( ch == NULL || IS_NPC(ch) || cabal <= CABAL_NONE || cabal >= MAX_CABAL )
        return FALSE;
    if ( ch->iclass == CLASS_WARRIOR && cabal == CABAL_SHALAFI )
        return FALSE;
    if ( cabal == CABAL_RULER && get_curr_stat( ch, STAT_INT ) < 19 )
        return FALSE;
    switch ( cabal )
    {
    case CABAL_RULER:   return ch->ethos == 1 && !IS_EVIL(ch);
    case CABAL_INVADER: return !IS_GOOD(ch);
    case CABAL_CHAOS:   return !IS_GOOD(ch) && ch->ethos != 1;
    case CABAL_SHALAFI: return is_caster_class( ch );
    case CABAL_BATTLE:  return !is_caster_class( ch );
    case CABAL_KNIGHT:  return !IS_EVIL(ch);
    case CABAL_LIONS:   return !IS_EVIL(ch);
    case CABAL_HUNTER:  return TRUE;
    }
    return FALSE;
}

int bot_choose_cabal( CHAR_DATA *ch )
{
    int order[8], n = 0, i, pref;

    if ( is_caster_class( ch ) )
        pref = ( ch->ethos == 1 && get_curr_stat( ch, STAT_INT ) >= 19 && number_percent() < 40 )
             ? CABAL_RULER : CABAL_SHALAFI;
    else if ( IS_EVIL(ch) )
        pref = number_percent() < 50 ? CABAL_INVADER : CABAL_CHAOS;
    else if ( ch->iclass == CLASS_PALADIN || ( IS_GOOD(ch) && ch->ethos == 1 ) )
        pref = CABAL_KNIGHT;
    else if ( ch->iclass == CLASS_RANGER || ch->iclass == CLASS_THIEF || ch->iclass == CLASS_NINJA )
        pref = IS_GOOD(ch) ? CABAL_LIONS : CABAL_HUNTER;
    else if ( ch->iclass == CLASS_WARRIOR || ch->iclass == CLASS_SAMURAI )
        pref = number_percent() < 70 ? CABAL_BATTLE : CABAL_KNIGHT;
    else
        pref = CABAL_HUNTER;

    if ( bot_cabal_fits( ch, pref ) )
        return pref;
    for ( i = 1; i < MAX_CABAL; i++ )
        if ( bot_cabal_fits( ch, i ) )
            order[n++] = i;
    return n > 0 ? order[number_range( 0, n - 1 )] : CABAL_NONE;
}

/* çevrimiçi lider (induct yetkili) botu */
static BOT_DATA *online_leader( int cabal )
{
    BOT_DATA *b;

    for ( b = bot_list; b != NULL; b = b->next )
        if ( b->ch != NULL && b->ch->cabal == cabal && IS_SET( b->ch->act, PLR_CANINDUCT ) )
            return b;
    return NULL;
}

/* ---------------------------------------------------------------------
 * liderlik: kadroda '!lider <bot> <kabal>' ile tanrılar tarafından
 * atanmış bot, şartları sağlayınca (seviye, katil hakkı) göreve başlar.
 * ------------------------------------------------------------------ */
static void announce_all( const char *text )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    snprintf( buf, sizeof(buf), "{C[Kabal] %s{x\n\r", text );
    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && d->character != NULL )
            send_to_char( buf, d->character );
}

static void bot_leader_check( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    int sn;
    char buf[MAX_STRING_LENGTH];

    if ( bot->leader_cabal <= CABAL_NONE || bot->leader_cabal >= MAX_CABAL )
        return;
    if ( ch->cabal == bot->leader_cabal )
    {
        if ( !IS_SET( ch->act, PLR_CANINDUCT ) )
            SET_BIT( ch->act, PLR_CANINDUCT );
        return;
    }
    if ( ch->cabal != CABAL_NONE || ch->level < BOT_CABAL_LEVEL || ch->pcdata->oyuncu_katli != 1 )
        return;
    if ( !bot_cabal_fits( ch, bot->leader_cabal ) )
        return;

    /* tanrı ataması: induct ile aynı etkiler */
    ch->cabal = bot->leader_cabal;
    for ( sn = 0; sn < MAX_SKILL; sn++ )
        if ( skill_table[sn].cabal == ch->cabal )
            ch->pcdata->learned[sn] = 70;
    SET_BIT( ch->act, PLR_CANINDUCT );
    snprintf( buf, sizeof(buf), "Tanrılar %s'i %s kabalının lideri ilan etti!", ch->name,
              cabal_table[ch->cabal].long_name );
    announce_all( buf );
    bot_log( bot, "%s kabalının lideri oldu.", cabal_table[ch->cabal].short_name );
    bot_chat_event( bot, BOT_EV_CABAL, NULL );
    save_char_obj( ch );
}

/* aday: uygun bir lidere kd ile başvur */
static void bot_cabal_ask( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    BOT_DATA *leader = NULL;
    int cabal, i;
    char buf[MAX_INPUT_LENGTH];

    if ( !bot->pk_istekli || ch->cabal != CABAL_NONE || ch->level < BOT_CABAL_LEVEL
      || ch->pcdata->oyuncu_katli != 1 || bot->leader_cabal > CABAL_NONE )
        return;
    if ( bot_pulse - bot->cabal_ask_pulse < 4 * 60 * 20 )
        return;
    bot->cabal_ask_pulse = bot_pulse;

    cabal = bot_choose_cabal( ch );
    if ( cabal != CABAL_NONE )
        leader = online_leader( cabal );
    for ( i = 1; leader == NULL && i < MAX_CABAL; i++ )
        if ( bot_cabal_fits( ch, i ) && ( leader = online_leader( i ) ) != NULL )
            cabal = i;
    if ( leader == NULL || leader->ch == NULL )
        return;

    snprintf( buf, sizeof(buf), "selam, %s kabalına katılmak istiyorum. seviyem %d, katil hakkım var",
              cabal_table[cabal].short_name, ch->level );
    bot_talk( bot, BOT_CH_TELL, leader->ch, buf );
}

/*
 * Lider bir üyelik isteği duydu (kd/söyle). Yanıt kanalı: konu dışı (kd).
 * Uygunsa 10-25 sn sonra gerçek 'induct' komutu verir.
 */
bool bot_leader_handle( BOT_DATA *leader, CHAR_DATA *speaker, int channel )
{
    CHAR_DATA *ch = leader->ch;
    const char *reply;
    char out[MAX_STRING_LENGTH];
    int reply_ch = ( channel == BOT_CH_SAY || channel == BOT_CH_YELL ) ? BOT_CH_SAY : BOT_CH_TELL;

    if ( ch == NULL || speaker == NULL || IS_NPC(speaker) )
        return FALSE;
    if ( ch->cabal == CABAL_NONE || !IS_SET( ch->act, PLR_CANINDUCT ) )
    {
        /* bir liderin cevabı (kabul/ret) bize geldi: teşekkür et ya da sus */
        if ( IS_SET( speaker->act, PLR_CANINDUCT ) )
        {
            if ( number_percent() < 50 )
                bot_queue_reply( leader, speaker->name, reply_ch, 4 + number_range( 0, 6 ),
                                 reply_ch == BOT_CH_SAY ? "Sağ ol, bekliyorum." : "sağ ol, bekliyorum" );
            return TRUE;
        }
        reply = ch->cabal == CABAL_NONE ? "ben de bir kabalda değilim ki" : "ben lider değilim, lidere sor";
        if ( reply_ch == BOT_CH_SAY )
            reply = ch->cabal == CABAL_NONE ? "Ben de bir kabalın üyesi değilim." : "Lider ben değilim; ona sor.";
    }
    else if ( speaker->cabal == ch->cabal )
        reply = reply_ch == BOT_CH_SAY ? "Zaten bizdensin." : "zaten bizdensin :)";
    else if ( speaker->cabal != CABAL_NONE )
        reply = reply_ch == BOT_CH_SAY ? "Sen başka bir kabala bağlısın." : "sen zaten başka kabaldasın";
    else if ( speaker->pcdata->oyuncu_katli != 1 )
        reply = reply_ch == BOT_CH_SAY ? "Önce görevciden katil hakkı almalısın." : "önce görevciden katlet hakkını al (100 gp), sonra gel";
    else if ( speaker->level < BOT_CABAL_LEVEL )
        reply = reply_ch == BOT_CH_SAY ? "Henüz erken. Biraz daha güçlen, sonra gel." : "biraz erken, 20 level sonra gel";
    else if ( !bot_cabal_fits( speaker, ch->cabal ) )
        reply = reply_ch == BOT_CH_SAY ? "Senin yolun bizimkiyle uyuşmuyor." : "senin yolun bizimkine uymuyor kusura bakma";
    else if ( leader->induct_id != 0 && leader->induct_id != speaker->id )
        reply = reply_ch == BOT_CH_SAY ? "Biraz bekle, önce başkasıyla ilgileniyorum." : "biraz bekle, önce başkasını alıyorum";
    else
    {
        leader->induct_id    = speaker->id;
        leader->induct_pulse = bot_pulse + 4 * number_range( 10, 25 );
        snprintf( out, sizeof(out), reply_ch == BOT_CH_SAY
                  ? "Peki. Seni %s saflarına kabul ediyorum; birazdan yeminini alacağım."
                  : "tamam, seni %s kabalına alıyorum, birazdan yemin töreni :)",
                  cabal_table[ch->cabal].long_name );
        reply = out;
    }
    bot_queue_reply( leader, speaker->name, reply_ch, 6 + number_range( 0, 8 ), reply );
    return TRUE;
}

static void bot_leader_induct( BOT_DATA *leader )
{
    CHAR_DATA *ch = leader->ch, *victim;
    char buf[MAX_INPUT_LENGTH];

    if ( leader->induct_id == 0 || bot_pulse < leader->induct_pulse )
        return;
    victim = bot_char_by_id( leader->induct_id );
    leader->induct_id = 0;
    if ( victim == NULL || IS_NPC(victim) || victim->cabal != CABAL_NONE || ch->cabal == CABAL_NONE )
        return;
    bot_cmd( leader, "induct %s %s", victim->name, cabal_table[ch->cabal].short_name );
    if ( victim->cabal != ch->cabal )
    {
        bot_talk( leader, BOT_CH_TELL, victim, "olmadı, bir sorun çıktı" );
        return;
    }
    bot_log( leader, "%s'i %s kabalına aldı.", victim->name, cabal_table[ch->cabal].short_name );
    snprintf( buf, sizeof(buf), "%s aramıza katıldı, hoş geldin!", victim->name );
    bot_talk( leader, BOT_CH_CABAL, NULL, buf );
    if ( IS_BOT(victim) )
        bot_chat_event( victim->pcdata->bot, BOT_EV_CABAL, NULL );
    save_char_obj( victim );
}

/* ---------------------------------------------------------------------
 * kabal savaşı: hedef seçimi
 * ------------------------------------------------------------------ */
static bool pk_target_ok( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( victim == NULL || victim == ch || IS_NPC(victim) || victim->in_room == NULL )
        return FALSE;
    if ( victim->cabal == CABAL_NONE || victim->cabal == ch->cabal )
        return FALSE;
    if ( victim->pcdata->oyuncu_katli != 1 || IS_SET( victim->act, PLR_GHOST ) )
        return FALSE;
    if ( IS_IMMORTAL(victim) || is_safe_nomessage( ch, victim ) )
        return FALSE;
    if ( !IS_BOT(victim) && ( victim->desc == NULL || victim->desc->connected != CON_PLAYING ) )
        return FALSE;
    return TRUE;
}

/* 'kim' listesinden: düşman kabal, saldırılabilir seviye */
static CHAR_DATA *bot_pk_candidate( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch, *wch, *pick = NULL, *revenge;
    int count = 0;

    if ( ( revenge = bot_char_by_id( bot->revenge_id ) ) != NULL && bot_pulse - bot->revenge_pulse < 4 * 60 * 30
      && visible_in_who( ch, revenge ) && pk_target_ok( ch, revenge ) )
        return revenge;
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
        int weight;

        if ( !visible_in_who( ch, wch ) || !pk_target_ok( ch, wch ) )
            continue;
        weight = 1;
        if ( !IS_BOT(wch) )
            weight = 2;                                  /* gerçek oyuncu: daha ilginç */
        if ( in_my_area( ch, wch ) )
            weight += 2;
        if ( last_seen_room( bot, wch->id, 4 * 60 * 30 ) != NULL )
            weight += 1;
        count += weight;
        if ( number_range( 1, count ) <= weight )
            pick = wch;
    }
    return pick;
}

/* hedefin seviyesine uygun devriye bölgeleri */
static void bot_pk_plan_areas( BOT_DATA *bot, CHAR_DATA *victim )
{
    CHAR_DATA *ch = bot->ch;
    AREA_DATA *area;
    ROOM_INDEX_DATA *seen;
    int n = 0, i;

    bot->pk_area_n = bot->pk_area_i = 0;
    if ( ( seen = last_seen_room( bot, victim->id, 4 * 60 * 40 ) ) != NULL && seen->area != ch->in_room->area )
        bot->pk_areas[n++] = seen->area;
    for ( area = area_first; area != NULL && n < BOT_PK_AREAS; area = area->next )
    {
        if ( IS_SET( area->area_flag, AREA_CABAL | AREA_HOMETOWN ) || area->min_vnum < 100 )
            continue;
        if ( area->low_range > victim->level || area->high_range < victim->level )
            continue;
        if ( area->high_range - area->low_range > 60 || area == ch->in_room->area )
            continue;
        if ( area->low_range > ch->level + 8 )
            continue;
        for ( i = 0; i < n; i++ )
            if ( bot->pk_areas[i] == area )
                break;
        if ( i < n )
            continue;
        if ( number_percent() < 50 )
            bot->pk_areas[n++] = area;
    }
    bot->pk_area_n = n;
}

static void bot_pk_start( BOT_DATA *bot, CHAR_DATA *victim, const char *why )
{
    bot->pk_target_id = victim->id;
    bot->pk_until     = bot_pulse + 4 * 60 * 15;
    bot->substate     = 0;
    bot_pk_plan_areas( bot, victim );
    bot_set_state( bot, BOT_ST_PK );
    bot_log( bot, "kabal savaşı (%s): hedef %s (seviye %d, %s).", why, victim->name, victim->level,
             cabal_table[victim->cabal].short_name );
}

/* odada saldırılabilir bir düşman var mı (av sırasında fırsat) */
bool bot_war_opportunity( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch, *rch;

    if ( !pk_capable( ch ) || ch->fighting != NULL || pct( ch->hit, ch->max_hit ) < 70 )
        return FALSE;
    if ( bot->state == BOT_ST_FOLLOW || bot->state == BOT_ST_CORPSE )
        return FALSE;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
        if ( rch != ch && pk_target_ok( ch, rch ) && can_see( ch, rch )
          && ( bot->pk_istekli || rch->id == bot->revenge_id ) )
        {
            bot_pk_start( bot, rch, "odada karşılaştı" );
            return TRUE;
        }
    return FALSE;
}

/* saldırıya uğradı (PvP): intikam hafızası ve kabal kanalından yardım çağrısı */
void bot_war_attacked( BOT_DATA *bot, CHAR_DATA *attacker )
{
    CHAR_DATA *ch = bot->ch;
    char buf[MAX_INPUT_LENGTH];

    if ( ch == NULL || attacker == NULL || IS_NPC(attacker) || attacker == ch )
        return;
    bot->revenge_id    = attacker->id;
    bot->revenge_pulse = bot_pulse;
    if ( ch->cabal == CABAL_NONE || bot_pulse - bot->help_call_pulse < 4 * 60 * 3 )
        return;
    bot->help_call_pulse = bot_pulse;
    snprintf( buf, sizeof(buf), "yardım! %s %s'de bana saldırdı", attacker->name, bot_area_name( bot ) );
    bot_talk( bot, BOT_CH_CABAL, NULL, buf );
}

/* kabal kanalında yardım çağrısı duyuldu: "yardım! <isim> <bölge>'de ..." */
bool bot_war_help( BOT_DATA *bot, CHAR_DATA *speaker, const char *text )
{
    CHAR_DATA *ch = bot->ch, *attacker;
    char name[MAX_INPUT_LENGTH];
    const char *p;
    AREA_DATA *area;
    ROOM_INDEX_DATA *entry;

    if ( ch == NULL || speaker == NULL || speaker->cabal != ch->cabal || !pk_capable( ch ) )
        return FALSE;
    if ( bot->state == BOT_ST_PK || bot->state == BOT_ST_RAID || bot->state == BOT_ST_FOLLOW
      || bot->state == BOT_ST_CORPSE || ch->fighting != NULL || pct( ch->hit, ch->max_hit ) < 70 )
        return FALSE;
    if ( ( p = strstr( text, "yardım! " ) ) == NULL )
        return FALSE;
    p += strlen( "yardım! " );
    one_argument( (char *) p, name );
    if ( name[0] == '\0' || ( attacker = online_pc_by_name( capitalize( name ) ) ) == NULL )
        return FALSE;
    if ( !pk_target_ok( ch, attacker ) )
        return FALSE;

    bot_pk_start( bot, attacker, "yardım çağrısı" );
    /* bölge adı: çağrıdaki bölgeye git */
    if ( speaker->in_room != NULL && ( area = speaker->in_room->area ) != ch->in_room->area
      && ( entry = area_entry_room( bot, area ) ) != NULL )
    {
        bot->pk_areas[0] = area;
        bot->pk_area_n = 1;
        bot->pk_area_i = 0;
        bot_set_travel( bot, entry->vnum, BOT_ST_PK );
    }
    bot_talk( bot, BOT_CH_CABAL, NULL, number_percent() < 50 ? "geliyorum, dayan" : "tamam yoldayım" );
    return TRUE;
}

/* ---------------------------------------------------------------------
 * kabal savaşı: durum makinesi
 * ------------------------------------------------------------------ */
void bot_pk( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *victim = bot_char_by_id( bot->pk_target_id );
    ROOM_INDEX_DATA *entry;

    if ( victim == NULL || bot_pulse > bot->pk_until || !visible_in_who( ch, victim )
      || !pk_target_ok( ch, victim ) || pct( ch->hit, ch->max_hit ) < 50 )
    {
        bot->pk_target_id = 0;
        bot->next_pk = bot_pulse + number_range( 4 * 60 * 10, 4 * 60 * 30 );
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }
    if ( victim->in_room == ch->in_room && can_see( ch, victim ) )
    {
        if ( bot_cast_buffs( bot ) )
            return;
        if ( bot->substate == 0 )
        {
            bot->substate = 1;
            if ( number_percent() < 50 )
            {
                bot_chat_event( bot, BOT_EV_PK_TAUNT, victim );
                return;
            }
        }
        bot_attack( bot, victim );
        return;
    }
    /* aynı bölgede: 'nerede' onu gösterir, odasına git */
    if ( in_my_area( ch, victim ) )
    {
        if ( bot->path_len == 0 || get_room_index( bot->target_vnum ) != victim->in_room )
            if ( !bot_set_travel( bot, victim->in_room->vnum, BOT_ST_PK ) )
                goto next_area;
        return;
    }
next_area:
    /* devriye: sıradaki aday bölge */
    while ( bot->pk_area_i < bot->pk_area_n )
    {
        AREA_DATA *area = bot->pk_areas[bot->pk_area_i++];

        if ( area == ch->in_room->area )
            continue;
        if ( ( entry = area_entry_room( bot, area ) ) != NULL && bot_set_travel( bot, entry->vnum, BOT_ST_PK ) )
        {
            if ( bot_debug )
                bot_log( bot, "devriye: %s", area->name );
            return;
        }
    }
    bot->pk_target_id = 0;
    bot->next_pk = bot_pulse + number_range( 4 * 60 * 15, 4 * 60 * 40 );
    bot_set_state( bot, BOT_ST_IDLE );
}

/* ---------------------------------------------------------------------
 * baskın: kabal eşyasını düşman karargâhından alıp kendi karargâhına
 * bırakmak (düşman kabalın kabal güçleri kesilir). Savunma: kendi eşyası
 * yerinde değilse peşine düşmek.
 * ------------------------------------------------------------------ */
static OBJ_DATA *cabal_item_root( int cabal, ROOM_INDEX_DATA **room, CHAR_DATA **carrier )
{
    OBJ_DATA *obj = cabal_table[cabal].obj_ptr;

    *room = NULL;
    *carrier = NULL;
    if ( obj == NULL )
        return NULL;
    while ( obj->in_obj != NULL )
        obj = obj->in_obj;
    *room = obj->in_room;
    *carrier = obj->carried_by;
    return cabal_table[cabal].obj_ptr;
}

static bool item_at_home( int cabal )
{
    ROOM_INDEX_DATA *room;
    CHAR_DATA *carrier;

    if ( cabal_item_root( cabal, &room, &carrier ) == NULL )
        return TRUE;                                  /* eşya yok: sorun yok */
    return room != NULL && room->vnum == cabal_table[cabal].room_vnum;
}

static int online_members( int cabal, int min_level )
{
    BOT_DATA *b;
    DESCRIPTOR_DATA *d;
    int n = 0;

    for ( b = bot_list; b != NULL; b = b->next )
        if ( b->ch != NULL && b->ch->cabal == cabal && b->ch->level >= min_level )
            n++;
    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && d->character != NULL && !IS_BOT(d->character)
          && d->character->cabal == cabal && d->character->level >= min_level )
            n++;
    return n;
}

static void raid_join( BOT_DATA *b, int target, long leader_id )
{
    b->raid_cabal     = target;
    b->raid_leader_id = leader_id;
    b->raid_pulse     = bot_pulse;
    b->raid_step       = 0;
    b->path_len = b->path_pos = 0;
    bot_set_state( b, BOT_ST_RAID );
}

static bool raid_ready( BOT_DATA *b )
{
    CHAR_DATA *ch = b->ch;

    return ch != NULL && pk_capable( ch ) && ch->level >= BOT_RAID_LEVEL && ch->fighting == NULL
        && pct( ch->hit, ch->max_hit ) >= 70
        && b->state != BOT_ST_FOLLOW && b->state != BOT_ST_CORPSE && b->state != BOT_ST_RAID
        && b->state != BOT_ST_PK;
}

/* lider ya da kıdemli üye: baskın başlat */
static bool bot_raid_consider( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    BOT_DATA *b;
    int cabal = ch->cabal, target = CABAL_NONE, i, count = 0, joined = 0;
    char buf[MAX_INPUT_LENGTH];

    if ( !raid_ready( bot ) || !bot->pk_istekli )
        return FALSE;
    if ( bot_pulse < raid_next_pulse[cabal] )
        return FALSE;
    if ( !item_at_home( cabal ) )
        return FALSE;                                 /* önce kendi eşyanı kurtar */
    if ( online_members( cabal, BOT_RAID_LEVEL ) < 2 )
        return FALSE;

    /* hedef: eşyası yerinde duran, tercihen üyesi çevrimiçi bir kabal */
    for ( i = 1; i < MAX_CABAL; i++ )
    {
        int w;

        if ( i == cabal || !item_at_home( i ) || cabal_table[i].obj_ptr == NULL )
            continue;
        {
            ROOM_INDEX_DATA *hq = get_room_index( cabal_table[i].room_vnum );
            sh_int tmp[BOT_MAX_PATH];

            if ( hq == NULL || bot_find_path( ch, ch->in_room, hq, tmp, BOT_MAX_PATH, TRUE ) < 0 )
                continue;                             /* karargâhına yol bilinmiyor */
        }
        w = 1 + online_members( i, 1 ) * 3;
        count += w;
        if ( number_range( 1, count ) <= w )
            target = i;
    }
    if ( target == CABAL_NONE )
        return FALSE;

    raid_next_pulse[cabal] = bot_pulse + number_range( 4 * 60 * 90, 4 * 60 * 240 );
    snprintf( buf, sizeof(buf), "baskın! hedef %s, karargâhta toplanıyoruz", cabal_table[target].short_name );
    bot_talk( bot, BOT_CH_CABAL, NULL, buf );
    bot_log( bot, "baskın başlattı: hedef %s.", cabal_table[target].short_name );
    for ( b = bot_list; b != NULL; b = b->next )
        if ( b != bot && b->ch != NULL && b->ch->cabal == cabal && raid_ready( b ) && joined < 5 )
        {
            raid_join( b, target, ch->id );
            joined++;
        }
    raid_join( bot, target, ch->id );
    return TRUE;
}

/* kendi eşyası çalınmış: savunma/kurtarma görevi */
void bot_cabal_alarm( int cabal, CHAR_DATA *thief )
{
    BOT_DATA *b;

    if ( cabal <= CABAL_NONE || cabal >= MAX_CABAL )
        return;
    for ( b = bot_list; b != NULL; b = b->next )
    {
        if ( b->ch == NULL || b->ch->cabal != cabal || b->state == BOT_ST_RAID
          || b->state == BOT_ST_CORPSE || b->state == BOT_ST_FOLLOW )
            continue;
        if ( thief != NULL && !IS_NPC(thief) )
        {
            b->revenge_id = thief->id;
            b->revenge_pulse = bot_pulse;
        }
        b->raid_cabal     = cabal;                    /* kendi kabalı: savunma */
        b->raid_leader_id = 0;
        b->raid_pulse     = bot_pulse;
        b->raid_step       = 10;
        b->path_len = b->path_pos = 0;
        bot_set_state( b, BOT_ST_RAID );
        /* başkasının komutu içindeyiz: hemen konuşma, sıraya koy */
        bot_queue_reply( b, "", BOT_CH_CABAL, number_range( 2, 8 ),
                         number_percent() < 50 ? "eşyamız alındı! peşine düşüyorum" : "karargâha! eşyayı geri alacağız" );
    }
}

static void raid_take_item( BOT_DATA *bot, OBJ_DATA *item )
{
    CHAR_DATA *ch = bot->ch;
    char kw[MAX_INPUT_LENGTH], ckw[MAX_INPUT_LENGTH];

    if ( item->in_obj != NULL && item->in_obj->in_room == ch->in_room )
    {
        bot_obj_keyword( ch, item, item->in_obj->contains, kw, sizeof(kw) );
        bot_obj_keyword( ch, item->in_obj, ch->in_room->contents, ckw, sizeof(ckw) );
        bot_cmd( bot, "al %s %s", kw, ckw );
    }
    else if ( item->in_room == ch->in_room )
    {
        bot_obj_keyword( ch, item, ch->in_room->contents, kw, sizeof(kw) );
        bot_cmd( bot, "al %s", kw );
    }
}


/* eve (karargâha) yol yoksa portal vb. ile cepten çık; çıkış da yoksa FALSE */
static bool raid_go_home( BOT_DATA *bot, ROOM_INDEX_DATA *home )
{
    if ( bot->path_len != 0 )
        return TRUE;
    if ( bot_set_travel( bot, home->vnum, BOT_ST_RAID ) )
        return TRUE;
    if ( bot_escape_pocket( bot ) )
        return TRUE;
    return FALSE;
}

static void raid_end( BOT_DATA *bot, const char *why )
{
    if ( bot_debug || why != NULL )
        bot_log( bot, "baskın bitti: %s.", why != NULL ? why : "-" );
    bot->raid_cabal = CABAL_NONE;
    bot->raid_step = 0;
    bot_set_state( bot, BOT_ST_IDLE );
}

void bot_raid( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch, *carrier, *rch;
    ROOM_INDEX_DATA *iroom, *home, *enemy_hq;
    OBJ_DATA *item;
    int target = bot->raid_cabal;
    bool defend;

    if ( target <= CABAL_NONE || target >= MAX_CABAL || ch->cabal == CABAL_NONE )
    {
        raid_end( bot, NULL );
        return;
    }
    defend = ( target == ch->cabal );
    home = get_room_index( cabal_table[ch->cabal].room_vnum );
    enemy_hq = get_room_index( cabal_table[target].room_vnum );
    if ( home == NULL || enemy_hq == NULL || bot_pulse - bot->raid_pulse > 4 * 60 * 40 )
    {
        raid_end( bot, "süre doldu" );
        return;
    }
    if ( pct( ch->hit, ch->max_hit ) < 35 )
    {
        bot_talk( bot, BOT_CH_CABAL, NULL, "ağır yaralıyım, geri çekiliyorum" );
        raid_end( bot, "yaralı" );
        return;
    }

    /* düşman oyuncu odadaysa (baskın/savunma sırasında) çarpış */
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
        if ( rch != ch && pk_target_ok( ch, rch ) && can_see( ch, rch ) && pct( ch->hit, ch->max_hit ) >= 50 )
        {
            if ( bot->raid_step % 10 != 2 )
                bot_chat_event( bot, BOT_EV_PK_TAUNT, rch );
            bot_attack( bot, rch );
            return;
        }

    item = cabal_item_root( target, &iroom, &carrier );

    if ( !defend )
    {
        switch ( bot->raid_step )
        {
        case 0:                                       /* karargâhta toplan */
            if ( ch->in_room != home )
            {
                if ( bot->path_len == 0 && !bot_set_travel( bot, home->vnum, BOT_ST_RAID ) )
                    raid_end( bot, "karargâha yol yok" );
                return;
            }
            {
                CHAR_DATA *leader = bot_char_by_id( bot->raid_leader_id );
                int mates = 0;

                for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
                    if ( rch != ch && !IS_NPC(rch) && rch->cabal == ch->cabal )
                        mates++;
                if ( ( leader == ch && mates >= 1 ) || ( leader != NULL && leader->in_room == home && mates >= 1
                     && leader->pcdata->bot != NULL && leader->pcdata->bot->raid_step >= 1 )
                  || bot_pulse - bot->raid_pulse > 4 * 60 * 5 )
                {
                    if ( leader == ch )
                        bot_talk( bot, BOT_CH_CABAL, NULL, "hadi, gidiyoruz" );
                    bot->raid_step = 1;
                }
                else if ( bot_cast_buffs( bot ) )
                    return;
            }
            return;
        case 1:                                       /* düşman karargâhına */
            if ( item == NULL || ( carrier == NULL && iroom != enemy_hq )
              || ( carrier != NULL && carrier->cabal != ch->cabal ) )
            {
                raid_end( bot, "eşya yerinde değil" );
                return;
            }
            if ( ch->in_room != enemy_hq )
            {
                if ( bot->path_len == 0 && !bot_set_travel( bot, enemy_hq->vnum, BOT_ST_RAID ) )
                    raid_end( bot, "düşman karargâhına yol yok" );
                return;
            }
            bot->raid_step = 2;
            bot_chat_event( bot, BOT_EV_PK_TAUNT, NULL );
            return;
        case 2:                                       /* eşyayı al */
            if ( carrier == ch )
            {
                bot->raid_step = 3;
                bot_talk( bot, BOT_CH_CABAL, NULL, "eşya bende, dönüyorum!" );
                return;
            }
            if ( carrier != NULL )
            {
                /* bir kabal arkadaşı aldı: eşlik et */
                if ( carrier->cabal == ch->cabal )
                    bot->raid_step = 3;
                else
                    raid_end( bot, "eşyayı başkası aldı" );
                return;
            }
            if ( iroom != ch->in_room )
            {
                raid_end( bot, "eşya burada değil" );
                return;
            }
            if ( !can_see_obj( ch, item ) )
                return;
            raid_take_item( bot, item );
            return;
        case 3:                                       /* eve dön */
            if ( ch->in_room != home )
            {
                if ( !raid_go_home( bot, home ) )
                    raid_end( bot, "eve yol yok" );
                return;
            }
            if ( carrier == ch )
            {
                char kw[MAX_INPUT_LENGTH];

                bot_obj_keyword( ch, item, ch->carrying, kw, sizeof(kw) );
                bot_cmd( bot, "bırak %s", kw );
                if ( item->in_room == home )
                {
                    bot_talk( bot, BOT_CH_CABAL, NULL, "eşya karargâhta, güçleri kesildi!" );
                    bot_log( bot, "%s kabalının eşyasını karargâha getirdi.", cabal_table[target].short_name );
                    raid_end( bot, "başarılı" );
                }
                return;
            }
            if ( iroom == home || carrier == NULL )
                raid_end( bot, "tamamlandı" );
            return;
        }
        raid_end( bot, NULL );
        return;
    }

    /* savunma / kurtarma */
    if ( item == NULL || ( iroom == home && carrier == NULL ) )
    {
        if ( bot->raid_step >= 12 )
            bot_talk( bot, BOT_CH_CABAL, NULL, "eşya yerinde, sakin" );
        raid_end( bot, "eşya yerinde" );
        return;
    }
    if ( carrier == ch )
    {
        if ( ch->in_room != home )
        {
            if ( !raid_go_home( bot, home ) )
                raid_end( bot, "eve yol yok" );
            return;
        }
        {
            char kw[MAX_INPUT_LENGTH];

            bot_obj_keyword( ch, item, ch->carrying, kw, sizeof(kw) );
            bot_cmd( bot, "bırak %s", kw );
            if ( item->in_room == home )
            {
                bot_talk( bot, BOT_CH_CABAL, NULL, "eşyamız yerine döndü!" );
                bot_log( bot, "kabal eşyasını geri getirdi." );
                raid_end( bot, "kurtarıldı" );
            }
        }
        return;
    }
    if ( carrier != NULL )
    {
        /* biri taşıyor: aynı bölgedeyse peşine düş, değilse onun karargâhında bekle */
        if ( carrier->cabal == ch->cabal )
            return;                                   /* arkadaş getiriyor */
        if ( in_my_area( ch, carrier ) )
        {
            if ( carrier->in_room != ch->in_room && bot->path_len == 0 )
                bot_set_travel( bot, carrier->in_room->vnum, BOT_ST_RAID );
            return;
        }
        if ( carrier->cabal != CABAL_NONE )
        {
            ROOM_INDEX_DATA *their = get_room_index( cabal_table[carrier->cabal].room_vnum );
            if ( their != NULL && ch->in_room != their && bot->path_len == 0 )
                bot_set_travel( bot, their->vnum, BOT_ST_RAID );
        }
        bot->raid_step = 12;
        return;
    }
    /* bir odada duruyor */
    if ( iroom != NULL )
    {
        if ( ch->in_room != iroom )
        {
            if ( bot->path_len == 0 && !bot_set_travel( bot, iroom->vnum, BOT_ST_RAID ) )
                raid_end( bot, "eşyanın odasına yol yok" );
            return;
        }
        if ( can_see_obj( ch, item ) )
            raid_take_item( bot, item );
        bot->raid_step = 13;
    }
}

/* ---------------------------------------------------------------------
 * karar noktası: boşta kalınca çağrılır. Bir durum seçtiyse TRUE.
 * ------------------------------------------------------------------ */
/* her düşünmede: liderlik, üyelik başvurusu, bekleyen induct */
void bot_war_tick( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL || ch->in_room == NULL )
        return;
    if ( bot_pulse - bot->cabal_check_pulse > 4 * 60 * 5 )
    {
        bot->cabal_check_pulse = bot_pulse;
        bot_leader_check( bot );
        if ( ch->fighting == NULL )
            bot_cabal_ask( bot );
    }
    if ( bot->induct_id != 0 && ch->fighting == NULL )
        bot_leader_induct( bot );
}

bool bot_war_goal( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL || ch->in_room == NULL )
        return FALSE;

    if ( !pk_capable( ch ) )
        return FALSE;

    /* kendi eşyası çalınmışsa kurtarma */
    if ( !item_at_home( ch->cabal ) && bot_pulse - bot->raid_pulse > 4 * 60 * 15
      && pct( ch->hit, ch->max_hit ) >= 70 && ch->level >= BOT_RAID_LEVEL )
    {
        bot->raid_cabal = ch->cabal;
        bot->raid_leader_id = 0;
        bot->raid_pulse = bot_pulse;
        bot->raid_step = 10;
        bot_set_state( bot, BOT_ST_RAID );
        return TRUE;
    }

    if ( bot_pulse > bot->next_pk && pct( ch->hit, ch->max_hit ) > 85 )
    {
        bot->next_pk = bot_pulse + number_range( 4 * 60 * 20, 4 * 60 * 60 );
        if ( bot_raid_consider( bot ) )
            return TRUE;
        if ( bot->pk_istekli || bot->revenge_id != 0 )
        {
            CHAR_DATA *victim = bot_pk_candidate( bot );

            if ( victim != NULL && number_percent() < ( IS_BOT(victim) ? 45 : 70 ) )
            {
                bot_pk_start( bot, victim, "kim listesi" );
                return TRUE;
            }
        }
    }
    return FALSE;
}
