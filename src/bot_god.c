/*
 * bot_god.c - Ölümsüz (tanrı) bot: tapınakta görünür durur, duaları duyar,
 *             sıkışan ölümlüyü tapınağa alır, insan oyuncuyu karşılar, ara
 *             sıra duyuru yapar, ölümsüz kanalında (imm) sohbet eder.
 *
 * Güvenlik: yalnızca şu komutları kullanır: transfer <isim> <oda>, restore
 * (yalnız insan oyuncu, dua ile, saatte bir), duyuru, immtalk, kd, kdg, söyle,
 * goto <tapınak>, holylight. Ölümlü botlara asla yardım etmez (adalet).
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "bot.h"

#define GOD_HOME_VNUM   ROOM_VNUM_TEMPLE
#define GOD_ACT_TRANSFER 1
#define GOD_ACT_RESTORE  2

#define POOL(name) static const char *name[] =
#define PN(a) ( (int) ( sizeof(a) / sizeof(a[0]) ) )

POOL(god_pray_reply) {
    "Duanı duydum, evladım. Sabırlı ol.", "Gökler seni izliyor; yolundan sapma.",
    "Tanrılar her şeyi görür, her şeyi bilir. Sen işine bak.", "Dilek dilemek kolay; yürümek zor. Yürü.",
    "Seni işittim. Kaderin kendi ellerinde.", "Bir tanrının vakti kıymetlidir; kısa kes.",
    "Dua ettin, karşılığını erdemle öde.", "Gücünü kılıçtan değil, sabırdan al."
};
POOL(god_transfer_reply) {
    "Sana bir kapı açıyorum; tapınakta olacaksın.", "Kaybolanlara yol göstermek görevimdir. Gel.",
    "Bu kez kurtarıyorum; bir daha uçuruma bakmadan atlama.", "Tapınağa dön ve yolunu yeniden düşün."
};
POOL(god_restore_reply) {
    "Yaraların sarıldı. Bunu bir lütuf bil.", "Gücünü geri verdim; boşa harcama.", "Bir kez daha ayağa kalk."
};
POOL(god_refuse_reply) {
    "Hayır. Tanrılar her isteğe boyun eğmez.", "Bunu kendin başarmalısın.", "Bugün değil."
};
POOL(god_tell_reply) {
    "Söyle, evladım.", "Seni dinliyorum.", "Kısa konuş; gökler sabırsızdır.", "Hmm.",
    "Bunu bilmene gerek yok.", "Diyar seninle olsun.", "Ölümlülerin dertleri bitmez.",
    "Doğru yoldasın.", "Gökler bugün sakin."
};
POOL(god_greet_human) {
    "Hoş geldin, {hedef}. Diyar seni bekliyordu.", "{hedef}, yine buradasın. Gökler memnun.",
    "Selam olsun, {hedef}. Yolun açık olsun.", "{hedef} döndü; kılıçlar bilensin."
};
POOL(god_greet_imm) {
    "Selam, {hedef}. Uzun zaman oldu.", "{hedef}, hoş geldin. Diyar sakin, botlar çalışkan.",
    "Yine beraberiz, {hedef}. Ölümlüler bugün hayli hareketli.", "{hedef}! Göklerde bir sandalye boştu."
};
POOL(god_imm_idle) {
    "Ölümlüler bugün de birbirini kovalıyor.", "Selenge'de yağmur var; tapınakta huzur.",
    "Kabal savaşlarını seyretmek eğlenceli.", "Bir ara dünyayı biraz sarsmak lazım.",
    "Yeni gelenler tapınakta dua ediyor; iyi işaret.", "Kim ne kadar güçlendi, izliyorum."
};
POOL(god_duyuru) {
    "Uzaklarda gök gürlüyor; tanrılar diyarı seyrediyor.", "Bu gece yıldızlar Selenge üzerinde parlıyor.",
    "Kadim tapınağın çanları çalıyor: cesurlara selam!", "Rüzgar kuzeyden esiyor; ejderler huzursuz.",
    "Tanrılar ölümlülerin cesaretini takdir etti.", "Gökyüzünde bir kıvılcım çaktı ve söndü."
};
POOL(god_kdg_idle) {
    "Yardıma muhtaç olan var mı?", "Diyar bugün nasıl, ölümlüler?", "Dua edenin sesi göklere ulaşır."
};

static const char *pick( const char **pool, int n )
{
    return pool[number_range( 0, n - 1 )];
}

static bool text_has( const char *text, const char *word )
{
    char low[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; text[i] != '\0' && i < (int) sizeof(low) - 1; i++ )
        low[i] = (char) ( (unsigned char) text[i] < 128 ? tolower( (unsigned char) text[i] ) : text[i] );
    low[i] = '\0';
    return strstr( low, word ) != NULL;
}

bool bot_is_god( BOT_DATA *bot )
{
    return bot != NULL && bot->god_level > 0;
}

/* girişte: taze karakterse ölümsüz seviyesine çıkar; her girişte tapınağa, görünür */
void bot_god_enter( BOT_DATA *bot, CHAR_DATA *ch, bool fresh )
{
    char buf[MAX_STRING_LENGTH];
    int level = URANGE( LEVEL_IMMORTAL, bot->god_level, MAX_LEVEL - 1 );

    if ( fresh || ch->level < LEVEL_IMMORTAL )
    {
        ch->level = level;
        ch->exp   = exp_per_level( ch, ch->pcdata->points ) * level;
        ch->max_hit  = UMAX( ch->max_hit, 1000 );
        ch->max_mana = UMAX( ch->max_mana, 1000 );
        ch->max_move = UMAX( ch->max_move, 1000 );
        ch->pcdata->perm_hit  = ch->max_hit;
        ch->pcdata->perm_mana = ch->max_mana;
        ch->pcdata->perm_move = ch->max_move;
        snprintf( buf, sizeof(buf), "%s", title_table[ch->iclass][level] );
        set_title( ch, buf );
    }
    ch->hit  = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->move = ch->max_move;
    ch->invis_level = 0;                                  /* 'kim' listesinde görünür */
    SET_BIT( ch->act, PLR_HOLYLIGHT );
    REMOVE_BIT( ch->comm, COMM_NOWIZ | COMM_NOKD | COMM_NOKDG | COMM_QUIET );
    if ( ch->in_room != NULL )
        char_from_room( ch );
    char_to_room( ch, get_room_index( GOD_HOME_VNUM ) );
    bot->god_next_act   = bot_pulse + 4 * number_range( 60, 180 );
    bot->god_next_gecho = bot_pulse + 4 * 60 * number_range( 20, 60 );
    bot->god_pending_id = 0;
}

