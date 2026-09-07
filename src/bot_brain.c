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
#include "db.h"
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
static bool bot_blind_in_dark( CHAR_DATA *ch );
static bool is_night( void );
static void bot_travel_step( BOT_DATA *bot );
static bool bot_prey_ok( CHAR_DATA *ch, CHAR_DATA *mob, int lo, int hi );
static int bot_prey_score( CHAR_DATA *ch, CHAR_DATA *mob, int dist );
static int level_bonus( CHAR_DATA *ch );
static void bot_mark_visited( BOT_DATA *bot, ROOM_INDEX_DATA *room );
static void bot_remember_spawn( BOT_DATA *bot, ROOM_INDEX_DATA *room );
static CHAR_DATA *prey_in_room( BOT_DATA *bot, ROOM_INDEX_DATA *room, int dist, int *best_score );
static ROOM_INDEX_DATA *bot_explore_target( BOT_DATA *bot );

/* aç ya da susuz: yenilenme durur, dinlenmek boşuna */
static bool is_starving( CHAR_DATA *ch )
{
    return ch->pcdata->condition[COND_HUNGER] < 0 || ch->pcdata->condition[COND_THIRST] < 0;
}

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
        return ( ch->level < 4 ? 1 : 2 ) + 1;
    if ( ch->level < 4 )
        return 0;
    if ( is_caster( ch ) )
        return ch->level < 12 ? 0 : 1;
    if ( is_rogue( ch ) )
        return ch->level < 8 ? 0 : 1;
    return ch->level < 8 ? 1 : 3;
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
        OBJ_DATA *left;

        worn = get_shield_char( ch );
        if ( worn != NULL )
            return bot_obj_score( ch, worn );
        /* iki elli silah kullanıyorsa kalkan tutamaz */
        if ( get_eq_char( ch, WEAR_BOTH ) != NULL )
            return 1000;
        /* gece elindeki ışık kalkandan değerli */
        if ( ( left = get_eq_char( ch, WEAR_LEFT ) ) != NULL && left->item_type == ITEM_LIGHT
          && ( weather_info.sunlight == SUN_DARK || weather_info.sunlight == SUN_SET ) )
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
        {
            if ( best->cost >= 60 || best->level >= 4 )
                bot_chat_event( bot, BOT_EV_LOOT, NULL );
        }
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
    if ( IS_SET( mob->act, ACT_AGGRESSIVE ) && mob->level > ch->level )
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
    if ( is_caster( ch ) && mob->max_hit > ch->max_hit * 2 + 20 )
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
    int score = 100 + ( mob->level - ch->level ) * 30 - dist * 12;

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

/* ---------------------------------------------------------------------
 * algı ve hafıza: bot dünya listesine bakmaz; odasını, tara menzilini ve
 * kendi hatırladıklarını kullanır (oyuncu gibi).
 * ------------------------------------------------------------------ */
static void bot_mark_visited( BOT_DATA *bot, ROOM_INDEX_DATA *room )
{
    if ( room == NULL || room->vnum < 0 )
        return;
    if ( bot->visited == NULL )
        bot->visited = (unsigned char *) calloc( 4096, 1 );
    if ( room->area != bot->visited_area )
    {
        memset( bot->visited, 0, 4096 );
        bot->visited_area = room->area;
    }
    bot->visited[room->vnum >> 3] |= (unsigned char) ( 1 << ( room->vnum & 7 ) );
}

static bool bot_was_visited( BOT_DATA *bot, ROOM_INDEX_DATA *room )
{
    if ( bot->visited == NULL || room == NULL || room->vnum < 0 || room->area != bot->visited_area )
        return FALSE;
    return ( bot->visited[room->vnum >> 3] & ( 1 << ( room->vnum & 7 ) ) ) != 0;
}

static void bot_forget_visits( BOT_DATA *bot )
{
    if ( bot->visited != NULL )
        memset( bot->visited, 0, 4096 );
}

/* av görülen odayı hatırla */
static void bot_remember_spawn( BOT_DATA *bot, ROOM_INDEX_DATA *room )
{
    int i;

    if ( room == NULL )
        return;
    for ( i = 0; i < BOT_SPAWN_MAX; i++ )
        if ( bot->spawn_vnum[i] == room->vnum )
        {
            bot->spawn_seen[i] = bot_pulse;
            return;
        }
    bot->spawn_vnum[bot->spawn_pos]  = room->vnum;
    bot->spawn_seen[bot->spawn_pos]  = bot_pulse;
    bot->spawn_visit[bot->spawn_pos] = 0;
    bot->spawn_pos = ( bot->spawn_pos + 1 ) % BOT_SPAWN_MAX;
}

