/*
 * bot_brain.c - Botların karar mekanizması.
 *
 * Her düşünme adımında (yaklaşık saniyede bir) bot dünyaya bakar ve tek bir
 * oyuncu komutu üretir: yürür, saldırır, dinlenir, alışveriş yapar, pratik
 * yapar, görev alır, cesedini toplar, bir liderin peşinden gider ya da kabal
 * savaşına girer. Hiçbir karar oyun çıktısını okumaz; doğrudan veri yapıları
 * incelenir.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "magic.h"
#include "bot.h"

extern AREA_DATA *area_first;
int  get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy );

/* önceden bulunan yetenek numaraları */
static int sn_bash = -1, sn_kick = -1, sn_trip = -1, sn_disarm = -1, sn_dirt = -1;
static int sn_circle = -1, sn_backstab = -1, sn_berserk = -1;
static int sn_cure_light = -1, sn_cure_serious = -1, sn_cure_critical = -1, sn_heal = -1;
static int sn_second_attack = -1, sn_third_attack = -1, sn_dodge = -1, sn_parry = -1;
static int sn_shield_block = -1, sn_enhanced_damage = -1, sn_fast_healing = -1, sn_meditation = -1;
static int sn_hand_to_hand = -1;

#define MAX_BUFF 16
static int buff_sn[MAX_BUFF];
static int buff_count = 0;

static SPEC_FUN *spec_ok[4];
static int spec_ok_count = 0;
static SPEC_FUN *spec_questmaster_fn = NULL;

#define BOT_JUNK_MAX 40

/* ---------------------------------------------------------------------
 * yardımcılar
 * ------------------------------------------------------------------ */
static int pct( int cur, int max )
{
    return max <= 0 ? 100 : cur * 100 / max;
}

static bool is_caster( CHAR_DATA *ch )
{
    switch ( ch->iclass )
    {
    case CLASS_INVOKER: case CLASS_CLERIC: case CLASS_TRANSMUTER:
    case CLASS_NECROMANCER: case CLASS_ELEMENTALIST: case CLASS_VAMPIRE:
        return TRUE;
    }
    return FALSE;
}

static bool is_rogue( CHAR_DATA *ch )
{
    return ch->iclass == CLASS_THIEF || ch->iclass == CLASS_NINJA;
}

static bool is_healer_class( CHAR_DATA *ch )
{
    return ch->iclass == CLASS_CLERIC || ch->iclass == CLASS_PALADIN;
}

static bool knows( CHAR_DATA *ch, int sn, int min )
{
    if ( sn < 0 )
        return FALSE;
    return get_skill( ch, sn ) >= min;
}

static bool can_cast_sn( CHAR_DATA *ch, int sn )
{
    if ( sn < 0 || IS_NPC(ch) )
        return FALSE;
    if ( ch->cabal == CABAL_BATTLE )
        return FALSE;
    if ( ch->level < skill_table[sn].skill_level[ch->iclass] )
        return FALSE;
    if ( ch->pcdata->learned[sn] < 20 )
        return FALSE;
    if ( skill_table[sn].spell_fun == spell_null )
        return FALSE;
    if ( !CABAL_OK( ch, sn ) || !RACE_OK( ch, sn ) )
        return FALSE;
    if ( ch->mana < UMAX( skill_table[sn].min_mana,
                          100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->iclass] ) ) )
        return FALSE;
    return TRUE;
}

static int group_size_here( CHAR_DATA *ch )
{
    CHAR_DATA *rch;
    int n = 0;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
        if ( rch != ch && !IS_NPC(rch) && is_same_group( ch, rch ) && rch->position >= POS_STANDING )
            n++;
    return n;
}

static int level_bonus( CHAR_DATA *ch )
{
    int grp = ch->in_room != NULL ? group_size_here( ch ) : 0;

    if ( grp > 0 )
        return ( ch->level < 4 ? 1 : 2 ) + UMIN( grp, 2 );
    if ( ch->level < 4 )
        return 0;
    if ( is_caster( ch ) )
        return ch->level < 12 ? 0 : 1;
    if ( is_rogue( ch ) )
        return ch->level < 8 ? 0 : 1;
    return ch->level < 8 ? 1 : 2;
}

static int inventory_signature( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int sig = 17;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        sig = sig * 31 + (int) ( (long) obj->pIndexData->vnum ) + obj->wear_loc * 7;
    return sig;
}

const char *bot_area_name( BOT_DATA *bot )
{
    if ( bot->ch == NULL || bot->ch->in_room == NULL )
        return "bilinmeyen bir yer";
    return bot->ch->in_room->area->name;
}

static bool spec_allowed( SPEC_FUN *fn )
{
    int i;

    if ( fn == NULL )
        return TRUE;
    for ( i = 0; i < spec_ok_count; i++ )
        if ( spec_ok[i] == fn )
            return TRUE;
    return FALSE;
}

static bool room_has_aggressor( CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
        if ( IS_NPC(rch) && IS_SET( rch->act, ACT_AGGRESSIVE ) && rch->level > ch->level - 3
          && !IS_AFFECTED( rch, AFF_CHARM ) )
            return TRUE;
    return FALSE;
}

/* ---------------------------------------------------------------------
 * yakın oda taraması (sınırlı BFS)
 * ------------------------------------------------------------------ */
#define NEAR_MAX 500
static ROOM_INDEX_DATA *near_room[NEAR_MAX];
static int              near_dist[NEAR_MAX];
static unsigned int    *near_stamp = NULL;
static unsigned int     near_cur = 0;

static int bot_near_rooms( CHAR_DATA *ch, int max_depth, bool same_area )
{
    int head = 0, tail = 0, d;
    ROOM_INDEX_DATA *start = ch->in_room;

    if ( near_stamp == NULL )
        near_stamp = (unsigned int *) calloc( 32768, sizeof(unsigned int) );
    if ( ++near_cur == 0 )
    {
        memset( near_stamp, 0, 32768 * sizeof(unsigned int) );
        near_cur = 1;
    }
    if ( start == NULL || start->vnum < 0 )
        return 0;

    near_room[tail] = start;
    near_dist[tail] = 0;
    tail++;
    near_stamp[start->vnum] = near_cur;

    while ( head < tail && tail < NEAR_MAX )
    {
        ROOM_INDEX_DATA *room = near_room[head];
        int dist = near_dist[head];

        head++;
        if ( dist >= max_depth )
            continue;
        for ( d = 0; d < 6 && tail < NEAR_MAX; d++ )
        {
            EXIT_DATA *pexit = room->exit[d];
            ROOM_INDEX_DATA *next;

            if ( pexit == NULL || ( next = pexit->u1.to_room ) == NULL )
                continue;
            if ( next->vnum < 0 || near_stamp[next->vnum] == near_cur )
                continue;
            if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
                continue;
            if ( same_area && next->area != start->area )
                continue;
            if ( !bot_room_passable( ch, next, FALSE ) )
                continue;
            if ( !bot_exit_back( next, room ) )
                continue;
            near_stamp[next->vnum] = near_cur;
            near_room[tail] = next;
            near_dist[tail] = dist + 1;
            tail++;
        }
    }
    return tail;
}

/* ---------------------------------------------------------------------
 * açılış
 * ------------------------------------------------------------------ */
static void add_buff( const char *name )
{
    int sn = skill_lookup( name );

    if ( sn >= 0 && buff_count < MAX_BUFF )
        buff_sn[buff_count++] = sn;
}

void bot_brain_boot( void )
{
    sn_bash            = skill_lookup( "bash" );
    sn_kick            = skill_lookup( "kick" );
    sn_trip            = skill_lookup( "trip" );
    sn_disarm          = skill_lookup( "disarm" );
    sn_dirt            = skill_lookup( "dirt kicking" );
    sn_circle          = skill_lookup( "circle" );
    sn_backstab        = skill_lookup( "backstab" );
    sn_berserk         = skill_lookup( "berserk" );
    sn_cure_light      = skill_lookup( "cure light" );
    sn_cure_serious    = skill_lookup( "cure serious" );
    sn_cure_critical   = skill_lookup( "cure critical" );
    sn_heal            = skill_lookup( "heal" );
    sn_second_attack   = skill_lookup( "second attack" );
    sn_third_attack    = skill_lookup( "third attack" );
    sn_dodge           = skill_lookup( "dodge" );
    sn_parry           = skill_lookup( "parry" );
    sn_shield_block    = skill_lookup( "shield block" );
    sn_enhanced_damage = skill_lookup( "enhanced damage" );
    sn_fast_healing    = skill_lookup( "fast healing" );
    sn_meditation      = skill_lookup( "meditation" );
    sn_hand_to_hand    = skill_lookup( "hand to hand" );

    buff_count = 0;
    add_buff( "armor" );
    add_buff( "bless" );
    add_buff( "shield" );
    add_buff( "stone skin" );
    add_buff( "giant strength" );
    add_buff( "detect invis" );
    add_buff( "detect hidden" );
    add_buff( "protection evil" );
    add_buff( "protection good" );
    add_buff( "sanctuary" );
    add_buff( "haste" );
    add_buff( "fly" );

    spec_ok_count = 0;
    if ( spec_lookup( "spec_fido" ) != NULL )    spec_ok[spec_ok_count++] = spec_lookup( "spec_fido" );
    if ( spec_lookup( "spec_janitor" ) != NULL ) spec_ok[spec_ok_count++] = spec_lookup( "spec_janitor" );
    spec_questmaster_fn = spec_lookup( "spec_questmaster" );
}

/* ---------------------------------------------------------------------
 * tanım (350 karakter altı tanımı olanlar TP kazanamıyor)
 * ------------------------------------------------------------------ */
static const char *desc_pool[] =
{
    "Yıllardır bu diyarlarda dolaşan, gözlerinde yorgun ama kararlı bir ışık taşıyan biri.",
    "Omuzlarında eski yolculukların tozunu, belinde sayısız dövüşün izini taşıyor.",
    "Kim olduğunu soranlara yalnızca gülümser; hikayesini ateş başında anlatmayı sever.",
    "Selenge'nin dar sokaklarında büyümüş, ekmeğini kılıcıyla kazanmayı erken öğrenmiş.",
    "Konuşurken elleriyle havada şekiller çizer, düşünürken alt dudağını ısırır.",
    "Üzerindeki eşyalar pek gösterişli değil ama her biri özenle bakılmış, yağlanmış.",
    "Gözleri sürekli çıkışları tarar; alışkanlık işte, uzun yollar insanı böyle yapıyor.",
    "Sesi alçak ama net; kalabalıkta bile duyulur, gerekirse bir haykırışa dönüşür.",
    "Saçları rüzgardan dağınık, yüzünde güneşin ve soğuğun bıraktığı ince çizgiler var.",
    "Yanından geçenlere başıyla selam verir; dostlarına karşı cömert, düşmanlarına karşı acımasızdır.",
    "Çantasında her zaman bir parça ekmek, bir matara su ve bir tutam tuz bulunur.",
    "Geceleri yıldızlara bakıp yön bulmayı bilir; kaybolduğunda bile telaşlanmaz.",
    "Bir zamanlar bir ustanın yanında çalışmış, oradan hem sabrı hem inadı öğrenmiş.",
    "Kolunda eski bir yaranın izi, boynunda küçük bir tılsım taşıyor.",
    "Ne kadar yorgun olursa olsun sırtını dik tutar; bu, ona öğretilen ilk şeydi.",
    "Söz verdiğinde tutar, tutamayacağı sözü baştan vermez.",
    "Diyarın haritalarını ezbere bilir, yine de her köşede yeni bir şey bulmayı umar.",
    "Kahkahası gürültülü, öfkesi kısa, sadakati uzundur.",
    "Ceplerinde hep birkaç akçe şıngırdar; pazarlıkta kolay kolay pes etmez.",
    "Yeni gelenlere yol gösterir, çünkü kendisi de bir zamanlar yolu soranlardandı."
};
#define DESC_POOL_N ( (int) ( sizeof(desc_pool) / sizeof(desc_pool[0]) ) )

static void bot_make_description( BOT_DATA *bot, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    unsigned int seed = 7;
    const char *p;
    int i, len;

    for ( p = bot->name; *p != '\0'; p++ )
        seed = seed * 131 + (unsigned char) *p;

    snprintf( buf, sizeof(buf), "%s, %s ırkından bir %s. ",
              bot->name, race_table[bot->race].name[1], class_table[bot->iclass].name[1] );
    len = (int) strlen( buf );
    for ( i = 0; i < DESC_POOL_N && len < 520; i++ )
    {
        const char *s = desc_pool[( seed + i * 7 ) % DESC_POOL_N];

        if ( strstr( buf, s ) != NULL )
            continue;
        strncat( buf, s, sizeof(buf) - strlen( buf ) - 3 );
        strcat( buf, " " );
        len = (int) strlen( buf );
    }
    strcat( buf, "\n\r" );
    free_string( ch->description );
    ch->description = str_dup( buf );
    REMOVE_BIT( ch->act, PLR_NO_DESCRIPTION );
}

void bot_brain_login( BOT_DATA *bot, bool fresh )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch->description == NULL || strlen( ch->description ) < 350 )
        bot_make_description( bot, ch );
    if ( ch->wimpy < ch->max_hit / 8 )
        ch->wimpy = ch->max_hit / 8;
    ch->pcdata->oyuncu_katli = ch->pcdata->oyuncu_katli;   /* dosyadan geleni koru */

    if ( fresh )
        bot->last_town_pulse = bot_pulse;
    if ( ch->level >= 10 && bot->lakap != NULL && ch->pcdata->title != NULL
      && strstr( ch->pcdata->title, title_table[ch->iclass][ch->level] ) != NULL
      && number_percent() < 60 )
    {
        char buf[MAX_INPUT_LENGTH];
        snprintf( buf, sizeof(buf), " %s", bot->lakap );
        set_title( ch, buf );
    }
}