static CHAR_DATA *human_immortal_online( void )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && d->character != NULL && IS_IMMORTAL(d->character)
          && !IS_BOT(d->character) )
            return d->character;
    return NULL;
}

/* bir insan oyuncu girdi: birkaç dakika içinde kd ile karşıla (ölümsüzse imm kanalından) */
void bot_god_greet( BOT_DATA *bot, CHAR_DATA *human )
{
    char out[MAX_STRING_LENGTH];

    if ( bot->ch == NULL || human == NULL || number_percent() > 70 )
        return;
    if ( IS_IMMORTAL(human) )
    {
        bot_fill_ch( bot, pick( god_greet_imm, PN(god_greet_imm) ), human, out, sizeof(out), TRUE );
        bot_queue_reply( bot, "", BOT_CH_IMM, number_range( 20, 90 ), out );
    }
    else
    {
        bot_fill_ch( bot, pick( god_greet_human, PN(god_greet_human) ), human, out, sizeof(out), TRUE );
        bot_queue_reply( bot, human->name, BOT_CH_TELL, number_range( 40, 150 ), out );
    }
}

/* dua, kd, kdg, söyle */
void bot_god_hear( BOT_DATA *bot, CHAR_DATA *speaker, int channel, const char *text )
{
    CHAR_DATA *ch = bot->ch;
    char out[MAX_STRING_LENGTH];
    bool human = bot_is_human( speaker );

    if ( ch == NULL || speaker == NULL || IS_NPC(speaker) )
        return;

    if ( channel == BOT_CH_PRAY )
    {
        if ( IS_IMMORTAL(speaker) )
            return;
        /* sıkışan ölümlü (bot ya da insan): tapınağa al */
        if ( text_has( text, "sıkış" ) || text_has( text, "sikis" ) || text_has( text, "çıkış" )
          || text_has( text, "cikis" ) || text_has( text, "mahsur" ) || text_has( text, "kurtar" )
          || text_has( text, "yol bulam" ) )
        {
            if ( bot->god_pending_id == 0 )
            {
                bot->god_pending_id    = speaker->id;
                bot->god_pending_kind  = GOD_ACT_TRANSFER;
                bot->god_pending_pulse = bot_pulse + 4 * number_range( 8, 25 );
                bot_fill_ch( bot, pick( god_transfer_reply, PN(god_transfer_reply) ), speaker, out, sizeof(out), TRUE );
                bot_queue_reply( bot, speaker->name, BOT_CH_TELL, number_range( 3, 8 ), out );
            }
            return;
        }
        /* yara sarma: yalnız insan oyuncu, saatte bir */
        if ( human && ( text_has( text, "iyileş" ) || text_has( text, "iyiles" ) || text_has( text, "restore" )
                     || text_has( text, "yara" ) || text_has( text, "şifa" ) || text_has( text, "sifa" ) ) )
        {
            if ( bot_pulse - bot->god_last_restore > 4 * 60 * 60 && bot->god_pending_id == 0 )
            {
                bot->god_pending_id    = speaker->id;
                bot->god_pending_kind  = GOD_ACT_RESTORE;
                bot->god_pending_pulse = bot_pulse + 4 * number_range( 6, 20 );
                bot_fill_ch( bot, pick( god_restore_reply, PN(god_restore_reply) ), speaker, out, sizeof(out), TRUE );
                bot_queue_reply( bot, speaker->name, BOT_CH_TELL, number_range( 3, 8 ), out );
            }
            else
                bot_queue_reply( bot, speaker->name, BOT_CH_TELL, number_range( 4, 10 ),
                                 pick( god_refuse_reply, PN(god_refuse_reply) ) );
            return;
        }
        if ( number_percent() < ( human ? 75 : 25 ) )
        {
            bot_fill_ch( bot, pick( god_pray_reply, PN(god_pray_reply) ), speaker, out, sizeof(out), TRUE );
            bot_queue_reply( bot, speaker->name, BOT_CH_TELL, number_range( 6, 20 ), out );
        }
        return;
    }

    /* kd / kdg / söyle: kısa, tanrı ağzı; botlarla nadiren konuşur */
    if ( !human && number_percent() > 15 )
        return;
    if ( channel == BOT_CH_KDG && number_percent() > 40 )
        return;
    bot_fill_ch( bot, pick( god_tell_reply, PN(god_tell_reply) ), speaker, out, sizeof(out), TRUE );
    bot_queue_reply( bot, speaker->name, channel == BOT_CH_SAY ? BOT_CH_SAY : BOT_CH_TELL,
                     number_range( 5, 15 ), out );
}