static void bot_note_spawn_visit( BOT_DATA *bot, ROOM_INDEX_DATA *room )
{
    int i;

    for ( i = 0; i < BOT_SPAWN_MAX; i++ )
        if ( bot->spawn_vnum[i] == room->vnum )
            bot->spawn_visit[i] = bot_pulse;
}

static struct bot_area_mem *bot_area_memory( BOT_DATA *bot, AREA_DATA *area, bool create )
{
    int i, oldest = 0;

    for ( i = 0; i < BOT_AREA_MEM; i++ )
        if ( bot->area_mem[i].area == area )
            return &bot->area_mem[i];
    if ( !create )
        return NULL;
    for ( i = 0; i < BOT_AREA_MEM; i++ )
    {
        if ( bot->area_mem[i].area == NULL )
        {
            oldest = i;
            break;
        }
        if ( bot->area_mem[i].last_pulse < bot->area_mem[oldest].last_pulse )
            oldest = i;
    }
    memset( &bot->area_mem[oldest], 0, sizeof(bot->area_mem[oldest]) );
    bot->area_mem[oldest].area = area;
    bot->area_mem[oldest].last_pulse = bot_pulse;
    return &bot->area_mem[oldest];
}

void bot_note_kill( BOT_DATA *bot, CHAR_DATA *victim )
{
    struct bot_area_mem *mem;

    if ( bot->ch == NULL || bot->ch->in_room == NULL )
        return;
    mem = bot_area_memory( bot, bot->ch->in_room->area, TRUE );
    mem->kills++;
    mem->last_pulse = bot_pulse;
    bot_remember_spawn( bot, bot->ch->in_room );
}

/* bir odada uygun av var mı (bakış açısı: bot'un kendi odası) */
static CHAR_DATA *prey_in_room( BOT_DATA *bot, ROOM_INDEX_DATA *room, int dist, int *best_score )
{
    CHAR_DATA *ch = bot->ch, *rch, *best = NULL;
    int lo = UMAX( 1, ch->level - ( ch->level >= 6 ? 1 : 3 ) - UMIN( bot->hunt_fail, 2 ) );
    int hi = UMAX( 2, ch->level + level_bonus( ch ) );

    for ( rch = room->people; rch != NULL; rch = rch->next_in_room )
    {
        int s;

        if ( !bot_prey_ok( ch, rch, lo, hi ) )
            continue;
        s = bot_prey_score( ch, rch, dist );
        if ( best == NULL || s > *best_score )
        {
            *best_score = s;
            best = rch;
        }
    }
    return best;
}

/*
 * 'tara' gibi: altı yönde düz hat boyunca, kapalı kapıda durarak,
 * 1 + seviye/10 oda uzağa bakar. En iyi av odasını döndürür.
 */
static ROOM_INDEX_DATA *bot_scan_prey( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    int range = 1 + ch->level / 10;
    int d, i, best_score = -1000;
    ROOM_INDEX_DATA *best = NULL;

    if ( bot_blind_in_dark( ch ) )
        return NULL;
    for ( d = 0; d < 6; d++ )
    {
        ROOM_INDEX_DATA *room = ch->in_room, *prev;

        for ( i = 1; i <= range; i++ )
        {
            EXIT_DATA *pexit = room->exit[d];
            int score = -1000;

            prev = room;
            if ( pexit == NULL || pexit->u1.to_room == NULL || IS_SET( pexit->exit_info, EX_CLOSED ) )
                break;
            room = pexit->u1.to_room;
            if ( prey_in_room( bot, room, i, &score ) != NULL )
            {
                bot_remember_spawn( bot, room );
                if ( score > best_score && bot_room_passable( ch, room, FALSE ) && bot_exit_back( room, prev ) )
                {
                    best_score = score;
                    best = room;
                }
            }
        }
    }
    return best;
}

/* hafızadaki, bu bölgede, bir süredir uğranmamış en yakın av odası */
static ROOM_INDEX_DATA *bot_recall_spawn( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    ROOM_INDEX_DATA *best = NULL;
    int i, best_len = 100000;
    sh_int tmp[BOT_MAX_PATH];

    for ( i = 0; i < BOT_SPAWN_MAX; i++ )
    {
        ROOM_INDEX_DATA *room;
        int len;

        if ( bot->spawn_vnum[i] <= 0 || bot_pulse - bot->spawn_seen[i] > 4 * 60 * 40 )
            continue;
        if ( bot_pulse - bot->spawn_visit[i] < 4 * 60 * 2 )
            continue;
        if ( ( room = get_room_index( bot->spawn_vnum[i] ) ) == NULL || room == ch->in_room )
            continue;
        if ( room->area != ch->in_room->area )
            continue;
        len = bot_find_path( ch, ch->in_room, room, tmp, 60, FALSE );
        if ( len >= 0 && len < best_len )
        {
            best_len = len;
            best = room;
        }
    }
    return best;
}