/* ---------------------------------------------------------------------
 * eşya değerlendirme
 * ------------------------------------------------------------------ */
static int weapon_gsn_for_type( int type )
{
    switch ( type )
    {
    case WEAPON_SWORD:   return gsn_sword;
    case WEAPON_DAGGER:  return gsn_dagger;
    case WEAPON_SPEAR:   return gsn_spear;
    case WEAPON_MACE:    return gsn_mace;
    case WEAPON_AXE:     return gsn_axe;
    case WEAPON_FLAIL:   return gsn_flail;
    case WEAPON_WHIP:    return gsn_whip;
    case WEAPON_POLEARM: return gsn_polearm;
    }
    return -1;
}

static bool bot_can_use_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    int wear_level = ch->level;

    if ( ( class_table[ch->iclass].fMana && obj->item_type == ITEM_ARMOR )
      || ( !class_table[ch->iclass].fMana && obj->item_type == ITEM_WEAPON ) )
        wear_level += 3;
    if ( obj->level > wear_level )
        return FALSE;
    if ( ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && IS_EVIL(ch) )
      || ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) && IS_GOOD(ch) )
      || ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL ) && IS_NEUTRAL(ch) ) )
        return FALSE;
    if ( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
        return FALSE;
    return TRUE;
}

static int affect_bonus( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf;
    int score = 0;

    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
        switch ( paf->location )
        {
        case APPLY_HITROLL: score += paf->modifier * 6; break;
        case APPLY_DAMROLL: score += paf->modifier * 8; break;
        case APPLY_HIT:     score += paf->modifier;     break;
        case APPLY_MANA:    score += is_caster( ch ) ? paf->modifier : paf->modifier / 4; break;
        case APPLY_AC:      score -= paf->modifier * 2; break;
        case APPLY_STR: case APPLY_DEX: case APPLY_CON: case APPLY_INT: case APPLY_WIS:
            score += paf->modifier * 5; break;
        case APPLY_SAVES:   score -= paf->modifier * 3; break;
        }
    }
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        switch ( paf->location )
        {
        case APPLY_HITROLL: score += paf->modifier * 6; break;
        case APPLY_DAMROLL: score += paf->modifier * 8; break;
        case APPLY_HIT:     score += paf->modifier;     break;
        case APPLY_AC:      score -= paf->modifier * 2; break;
        }
    }
    return score;
}

/* Botun bakış açısından eşyanın değeri. Kullanılamıyorsa 0. */
static int bot_obj_score( CHAR_DATA *ch, OBJ_DATA *obj )
{
    int score = 0;

    if ( !bot_can_use_obj( ch, obj ) )
        return 0;

    if ( obj->item_type == ITEM_WEAPON )
    {
        int avg = obj->value[1] * ( obj->value[2] + 1 ) / 2;
        int gsn = weapon_gsn_for_type( obj->value[0] );
        int skill = gsn >= 0 ? get_skill( ch, gsn ) : 30;

        if ( !CAN_WEAR( obj, ITEM_WIELD ) )
            return 0;
        if ( is_caster( ch ) && get_obj_weight( obj ) > 120 )
            return 0;
        score = avg * 10 + affect_bonus( ch, obj ) + obj->level;
        if ( obj->value[0] == WEAPON_EXOTIC || obj->value[0] == WEAPON_BOW )
            score /= 2;
        score = score * ( skill + 40 ) / 140;
        if ( is_rogue( ch ) && obj->value[0] == WEAPON_DAGGER )
            score = score * 13 / 10;
        return UMAX( 1, score );
    }
    if ( obj->item_type == ITEM_ARMOR )
    {
        score = ( obj->value[0] + obj->value[1] + obj->value[2] ) * 3 + obj->value[3] * 2
              + affect_bonus( ch, obj ) + obj->level;
        return UMAX( 1, score );
    }
    if ( obj->item_type == ITEM_LIGHT )
        return obj->value[2] == 0 ? 0 : 5 + affect_bonus( ch, obj );
    if ( obj->item_type == ITEM_JEWELRY || obj->item_type == ITEM_CLOTHING || obj->item_type == ITEM_TREASURE )
        return UMAX( 1, 1 + affect_bonus( ch, obj ) );
    return 0;
}

/* eşyanın giyileceği yuva (çoklu yuvalar için ilk boş ya da en zayıf) */
static int bot_wear_slot( OBJ_DATA *obj )
{
    if ( obj->item_type == ITEM_LIGHT )              return WEAR_LEFT;
    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )         return WEAR_FINGER;
    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )           return WEAR_NECK;
    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )           return WEAR_BODY;
    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )           return WEAR_HEAD;
    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )           return WEAR_LEGS;
    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )           return WEAR_FEET;
    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )          return WEAR_HANDS;
    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )           return WEAR_ARMS;
    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )          return WEAR_ABOUT;
    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )          return WEAR_WAIST;
    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )          return WEAR_WRIST;
    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )         return WEAR_LEFT;
    if ( CAN_WEAR( obj, ITEM_WIELD ) )               return WEAR_RIGHT;
    if ( CAN_WEAR( obj, ITEM_HOLD ) )                return WEAR_LEFT;
    if ( CAN_WEAR( obj, ITEM_WEAR_FLOAT ) )          return WEAR_FLOAT;
    return WEAR_NONE;
}

/* yuvadaki en zayıf eşyanın puanı (boşsa 0) */
static int bot_slot_score( CHAR_DATA *ch, OBJ_DATA *obj, int slot )
{
    OBJ_DATA *worn;
    int best = -1, count = 0;

    if ( slot == WEAR_NONE )
        return 0;
    if ( obj->item_type == ITEM_WEAPON )
    {
        worn = get_wield_char( ch, FALSE );
        return worn != NULL ? bot_obj_score( ch, worn ) : 0;
    }
    if ( obj->item_type == ITEM_LIGHT )
    {
        if ( get_light_char( ch ) != NULL )
            return 1000;
        /* sol el dolu (kalkan, tutulan eşya, iki elli silah) ise ışık tutulamaz */
        if ( get_eq_char( ch, WEAR_LEFT ) != NULL || get_eq_char( ch, WEAR_BOTH ) != NULL )
            return 1000;
        return 0;
    }
    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
        worn = get_shield_char( ch );
        if ( worn != NULL )
            return bot_obj_score( ch, worn );
        /* iki elli silah kullanıyorsa kalkan tutamaz */
        if ( get_eq_char( ch, WEAR_BOTH ) != NULL )
            return 1000;
        return 0;
    }
    for ( worn = ch->carrying; worn != NULL; worn = worn->next_content )
    {
        if ( worn->wear_loc != slot )
            continue;
        count++;
        {
            int s = bot_obj_score( ch, worn );
            if ( best < 0 || s < best )
                best = s;
        }
    }
    if ( count < max_can_wear( ch, slot ) )
        return 0;
    return best < 0 ? 0 : best;
}

static bool bot_is_junk( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( obj->wear_loc != WEAR_NONE )
        return FALSE;
    if ( IS_OBJ_STAT( obj, ITEM_NODROP ) || gorev_ekipmani_mi( obj ) )
        return FALSE;
    switch ( obj->item_type )
    {
    case ITEM_POTION: case ITEM_FOOD: case ITEM_DRINK_CON: case ITEM_MAP:
    case ITEM_KEY: case ITEM_MONEY: case ITEM_BOAT: case ITEM_PILL:
        return FALSE;
    case ITEM_LIGHT:
        return get_light_char( ch ) != NULL && obj->value[2] != -1;
    case ITEM_CONTAINER:
        return obj->contains == NULL && obj->pIndexData->vnum != OBJ_VNUM_MAP;
    case ITEM_WEAPON: case ITEM_ARMOR: case ITEM_JEWELRY: case ITEM_CLOTHING:
    case ITEM_TREASURE: case ITEM_TRASH: case ITEM_GEM: case ITEM_FURNITURE:
    case ITEM_SCROLL: case ITEM_WAND: case ITEM_STAFF:
        break;
    default:
        return obj->cost > 0;
    }
    if ( obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_ARMOR )
    {
        int score = bot_obj_score( ch, obj );
        int slot  = bot_wear_slot( obj );

        if ( score > 0 && slot != WEAR_NONE && score > bot_slot_score( ch, obj, slot ) )
            return FALSE;                    /* giyilecek */
        if ( obj->level > ch->level && obj->level <= ch->level + 4 && score > 0 )
            return FALSE;                    /* yakında giyilir */
    }
    return TRUE;
}

int bot_sell_candidates( BOT_DATA *bot )
{
    OBJ_DATA *obj;
    int n = 0;

    for ( obj = bot->ch->carrying; obj != NULL; obj = obj->next_content )
        if ( bot_is_junk( bot->ch, obj ) )
            n++;
    return n;
}

int bot_count_potions( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int n = 0;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        if ( obj->item_type == ITEM_POTION && obj->level <= ch->level )
            n++;
    return n;
}

static OBJ_DATA *bot_find_cure_potion( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        int i;

        if ( obj->item_type != ITEM_POTION || obj->level > ch->level )
            continue;
        for ( i = 1; i <= 3; i++ )
            if ( obj->value[i] == sn_cure_light || obj->value[i] == sn_cure_serious
              || obj->value[i] == sn_cure_critical || obj->value[i] == sn_heal )
                return obj;
    }
    return NULL;
}

static bool bot_wear_failed( BOT_DATA *bot, int vnum )
{
    int i;

    for ( i = 0; i < BOT_WEAR_FAIL_MAX; i++ )
        if ( bot->wear_fail[i] == vnum )
            return TRUE;
    return FALSE;
}

static void bot_wear_fail_add( BOT_DATA *bot, int vnum )
{
    if ( bot_wear_failed( bot, vnum ) )
        return;
    bot->wear_fail[bot->wear_fail_pos] = vnum;
    bot->wear_fail_pos = ( bot->wear_fail_pos + 1 ) % BOT_WEAR_FAIL_MAX;
}

/* envanterde daha iyi bir eşya varsa giy; bir eşya giyildiyse TRUE */
static bool bot_wear_upgrades( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    OBJ_DATA *obj, *best = NULL;
    int best_gain = 0;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        int slot, score, gain;

        if ( obj->wear_loc != WEAR_NONE || !can_see_obj( ch, obj ) )
            continue;
        if ( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR
          && obj->item_type != ITEM_LIGHT && obj->item_type != ITEM_JEWELRY
          && obj->item_type != ITEM_CLOTHING )
            continue;
        if ( ( slot = bot_wear_slot( obj ) ) == WEAR_NONE )
            continue;
        /* yalnızca elde tutulabilen eşyalar (ışık hariç) botun işine yaramaz */
        if ( obj->item_type != ITEM_LIGHT && slot == WEAR_LEFT
          && !CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
            continue;
        if ( bot_wear_failed( bot, obj->pIndexData->vnum ) )
            continue;
        if ( ( score = bot_obj_score( ch, obj ) ) <= 0 )
            continue;
        gain = score - bot_slot_score( ch, obj, slot );
        if ( gain > best_gain )
        {
            best_gain = gain;
            best = obj;
        }
    }
    if ( best == NULL )
        return FALSE;
    {
        char kw[MAX_INPUT_LENGTH];
        int vnum = best->pIndexData->vnum;

        bot_obj_keyword( ch, best, ch->carrying, kw, sizeof(kw) );
        snprintf( bot->last_item, sizeof(bot->last_item), "%s", best->short_descr );
        bot_cmd( bot, "giy %s", kw );
        if ( bot->ch == NULL )
            return TRUE;
        if ( best->wear_loc != WEAR_NONE )
            bot_chat_event( bot, BOT_EV_LOOT, NULL );
        else
            bot_wear_fail_add( bot, vnum );
    }
    return TRUE;
}

void bot_note_loot( BOT_DATA *bot )
{
    bot->inv_signature = -1;
}

/* ---------------------------------------------------------------------
 * hedef arama
 * ------------------------------------------------------------------ */
static bool bot_prey_ok( CHAR_DATA *ch, CHAR_DATA *mob, int lo, int hi )
{
    if ( mob == NULL || mob == ch || !IS_NPC(mob) || mob->in_room == NULL )
        return FALSE;
    if ( mob->level > hi || mob->level < lo )
        return FALSE;
    if ( !can_see( ch, mob ) )
        return FALSE;
    if ( mob->fighting != NULL || mob->position <= POS_STUNNED )
        return FALSE;
    if ( mob->pIndexData->pShop != NULL || mob->pIndexData->vnum < 100 )
        return FALSE;
    if ( mob->pIndexData->vnum >= 500 && mob->pIndexData->vnum <= 580 )
        return FALSE;
    if ( IS_SET( mob->act, ACT_TRAIN | ACT_PRACTICE | ACT_GAIN | ACT_IS_HEALER | ACT_PET | ACT_MAFYA ) )
        return FALSE;
    if ( IS_AFFECTED( mob, AFF_CHARM ) || mob->master != NULL || mob->leader != NULL )
        return FALSE;
    if ( !spec_allowed( mob->spec_fun ) )
        return FALSE;
    if ( IS_SET( mob->in_room->room_flags, ROOM_SAFE ) )
        return FALSE;
    if ( IS_SET( mob->in_room->area->area_flag, AREA_HOMETOWN | AREA_CABAL ) )
        return FALSE;
    if ( ch->level < 5 ? mob->max_hit > ch->max_hit * 2 + 15 : mob->max_hit > ch->max_hit * 3 + 40 )
        return FALSE;
    if ( mob->cabal != CABAL_NONE )
        return FALSE;
    {
        BOT_DATA *bot = bot_of( ch );
        int i;
        if ( bot != NULL )
            for ( i = 0; i < BOT_AVOID_MAX; i++ )
                if ( bot->avoid_vnum[i] == mob->pIndexData->vnum && bot_pulse < bot->avoid_until[i] )
                    return FALSE;
    }
    if ( !IS_SET( mob->act, ACT_NOALIGN ) )
    {
        if ( IS_GOOD(ch) && IS_GOOD(mob) )
            return FALSE;
        if ( IS_EVIL(ch) && IS_EVIL(mob) )
            return FALSE;
    }
    return TRUE;
}