static void bot_god_pending( BOT_DATA *bot )
{
    CHAR_DATA *victim = bot_char_by_id( bot->god_pending_id );
    int kind = bot->god_pending_kind;

    bot->god_pending_id = 0;
    if ( victim == NULL || IS_NPC(victim) || victim->in_room == NULL || IS_IMMORTAL(victim) )
        return;
    if ( kind == GOD_ACT_TRANSFER )
    {
        bot_log( bot, "%s'i tapınağa aldı (dua).", victim->name );
        bot_cmd( bot, "transfer %s %d", victim->name, GOD_HOME_VNUM );
    }
    else if ( kind == GOD_ACT_RESTORE && bot_is_human( victim ) )
    {
        bot->god_last_restore = bot_pulse;
        bot_log( bot, "%s'i iyileştirdi (dua).", victim->name );
        bot_cmd( bot, "restore %s", victim->name );
    }
}

void bot_god_think( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    ROOM_INDEX_DATA *home = get_room_index( GOD_HOME_VNUM );

    if ( ch == NULL || ch->in_room == NULL )
        return;

    if ( bot->god_pending_id != 0 && bot_pulse >= bot->god_pending_pulse )
    {
        bot_god_pending( bot );
        return;
    }
    if ( home != NULL && ch->in_room != home && bot->god_pending_id == 0 )
    {
        bot_cmd( bot, "goto %d", GOD_HOME_VNUM );
        return;
    }
    if ( ch->position < POS_STANDING )
    {
        bot_cmd( bot, "kalk" );
        return;
    }

    /* ara sıra: duyuru (atmosfer) */
    if ( bot_pulse >= bot->god_next_gecho )
    {
        bot->god_next_gecho = bot_pulse + 4 * 60 * number_range( 40, 120 );
        if ( bot_random_human() != NULL )
            bot_cmd( bot, "duyuru %s", pick( god_duyuru, PN(god_duyuru) ) );
        return;
    }

    /* ara sıra: imm kanalında sohbet (insan ölümsüz varsa) ya da kdg */
    if ( bot_pulse >= bot->god_next_act )
    {
        CHAR_DATA *imm = human_immortal_online();

        bot->god_next_act = bot_pulse + 4 * 60 * number_range( 15, 45 );
        if ( imm != NULL && number_percent() < 60 )
            bot_talk( bot, BOT_CH_IMM, NULL, pick( god_imm_idle, PN(god_imm_idle) ) );
        else if ( bot_random_human() != NULL && number_percent() < 30 )
            bot_talk( bot, BOT_CH_KDG, NULL, pick( god_kdg_idle, PN(god_kdg_idle) ) );
        return;
    }
}