/* bölgede henüz uğranmamış en yakın oda (harita bilgisi) */
static ROOM_INDEX_DATA *bot_explore_target( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    int n = bot_near_rooms( ch, 30, TRUE ), i;

    for ( i = 1; i < n; i++ )
        if ( !bot_was_visited( bot, near_room[i] ) )
            return near_room[i];
    return NULL;
}

/* ---------------------------------------------------------------------
 * av bölgesi seçimi: 'bölge' listesindeki seviye aralıkları (kamusal
 * bilgi), botun kendi deneyimi (kesim/ölüm) ve yol uzunluğu.
 * ------------------------------------------------------------------ */
/* bölgenin bota en yakın odası (harita bilgisi) */
ROOM_INDEX_DATA *area_entry_room( BOT_DATA *bot, AREA_DATA *area )
{
    CHAR_DATA *ch = bot->ch;
    ROOM_INDEX_DATA *room, *best = NULL;
    int i, iHash, best_len = 100000, tries = 0;
    sh_int tmp[BOT_MAX_PATH];

    /* hatırlanan av odası */
    for ( i = 0; i < BOT_SPAWN_MAX; i++ )
        if ( bot->spawn_vnum[i] > 0 && ( room = get_room_index( bot->spawn_vnum[i] ) ) != NULL
          && room->area == area && bot_pulse - bot->spawn_seen[i] < 4 * 60 * 60 )
        {
            int len = bot_find_path( ch, ch->in_room, room, tmp, BOT_MAX_PATH, FALSE );
            if ( len >= 0 && len < best_len )
            {
                best_len = len;
                best = room;
            }
        }
    if ( best != NULL )
        return best;

    /* bölgenin geçilebilir bir odası; birkaç aday arasından en yakını */
    for ( iHash = 0; iHash < MAX_KEY_HASH && tries < 6; iHash++ )
        for ( room = room_index_hash[iHash]; room != NULL && tries < 6; room = room->next )
        {
            int len;

            if ( room->area != area || !bot_room_passable( ch, room, FALSE ) || !room_has_exit( room ) )
                continue;
            if ( number_percent() > 20 )
                continue;
            tries++;
            len = bot_find_path( ch, ch->in_room, room, tmp, BOT_MAX_PATH, FALSE );
            if ( len >= 0 && len < best_len )
            {
                best_len = len;
                best = room;
            }
        }
    return best;
}