static int bot_prey_score( CHAR_DATA *ch, CHAR_DATA *mob, int dist )
{
    int score = 100 + ( mob->level - ch->level ) * 15 - dist * 14;

    if ( ( IS_GOOD(ch) && IS_EVIL(mob) ) || ( IS_EVIL(ch) && IS_GOOD(mob) ) )
        score += 35;
    if ( IS_NEUTRAL(ch) && IS_NEUTRAL(mob) )
        score -= 25;
    if ( mob->hit > ch->max_hit * 2 )
        score -= 30;
    if ( mob->position == POS_SLEEPING )
        score += 10;
    if ( IS_SET( mob->act, ACT_AGGRESSIVE ) )
        score += 5;
    return score + number_range( 0, 15 );
}

/* bulunduğu odada uygun av var mı? */
static CHAR_DATA *bot_prey_here( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch, *rch, *best = NULL;
    int lo = UMAX( 1, ch->level - 2 - bot->hunt_fail );
    int hi = UMAX( 2, ch->level + level_bonus( ch ) );
    int best_score = -1000;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        int s;

        if ( !bot_prey_ok( ch, rch, lo, hi ) )
            continue;
        s = bot_prey_score( ch, rch, 0 );
        if ( s > best_score )
        {
            best_score = s;
            best = rch;
        }
    }
    return best;
}

/* yakın odalarda av: hedef odayı döndürür */
static ROOM_INDEX_DATA *bot_prey_near( BOT_DATA *bot, int depth )
{
    CHAR_DATA *ch = bot->ch;
    int n = bot_near_rooms( ch, depth, TRUE );
    int i, best_score = -1000;
    ROOM_INDEX_DATA *best = NULL;
    int lo = UMAX( 1, ch->level - 2 - bot->hunt_fail );
    int hi = UMAX( 2, ch->level + level_bonus( ch ) );

    for ( i = 1; i < n; i++ )
    {
        CHAR_DATA *rch;

        for ( rch = near_room[i]->people; rch != NULL; rch = rch->next_in_room )
        {
            int s;

            if ( !bot_prey_ok( ch, rch, lo, hi ) )
                continue;
            s = bot_prey_score( ch, rch, near_dist[i] );
            if ( s > best_score )
            {
                best_score = s;
                best = near_room[i];
            }
        }
    }
    return best;
}

/* ---------------------------------------------------------------------
 * av bölgesi seçimi
 * ------------------------------------------------------------------ */
static int area_suitable_mobs( CHAR_DATA *ch, AREA_DATA *area, int lo, int hi )
{
    CHAR_DATA *mob;
    int n = 0;

    for ( mob = char_list; mob != NULL; mob = mob->next )
    {
        if ( !IS_NPC(mob) || mob->in_room == NULL || mob->in_room->area != area )
            continue;
        if ( bot_prey_ok( ch, mob, lo, hi ) )
            n++;
    }
    return n;
}

/* botun seviyesinden belirgin güçlü saldırgan yaratık sayısı */
static int area_danger( CHAR_DATA *ch, AREA_DATA *area )
{
    CHAR_DATA *mob;
    int n = 0;

    for ( mob = char_list; mob != NULL; mob = mob->next )
    {
        if ( !IS_NPC(mob) || mob->in_room == NULL || mob->in_room->area != area )
            continue;
        if ( IS_SET( mob->act, ACT_AGGRESSIVE ) && mob->level > ch->level + 2 )
            n++;
    }
    return n;
}

static ROOM_INDEX_DATA *area_entry_room( CHAR_DATA *ch, AREA_DATA *area )
{
    CHAR_DATA *mob;
    ROOM_INDEX_DATA *pick = NULL;
    int count = 0;

    /* alanda uygun bir mobun bulunduğu rastgele bir oda */
    for ( mob = char_list; mob != NULL; mob = mob->next )
    {
        if ( !IS_NPC(mob) || mob->in_room == NULL || mob->in_room->area != area )
            continue;
        if ( !bot_room_passable( ch, mob->in_room, FALSE ) )
            continue;
        if ( number_range( 0, count++ ) == 0 )
            pick = mob->in_room;
    }
    return pick;
}

static AREA_DATA *bot_pick_hunt_area( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    AREA_DATA *area, *best = NULL;
    int best_score = -1;
    int lo = UMAX( 1, ch->level - 2 );
    int hi = UMAX( 2, ch->level + level_bonus( ch ) );
    int tries = 0;

    for ( area = area_first; area != NULL; area = area->next )
    {
        int score, mobs;

        if ( IS_SET( area->area_flag, AREA_CABAL | AREA_HOMETOWN ) )
            continue;
        if ( area->low_range > ch->level || area->high_range < ch->level )
            continue;
        if ( area->high_range - area->low_range > 60 )
            continue;
        if ( area->min_vnum < 100 )
            continue;
        if ( area == bot->hunt_area && bot->hunt_fail > 2 )
            continue;
        mobs = area_suitable_mobs( ch, area, lo, hi );
        if ( mobs < 3 )
            continue;
        score = UMIN( mobs, 40 ) * 3;
        score -= area_danger( ch, area ) * ( ch->level < 10 ? 12 : 5 );
        {
            BOT_DATA *ob;
            for ( ob = bot_list; ob != NULL; ob = ob->next )
                if ( ob != bot && ob->ch != NULL && ob->hunt_area == area && !is_same_group( ch, ob->ch ) )
                    score -= 18;
        }
        if ( ch->level < 10 && area->high_range - area->low_range > 20 )
            score -= 25;
        /* seviye aralığının alt-orta kısmını tercih et */
        if ( ch->level <= area->low_range + ( area->high_range - area->low_range ) / 2 )
            score += 30;
        if ( area == bot->hunt_area )
            score += 20;
        if ( ch->in_room != NULL && ch->in_room->area == area )
            score += 25;
        score += number_range( 0, 40 );
        if ( ++tries > 60 )
            break;
        if ( score > best_score )
        {
            best_score = score;
            best = area;
        }
    }
    return best;
}

/* tanı: av bölgesi adayları */
void bot_debug_areas( CHAR_DATA *viewer, BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    AREA_DATA *area;
    int lo, hi, shown = 0;

    if ( ch == NULL )
    {
        send_to_char( "Bot çevrimdışı.\n\r", viewer );
        return;
    }
    lo = UMAX( 1, ch->level - 2 );
    hi = UMAX( 2, ch->level + level_bonus( ch ) );
    printf_to_char( viewer, "Seviye %d, av bandı %d-%d, oda %d (%s), av bölgesi %s, başarısızlık %d\n\r",
                    ch->level, lo, hi, ch->in_room->vnum, ch->in_room->area->name,
                    bot->hunt_area != NULL ? bot->hunt_area->name : "-", bot->hunt_fail );
    for ( area = area_first; area != NULL; area = area->next )
    {
        int mobs;
        ROOM_INDEX_DATA *entry;
        sh_int tmp[BOT_MAX_PATH];
        int len = -2;

        if ( area->low_range > ch->level || area->high_range < ch->level )
            continue;
        mobs = area_suitable_mobs( ch, area, lo, hi );
        entry = area_entry_room( ch, area );
        if ( entry != NULL )
            len = bot_find_path( ch, ch->in_room, entry, tmp, BOT_MAX_PATH, FALSE );
        printf_to_char( viewer, "%-28.28s %3d-%-3d vnum %5d bayrak %2ld uygun mob %3d giriş %5d yol %d\n\r",
                        area->name, area->low_range, area->high_range, area->min_vnum, area->area_flag,
                        mobs, entry != NULL ? entry->vnum : 0, len );
        if ( ++shown > 40 )
            break;
    }
}

/* ---------------------------------------------------------------------
 * hareket
 * ------------------------------------------------------------------ */
static void bot_travel_step( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    ROOM_INDEX_DATA *from = ch->in_room;
    EXIT_DATA *pexit;
    int dir;

    if ( bot->path_pos >= bot->path_len )
    {
        int after = bot->after_travel;

        bot->path_len = bot->path_pos = 0;
        bot_set_state( bot, after );
        return;
    }
    if ( bot_pulse - bot->travel_started > 4 * 60 * 12 )     /* 12 dakika: vazgeç */
    {
        bot->path_len = bot->path_pos = 0;
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }

    if ( ch->move < 12 )
    {
        if ( ch->position != POS_RESTING )
            bot_cmd( bot, "dinlen" );
        return;
    }
    if ( ch->position >= POS_SLEEPING && ch->position < POS_STANDING )
    {
        if ( ch->move < ch->max_move / 2 && ch->position != POS_SLEEPING )
            return;
        if ( !IS_AFFECTED( ch, AFF_SLEEP ) )
            bot_cmd( bot, "kalk" );
        return;
    }
    if ( ch->position != POS_STANDING )
        return;

    dir = bot->path[bot->path_pos];
    pexit = from->exit[dir];
    if ( pexit == NULL || pexit->u1.to_room == NULL )
    {
        ROOM_INDEX_DATA *to = get_room_index( bot->target_vnum );
        int len;

        if ( to == NULL || ++bot->stuck > 4
          || ( len = bot_find_path( ch, from, to, bot->path, BOT_MAX_PATH, bot->state == BOT_ST_PK ) ) < 0 )
        {
            bot->path_len = bot->path_pos = 0;
            bot_set_state( bot, BOT_ST_IDLE );
            return;
        }
        bot->path_len = len;
        bot->path_pos = 0;
        return;
    }
    if ( IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
    {
        if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
        {
            bot->stuck += 3;
            bot->path_len = bot->path_pos = 0;
            bot_set_state( bot, BOT_ST_IDLE );
            return;
        }
        bot_cmd( bot, "aç %s", dir_name[dir] );
        return;
    }

    bot_cmd( bot, "%s", dir_name[dir] );
    if ( bot->ch == NULL )
        return;
    if ( ch->in_room == pexit->u1.to_room )
    {
        bot->path_pos++;
        bot->stuck = 0;
        bot->last_room_vnum = ch->in_room->vnum;
    }
    else if ( ch->in_room != from )
    {
        /* başka bir yere düştük (rastgele çıkış vb.): yeniden hesapla */
        ROOM_INDEX_DATA *to = get_room_index( bot->target_vnum );
        int len = to != NULL ? bot_find_path( ch, ch->in_room, to, bot->path, BOT_MAX_PATH, FALSE ) : -1;

        if ( len < 0 )
        {
            bot->path_len = bot->path_pos = 0;
            bot_set_state( bot, BOT_ST_IDLE );
        }
        else
        {
            bot->path_len = len;
            bot->path_pos = 0;
        }
    }
    else if ( ++bot->stuck > 3 )
    {
        bot->path_len = bot->path_pos = 0;
        bot_set_state( bot, BOT_ST_IDLE );
    }
}

/* ---------------------------------------------------------------------
 * dinlenme
 * ------------------------------------------------------------------ */
static bool bot_need_rest( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;

    if ( pct( ch->hit, ch->max_hit ) < 45 )
        return TRUE;
    if ( is_caster( ch ) && pct( ch->mana, ch->max_mana ) < 25 )
        return TRUE;
    if ( pct( ch->move, ch->max_move ) < 15 )
        return TRUE;
    return FALSE;
}

static bool bot_rested_enough( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;

    if ( pct( ch->hit, ch->max_hit ) < 88 )
        return FALSE;
    if ( is_caster( ch ) && pct( ch->mana, ch->max_mana ) < 70 )
        return FALSE;
    if ( pct( ch->move, ch->max_move ) < 60 )
        return FALSE;
    return TRUE;
}

static void bot_start_rest( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;

    bot_set_state( bot, BOT_ST_REST );
    bot->rest_reason = 0;
    if ( ch->position == POS_SLEEPING )
        return;
    /* önce şifa bilen kendini iyileştirsin */
    if ( is_healer_class( ch ) && pct( ch->hit, ch->max_hit ) < 70 )
    {
        int sn = can_cast_sn( ch, sn_cure_critical ) ? sn_cure_critical
               : can_cast_sn( ch, sn_cure_serious ) ? sn_cure_serious
               : can_cast_sn( ch, sn_cure_light ) ? sn_cure_light : -1;
        if ( sn >= 0 )
        {
            bot_cmd( bot, "büyü '%s'", skill_table[sn].name[1] );
            return;
        }
    }
    if ( room_has_aggressor( ch ) && bot->stuck < 3 )
    {
        /* saldırgan yaratıkların yanında uyunmaz: bir adım uzaklaş */
        int d;
        for ( d = 0; d < 6; d++ )
        {
            EXIT_DATA *pexit = ch->in_room->exit[d];
            if ( pexit != NULL && pexit->u1.to_room != NULL && !IS_SET( pexit->exit_info, EX_CLOSED )
              && bot_room_passable( ch, pexit->u1.to_room, FALSE ) )
            {
                bot->stuck++;
                bot_cmd( bot, "%s", dir_name[d] );
                return;
            }
        }
    }
    bot->stuck = 0;
    bot_cmd( bot, "uyu" );
}

static void bot_resting( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;

    if ( bot_rested_enough( bot ) || bot_pulse - bot->state_pulse > 4 * 60 * 14 )
    {
        if ( ch->position < POS_STANDING )
        {
            bot_cmd( bot, "kalk" );
            return;
        }
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }
    if ( is_healer_class( ch ) && pct( ch->hit, ch->max_hit ) < 75 && ch->position >= POS_RESTING )
    {
        int sn = can_cast_sn( ch, sn_cure_critical ) ? sn_cure_critical
               : can_cast_sn( ch, sn_cure_serious ) ? sn_cure_serious
               : can_cast_sn( ch, sn_cure_light ) ? sn_cure_light : -1;
        if ( sn >= 0 && ch->mana > ch->max_mana / 3 )
        {
            if ( ch->position == POS_SLEEPING )
                bot_cmd( bot, "kalk" );
            else
                bot_cmd( bot, "büyü '%s'", skill_table[sn].name[1] );
            return;
        }
    }
    if ( ch->position > POS_SLEEPING && !IS_AFFECTED( ch, AFF_SLEEP ) )
    {
        if ( pct( ch->hit, ch->max_hit ) < 80 || pct( ch->move, ch->max_move ) < 50 )
            bot_cmd( bot, "uyu" );
        else if ( ch->position != POS_RESTING )
            bot_cmd( bot, "dinlen" );
    }
}

/* ---------------------------------------------------------------------
 * yeme / içme / ışık
 * ------------------------------------------------------------------ */
static OBJ_DATA *carried_type( CHAR_DATA *ch, int type )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        if ( obj->item_type == type && obj->wear_loc == WEAR_NONE )
            return obj;
    return NULL;
}

static OBJ_DATA *room_fountain( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
        if ( obj->item_type == ITEM_FOUNTAIN )
            return obj;
    return NULL;
}

static OBJ_DATA *carried_drink( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        if ( obj->item_type == ITEM_DRINK_CON && obj->value[1] > 0 )
            return obj;
    return NULL;
}

/* bir komut ürettiyse TRUE */
static bool bot_eat_drink( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    OBJ_DATA *obj;
    char kw[MAX_INPUT_LENGTH];

    if ( ch->position < POS_RESTING )
        return FALSE;
    if ( ch->pcdata->condition[COND_HUNGER] < 14 )
    {
        if ( ( obj = carried_type( ch, ITEM_FOOD ) ) != NULL )
        {
            bot_obj_keyword( ch, obj, ch->carrying, kw, sizeof(kw) );
            bot_cmd( bot, "ye %s", kw );
            return TRUE;
        }
        SET_BIT( bot->town_tasks, BOT_TOWN_FOOD );
    }
    if ( ch->pcdata->condition[COND_THIRST] < 14 )
    {
        if ( ( obj = carried_drink( ch ) ) != NULL )
        {
            bot_obj_keyword( ch, obj, ch->carrying, kw, sizeof(kw) );
            bot_cmd( bot, "iç %s", kw );
            return TRUE;
        }
        if ( room_fountain( ch ) != NULL )
        {
            bot_cmd( bot, "iç" );
            return TRUE;
        }
        SET_BIT( bot->town_tasks, BOT_TOWN_FOUNTAIN );
    }
    /* çeşme başındaysa matarayı doldur */
    if ( room_fountain( ch ) != NULL )
    {
        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
            if ( obj->item_type == ITEM_DRINK_CON && obj->value[1] < obj->value[0] / 2 )
            {
                bot_obj_keyword( ch, obj, ch->carrying, kw, sizeof(kw) );
                bot_cmd( bot, "doldur %s", kw );
                return TRUE;
            }
    }
    return FALSE;
}

/* ---------------------------------------------------------------------
 * dövüş
 * ------------------------------------------------------------------ */
static int bot_best_attack_spell( CHAR_DATA *ch )
{
    int sn, best = -1, best_level = -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
        if ( skill_table[sn].name[0] == NULL )
            break;
        if ( skill_table[sn].target != TAR_CHAR_OFFENSIVE )
            continue;
        if ( skill_table[sn].noun_damage == NULL || skill_table[sn].noun_damage[0] == '\0' )
            continue;
        if ( skill_table[sn].minimum_position > POS_FIGHTING )
            continue;
        if ( !can_cast_sn( ch, sn ) )
            continue;
        if ( skill_table[sn].skill_level[ch->iclass] > best_level )
        {
            best_level = skill_table[sn].skill_level[ch->iclass];
            best = sn;
        }
    }
    return best;
}

static void bot_escape( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    OBJ_DATA *potion;
    char kw[MAX_INPUT_LENGTH];

    if ( ( potion = bot_find_cure_potion( ch ) ) != NULL && ch->cabal != CABAL_BATTLE )
    {
        bot_obj_keyword( ch, potion, ch->carrying, kw, sizeof(kw) );
        bot_cmd( bot, "yudumla %s", kw );
        return;
    }
    bot->fled_pulse = bot_pulse;
    if ( ch->level < KIDEMLI_OYUNCU_SEVIYESI && number_percent() < 60 )
        bot_cmd( bot, "anımsa" );
    else
        bot_cmd( bot, "kaç" );
    if ( bot->ch != NULL && bot->ch->fighting == NULL )
    {
        bot_chat_event( bot, BOT_EV_FLEE, NULL );
        bot_set_state( bot, BOT_ST_REST );
    }
}

static void bot_combat( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *victim = ch->fighting;
    int hp = pct( ch->hit, ch->max_hit );
    int vhp = pct( victim->hit, victim->max_hit );
    int roll;

    bot->last_fight_pulse = bot_pulse;

    if ( ch->position < POS_FIGHTING )
    {
        if ( ch->position >= POS_RESTING )
            bot_cmd( bot, "kalk" );
        return;
    }

    if ( hp < 22 && vhp > 25 )
    {
        bot_escape( bot );
        return;
    }
    if ( hp < 45 && vhp > 60 && is_healer_class( ch ) )
    {
        int sn = can_cast_sn( ch, sn_cure_critical ) ? sn_cure_critical
               : can_cast_sn( ch, sn_cure_serious ) ? sn_cure_serious
               : can_cast_sn( ch, sn_cure_light ) ? sn_cure_light : -1;
        if ( sn >= 0 )
        {
            bot_cmd( bot, "büyü '%s'", skill_table[sn].name[1] );
            return;
        }
    }

    if ( is_caster( ch ) )
    {
        int sn = bot_best_attack_spell( ch );

        if ( sn >= 0 && number_percent() < 88 )
        {
            bot_cmd( bot, "büyü '%s'", skill_table[sn].name[1] );
            return;
        }
    }

    roll = number_percent();
    if ( roll > 70 )
        return;                                   /* sıradan vuruşlar devam eder */

    if ( is_rogue( ch ) )
    {
        if ( knows( ch, sn_circle, 20 ) && number_percent() < 60 )
        {
            bot_cmd( bot, "çevrel" );
            return;
        }
        if ( knows( ch, sn_dirt, 20 ) && !IS_AFFECTED( victim, AFF_BLIND ) && number_percent() < 50 )
        {
            bot_cmd( bot, "toz" );
            return;
        }
        if ( knows( ch, sn_trip, 20 ) && number_percent() < 50 )
        {
            bot_cmd( bot, "çelme" );
            return;
        }
        if ( knows( ch, sn_kick, 20 ) )
            bot_cmd( bot, "tekme" );
        return;
    }

    if ( knows( ch, sn_berserk, 20 ) && !IS_AFFECTED( ch, AFF_BERSERK ) && hp > 50 && number_percent() < 35 )
    {
        bot_cmd( bot, "vahşileş" );
        return;
    }
    if ( knows( ch, sn_bash, 20 ) && victim->daze <= 0 && number_percent() < 55 )
    {
        bot_cmd( bot, "omuz" );
        return;
    }
    if ( knows( ch, sn_disarm, 25 ) && get_wield_char( victim, FALSE ) != NULL && number_percent() < 40 )
    {
        bot_cmd( bot, "silahsızlandır" );
        return;
    }
    if ( knows( ch, sn_trip, 20 ) && number_percent() < 45 )
    {
        bot_cmd( bot, "çelme" );
        return;
    }
    if ( knows( ch, sn_kick, 20 ) )
        bot_cmd( bot, "tekme" );
}

/* dövüş öncesi güçlendirmeler; komut ürettiyse TRUE */
static bool bot_cast_buffs( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    int i;

    if ( ch->cabal == CABAL_BATTLE || pct( ch->mana, ch->max_mana ) < 45 )
        return FALSE;
    for ( i = 0; i < buff_count; i++ )
    {
        int sn = buff_sn[i];

        if ( !can_cast_sn( ch, sn ) || is_affected( ch, sn ) )
            continue;
        if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE )
            continue;
        bot_cmd( bot, "büyü '%s'", skill_table[sn].name[1] );
        return TRUE;
    }
    return FALSE;
}

static void bot_attack( BOT_DATA *bot, CHAR_DATA *victim )
{
    CHAR_DATA *ch = bot->ch;
    char kw[MAX_INPUT_LENGTH];
    OBJ_DATA *wield = get_wield_char( ch, FALSE );

    bot->target_id = victim->id;
    bot_char_keyword( ch, victim, kw, sizeof(kw) );
    if ( is_rogue( ch ) && knows( ch, sn_backstab, 20 ) && wield != NULL
      && wield->value[0] == WEAPON_DAGGER && victim->fighting == NULL
      && victim->hit >= victim->max_hit )
        bot_cmd( bot, "ardılan %s", kw );
    else if ( !IS_NPC(victim) )
        bot_cmd( bot, "katlet %s", kw );
    else
        bot_cmd( bot, "öldür %s", kw );
}

/* Tapınağa yol yoksa (tek yönlü cep) oradan kurtul; komut ürettiyse TRUE */
static bool bot_escape_pocket( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    ROOM_INDEX_DATA *temple = get_room_index( ROOM_VNUM_TEMPLE );
    sh_int tmp[BOT_MAX_PATH];
    OBJ_DATA *obj;

    if ( temple == NULL || ch->in_room == temple )
        return FALSE;
    if ( bot_find_path( ch, ch->in_room, temple, tmp, BOT_MAX_PATH, FALSE ) >= 0 )
    {
        bot->nopath_pulse = 0;
        return FALSE;
    }
    if ( bot->nopath_pulse == 0 )
        bot->nopath_pulse = bot_pulse;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
        if ( obj->item_type == ITEM_PORTAL && can_see_obj( ch, obj ) )
        {
            char kw[MAX_INPUT_LENGTH];
            bot_obj_keyword( ch, obj, ch->in_room->contents, kw, sizeof(kw) );
            bot_cmd( bot, "gir %s", kw );
            return TRUE;
        }
    if ( ch->level < KIDEMLI_OYUNCU_SEVIYESI )
    {
        bot_log( bot, "oda %d'den çıkış yolu yok; anımsa.", ch->in_room->vnum );
        bot_cmd( bot, "anımsa" );
        return TRUE;
    }
    if ( bot_pulse - bot->nopath_pulse > 4 * 60 * 5 )
    {
        bot_log( bot, "oda %d'den çıkış yolu yok; oyundan çıkıp tapınaktan dönecek.", ch->in_room->vnum );
        bot->nopath_pulse = 0;
        bot_logout( bot, TRUE );
        if ( bot->ch == NULL )
            bot->next_login_try = current_time + number_range( 60, 180 );
        return TRUE;
    }
    return FALSE;
}

/* ---------------------------------------------------------------------
 * botlar arası grup
 * ------------------------------------------------------------------ */
bool bot_in_group( CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    if ( ch->leader != NULL || ch->master != NULL )
        return TRUE;
    for ( rch = char_list; rch != NULL; rch = rch->next )
        if ( rch != ch && ( rch->leader == ch || rch->master == ch ) )
            return TRUE;
    return FALSE;
}

/* odadaki uygun bir bota grup teklif et; komut ürettiyse TRUE */
static bool bot_consider_grouping( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch, *rch;
    char out[MAX_STRING_LENGTH];

    if ( ch->master != NULL || ch->leader != NULL )
        return FALSE;
    if ( bot_pulse - bot->group_offer_pulse < 4 * 60 * 6 )
        return FALSE;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        BOT_DATA *other = bot_of( rch );

        if ( other == NULL || other == bot || rch->master != NULL || rch->leader != NULL )
            continue;
        if ( other->state != BOT_ST_HUNT && other->state != BOT_ST_IDLE && other->state != BOT_ST_TRAVEL )
            continue;
        if ( abs( ch->level - rch->level ) > 4 || rch->fighting != NULL )
            continue;
        if ( bot_pulse - other->group_offer_pulse < 4 * 60 * 6 )
            continue;
        if ( number_percent() > 65 )
            continue;
        bot->group_offer_pulse = bot_pulse;
        other->group_offer_pulse = bot_pulse;
        bot_fill( bot, number_percent() < 50 ? "grup olalım mı?" : "{hedef}, beraber keselim mi?", rch, out, sizeof(out) );
        bot_talk( bot, BOT_CH_SAY, NULL, out );
        return TRUE;
    }
    return FALSE;
}