static AREA_DATA *bot_pick_hunt_area( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    AREA_DATA *area, *best = NULL;
    int best_score = -1000;

    for ( area = area_first; area != NULL; area = area->next )
    {
        int score, mid;
        struct bot_area_mem *mem;

        if ( IS_SET( area->area_flag, AREA_CABAL | AREA_HOMETOWN ) )
            continue;
        if ( area->low_range > ch->level || area->high_range < ch->level )
            continue;
        if ( area->high_range - area->low_range > 60 || area->min_vnum < 100 )
            continue;
        /* deneyimsiz oyuncu bilgisi: seviyesinin çok üstüne çıkan derin bölgelere girme */
        if ( ch->level < 12 && area->high_range > ch->level + 12 && area->high_range - area->low_range > 15 )
            continue;

        mid = area->low_range + ( area->high_range - area->low_range ) / 3;
        score = 60 - abs( ch->level - mid ) * 3;
        if ( ( mem = bot_area_memory( bot, area, FALSE ) ) != NULL )
        {
            score += UMIN( mem->kills, 30 ) * 2 - mem->deaths * 30;
            if ( mem->deaths >= 2 && bot_pulse - mem->last_pulse < 4 * 60 * 60 )
                continue;
        }
        if ( area == bot->hunt_area )
            score += 15;
        if ( ch->in_room != NULL && ch->in_room->area == area )
            score += 25;
        score += number_range( 0, 25 );
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
    int shown = 0, i;

    if ( ch == NULL )
    {
        send_to_char( "Bot çevrimdışı.\n\r", viewer );
        return;
    }
    printf_to_char( viewer, "Seviye %d, oda %d (%s), av bölgesi %s, başarısızlık %d\n\r",
                    ch->level, ch->in_room->vnum, ch->in_room->area->name,
                    bot->hunt_area != NULL ? bot->hunt_area->name : "-", bot->hunt_fail );
    for ( area = area_first; area != NULL; area = area->next )
    {
        struct bot_area_mem *mem;

        if ( area->low_range > ch->level || area->high_range < ch->level )
            continue;
        mem = bot_area_memory( bot, area, FALSE );
        printf_to_char( viewer, "%-28.28s %3d-%-3d kesim %3d ölüm %2d\n\r",
                        area->name, area->low_range, area->high_range,
                        mem != NULL ? mem->kills : 0, mem != NULL ? mem->deaths : 0 );
        if ( ++shown > 40 )
            break;
    }
    send_to_char( "Hatırlanan av odaları:", viewer );
    for ( i = 0; i < BOT_SPAWN_MAX; i++ )
        if ( bot->spawn_vnum[i] > 0 )
            printf_to_char( viewer, " %d", bot->spawn_vnum[i] );
    send_to_char( "\n\r", viewer );
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

    if ( ch->move < ( is_starving( ch ) ? 2 : 6 ) )
    {
        if ( ch->position != POS_SLEEPING && !room_has_aggressor( ch ) )
            bot_cmd( bot, "uyu" );
        else if ( ch->position != POS_RESTING && ch->position != POS_SLEEPING )
            bot_cmd( bot, "dinlen" );
        return;
    }
    if ( ch->position >= POS_SLEEPING && ch->position < POS_STANDING )
    {
        if ( ch->move < ch->max_move * 3 / 10 && !is_starving( ch ) )
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
          || ( len = bot_find_path( ch, from, to, bot->path, BOT_MAX_PATH, bot->after_travel == BOT_ST_PK || bot->after_travel == BOT_ST_RAID ) ) < 0 )
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

    if ( bot->after_travel == BOT_ST_RAID && pexit->u1.to_room != NULL && bot_raid_scout( bot, pexit->u1.to_room ) )
        return;
    bot_cmd( bot, "%s", dir_name[dir] );
    if ( bot->ch == NULL )
        return;
    if ( ch->in_room == pexit->u1.to_room )
    {
        int sc = -1000;

        bot->path_pos++;
        bot->stuck = 0;
        bot->last_room_vnum = ch->in_room->vnum;
        bot_mark_visited( bot, ch->in_room );
        if ( prey_in_room( bot, ch->in_room, 0, &sc ) != NULL )
            bot_remember_spawn( bot, ch->in_room );
        bot_war_note_room( bot );
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

    if ( is_starving( ch ) )
        return pct( ch->hit, ch->max_hit ) < 15;
    if ( pct( ch->hit, ch->max_hit ) < 45 )
        return TRUE;
    if ( is_caster( ch ) && pct( ch->mana, ch->max_mana ) < 25 )
        return TRUE;
    if ( ch->move < 8 )
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
    if ( pct( ch->move, ch->max_move ) < 40 )
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

    /* açlıktan yenilenme durmuş: yatmak boşuna, yiyecek bulmaya git */
    if ( is_starving( ch ) && pct( ch->hit, ch->max_hit ) >= 15 )
    {
        if ( ch->position < POS_STANDING && !IS_AFFECTED( ch, AFF_SLEEP ) )
        {
            bot_cmd( bot, "kalk" );
            return;
        }
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }

    /* gece ışıksız karanlıkta: sabahı bekle (14 dk sınırı da uygulanır) */
    if ( bot_blind_in_dark( ch ) && is_night() && ch->silver < 20
      && bot_pulse - bot->state_pulse < 4 * 60 * 14 )
    {
        /* karanlıkta bekle: yorgunsa uyu, değilse dinlen (her ikisi de yenilenme sağlar) */
        if ( ch->position > POS_SLEEPING && !IS_AFFECTED( ch, AFF_SLEEP ) )
        {
            if ( !bot_rested_enough( bot ) )
                bot_cmd( bot, "uyu" );
            else if ( ch->position != POS_RESTING )
                bot_cmd( bot, "dinlen" );
        }
        return;
    }
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
        if ( ( obj = carried_type( ch, ITEM_FOOD ) ) != NULL && can_see_obj( ch, obj ) )
        {
            bot_obj_keyword( ch, obj, ch->carrying, kw, sizeof(kw) );
            bot_cmd( bot, "ye %s", kw );
            return TRUE;
        }
        if ( obj != NULL && bot->state != BOT_ST_TRAVEL && bot->state != BOT_ST_FOLLOW && ch->position == POS_STANDING )
        {
            /* yiyeceği göremiyor (karanlık): aydınlık bir yere, tapınağa git */
            ROOM_INDEX_DATA *temple = get_room_index( ROOM_VNUM_TEMPLE );
            if ( temple != NULL && temple != ch->in_room && bot_set_travel( bot, temple->vnum, BOT_ST_IDLE ) )
                return TRUE;
        }
        if ( obj == NULL )
            SET_BIT( bot->town_tasks, BOT_TOWN_FOOD );
    }
    if ( ch->pcdata->condition[COND_THIRST] < 14 )
    {
        if ( ( obj = carried_drink( ch ) ) != NULL && can_see_obj( ch, obj ) )
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
    /* çeşme başındaysa matarayı doldur (başarısız olursa bir süre deneme: sonsuz döngü yok) */
    if ( room_fountain( ch ) != NULL && bot_pulse >= bot->fill_block_until )
    {
        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
            if ( obj->item_type == ITEM_DRINK_CON && obj->value[1] < obj->value[0] / 2 && can_see_obj( ch, obj ) )
            {
                if ( bot->fill_vnum == obj->pIndexData->vnum && bot->fill_amount == obj->value[1] )
                {
                    /* son deneme bir şey değiştirmedi */
                    bot->fill_block_until = bot_pulse + 4 * 60 * 15;
                    bot->fill_vnum = 0;
                    return FALSE;
                }
                bot->fill_vnum   = obj->pIndexData->vnum;
                bot->fill_amount = obj->value[1];
                bot_obj_keyword( ch, obj, ch->carrying, kw, sizeof(kw) );
                bot_cmd( bot, "doldur %s", kw );
                return TRUE;
            }
    }
    return FALSE;
}

/* ---------------------------------------------------------------------
 * ışık ve karanlık
 * ------------------------------------------------------------------ */
static bool is_night( void )
{
    return weather_info.sunlight == SUN_DARK || weather_info.sunlight == SUN_SET;
}

/* gece ışık tut (gerekirse kalkanı çıkar); komut ürettiyse TRUE */
static bool bot_manage_light( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    OBJ_DATA *light, *left;
    char kw[MAX_INPUT_LENGTH];

    if ( get_light_char( ch ) != NULL || !is_night() || room_is_dark( ch ) )
        return FALSE;
    if ( ( light = carried_type( ch, ITEM_LIGHT ) ) == NULL || light->value[2] == 0 || !can_see_obj( ch, light ) )
        return FALSE;
    if ( bot_wear_failed( bot, light->pIndexData->vnum ) )
        return FALSE;
    if ( get_eq_char( ch, WEAR_BOTH ) != NULL )
        return FALSE;
    if ( ( left = get_eq_char( ch, WEAR_LEFT ) ) != NULL )
    {
        if ( left->item_type == ITEM_LIGHT )
            return FALSE;
        bot_obj_keyword( ch, left, ch->carrying, kw, sizeof(kw) );
        bot_cmd( bot, "çıkar %s", kw );
        return TRUE;
    }
    bot_obj_keyword( ch, light, ch->carrying, kw, sizeof(kw) );
    bot_cmd( bot, "giy %s", kw );
    if ( bot->ch != NULL && light->wear_loc == WEAR_NONE )
        bot_wear_fail_add( bot, light->pIndexData->vnum );
    return TRUE;
}

/* karanlıkta ışıksız kalan bot: ışık al, yoksa tapınakta sabahı bekle; komut/durum ürettiyse TRUE */
static bool bot_blind_in_dark( CHAR_DATA *ch )
{
    return room_is_dark( ch ) && get_light_char( ch ) == NULL && !IS_AFFECTED( ch, AFF_INFRARED );
}

static bool bot_handle_darkness( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    ROOM_INDEX_DATA *temple;

    if ( !bot_blind_in_dark( ch ) )
        return FALSE;
    if ( bot->state == BOT_ST_TRAVEL || bot->state == BOT_ST_TOWN || bot->state == BOT_ST_FOLLOW
      || bot->state == BOT_ST_CORPSE || bot->state == BOT_ST_REST )
        return FALSE;
    if ( ch->silver >= 20 && bot_pulse >= bot->town_retry[6] )      /* BOT_TOWN_LIGHT = bit 6 */
    {
        SET_BIT( bot->town_tasks, BOT_TOWN_LIGHT );
        bot_set_state( bot, BOT_ST_TOWN );
        return TRUE;
    }
    if ( is_night() && ch->level <= 10 )
    {
        /* parasız: aydınlık okul arenasında ufak avdan para topla */
        ROOM_INDEX_DATA *school = get_room_index( ROOM_VNUM_SCHOOL );

        if ( school != NULL && school->area != NULL )
        {
            bot->hunt_area = school->area;
            bot->hunt_area_pulse = bot_pulse;
            bot->hunt_fail = 2;
            if ( ch->in_room->area == school->area )
                bot_set_state( bot, BOT_ST_HUNT );
            else if ( !bot_set_travel( bot, school->vnum, BOT_ST_HUNT ) )
                bot_set_state( bot, BOT_ST_REST );
            return TRUE;
        }
    }
    temple = get_room_index( ROOM_VNUM_TEMPLE );
    if ( temple != NULL && temple != ch->in_room
      && bot_set_travel( bot, temple->vnum, is_night() ? BOT_ST_REST : BOT_ST_IDLE ) )
        return TRUE;
    if ( is_night() )
        bot_set_state( bot, BOT_ST_REST );
    else
    {
        /* gündüz karanlık odada (bayraklı): burada av olmaz, bölge değiştir */
        bot->hunt_area = NULL;
        bot->hunt_fail += 2;
        bot_set_state( bot, BOT_ST_IDLE );
    }
    return TRUE;
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

    if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
        bot_war_attacked( bot, ch->fighting );
    CHAR_DATA *victim = ch->fighting;
    int hp = pct( ch->hit, ch->max_hit );
    int vhp = pct( victim->hit, victim->max_hit );
    int roll;

    bot->last_fight_pulse = bot_pulse;
    snprintf( bot->last_opp, sizeof(bot->last_opp), "%s (lvl %d)",
              IS_NPC(victim) ? victim->short_descr : victim->name, victim->level );
    bot->last_opp_vnum = IS_NPC(victim) ? victim->pIndexData->vnum : 0;

    if ( ch->position < POS_FIGHTING )
    {
        if ( ch->position >= POS_RESTING )
            bot_cmd( bot, "kalk" );
        return;
    }

    if ( hp < ( is_caster( ch ) ? 30 : 22 ) && vhp > 25 )
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
bool bot_cast_buffs( BOT_DATA *bot )
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

void bot_attack( BOT_DATA *bot, CHAR_DATA *victim )
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
bool bot_escape_pocket( BOT_DATA *bot )
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
    /* kıdemli: anımsa yok; oyuncu gibi yardım ister, sonra elinden geleni yapmayı sürdürür */
    if ( bot->nopath_pulse == bot_pulse )
    {
        bot_log( bot, "oda %d'den çıkış yolu yok; yardım istiyor.", ch->in_room->vnum );
        bot_cmd( bot, "dua Sıkıştım, %s'den çıkış bulamıyorum.", ch->in_room->name );
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
            /* 'nerede' gibi: yalnızca aynı bölgedekileri görebilir */
            if ( och->in_room->area != ch->in_room->area || !can_see( ch, och )
              || bot_find_path( ch, ch->in_room, och->in_room, tmp, 40, FALSE ) < 0 )
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
    int score = -1000;

    bot_mark_visited( bot, ch->in_room );
    bot_note_spawn_visit( bot, ch->in_room );

    if ( bot->hunt_area == NULL || bot_pulse - bot->hunt_area_pulse > 4 * 60 * 30
      || ( ch->in_room->area != bot->hunt_area && bot->hunt_fail > 1 ) )
    {
        AREA_DATA *area = bot_pick_hunt_area( bot );

        if ( area == NULL )
        {
            if ( bot_escape_pocket( bot ) )
                return;
            bot->hunt_fail++;
            bot_set_state( bot, BOT_ST_REST );
            return;
        }
        if ( area != bot->hunt_area )
        {
            bot->hunt_fail = 0;
            bot_forget_visits( bot );
        }
        bot->hunt_area = area;
        bot->hunt_area_pulse = bot_pulse;
    }

    if ( ch->in_room->area != bot->hunt_area )
    {
        ROOM_INDEX_DATA *entry = area_entry_room( bot, bot->hunt_area );

        if ( entry == NULL || !bot_set_travel( bot, entry->vnum, BOT_ST_HUNT ) )
        {
            if ( bot_escape_pocket( bot ) )
                return;
            bot->hunt_fail += 2;
            bot->hunt_area = NULL;
            bot_set_state( bot, BOT_ST_IDLE );
        }
        return;
    }

    if ( bot_group_followers( bot ) )
        return;
    if ( bot_war_opportunity( bot ) )
        return;
    if ( bot_cast_buffs( bot ) )
        return;

    if ( ( prey = prey_in_room( bot, ch->in_room, 0, &score ) ) != NULL )
    {
        bot_remember_spawn( bot, ch->in_room );
        bot_attack( bot, prey );
        bot->hunt_fail = 0;
        return;
    }
    if ( bot_consider_grouping( bot ) )
        return;
    if ( bot_seek_group( bot ) )
        return;

    /* tara menzilinde av */
    if ( ( room = bot_scan_prey( bot ) ) != NULL && bot_set_travel( bot, room->vnum, BOT_ST_HUNT ) )
        return;
    /* hatırlanan av odaları */
    if ( ( room = bot_recall_spawn( bot ) ) != NULL && bot_set_travel( bot, room->vnum, BOT_ST_HUNT ) )
        return;
    /* keşfet */
    if ( ( room = bot_explore_target( bot ) ) != NULL && bot_set_travel( bot, room->vnum, BOT_ST_HUNT ) )
        return;

    /* bölge gezildi ve av yok: biraz bekle, sonra yeniden keşfet ya da bölge değiştir */
    if ( ++bot->hunt_fail > 3 )
    {
        struct bot_area_mem *mem = bot_area_memory( bot, bot->hunt_area, TRUE );
        mem->last_pulse = bot_pulse;
        bot->hunt_area = NULL;
        bot->hunt_fail = 0;
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }
    bot_forget_visits( bot );
    bot->rest_reason = 1;
    bot_set_state( bot, BOT_ST_REST );
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

    if ( ( ch->practice >= 6 || ( ch->practice >= 3 && ch->level <= 3 ) ) && bot_pulse > bot->practice_block_until )
        SET_BIT( tasks, BOT_TOWN_PRACTICE );
    if ( ch->train >= 2 && bot_pulse > bot->practice_block_until )
        SET_BIT( tasks, BOT_TOWN_TRAIN );
    if ( bot_sell_candidates( bot ) >= 4 || ch->carry_number >= can_carry_n( ch ) - 2
      || ( ch->silver < 60 && bot_sell_candidates( bot ) >= 2 ) )
        SET_BIT( tasks, BOT_TOWN_SELL );
    if ( carried_type( ch, ITEM_FOOD ) == NULL && ch->silver >= 5 )
        SET_BIT( tasks, BOT_TOWN_FOOD );
    if ( carried_drink( ch ) == NULL && ch->silver >= 40 )
        SET_BIT( tasks, BOT_TOWN_DRINK );
    if ( ch->pcdata->condition[COND_THIRST] < 25 && carried_drink( ch ) == NULL )
        SET_BIT( tasks, BOT_TOWN_FOUNTAIN );
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
    if ( IS_SET( t, BOT_TOWN_FOOD ) && bot->ch->pcdata->condition[COND_HUNGER] < 25 )
        return TRUE;
    if ( IS_SET( t, BOT_TOWN_DRINK ) && bot->ch->pcdata->condition[COND_THIRST] < 25 )
        return TRUE;
    if ( IS_SET( t, BOT_TOWN_PRACTICE ) && ( bot->ch->practice >= 6 || bot->ch->level <= 3 ) )
        return TRUE;
    if ( IS_SET( t, BOT_TOWN_SELL ) && ( bot_sell_candidates( bot ) >= 5 || bot->ch->silver < 60 ) )
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
        BOT_TOWN_FOOD, BOT_TOWN_FOUNTAIN, BOT_TOWN_DRINK, BOT_TOWN_QUEST_DONE, BOT_TOWN_LIGHT,
        BOT_TOWN_SELL, BOT_TOWN_PRACTICE, BOT_TOWN_TRAIN, BOT_TOWN_POTION, BOT_TOWN_UPGRADE,
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
        if ( task == BOT_TOWN_FOOD )
        {
            OBJ_DATA *fo;
            int nfood = 0;
            for ( fo = ch->carrying; fo != NULL; fo = fo->next_content )
                if ( fo->item_type == ITEM_FOOD )
                    nfood++;
            have = nfood >= 3 ? 1 : 0;
        }
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

    /* görev yaratığı nerede? Görevci odayı söyledi; oradan sonra 'nerede' gibi
       yalnızca aynı bölgede bakılır. */
    for ( mob = char_list; mob != NULL; mob = mob->next )
        if ( IS_NPC(mob) && mob->pIndexData->vnum == ch->pcdata->questmob && mob->in_room != NULL
          && mob->in_room->area == ch->in_room->area && can_see( ch, mob ) )
            break;

    if ( mob == NULL )
    {
        ROOM_INDEX_DATA *qroom = get_room_index( ch->pcdata->questroom );

        if ( qroom != NULL && qroom != ch->in_room && bot->quest_tries < 3 )
        {
            bot->quest_tries++;
            if ( !bot_set_travel( bot, qroom->vnum, BOT_ST_QUEST ) )
                bot_set_state( bot, BOT_ST_IDLE );
            return;
        }
        /* odada değil: bölgeyi biraz dolaşarak ara */
        if ( ++bot->quest_tries > 14 )
        {
            bot_set_state( bot, BOT_ST_IDLE );
            bot->quest_tries = 0;
            return;
        }
        bot_mark_visited( bot, ch->in_room );
        if ( ( qroom = bot_explore_target( bot ) ) != NULL && bot_set_travel( bot, qroom->vnum, BOT_ST_QUEST ) )
            return;
        bot_set_state( bot, BOT_ST_IDLE );
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
    /* öldüğüm bölgeyi hatırla */
    {
        ROOM_INDEX_DATA *droom = get_room_index( bot->death_room );
        if ( droom != NULL )
        {
            struct bot_area_mem *mem = bot_area_memory( bot, droom->area, TRUE );
            mem->deaths++;
            mem->last_pulse = bot_pulse;
        }
    }
    /* beni öldüren yaratık türünden bir süre uzak dur */
    if ( bot->last_opp_vnum > 0 )
    {
        bot->avoid_vnum[bot->avoid_pos]  = bot->last_opp_vnum;
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
    if ( bot->state == BOT_ST_FOLLOW || bot->state == BOT_ST_CORPSE || bot->state == BOT_ST_PK || bot->state == BOT_ST_RAID )
        return FALSE;
    if ( bot->state == BOT_ST_MEET && bot->meet_id != other->id )
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

/* uzaktaki biri grup istedi: bulunduğu yerde bekle, gelirse takip et */
void bot_offer_meeting( BOT_DATA *bot, CHAR_DATA *other )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL || other == NULL || ch->fighting != NULL )
        return;
    if ( bot->state == BOT_ST_FOLLOW || bot->state == BOT_ST_CORPSE || bot->state == BOT_ST_PK
      || bot->state == BOT_ST_MEET )
        return;
    bot->meet_id = other->id;
    bot->meet_until = bot_pulse + 4 * 60 * 6;
    bot->path_len = bot->path_pos = 0;
    bot_set_state( bot, BOT_ST_MEET );
}

static void bot_meeting( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *other = bot_char_by_id( bot->meet_id );

    if ( other == NULL || other->in_room == NULL || bot_pulse > bot->meet_until
      || ( !IS_BOT(other) && ( other->desc == NULL || other->desc->connected != CON_PLAYING ) ) )
    {
        bot->meet_id = 0;
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    }
    if ( other->in_room == ch->in_room && can_see( ch, other ) )
    {
        bot->meet_id = 0;
        bot_talk( bot, BOT_CH_SAY, other, "İşte geldin. Ardındayım." );
        bot_start_follow( bot, other );
        return;
    }
    /* beklerken odada av varsa kes */
    {
        int score = -1000;
        CHAR_DATA *prey = prey_in_room( bot, ch->in_room, 0, &score );
        if ( prey != NULL )
            bot_attack( bot, prey );
        else if ( ch->position == POS_STANDING && ch->hit < ch->max_hit && number_percent() < 30 )
            bot_cmd( bot, "dinlen" );
    }
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
/* ---------------------------------------------------------------------
 * hedef seçimi
 * ------------------------------------------------------------------ */
static void bot_choose_goal( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;

    if ( bot_war_goal( bot ) )
        return;

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

    /* kabal yaşamı: liderlik, üyelik, bekleyen induct */
    bot_war_tick( bot );
    if ( bot->ch == NULL || ch->in_room == NULL )
        return;

    /* biriyle konuşuyor: cevabı verene dek odada kal */
    if ( bot_pulse < bot->hold_until && !IS_SET( ch->act, PLR_GHOST ) )
        return;

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
        if ( bot->state == BOT_ST_TRAVEL && ch->position >= POS_SLEEPING )
        {
            bot_travel_step( bot );          /* hareket puanı için uyuyorsa o karar verir */
            return;
        }
        if ( ch->position >= POS_SLEEPING )
            bot_cmd( bot, "kalk" );
        return;
    }

    /* ışık */
    if ( bot_manage_light( bot ) )
        return;
    if ( bot_handle_darkness( bot ) )
        return;

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
    case BOT_ST_MEET:   bot_meeting( bot );     return;
    case BOT_ST_RAID:   bot_raid( bot );        return;
    case BOT_ST_HUNT:   bot_hunt( bot );        return;
    case BOT_ST_LOGOUT:
        bot_set_state( bot, BOT_ST_IDLE );
        return;
    default:
        bot_choose_goal( bot );
        return;
    }
}