/* seviyesi yakın, gruba girmemiş bir bota yürüyüp grup teklif et; komut ürettiyse TRUE */
static bool bot_seek_group( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *target;
    char out[MAX_STRING_LENGTH];

    if ( ch->master != NULL || ch->leader != NULL || bot_in_group( ch ) )
        return FALSE;
    if ( bot_pulse - bot->group_offer_pulse < 4 * 60 * 5 )
        return FALSE;

    target = bot_char_by_id( bot->invite_id );
    if ( target == NULL || !IS_BOT(target) || target->master != NULL || target->leader != NULL
      || abs( ch->level - target->level ) > 4 || bot_pulse - bot->invite_pulse > 4 * 60 * 4 )
    {
        BOT_DATA *ob, *pick = NULL;
        int count = 0;

        bot->invite_id = 0;
        for ( ob = bot_list; ob != NULL; ob = ob->next )
        {
            CHAR_DATA *och = ob->ch;
            sh_int tmp[BOT_MAX_PATH];

            if ( ob == bot || och == NULL || och->in_room == NULL )
                continue;
            if ( ob->state != BOT_ST_HUNT && ob->state != BOT_ST_TRAVEL && ob->state != BOT_ST_IDLE )
                continue;
            if ( och->master != NULL || och->leader != NULL || abs( ch->level - och->level ) > 4 )
                continue;
            if ( bot_pulse - ob->group_offer_pulse < 4 * 60 * 5 )
                continue;
            if ( och->in_room->area != ch->in_room->area
              && bot_find_path( ch, ch->in_room, och->in_room, tmp, 40, FALSE ) < 0 )
                continue;
            if ( number_range( 0, count++ ) == 0 )
                pick = ob;
        }
        if ( pick == NULL )
        {
            bot->group_offer_pulse = bot_pulse - 4 * 60 * 3;    /* 2 dk sonra tekrar bak */
            return FALSE;
        }
        bot->invite_id = pick->ch->id;
        bot->invite_pulse = bot_pulse;
        target = pick->ch;
    }

    if ( target->in_room == ch->in_room )
    {
        BOT_DATA *other = bot_of( target );

        bot->group_offer_pulse = bot_pulse;
        if ( other != NULL )
            other->group_offer_pulse = bot_pulse;
        bot->invite_id = 0;
        if ( target->fighting != NULL )
            return FALSE;
        bot_fill( bot, number_percent() < 50 ? "grup olalım mı?" : "{hedef}, beraber keselim mi?", target, out, sizeof(out) );
        bot_talk( bot, BOT_CH_SAY, NULL, out );
        return TRUE;
    }
    if ( !bot_set_travel( bot, target->in_room->vnum, BOT_ST_HUNT ) )
    {
        bot->invite_id = 0;
        return FALSE;
    }
    return TRUE;
}

/* beni takip eden botları gruba al; komut ürettiyse TRUE */
static bool bot_group_followers( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch, *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( rch == ch || rch->master != ch || !IS_BOT(rch) )
            continue;
        if ( is_same_group( ch, rch ) )
            continue;
        bot_cmd( bot, "grup %s", rch->name );
        return TRUE;
    }
    return FALSE;
}

/* ---------------------------------------------------------------------
 * av
 * ------------------------------------------------------------------ */
static void bot_hunt( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *prey;
    ROOM_INDEX_DATA *room;

    if ( bot->hunt_area == NULL || bot_pulse - bot->hunt_area_pulse > 4 * 60 * 25
      || ( ch->in_room->area != bot->hunt_area && bot->hunt_fail > 1 ) )
    {
        AREA_DATA *area = bot_pick_hunt_area( bot );

        if ( area == NULL )
        {
            /* uygun bölge yok: dünyaya dönüş yolu var mı? yoksa anımsa */
            if ( bot_escape_pocket( bot ) )
                return;
            bot->hunt_fail++;
            bot_set_state( bot, BOT_ST_REST );
            return;
        }
        if ( area != bot->hunt_area )
            bot->hunt_fail = 0;
        bot->hunt_area = area;
        bot->hunt_area_pulse = bot_pulse;
    }

    if ( ch->in_room->area != bot->hunt_area )
    {
        ROOM_INDEX_DATA *entry = area_entry_room( ch, bot->hunt_area );

        if ( entry == NULL || !bot_set_travel( bot, entry->vnum, BOT_ST_HUNT ) )
        {
            bot->hunt_fail += 2;
            bot->hunt_area = NULL;
            bot_set_state( bot, BOT_ST_IDLE );
        }
        return;
    }

    if ( bot_group_followers( bot ) )
        return;
    if ( bot_cast_buffs( bot ) )
        return;

    if ( ( prey = bot_prey_here( bot ) ) != NULL )
    {
        bot_attack( bot, prey );
        bot->hunt_fail = 0;
        return;
    }
    if ( bot_consider_grouping( bot ) )
        return;
    if ( bot_seek_group( bot ) )
        return;

    if ( ( room = bot_prey_near( bot, 14 ) ) != NULL )
    {
        if ( bot_set_travel( bot, room->vnum, BOT_ST_HUNT ) )
            return;
    }

    /* yakında av yok: bölgede biraz dolaş, birkaç denemeden sonra bölge değiştir */
    if ( ++bot->hunt_fail > 4 )
    {
        bot->hunt_area = NULL;
        bot->hunt_fail = 0;
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }
    {
        int n = bot_near_rooms( ch, 12, TRUE );
        ROOM_INDEX_DATA *dest = NULL;

        if ( n > 3 )
            dest = near_room[number_range( n / 2, n - 1 )];
        if ( dest == NULL || dest == ch->in_room )
            dest = area_entry_room( ch, bot->hunt_area );
        if ( dest == NULL || dest == ch->in_room || !bot_set_travel( bot, dest->vnum, BOT_ST_HUNT ) )
            bot_set_state( bot, BOT_ST_REST );
    }
}

/* ---------------------------------------------------------------------
 * hedef odalar (şehir)
 * ------------------------------------------------------------------ */
static ROOM_INDEX_DATA *nearest_of( CHAR_DATA *ch, ROOM_INDEX_DATA **rooms, int n )
{
    int i, best = -1, best_len = 100000;
    sh_int tmp[BOT_MAX_PATH];

    for ( i = 0; i < n && i < 16; i++ )
    {
        int len = bot_find_path( ch, ch->in_room, rooms[i], tmp, BOT_MAX_PATH, FALSE );
        if ( len >= 0 && len < best_len )
        {
            best_len = len;
            best = i;
        }
    }
    return best >= 0 ? rooms[best] : NULL;
}

static bool trainer_here( CHAR_DATA *ch, long flags )
{
    CHAR_DATA *mob;

    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
        if ( IS_NPC(mob) && IS_SET( mob->act, flags ) && can_see( ch, mob ) )
            return TRUE;
    return FALSE;
}

static ROOM_INDEX_DATA *bot_mob_room( CHAR_DATA *ch, bool (*pred)( CHAR_DATA *, CHAR_DATA * ) )
{
    CHAR_DATA *mob;
    ROOM_INDEX_DATA *rooms[16];
    int n = 0, pass;

    /* önce bulunduğu bölge, sonra memleket, sonra her yer */
    for ( pass = 0; pass < 3 && n == 0; pass++ )
    {
        for ( mob = char_list; mob != NULL && n < 16; mob = mob->next )
        {
            if ( !IS_NPC(mob) || mob->in_room == NULL || mob->position < POS_RESTING )
                continue;
            if ( pass == 0 && mob->in_room->area != ch->in_room->area )
                continue;
            if ( pass == 1 && !IS_SET( mob->in_room->area->area_flag, AREA_HOMETOWN ) )
                continue;
            if ( !pred( ch, mob ) )
                continue;
            if ( !bot_room_passable( ch, mob->in_room, FALSE ) )
                continue;
            rooms[n++] = mob->in_room;
        }
    }
    return nearest_of( ch, rooms, n );
}

static bool pred_practicer( CHAR_DATA *ch, CHAR_DATA *mob )
{
    return IS_SET( mob->act, ACT_PRACTICE ) && mob->pIndexData->pShop == NULL;
}

static bool pred_trainer( CHAR_DATA *ch, CHAR_DATA *mob )
{
    return IS_SET( mob->act, ACT_PRACTICE | ACT_TRAIN | ACT_GAIN ) && mob->pIndexData->pShop == NULL;
}

static bool pred_questmaster( CHAR_DATA *ch, CHAR_DATA *mob )
{
    return spec_questmaster_fn != NULL && mob->spec_fun == spec_questmaster_fn;
}

static bool pred_healer( CHAR_DATA *ch, CHAR_DATA *mob )
{
    if ( !IS_SET( mob->act, ACT_IS_HEALER ) )
        return FALSE;
    if ( is_name( "kabal", mob->name ) || is_name( "cabal", mob->name ) )
        return FALSE;
    return TRUE;
}

static int shop_want_type = 0;
static bool pred_shop_buys( CHAR_DATA *ch, CHAR_DATA *mob )
{
    SHOP_DATA *shop = mob->pIndexData->pShop;
    int i;

    if ( shop == NULL )
        return FALSE;
    for ( i = 0; i < MAX_TRADE; i++ )
        if ( shop->buy_type[i] == shop_want_type )
            return TRUE;
    return FALSE;
}

static bool keeper_sells( CHAR_DATA *keeper, int type, int max_level )
{
    OBJ_DATA *obj;

    for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
        if ( obj->item_type == type && obj->level <= max_level && obj->wear_loc == WEAR_NONE )
            return TRUE;
    return FALSE;
}

static bool pred_sells_type( CHAR_DATA *ch, CHAR_DATA *mob )
{
    if ( mob->pIndexData->pShop == NULL )
        return FALSE;
    if ( shop_want_type == ITEM_POTION )
    {
        OBJ_DATA *obj;
        for ( obj = mob->carrying; obj != NULL; obj = obj->next_content )
        {
            int i;
            if ( obj->item_type != ITEM_POTION || obj->level > ch->level )
                continue;
            for ( i = 1; i <= 3; i++ )
                if ( obj->value[i] == sn_cure_light || obj->value[i] == sn_cure_serious
                  || obj->value[i] == sn_cure_critical || obj->value[i] == sn_heal )
                    return TRUE;
        }
        return FALSE;
    }
    return keeper_sells( mob, shop_want_type, ch->level + 3 );
}

static ROOM_INDEX_DATA *bot_fountain_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *rooms[16];
    int n = 0;

    for ( obj = object_list; obj != NULL && n < 16; obj = obj->next )
    {
        if ( obj->item_type != ITEM_FOUNTAIN || obj->in_room == NULL )
            continue;
        if ( obj->in_room->area != ch->in_room->area
          && !IS_SET( obj->in_room->area->area_flag, AREA_HOMETOWN ) )
            continue;
        if ( !bot_room_passable( ch, obj->in_room, FALSE ) )
            continue;
        rooms[n++] = obj->in_room;
    }
    return nearest_of( ch, rooms, n );
}

/* odadaki dükkâncı */
static CHAR_DATA *keeper_here( CHAR_DATA *ch )
{
    CHAR_DATA *mob;

    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
        if ( IS_NPC(mob) && mob->pIndexData->pShop != NULL )
            return mob;
    return NULL;
}

/* ---------------------------------------------------------------------
 * şehir işleri
 * ------------------------------------------------------------------ */
static int task_index( long task )
{
    int i;

    for ( i = 0; i < 16; i++ )
        if ( task == ( 1L << i ) )
            return i;
    return 0;
}

/* bu iş bir süre denenmesin */
static void bot_task_defer( BOT_DATA *bot, long task, int minutes )
{
    bot->town_retry[task_index( task )] = bot_pulse + 4 * 60 * minutes;
    REMOVE_BIT( bot->town_tasks, task );
}

static void bot_compute_town_tasks( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    long tasks = bot->town_tasks;
    int i;

    if ( ch->practice >= 3 && bot_pulse > bot->practice_block_until )
        SET_BIT( tasks, BOT_TOWN_PRACTICE );
    if ( ch->train >= 1 && bot_pulse > bot->practice_block_until )
        SET_BIT( tasks, BOT_TOWN_TRAIN );
    if ( bot_sell_candidates( bot ) >= 4 || ch->carry_number >= can_carry_n( ch ) - 2 )
        SET_BIT( tasks, BOT_TOWN_SELL );
    if ( carried_type( ch, ITEM_FOOD ) == NULL && ch->silver >= 15 )
        SET_BIT( tasks, BOT_TOWN_FOOD );
    if ( carried_drink( ch ) == NULL && ch->silver >= 40 )
        SET_BIT( tasks, BOT_TOWN_DRINK );
    if ( get_light_char( ch ) == NULL && carried_type( ch, ITEM_LIGHT ) == NULL && ch->silver >= 20 )
        SET_BIT( tasks, BOT_TOWN_LIGHT );
    if ( bot_count_potions( ch ) < 2 && ch->silver >= 400 && ch->level >= 5 && ch->cabal != CABAL_BATTLE )
        SET_BIT( tasks, BOT_TOWN_POTION );
    if ( IS_QUESTOR(ch) && ch->pcdata->questmob == -1 )
        SET_BIT( tasks, BOT_TOWN_QUEST_DONE );
    else if ( !IS_QUESTOR(ch) && ch->pcdata->nextquest == 0 && ch->level >= 5
           && bot_pulse - bot->last_town_pulse > 4 * 60 * 10 )
        SET_BIT( tasks, BOT_TOWN_QUEST_GET );
    if ( ch->pcdata->questpoints >= 1000
      || ( bot->pk_istekli && ch->pcdata->questpoints >= 100 && ch->pcdata->oyuncu_katli == 0 && ch->level >= 12 ) )
        SET_BIT( tasks, BOT_TOWN_QUEST_BUY );
    if ( ch->silver >= 1500 && ch->level >= 6 && bot_pulse - bot->last_town_pulse > 4 * 60 * 30 )
        SET_BIT( tasks, BOT_TOWN_UPGRADE );
    for ( i = 0; i < 16; i++ )
        if ( bot_pulse < bot->town_retry[i] )
            REMOVE_BIT( tasks, 1L << i );
    bot->town_tasks = tasks;
}

static bool bot_town_worth_it( BOT_DATA *bot )
{
    long t = bot->town_tasks;

    if ( IS_SET( t, BOT_TOWN_QUEST_DONE | BOT_TOWN_LIGHT | BOT_TOWN_FOUNTAIN ) )
        return TRUE;
    if ( IS_SET( t, BOT_TOWN_FOOD ) && bot->ch->pcdata->condition[COND_HUNGER] < 20 )
        return TRUE;
    if ( IS_SET( t, BOT_TOWN_DRINK ) && bot->ch->pcdata->condition[COND_THIRST] < 20 )
        return TRUE;
    if ( IS_SET( t, BOT_TOWN_PRACTICE ) && bot->ch->practice >= 4 )
        return TRUE;
    if ( IS_SET( t, BOT_TOWN_SELL ) && bot_sell_candidates( bot ) >= 5 )
        return TRUE;
    if ( IS_SET( t, BOT_TOWN_QUEST_GET | BOT_TOWN_QUEST_BUY | BOT_TOWN_UPGRADE ) )
        return TRUE;
    if ( t != 0 && bot_pulse - bot->last_town_pulse > 4 * 60 * 40 )
        return TRUE;
    return FALSE;
}

static long bot_town_next_task( BOT_DATA *bot )
{
    static const long order[] =
    {
        BOT_TOWN_QUEST_DONE, BOT_TOWN_FOUNTAIN, BOT_TOWN_SELL, BOT_TOWN_FOOD, BOT_TOWN_DRINK,
        BOT_TOWN_LIGHT, BOT_TOWN_PRACTICE, BOT_TOWN_TRAIN, BOT_TOWN_POTION, BOT_TOWN_UPGRADE,
        BOT_TOWN_HEAL, BOT_TOWN_QUEST_BUY, BOT_TOWN_QUEST_GET, 0
    };
    int i;

    for ( i = 0; order[i] != 0; i++ )
        if ( IS_SET( bot->town_tasks, order[i] ) )
            return order[i];
    return 0;
}

/* görevde satın alma sırası */
static const char *bot_quest_buy_item( CHAR_DATA *ch, BOT_DATA *bot )
{
    int gp = ch->pcdata->questpoints;

    if ( bot->pk_istekli && ch->pcdata->oyuncu_katli == 0 && gp >= 100 && ch->level >= 12 )
        return "katlet";
    if ( gp < 1000 )
        return NULL;
    if ( get_wield_char( ch, FALSE ) == NULL || get_wield_char( ch, FALSE )->pIndexData->vnum != QUEST_ITEM_SILAH1 )
    {
        OBJ_DATA *obj;
        bool has = FALSE;
        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
            if ( gorev_ekipmani_mi( obj ) && obj->item_type == ITEM_WEAPON )
                has = TRUE;
        if ( !has )
            return IS_EVIL(ch) ? "mat" : "parlak";
    }
    if ( gp >= 1000 )
    {
        static const char *rest[] = { "miğfer", "kolluk", "bacaklık", "kalkan", "kemer", NULL };
        static const int vn[] = { QUEST_ITEM_MIGFER, QUEST_ITEM_KOLLUK, QUEST_ITEM_BACAKLIK, QUEST_ITEM_KALKAN, QUEST_ITEM5 };
        int i;
        for ( i = 0; rest[i] != NULL; i++ )
        {
            OBJ_DATA *obj;
            bool has = FALSE;
            for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
                if ( obj->pIndexData->vnum == vn[i] )
                    has = TRUE;
            if ( !has )
                return rest[i];
        }
        return "pratik";
    }
    return NULL;
}

/* şehir görevi için hedef oda; yoksa NULL */
static ROOM_INDEX_DATA *bot_town_target_room( BOT_DATA *bot, long task )
{
    CHAR_DATA *ch = bot->ch;

    switch ( task )
    {
    case BOT_TOWN_PRACTICE:
        return bot_mob_room( ch, pred_practicer );
    case BOT_TOWN_TRAIN:
        return bot_mob_room( ch, pred_trainer );
    case BOT_TOWN_QUEST_GET:
    case BOT_TOWN_QUEST_DONE:
    case BOT_TOWN_QUEST_BUY:
        return bot_mob_room( ch, pred_questmaster );
    case BOT_TOWN_HEAL:
        return bot_mob_room( ch, pred_healer );
    case BOT_TOWN_FOUNTAIN:
        return bot_fountain_room( ch );
    case BOT_TOWN_SELL:
    {
        OBJ_DATA *obj;
        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
            if ( bot_is_junk( ch, obj ) && obj->cost > 0 && !IS_OBJ_STAT( obj, ITEM_NOSELL ) )
            {
                ROOM_INDEX_DATA *room;
                shop_want_type = obj->item_type;
                if ( ( room = bot_mob_room( ch, pred_shop_buys ) ) != NULL )
                    return room;
            }
        return NULL;
    }
    case BOT_TOWN_FOOD:   shop_want_type = ITEM_FOOD;      return bot_mob_room( ch, pred_sells_type );
    case BOT_TOWN_DRINK:  shop_want_type = ITEM_DRINK_CON; return bot_mob_room( ch, pred_sells_type );
    case BOT_TOWN_LIGHT:  shop_want_type = ITEM_LIGHT;     return bot_mob_room( ch, pred_sells_type );
    case BOT_TOWN_POTION: shop_want_type = ITEM_POTION;    return bot_mob_room( ch, pred_sells_type );
    case BOT_TOWN_UPGRADE:
        shop_want_type = is_caster( ch ) && number_percent() < 50 ? ITEM_ARMOR
                       : ( number_percent() < 50 ? ITEM_ARMOR : ITEM_WEAPON );
        return bot_mob_room( ch, pred_sells_type );
    }
    return NULL;
}

/* dükkânda satın alınacak eşya */
static OBJ_DATA *bot_pick_purchase( CHAR_DATA *ch, CHAR_DATA *keeper, long task )
{
    OBJ_DATA *obj, *best = NULL;
    int best_score = 0;

    for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
    {
        int cost = get_cost( keeper, obj, TRUE );
        int score = 0;

        if ( cost <= 0 || cost > ch->silver || obj->wear_loc != WEAR_NONE || !can_see_obj( ch, obj ) )
            continue;
        switch ( task )
        {
        case BOT_TOWN_FOOD:
            if ( obj->item_type != ITEM_FOOD || cost > 150 ) continue;
            score = 100 - cost + obj->value[0];
            break;
        case BOT_TOWN_DRINK:
            if ( obj->item_type != ITEM_DRINK_CON || cost > 300 ) continue;
            score = obj->value[0];
            break;
        case BOT_TOWN_LIGHT:
            if ( obj->item_type != ITEM_LIGHT || obj->level > ch->level || cost > ch->silver / 2 ) continue;
            score = ( obj->value[2] == -1 ? 500 : obj->value[2] ) - cost / 10;
            break;
        case BOT_TOWN_POTION:
        {
            int i;
            bool cure = FALSE;
            if ( obj->item_type != ITEM_POTION || obj->level > ch->level || cost > ch->silver / 3 ) continue;
            for ( i = 1; i <= 3; i++ )
                if ( obj->value[i] == sn_cure_light || obj->value[i] == sn_cure_serious
                  || obj->value[i] == sn_cure_critical || obj->value[i] == sn_heal )
                    cure = TRUE;
            if ( !cure ) continue;
            score = obj->value[0] * 10 - cost / 20;
            break;
        }
        case BOT_TOWN_UPGRADE:
        {
            int slot, gain, s;
            if ( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR ) continue;
            if ( cost > ch->silver * 7 / 10 ) continue;
            if ( ( s = bot_obj_score( ch, obj ) ) <= 0 ) continue;
            slot = bot_wear_slot( obj );
            gain = s - bot_slot_score( ch, obj, slot );
            if ( gain <= 3 ) continue;
            score = gain * 10 - cost / 50;
            break;
        }
        default:
            continue;
        }
        if ( score > best_score )
        {
            best_score = score;
            best = obj;
        }
    }
    return best;
}

/* pratik yapılacak yetenek */
static int bot_pick_practice( CHAR_DATA *ch )
{
    int sn, best = -1, best_score = 0;
    OBJ_DATA *wield = get_wield_char( ch, FALSE );
    int wield_gsn = wield != NULL ? weapon_gsn_for_type( wield->value[0] ) : -1;
    int adept = class_table[ch->iclass].skill_adept;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
        int score;

        if ( skill_table[sn].name[0] == NULL )
            break;
        if ( ch->level < skill_table[sn].skill_level[ch->iclass] )
            continue;
        if ( !RACE_OK( ch, sn ) )
            continue;
        if ( skill_table[sn].cabal != ch->cabal && skill_table[sn].cabal != CABAL_NONE )
            continue;
        if ( skill_table[sn].rating[ch->iclass] <= 0 )
            continue;
        if ( ch->pcdata->learned[sn] >= adept )
            continue;
        if ( !str_cmp( skill_table[sn].name[0], "vampire" ) )
            continue;

        score = 10;
        if ( sn == wield_gsn )                                   score = 120;
        else if ( sn == sn_second_attack )                       score = 110;
        else if ( sn == sn_third_attack )                        score = 100;
        else if ( sn == sn_dodge || sn == sn_parry )              score = 80;
        else if ( sn == sn_shield_block )                        score = 70;
        else if ( sn == sn_enhanced_damage )                     score = 90;
        else if ( sn == sn_fast_healing )                        score = 75;
        else if ( sn == sn_bash || sn == sn_kick || sn == sn_trip || sn == sn_disarm ) score = 60;
        else if ( sn == sn_backstab || sn == sn_circle || sn == sn_dirt ) score = is_rogue( ch ) ? 95 : 20;
        else if ( sn == sn_meditation )                          score = is_caster( ch ) ? 85 : 30;
        else if ( sn == sn_hand_to_hand )                        score = 40;
        else if ( sn == gsn_recall )                             score = ch->level < 16 ? 50 : 0;
        else if ( skill_table[sn].spell_fun != spell_null )
        {
            if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
              && skill_table[sn].noun_damage != NULL && skill_table[sn].noun_damage[0] != '\0' )
                score = is_caster( ch ) ? 100 : 30;
            else if ( sn == sn_cure_light || sn == sn_cure_serious || sn == sn_cure_critical || sn == sn_heal )
                score = is_healer_class( ch ) ? 95 : 40;
            else
            {
                int i;
                score = is_caster( ch ) ? 35 : 15;
                for ( i = 0; i < buff_count; i++ )
                    if ( buff_sn[i] == sn )
                        score = is_caster( ch ) ? 70 : 45;
            }
        }
        else if ( weapon_gsn_for_type( WEAPON_SWORD ) == sn || weapon_gsn_for_type( WEAPON_DAGGER ) == sn )
            score = 35;

        /* öğrenilmemiş ama önemli olanlar önce, sonra düşük yüzdeliler */
        score = score * ( 100 - ch->pcdata->learned[sn] ) / 100 + number_range( 0, 8 );
        if ( score > best_score )
        {
            best_score = score;
            best = sn;
        }
    }
    return best;
}

static void bot_town( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    long task = bot->substate;

    if ( task == 0 )
    {
        ROOM_INDEX_DATA *room;

        bot_compute_town_tasks( bot );
        if ( ( task = bot_town_next_task( bot ) ) == 0 )
        {
            bot->last_town_pulse = bot_pulse;
            bot_set_state( bot, BOT_ST_IDLE );
            return;
        }
        room = bot_town_target_room( bot, task );
        if ( room == NULL )
        {
            bot_task_defer( bot, task, 20 );
            if ( task == BOT_TOWN_PRACTICE || task == BOT_TOWN_TRAIN )
                bot->practice_block_until = bot_pulse + 4 * 60 * 20;
            return;
        }
        bot->substate  = (int) task;
        bot->town_step = 0;
        if ( room != ch->in_room )
        {
            if ( !bot_set_travel( bot, room->vnum, BOT_ST_TOWN ) )
            {
                bot_task_defer( bot, task, 20 );
                bot->substate = 0;
            }
            else
                bot->substate = (int) task;   /* seyahat sonrası aynı iş */
            return;
        }
    }

    /* hedef odadayız: işi yap */
    if ( ch->position < POS_STANDING && ch->position >= POS_SLEEPING )
    {
        if ( !IS_AFFECTED( ch, AFF_SLEEP ) )
            bot_cmd( bot, "kalk" );
        return;
    }
    if ( ++bot->town_step > 40 )
    {
        bot_task_defer( bot, task, 15 );
        bot->substate = 0;
        return;
    }

    switch ( task )
    {
    case BOT_TOWN_SELL:
    {
        OBJ_DATA *obj;
        CHAR_DATA *keeper = keeper_here( ch );
        char kw[MAX_INPUT_LENGTH];

        if ( keeper == NULL )
            break;
        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        {
            int cost;
            if ( !bot_is_junk( ch, obj ) || obj->cost <= 0 )
                continue;
            cost = get_cost( keeper, obj, FALSE );
            bot_obj_keyword( ch, obj, ch->carrying, kw, sizeof(kw) );
            if ( cost > 0 )
                bot_cmd( bot, "sat %s", kw );
            else
                bot_cmd( bot, "kurban %s", kw );
            bot->inv_signature = -1;
            return;
        }
        /* satılacak bir şey kalmadı: farklı türden eşyalar için dükkân değiştir */
        {
            ROOM_INDEX_DATA *room;
            REMOVE_BIT( bot->town_tasks, task );
            bot->substate = 0;
            if ( bot_sell_candidates( bot ) > 0 && ( room = bot_town_target_room( bot, BOT_TOWN_SELL ) ) != NULL
              && room != ch->in_room && bot->town_step < 20 )
            {
                SET_BIT( bot->town_tasks, BOT_TOWN_SELL );
                bot->substate = BOT_TOWN_SELL;
                bot_set_travel( bot, room->vnum, BOT_ST_TOWN );
            }
        }
        return;
    }
    case BOT_TOWN_FOOD: case BOT_TOWN_DRINK: case BOT_TOWN_LIGHT: case BOT_TOWN_POTION: case BOT_TOWN_UPGRADE:
    {
        CHAR_DATA *keeper = keeper_here( ch );
        OBJ_DATA *want;
        char kw[MAX_INPUT_LENGTH];
        int have;

        if ( keeper == NULL )
            break;
        if ( task == BOT_TOWN_FOOD ) have = carried_type( ch, ITEM_FOOD ) != NULL ? 1 : 0;
        else if ( task == BOT_TOWN_DRINK ) have = carried_type( ch, ITEM_DRINK_CON ) != NULL ? 1 : 0;
        else if ( task == BOT_TOWN_LIGHT ) have = get_light_char( ch ) != NULL ? 1 : 0;
        else if ( task == BOT_TOWN_POTION ) have = bot_count_potions( ch ) >= 3 ? 1 : 0;
        else have = bot->town_step > 3 ? 1 : 0;
        if ( have || ( want = bot_pick_purchase( ch, keeper, task ) ) == NULL )
        {
            if ( task == BOT_TOWN_LIGHT && carried_type( ch, ITEM_LIGHT ) != NULL )
                bot_wear_upgrades( bot );
            break;
        }
        bot_obj_keyword( ch, want, keeper->carrying, kw, sizeof(kw) );
        snprintf( bot->last_item, sizeof(bot->last_item), "%s", want->short_descr );
        bot_cmd( bot, "satınal %s", kw );
        bot->inv_signature = -1;
        if ( bot->ch != NULL && task == BOT_TOWN_FOOD && ch->pcdata->condition[COND_HUNGER] < 20 )
            bot_eat_drink( bot );
        return;
    }
    case BOT_TOWN_FOUNTAIN:
        if ( room_fountain( ch ) == NULL )
            break;
        if ( ch->pcdata->condition[COND_THIRST] < 40 )
        {
            bot_cmd( bot, "iç" );
            return;
        }
        if ( bot_eat_drink( bot ) )
            return;
        break;
    case BOT_TOWN_PRACTICE:
    {
        int sn, before = ch->practice;
        if ( !trainer_here( ch, ACT_PRACTICE ) )
        {
            bot->practice_block_until = bot_pulse + 4 * 60 * 20;
            break;
        }
        if ( ch->practice <= 0 || ( sn = bot_pick_practice( ch ) ) < 0 )
        {
            REMOVE_BIT( bot->town_tasks, BOT_TOWN_PRACTICE );
            if ( ch->practice > 0 )
                bot->practice_block_until = bot_pulse + 4 * 60 * 30;
            bot->substate = IS_SET( bot->town_tasks, BOT_TOWN_TRAIN ) ? BOT_TOWN_TRAIN : 0;
            bot->town_step = 0;
            return;
        }
        bot_cmd( bot, "pratik %s", skill_table[sn].name[1] );
        if ( bot->ch != NULL && ch->practice == before && ++bot->quest_tries > 3 )
        {
            /* pratik ilerlemiyor: bir süre deneme */
            bot->quest_tries = 0;
            bot->practice_block_until = bot_pulse + 4 * 60 * 20;
            REMOVE_BIT( bot->town_tasks, BOT_TOWN_PRACTICE );
            bot->substate = 0;
        }
        else if ( bot->ch != NULL && ch->practice < before )
            bot->quest_tries = 0;
        return;
    }
    case BOT_TOWN_TRAIN:
    {
        int prime = class_table[ch->iclass].attr_prime;
        const char *what;
        int before = ch->train;
        if ( !trainer_here( ch, ACT_PRACTICE | ACT_TRAIN | ACT_GAIN ) )
        {
            bot->practice_block_until = bot_pulse + 4 * 60 * 20;
            break;
        }
        if ( ch->train <= 0 || bot->town_step > 8 )
            break;
        (void) before;
        if ( ch->perm_stat[prime] < get_max_train( ch, prime ) )
            what = prime == STAT_STR ? "güç" : prime == STAT_INT ? "zeka" : prime == STAT_WIS ? "bilgelik"
                 : prime == STAT_DEX ? "çeviklik" : "bünye";
        else if ( ch->perm_stat[STAT_CON] < get_max_train( ch, STAT_CON ) && number_percent() < 50 )
            what = "bünye";
        else
            what = is_caster( ch ) ? "mana" : "yp";
        bot_cmd( bot, "eğitim %s", what );
        return;
    }
    case BOT_TOWN_HEAL:
    {
        CHAR_DATA *mob;
        for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
            if ( IS_NPC(mob) && IS_SET( mob->act, ACT_IS_HEALER ) )
                break;
        if ( mob == NULL || pct( ch->hit, ch->max_hit ) > 85 || ch->silver < 100 )
            break;
        if ( ch->max_hit > 200 && ch->silver >= 600 )
            bot_cmd( bot, "iyileştir şifa" );
        else if ( ch->silver >= 260 )
            bot_cmd( bot, "iyileştir kritik" );
        else
            bot_cmd( bot, "iyileştir hafif" );
        return;
    }
    case BOT_TOWN_QUEST_GET:
        if ( bot_mob_room( ch, pred_questmaster ) != ch->in_room )
            break;
        if ( IS_QUESTOR(ch) || ch->pcdata->nextquest > 0 )
            break;
        bot_cmd( bot, "görev iste" );
        if ( bot->ch != NULL && IS_QUESTOR(ch) && ch->pcdata->questmob > 0 )
        {
            bot->quest_stage = 1;
            bot->quest_tries = 0;
            bot_chat_event( bot, BOT_EV_QUEST_GET, NULL );
        }
        break;
    case BOT_TOWN_QUEST_DONE:
        if ( bot_mob_room( ch, pred_questmaster ) != ch->in_room )
            break;
        if ( IS_QUESTOR(ch) && ch->pcdata->questmob == -1 )
        {
            int before = ch->pcdata->questpoints;
            bot_cmd( bot, "görev bitti" );
            if ( bot->ch != NULL && ch->pcdata->questpoints > before )
            {
                bot->quests++;
                bot_chat_event( bot, BOT_EV_QUEST_DONE, NULL );
                bot_log( bot, "görev tamamladı (%d gp).", ch->pcdata->questpoints );
            }
        }
        break;
    case BOT_TOWN_QUEST_BUY:
    {
        const char *item;
        if ( bot_mob_room( ch, pred_questmaster ) != ch->in_room )
            break;
        if ( ( item = bot_quest_buy_item( ch, bot ) ) == NULL )
            break;
        bot_cmd( bot, "görev satınal %s", item );
        bot->inv_signature = -1;
        if ( bot->ch != NULL && bot_quest_buy_item( ch, bot ) != NULL && bot->town_step < 6 )
            return;
        break;
    }
    }

    bot_task_defer( bot, task, 10 );
    bot->substate = 0;
}

/* ---------------------------------------------------------------------
 * görev (av)
 * ------------------------------------------------------------------ */
static void bot_quest( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *mob;

    if ( !IS_QUESTOR(ch) || ch->pcdata->questmob == 0 )
    {
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }
    if ( ch->pcdata->questmob == -1 )
    {
        SET_BIT( bot->town_tasks, BOT_TOWN_QUEST_DONE );
        bot_set_state( bot, BOT_ST_TOWN );
        return;
    }

    /* görev yaratığı nerede? */
    for ( mob = char_list; mob != NULL; mob = mob->next )
        if ( IS_NPC(mob) && mob->pIndexData->vnum == ch->pcdata->questmob && mob->in_room != NULL )
            break;

    if ( mob == NULL )
    {
        if ( ++bot->quest_tries > 20 )
        {
            bot_set_state( bot, BOT_ST_IDLE );
            bot->quest_tries = 0;
        }
        return;
    }

    if ( mob->level > ch->level + level_bonus( ch ) + 2 && ch->pcdata->countdown < 25 )
    {
        /* çok güçlü: vazgeç (görevciye gidip iptal etmek yerine süresi dolsun) */
        bot_set_state( bot, BOT_ST_IDLE );
        REMOVE_BIT( bot->town_tasks, BOT_TOWN_QUEST_GET );
        return;
    }

    if ( mob->in_room == ch->in_room )
    {
        if ( bot_cast_buffs( bot ) )
            return;
        if ( mob->fighting != NULL && mob->fighting != ch )
        {
            if ( ++bot->quest_tries > 30 )
                bot_set_state( bot, BOT_ST_IDLE );
            return;
        }
        bot_attack( bot, mob );
        return;
    }
    if ( bot->quest_tries++ > 40 )
    {
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }
    if ( !bot_set_travel( bot, mob->in_room->vnum, BOT_ST_QUEST ) )
        bot_set_state( bot, BOT_ST_IDLE );
}

/* ---------------------------------------------------------------------
 * ceset
 * ------------------------------------------------------------------ */
static OBJ_DATA *bot_own_corpse( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
        if ( obj->item_type == ITEM_CORPSE_PC && obj->owner != NULL && !str_cmp( obj->owner, ch->name ) )
            return obj;
    return NULL;
}

void bot_after_death( BOT_DATA *bot )
{
    CHAR_DATA *killer = bot_char_by_id( bot->target_id );

    /* beni öldüren yaratık türünden bir süre uzak dur */
    if ( killer != NULL && IS_NPC(killer) )
    {
        bot->avoid_vnum[bot->avoid_pos]  = killer->pIndexData->vnum;
        bot->avoid_until[bot->avoid_pos] = bot_pulse + 4 * 60 * 90;
        bot->avoid_pos = ( bot->avoid_pos + 1 ) % BOT_AVOID_MAX;
    }
    bot->hunt_area = NULL;
    bot->target_id = 0;
    bot->leader_id = 0;
    bot->path_len = bot->path_pos = 0;
    if ( bot->death_room > 0 )
        bot_set_state( bot, BOT_ST_CORPSE );
    else
        bot_set_state( bot, BOT_ST_REST );
    bot_chat_event( bot, BOT_EV_DEATH, NULL );
}

static void bot_corpse( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    OBJ_DATA *corpse;
    char kw[MAX_INPUT_LENGTH];

    if ( ch->position < POS_STANDING )
    {
        bot_cmd( bot, "kalk" );
        return;
    }
    if ( ( corpse = bot_own_corpse( ch ) ) != NULL )
    {
        if ( corpse->contains != NULL && bot->corpse_tries < 6 )
        {
            bot->corpse_tries++;
            bot_obj_keyword( ch, corpse, ch->in_room->contents, kw, sizeof(kw) );
            bot_cmd( bot, "al tümü %s", kw );
            bot->inv_signature = -1;
            return;
        }
        bot_cmd( bot, "giy tümü" );
        bot->death_room = 0;
        bot_set_state( bot, BOT_ST_REST );
        return;
    }
    if ( bot->death_room == 0 || ch->in_room->vnum == bot->death_room )
    {
        /* ceset yok (çürümüş ya da alınmış) */
        bot->death_room = 0;
        bot_set_state( bot, BOT_ST_REST );
        return;
    }
    if ( bot->corpse_tries++ > 3 || !bot_set_travel( bot, bot->death_room, BOT_ST_CORPSE ) )
    {
        bot->death_room = 0;
        bot_set_state( bot, BOT_ST_REST );
    }
}

/* ---------------------------------------------------------------------
 * takip / grup
 * ------------------------------------------------------------------ */
bool bot_wants_group_with( BOT_DATA *bot, CHAR_DATA *other )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL || other == NULL || other == ch || IS_NPC(other) )
        return FALSE;
    if ( abs( ch->level - other->level ) > 8 )
        return FALSE;
    if ( bot->state == BOT_ST_FOLLOW || bot->state == BOT_ST_CORPSE || bot->state == BOT_ST_PK )
        return FALSE;
    if ( IS_SET( ch->act, PLR_GHOST ) )
        return FALSE;
    return TRUE;
}

void bot_start_follow( BOT_DATA *bot, CHAR_DATA *leader )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL || leader == NULL || leader->in_room != ch->in_room )
        return;
    bot->leader_id = leader->id;
    bot->follow_since = bot_pulse;
    bot->follow_until = bot_pulse + number_range( 4 * 60 * 40, 4 * 60 * 90 );
    bot->leader_last_action = bot_pulse;
    bot->path_len = bot->path_pos = 0;
    bot_set_state( bot, BOT_ST_FOLLOW );
    bot_cmd( bot, "takip %s", leader->name );
}

void bot_stop_follow( BOT_DATA *bot, bool say )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL )
        return;
    bot->leader_id = 0;
    if ( ch->master != NULL )
        bot_cmd( bot, "takip %s", ch->name );
    if ( bot->ch != NULL )
        bot_set_state( bot, BOT_ST_IDLE );
}

static void bot_following( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *leader = bot_char_by_id( bot->leader_id );

    if ( leader == NULL || leader->in_room == NULL
      || ( !IS_BOT(leader) && ( leader->desc == NULL || leader->desc->connected != CON_PLAYING ) ) )
    {
        bot_stop_follow( bot, FALSE );
        return;
    }
    if ( IS_SET( ch->act, PLR_GHOST ) )
    {
        bot_stop_follow( bot, TRUE );
        return;
    }

    if ( IS_BOT(leader) && bot_pulse > bot->follow_until )
    {
        bot_chat_event( bot, BOT_EV_LOGOUT, leader );
        bot_stop_follow( bot, TRUE );
        return;
    }

    if ( leader->in_room == ch->in_room )
    {
        if ( ch->master != leader && !IS_AFFECTED( ch, AFF_CHARM ) )
        {
            bot_cmd( bot, "takip %s", leader->name );
            return;
        }
        if ( leader->fighting != NULL || leader->position == POS_FIGHTING )
            bot->leader_last_action = bot_pulse;
        if ( leader->fighting != NULL && ch->fighting == NULL && ch->position >= POS_STANDING
          && IS_NPC(leader->fighting) && !is_safe_nomessage( ch, leader->fighting ) )
        {
            bot_attack( bot, leader->fighting );
            return;
        }
        if ( ch->position < POS_STANDING && leader->position >= POS_STANDING && pct( ch->hit, ch->max_hit ) > 60 )
        {
            bot_cmd( bot, "kalk" );
            return;
        }
        if ( leader->position == POS_SLEEPING || leader->position == POS_RESTING )
        {
            if ( ch->position == POS_STANDING && pct( ch->hit, ch->max_hit ) < 95 )
                bot_cmd( bot, leader->position == POS_SLEEPING ? "uyu" : "dinlen" );
            return;
        }
        if ( bot_cast_buffs( bot ) )
            return;
        if ( is_healer_class( ch ) && bot_is_human( leader ) && pct( leader->hit, leader->max_hit ) < 55
          && can_cast_sn( ch, sn_cure_light ) )
        {
            int sn = can_cast_sn( ch, sn_cure_critical ) ? sn_cure_critical
                   : can_cast_sn( ch, sn_cure_serious ) ? sn_cure_serious : sn_cure_light;
            bot_cmd( bot, "büyü '%s' %s", skill_table[sn].name[1], leader->name );
            return;
        }
        if ( bot_eat_drink( bot ) )
            return;
        if ( ch->position == POS_STANDING && pct( ch->hit, ch->max_hit ) < 40 && ch->fighting == NULL )
        {
            bot_cmd( bot, "dinlen" );
            return;
        }
        /* lider uzun süredir hiçbir şey yapmıyorsa kendi yoluna git */
        if ( bot_pulse - bot->leader_last_action > 4 * 60 * ( bot_is_human( leader ) ? 40 : 15 ) )
        {
            bot_chat_event( bot, BOT_EV_LOGOUT, leader );     /* "ben kesmeye gidiyorum" tarzı */
            bot_stop_follow( bot, TRUE );
        }
        return;
    }

    /* lideri kaybettik: peşinden git */
    bot->leader_last_action = bot_pulse;
    if ( ch->position < POS_STANDING )
    {
        bot_cmd( bot, "kalk" );
        return;
    }
    if ( bot->path_len == 0 || get_room_index( bot->target_vnum ) != leader->in_room )
    {
        if ( !bot_set_travel( bot, leader->in_room->vnum, BOT_ST_FOLLOW ) )
        {
            bot_stop_follow( bot, TRUE );
            return;
        }
        bot_set_state( bot, BOT_ST_FOLLOW );
    }
    if ( bot->path_pos < bot->path_len )
    {
        int dir = bot->path[bot->path_pos];
        EXIT_DATA *pexit = ch->in_room->exit[dir];

        if ( pexit == NULL || pexit->u1.to_room == NULL )
        {
            bot->path_len = 0;
            return;
        }
        if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
        {
            bot_cmd( bot, "aç %s", dir_name[dir] );
            return;
        }
        bot_cmd( bot, "%s", dir_name[dir] );
        if ( bot->ch != NULL && ch->in_room == pexit->u1.to_room )
            bot->path_pos++;
        else
            bot->path_len = 0;
    }
}

/* ---------------------------------------------------------------------
 * kabal ve oyuncu katli
 * ------------------------------------------------------------------ */
static int bot_choose_cabal( CHAR_DATA *ch )
{
    if ( is_caster( ch ) )
    {
        if ( ch->ethos == 1 && get_curr_stat( ch, STAT_INT ) >= 19 && number_percent() < 40 )
            return CABAL_RULER;
        return CABAL_SHALAFI;
    }
    if ( IS_EVIL(ch) )
        return number_percent() < 50 ? CABAL_INVADER : CABAL_CHAOS;
    if ( ch->iclass == CLASS_PALADIN || ( IS_GOOD(ch) && ch->ethos == 1 ) )
        return CABAL_KNIGHT;
    if ( ch->iclass == CLASS_RANGER || ch->iclass == CLASS_THIEF || ch->iclass == CLASS_NINJA )
        return IS_GOOD(ch) ? CABAL_LIONS : CABAL_HUNTER;
    if ( ch->iclass == CLASS_WARRIOR || ch->iclass == CLASS_SAMURAI )
        return number_percent() < 70 ? CABAL_BATTLE : CABAL_KNIGHT;
    return CABAL_HUNTER;
}

static void bot_announce( const char *text )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    snprintf( buf, sizeof(buf), "{C[Kabal] %s{x\n\r", text );
    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && d->character != NULL )
            send_to_char( buf, d->character );
}

static void bot_check_cabal( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    char buf[MAX_STRING_LENGTH];
    int cabal;

    bot->cabal_check_pulse = bot_pulse;
    if ( !bot->pk_istekli || ch->cabal != CABAL_NONE || ch->level < 18 )
        return;
    if ( ch->pcdata->oyuncu_katli == 0 )
        return;
    cabal = bot_choose_cabal( ch );
    if ( ch->iclass == CLASS_WARRIOR && cabal == CABAL_SHALAFI )
        cabal = CABAL_BATTLE;
    ch->cabal = cabal;
    snprintf( buf, sizeof(buf), "%s, %s kabalına kabul edildi!", ch->name, cabal_table[cabal].long_name );
    bot_announce( buf );
    bot_log( bot, "%s kabalına katıldı.", cabal_table[cabal].short_name );
    bot_chat_event( bot, BOT_EV_CABAL, NULL );
    save_char_obj( ch );
}

static bool pk_target_ok( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( victim == NULL || victim == ch || IS_NPC(victim) || victim->in_room == NULL )
        return FALSE;
    if ( victim->cabal == CABAL_NONE || victim->cabal == ch->cabal )
        return FALSE;
    if ( victim->pcdata->oyuncu_katli == 0 || IS_SET( victim->act, PLR_GHOST ) )
        return FALSE;
    if ( IS_IMMORTAL(victim) || victim->level < 10 )
        return FALSE;
    if ( is_safe_nomessage( ch, victim ) )
        return FALSE;
    if ( IS_SET( victim->in_room->area->area_flag, AREA_CABAL ) )
        return FALSE;
    if ( !IS_BOT(victim) && ( victim->desc == NULL || number_percent() < 60 ) )
        return FALSE;
    return TRUE;
}

static CHAR_DATA *bot_pick_pk_target( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch, *vch, *pick = NULL;
    int count = 0;

    for ( vch = char_list; vch != NULL; vch = vch->next )
    {
        if ( !pk_target_ok( ch, vch ) )
            continue;
        if ( number_range( 0, count++ ) == 0 )
            pick = vch;
    }
    return pick;
}

static void bot_pk( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *victim = bot_char_by_id( bot->pk_target_id );

    if ( victim == NULL || bot_pulse > bot->pk_until || !pk_target_ok( ch, victim )
      || pct( ch->hit, ch->max_hit ) < 60 )
    {
        bot->pk_target_id = 0;
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }
    if ( victim->in_room == ch->in_room )
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
    if ( bot->path_len == 0 || get_room_index( bot->target_vnum ) != victim->in_room )
    {
        if ( !bot_set_travel( bot, victim->in_room->vnum, BOT_ST_PK ) )
        {
            bot->pk_target_id = 0;
            bot_set_state( bot, BOT_ST_IDLE );
            return;
        }
    }
}

/* ---------------------------------------------------------------------
 * hedef seçimi
 * ------------------------------------------------------------------ */
static void bot_choose_goal( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;

    if ( bot_pulse - bot->cabal_check_pulse > 4 * 60 * 5 )
        bot_check_cabal( bot );

    bot_compute_town_tasks( bot );
    if ( bot_town_worth_it( bot ) )
    {
        bot_set_state( bot, BOT_ST_TOWN );
        return;
    }
    if ( IS_QUESTOR(ch) && ch->pcdata->questmob > 0 )
    {
        bot_set_state( bot, BOT_ST_QUEST );
        return;
    }
    if ( ch->cabal != CABAL_NONE && ch->pcdata->oyuncu_katli == 1 && bot_pulse > bot->next_pk
      && pct( ch->hit, ch->max_hit ) > 85 )
    {
        CHAR_DATA *victim = bot_pick_pk_target( bot );

        bot->next_pk = bot_pulse + number_range( 4 * 60 * 25, 4 * 60 * 70 );
        if ( victim != NULL && number_percent() < 60 )
        {
            bot->pk_target_id = victim->id;
            bot->pk_until = bot_pulse + 4 * 60 * 15;
            bot_set_state( bot, BOT_ST_PK );
            bot_log( bot, "kabal savaşı: hedef %s.", victim->name );
            return;
        }
    }
    bot_set_state( bot, BOT_ST_HUNT );
}

/* ---------------------------------------------------------------------
 * ana döngü
 * ------------------------------------------------------------------ */
void bot_think( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL || ch->in_room == NULL )
        return;

    /* seviye atlama */
    if ( ch->level > bot->last_level )
    {
        bot->last_level = ch->level;
        ch->wimpy = UMAX( ch->wimpy, ch->max_hit / 8 );
        memset( bot->wear_fail, 0, sizeof(bot->wear_fail) );
        bot_chat_event( bot, BOT_EV_LEVEL, NULL );
        if ( ch->level == 2 || ch->level == 5 || ch->level == 10 )
            bot->last_town_pulse = 0;
        bot->hunt_area_pulse = 0;                    /* bölgeyi yeniden değerlendir */
    }

    /* dövüş */
    if ( ch->fighting != NULL )
    {
        bot_combat( bot );
        return;
    }

    /* gevezelik */
    if ( bot_pulse >= bot->next_chat )
        bot_chat_idle( bot );
    if ( bot->ch == NULL || ch->in_room == NULL )
        return;
    if ( ch->position == POS_FIGHTING )
        ch->position = POS_STANDING;

    /* ölüm sonrası */
    if ( IS_SET( ch->act, PLR_GHOST ) )
    {
        if ( bot->state != BOT_ST_CORPSE && bot->state != BOT_ST_REST && bot->state != BOT_ST_TRAVEL )
            bot_after_death( bot );
        if ( bot->state == BOT_ST_REST && pct( ch->hit, ch->max_hit ) >= 60 && bot->death_room == 0 )
        {
            if ( ch->position < POS_STANDING )
                bot_cmd( bot, "kalk" );
            else
                bot_cmd( bot, "diril" );
            return;
        }
    }
    else if ( bot->state == BOT_ST_CORPSE && bot->death_room == 0 )
        bot_set_state( bot, BOT_ST_REST );

    /* çok yorgun / yaralı */
    if ( bot->state == BOT_ST_REST )
    {
        if ( bot_eat_drink( bot ) )
            return;
        bot_resting( bot );
        return;
    }
    if ( bot_need_rest( bot ) && bot->state != BOT_ST_FOLLOW && bot->state != BOT_ST_CORPSE )
    {
        bot_start_rest( bot );
        return;
    }

    /* pozisyon */
    if ( ch->position < POS_STANDING )
    {
        if ( IS_AFFECTED( ch, AFF_SLEEP ) )
            return;
        if ( ch->position >= POS_SLEEPING )
            bot_cmd( bot, "kalk" );
        return;
    }

    /* karın / matara */
    if ( bot->state != BOT_ST_TOWN && bot_eat_drink( bot ) )
        return;

    /* eşya değerlendirme (envanter değiştiyse) */
    {
        int sig = inventory_signature( ch );
        if ( sig != bot->inv_signature )
        {
            bot->inv_signature = sig;
            if ( bot_wear_upgrades( bot ) )
            {
                bot->inv_signature = -1;
                return;
            }
        }
    }

    switch ( bot->state )
    {
    case BOT_ST_TRAVEL: bot_travel_step( bot ); return;
    case BOT_ST_FOLLOW: bot_following( bot );   return;
    case BOT_ST_TOWN:   bot_town( bot );        return;
    case BOT_ST_QUEST:  bot_quest( bot );       return;
    case BOT_ST_CORPSE: bot_corpse( bot );      return;
    case BOT_ST_PK:     bot_pk( bot );          return;
    case BOT_ST_HUNT:   bot_hunt( bot );        return;
    case BOT_ST_LOGOUT:
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    default:
        bot_choose_goal( bot );
        return;
    }
}
