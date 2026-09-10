/*
 * bot.c - Otonom oyuncu botları: kadro, giriş/çıkış, zamanlama, yol bulma,
 *         oyun kancaları ve ölümsüz "botlar" komutu.
 *
 * Bir bot, soketi olmayan sıradan bir oyuncu karakteridir (bkz. bot.h).
 * Karar mekanizması bot_brain.c, konuşma bot_chat.c içindedir.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "password.h"
#include "bot.h"

DECLARE_DO_FUN( do_outfit );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_say );

extern AREA_DATA *      area_first;
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern int              max_on;
extern int              max_on_so_far;
bool    check_parse_name( char *name );
bool    cabal_area_check( CHAR_DATA *ch );

BOT_DATA *  bot_list        = NULL;
bool        bots_enabled    = FALSE;
int         bot_pulse       = 0;
int         bot_min_online  = 6;
int         bot_max_online  = 14;
bool        bot_debug       = FALSE;

static int  boot_pulse_scheduled = 0;

/* insan oyuncu giriş takibi (selamlama için) */
#define BOT_SEEN_MAX 64
static long seen_humans[BOT_SEEN_MAX];
static int  seen_count = 0;

/* ---------------------------------------------------------------------
 * küçük yardımcılar
 * ------------------------------------------------------------------ */
void bot_log( BOT_DATA *bot, const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH];
    char out[MAX_STRING_LENGTH + 64];
    va_list args;

    va_start( args, fmt );
    vsnprintf( buf, sizeof(buf), fmt, args );
    va_end( args );
    snprintf( out, sizeof(out), "[bot] %s: %s", bot != NULL ? bot->name : "-", buf );
    log_string( out );
}

BOT_DATA *bot_of( CHAR_DATA *ch )
{
    if ( ch == NULL || IS_NPC(ch) || ch->pcdata == NULL )
        return NULL;
    return ch->pcdata->bot;
}

BOT_DATA *bot_find( const char *name )
{
    BOT_DATA *bot;

    for ( bot = bot_list; bot != NULL; bot = bot->next )
        if ( !str_cmp( bot->name, name ) )
            return bot;
    return NULL;
}

int bot_hour( void )
{
    time_t now = current_time;
    struct tm *lt = localtime( &now );
    return lt != NULL ? lt->tm_hour : 12;
}

static bool bot_in_hours( BOT_DATA *bot )
{
    int h = bot_hour();

    if ( bot->hour_from == bot->hour_to )
        return TRUE;
    if ( bot->hour_from < bot->hour_to )
        return h >= bot->hour_from && h < bot->hour_to;
    return h >= bot->hour_from || h < bot->hour_to;       /* gece yarısını aşan aralık */
}

const char *bot_state_name( int state )
{
    switch ( state )
    {
    case BOT_ST_IDLE:   return "boşta";
    case BOT_ST_TRAVEL: return "yolda";
    case BOT_ST_HUNT:   return "avda";
    case BOT_ST_REST:   return "dinleniyor";
    case BOT_ST_TOWN:   return "şehirde";
    case BOT_ST_QUEST:  return "görevde";
    case BOT_ST_CORPSE: return "ceset peşinde";
    case BOT_ST_FOLLOW: return "takipte";
    case BOT_ST_PK:     return "kabal savaşında";
    case BOT_ST_MEET:   return "buluşmada";
    case BOT_ST_RAID:   return "baskında";
    case BOT_ST_LOGOUT: return "çıkıyor";
    }
    return "?";
}

void bot_set_state( BOT_DATA *bot, int state )
{
    if ( bot->state != state )
    {
        if ( bot_debug )
            bot_log( bot, "durum %s -> %s", bot_state_name( bot->state ), bot_state_name( state ) );
        bot->state       = state;
        bot->substate    = 0;
        bot->state_pulse = bot_pulse;
    }
}

bool bot_is_human( CHAR_DATA *ch )
{
    return ch != NULL && !IS_NPC(ch) && ch->desc != NULL
        && ch->desc->connected == CON_PLAYING && !IS_BOT(ch);
}

CHAR_DATA *bot_random_human( void )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *pick = NULL;
    int count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;

        if ( d->connected != CON_PLAYING || d->character == NULL )
            continue;
        wch = d->original != NULL ? d->original : d->character;
        if ( IS_BOT(wch) || wch->in_room == NULL || wch->invis_level > 0 || wch->incog_level > 0 )
            continue;
        if ( number_range( 0, count++ ) == 0 )
            pick = wch;
    }
    return pick;
}

/* çevrimiçi insan ölümsüz (bot değil); yoksa NULL */
CHAR_DATA *bot_human_immortal( void )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && d->character != NULL && IS_IMMORTAL(d->character)
          && !IS_BOT(d->character) )
            return d->character;
    return NULL;
}

CHAR_DATA *bot_char_by_id( long id )
{
    CHAR_DATA *ch;

    if ( id == 0 )
        return NULL;
    for ( ch = char_list; ch != NULL; ch = ch->next )
        if ( ch->id == id && ch->in_room != NULL )
            return ch;
    return NULL;
}

CHAR_DATA *bot_room_char_by_id( ROOM_INDEX_DATA *room, long id )
{
    CHAR_DATA *ch;

    if ( room == NULL || id == 0 )
        return NULL;
    for ( ch = room->people; ch != NULL; ch = ch->next_in_room )
        if ( ch->id == id )
            return ch;
    return NULL;
}

/*
 * get_char_room ile hedefi tam olarak bulan "N.anahtar" ifadesi.
 */
void bot_char_keyword( CHAR_DATA *viewer, CHAR_DATA *victim, char *buf, size_t size )
{
    char word[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int count = 0;

    one_argument( victim->name, word );
    if ( word[0] == '\0' )
        snprintf( word, sizeof(word), "%s", "birisi" );
    if ( !IS_NPC(victim) )
    {
        snprintf( buf, size, "%s", victim->name );
        return;
    }
    for ( rch = viewer->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( !can_see( viewer, rch ) )
            continue;
        if ( is_name( word, rch->name ) )
        {
            count++;
            if ( rch == victim )
                break;
        }
    }
    if ( count <= 1 )
        snprintf( buf, size, "%s", word );
    else
        snprintf( buf, size, "%d.%s", count, word );
}

/*
 * get_obj_list/get_obj_carry ile eşyayı tam bulan "N.anahtar" ifadesi.
 */
void bot_obj_keyword( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *list, char *buf, size_t size )
{
    char word[MAX_INPUT_LENGTH];
    OBJ_DATA *o;
    int count = 0;

    one_argument( obj->name, word );
    if ( word[0] == '\0' )
    {
        snprintf( buf, size, "%s", "esya" );
        return;
    }
    for ( o = list; o != NULL; o = o->next_content )
    {
        if ( !can_see_obj( ch, o ) )
            continue;
        if ( is_name( word, o->name ) )
        {
            count++;
            if ( o == obj )
                break;
        }
    }
    if ( count <= 1 )
        snprintf( buf, size, "%s", word );
    else
        snprintf( buf, size, "%d.%s", count, word );
}

/*
 * Botun bir oyuncu komutu çalıştırması. Komut karakteri oyundan
 * çıkarabilir (ayrıl, ölüm); çağıran bot->ch'yi yeniden denetlemelidir.
 */
void bot_cmd( BOT_DATA *bot, const char *fmt, ... )
{
    char buf[MAX_INPUT_LENGTH];
    va_list args;

    if ( bot->ch == NULL )
        return;
    va_start( args, fmt );
    vsnprintf( buf, sizeof(buf), fmt, args );
    va_end( args );
    if ( bot_debug )
        bot_log( bot, "komut: %s", buf );
    interpret( bot->ch, buf, FALSE );
}

/* ---------------------------------------------------------------------
 * kadro dosyası
 * ------------------------------------------------------------------ */
static int kisilik_lookup( const char *s )
{
    if ( !str_prefix( s, "geveze" ) )       return BOT_K_GEVEZE;
    if ( !str_prefix( s, "agresif" ) )      return BOT_K_AGRESIF;
    if ( !str_prefix( s, "yardımsever" ) || !str_prefix( s, "yardimsever" ) ) return BOT_K_YARDIMSEVER;
    if ( !str_prefix( s, "esprili" ) )      return BOT_K_ESPRILI;
    if ( !str_prefix( s, "gizemli" ) )      return BOT_K_GIZEMLI;
    if ( !str_prefix( s, "acemi" ) )        return BOT_K_ACEMI;
    if ( !str_prefix( s, "bilge" ) )        return BOT_K_BILGE;
    return BOT_K_SAKIN;
}

static char *trim( char *s )
{
    char *e;

    while ( *s == ' ' || *s == '\t' )
        s++;
    e = s + strlen( s );
    while ( e > s && ( e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\n' || e[-1] == '\r' ) )
        *--e = '\0';
    return s;
}

static bool bot_parse_line( char *line )
{
    char *field[16];
    int n = 0;
    char *p = line;
    BOT_DATA *bot;
    int race, iclass;

    while ( n < 16 && p != NULL )
    {
        char *bar = strchr( p, '|' );
        field[n++] = p;
        if ( bar == NULL )
            break;
        *bar = '\0';
        p = bar + 1;
    }
    if ( n < 12 )
        return FALSE;

    for ( int i = 0; i < n; i++ )
        field[i] = trim( field[i] );

    if ( strlen( field[0] ) < 2 || strlen( field[0] ) >= BOT_NAME_LEN )
        return FALSE;

    race = race_lookup( field[1] );
    if ( race == 0 || !race_table[race].pc_race )
        return FALSE;
    iclass = class_lookup( field[2] );
    if ( iclass < 0 )
        return FALSE;

    bot = (BOT_DATA *) alloc_perm( sizeof(*bot) );
    memset( bot, 0, sizeof(*bot) );
    snprintf( bot->name, sizeof(bot->name), "%s", capitalize( field[0] ) );
    bot->race       = race;
    bot->iclass     = iclass;
    bot->sex        = ( field[3][0] == 'k' || field[3][0] == 'K' ) ? SEX_FEMALE : SEX_MALE;
    bot->align      = ( field[4][0] == 'i' ) ? 1000 : ( field[4][0] == 'k' ) ? -1000 : 0;
    bot->ethos      = ( field[5][0] == 't' ) ? 1 : ( field[5][0] == 'k' ) ? 3 : 2;
    bot->kisilik    = kisilik_lookup( field[6] );
    bot->hour_from  = URANGE( 0, atoi( field[7] ), 23 );
    bot->hour_to    = URANGE( 0, atoi( field[8] ), 24 ) % 24;
    bot->turkce     = atoi( field[9] ) != 0;
    bot->kucuk_harf = atoi( field[10] ) != 0;
    bot->pk_istekli = atoi( field[11] ) != 0;
    bot->lakap      = ( n > 12 && field[12][0] != '\0' ) ? str_dup( field[12] ) : NULL;

    /* sınıf-yönelim uyumu */
    if ( class_table[iclass].align == CR_GOOD ) bot->align = 1000;
    if ( class_table[iclass].align == CR_EVIL ) bot->align = -1000;
    if ( iclass == CLASS_PALADIN )  bot->ethos = 1;

    bot->next = bot_list;
    bot_list  = bot;
    return TRUE;
}

static void bot_load_roster( void )
{
    FILE *fp;
    char line[1024];
    int count = 0, leader_count = 0, god_count = 0, i;
    BOT_DATA *rev = NULL, *bot, *next;
    struct { char name[64]; char cabal[64]; } leaders[16];
    struct { char name[64]; int level; } gods[8];

    if ( ( fp = fopen( BOT_ROSTER_FILE, "r" ) ) == NULL )
        return;

    while ( fgets( line, sizeof(line), fp ) != NULL )
    {
        char *s = trim( line );

        if ( s[0] == '\0' || s[0] == '#' || s[0] == '*' )
            continue;
        if ( s[0] == '!' )
        {
            char key[64];
            int val;

            if ( sscanf( s + 1, "%63s %d", key, &val ) == 2 )
            {
                if ( !str_cmp( key, "enaz" ) )  bot_min_online = URANGE( 0, val, 60 );
                if ( !str_cmp( key, "encok" ) ) bot_max_online = URANGE( 0, val, 60 );
            }
            else if ( ( !str_prefix( "tanrı ", s + 1 ) || !str_prefix( "tanri ", s + 1 ) ) && god_count < 8 )
            {
                /* !tanrı <bot> <seviye>: ölümsüz bot */
                char who[64];
                int lvl = 0;
                const char *rest = strchr( s + 1, ' ' );

                if ( rest != NULL && sscanf( rest + 1, "%63s %d", who, &lvl ) >= 1 )
                {
                    snprintf( gods[god_count].name, sizeof(gods[god_count].name), "%s", who );
                    gods[god_count].level = lvl > 0 ? lvl : LEVEL_IMMORTAL;
                    god_count++;
                }
            }
            else if ( !str_prefix( "lider ", s + 1 ) && leader_count < 16 )
            {
                /* !lider <bot> <kabal>: tanrıların atadığı kabal lideri */
                char who[64], cab[64];

                if ( sscanf( s + 7, "%63s %63s", who, cab ) == 2 )
                {
                    snprintf( leaders[leader_count].name, sizeof(leaders[leader_count].name), "%s", who );
                    snprintf( leaders[leader_count].cabal, sizeof(leaders[leader_count].cabal), "%s", cab );
                    leader_count++;
                }
            }
            continue;
        }
        if ( bot_parse_line( s ) )
            count++;
        else
        {
            char buf[1200];
            snprintf( buf, sizeof(buf), "[bot] kadro satırı okunamadı: %s", s );
            log_string( buf );
        }
    }
    fclose( fp );

    /* dosya sırasını koru */
    for ( bot = bot_list; bot != NULL; bot = next )
    {
        next = bot->next;
        bot->next = rev;
        rev = bot;
    }
    bot_list = rev;

    for ( i = 0; i < leader_count; i++ )
    {
        int cabal = cabal_lookup( leaders[i].cabal );

        for ( bot = bot_list; bot != NULL; bot = bot->next )
            if ( !str_cmp( bot->name, leaders[i].name ) )
                break;
        if ( bot == NULL || cabal <= CABAL_NONE )
        {
            char buf[256];
            snprintf( buf, sizeof(buf), "[bot] lider satırı tanınmadı: %s %s", leaders[i].name, leaders[i].cabal );
            log_string( buf );
            continue;
        }
        bot->leader_cabal = cabal;
        bot->pk_istekli = TRUE;
    }

    for ( i = 0; i < god_count; i++ )
    {
        for ( bot = bot_list; bot != NULL; bot = bot->next )
            if ( !str_cmp( bot->name, gods[i].name ) )
                break;
        if ( bot == NULL )
        {
            char buf[256];
            snprintf( buf, sizeof(buf), "[bot] tanrı satırı tanınmadı: %s", gods[i].name );
            log_string( buf );
            continue;
        }
        bot->god_level = URANGE( LEVEL_IMMORTAL, gods[i].level, MAX_LEVEL - 1 );
        bot->pk_istekli = FALSE;
        bot->leader_cabal = 0;
    }

    if ( bot_max_online < bot_min_online )
        bot_max_online = bot_min_online;
    if ( count > 0 )
        bots_enabled = TRUE;
    {
        char buf[200];
        snprintf( buf, sizeof(buf), "[bot] %d bot tanımı yüklendi (en az %d, en çok %d çevrimiçi).",
                  count, bot_min_online, bot_max_online );
        log_string( buf );
    }
}

/* ---------------------------------------------------------------------
 * açılış
 * ------------------------------------------------------------------ */
void bot_boot( void )
{
    BOT_DATA *bot;
    int order = 0;

    bot_load_roster();
    if ( !bots_enabled )
        return;

    for ( bot = bot_list; bot != NULL; bot = bot->next )
    {
        char name[BOT_NAME_LEN];

        snprintf( name, sizeof(name), "%s", bot->name );
        if ( !check_parse_name( name ) )
        {
            bot->disabled = TRUE;
            bot_log( bot, "isim kurallara uymuyor ya da bir yaratık adı; bot devre dışı." );
            continue;
        }
    }

    bot_brain_boot();
    bot_chat_boot();

    /* açılıştan sonra kademeli girişler */
    for ( bot = bot_list; bot != NULL; bot = bot->next )
    {
        if ( bot->disabled )
            continue;
        if ( order < bot_min_online + 2 )
            bot->next_login_try = current_time + 8 + order * 9 + number_range( 0, 15 );
        else
            bot->next_login_try = current_time + number_range( 120, 900 );
        order++;
    }
    boot_pulse_scheduled = 1;
}

/* ---------------------------------------------------------------------
 * giriş / çıkış
 * ------------------------------------------------------------------ */
static void bot_apply_creation( BOT_DATA *bot, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    int i, race = bot->race;

    /* parola: kimsenin bilmediği rastgele bir dizgi */
    snprintf( buf, sizeof(buf), "bot-%s-%lld-%d", bot->name, (long long) current_time, number_range( 1000, 999999 ) );
    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwd_hash( buf ) );

    /* ırk (nanny CON_GET_NEW_RACE) */
    ch->pcdata->race = race;
    RACE(ch) = race;
    for ( i = 0; i < MAX_STATS; i++ )
        ch->mod_stat[i] = 0;
    ch->hit  = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->detection   = ch->affected_by | race_table[race].det;
    ch->affected_by = ch->affected_by | race_table[race].aff;
    ch->imm_flags   = ch->imm_flags | race_table[race].imm;
    ch->res_flags   = ch->res_flags | race_table[race].res;
    ch->vuln_flags  = ch->vuln_flags | race_table[race].vuln;
    ch->form        = race_table[race].form;
    ch->parts       = race_table[race].parts;
    for ( i = 0; i < 5; i++ )
    {
        int sn;
        if ( race_table[race].skills[i] == NULL )
            break;
        if ( ( sn = skill_lookup( race_table[race].skills[i] ) ) >= 0 )
            ch->pcdata->learned[sn] = 100;
    }
    ch->pcdata->points = race_table[race].points;
    ch->size = race_table[race].size;
    ch->pcdata->familya[race] = 75;

    /* cinsiyet */
    ch->sex = bot->sex;
    ch->pcdata->true_sex = bot->sex;

    /* sınıf (nanny CON_GET_NEW_CLASS) */
    ch->iclass = bot->iclass;
    ch->pcdata->points += class_table[bot->iclass].points;
    for ( i = 0; i < MAX_STATS; i++ )
        ch->perm_stat[i] = get_max_train( ch, i );
    ch->perm_stat[STAT_CHA] = 15;

    /* yönelim, etik, memleket */
    ch->alignment = bot->align;
    ch->ethos     = bot->ethos;
    ch->hometown  = 0;
    ch->endur     = 0;

    /* CON_CREATE_DONE */
    group_add( ch );
    ch->pcdata->learned[gsn_recall] = 75;
}

/* dosyası olmayan (seviye 0) karakter: nanny'nin CON_CREATE_DONE başlangıcı */
static void bot_init_fresh( CHAR_DATA *ch )
{
    int l, today, day;

    ch->level = 1;
    ch->exp   = base_exp( ch, ch->pcdata->points );
    ch->pcdata->death = 0;
    today = parse_date( current_time );
    for ( l = 0; l < MAX_TIME_LOG; l++ )
    {
        day = ( ( 365 + today - l ) % 365 );
        ch->pcdata->log_date[l] = day ? day : 365;
        ch->pcdata->log_time[l] = 60;
    }
}

static void bot_enter_world( BOT_DATA *bot, CHAR_DATA *ch, bool fresh )
{
    char buf[MAX_STRING_LENGTH];
    int i;

    ch->next  = char_list;
    char_list = ch;
    iNumPlayers++;

    SET_BIT( ch->act, PLR_AUTOEXIT | PLR_AUTOAKCE | PLR_AUTOLOOT | PLR_AUTOSAC
                    | PLR_AUTOASSIST | PLR_AUTOSPLIT );
    REMOVE_BIT( ch->act, PLR_NOFOLLOW );
    REMOVE_BIT( ch->comm, COMM_NOKD | COMM_QUIET | COMM_DEAF );

    if ( bot->god_level > 0 )
    {
        bool new_char = ( ch->level == 0 );

        if ( new_char )
            bot_init_fresh( ch );
        bot_god_enter( bot, ch, new_char );
    }
    else if ( ch->level == 0 )
    {
        bot_init_fresh( ch );
        ch->hit   = ch->max_hit;
        ch->mana  = ch->max_mana;
        ch->move  = ch->max_move;
        ch->pcdata->perm_hit  = ch->max_hit;
        ch->pcdata->perm_mana = ch->max_mana;
        ch->pcdata->perm_move = ch->max_move;
        ch->train    += 3;
        ch->practice += 5;

        snprintf( buf, sizeof(buf), "%s", title_table[ch->iclass][ch->level] );
        set_title( ch, buf );

        obj_to_char( create_object( get_obj_index( OBJ_VNUM_MAP ), 0 ), ch );
        obj_to_char( create_object( get_obj_index( OBJ_VNUM_NMAP1 ), 0 ), ch );
        obj_to_char( create_object( get_obj_index( OBJ_VNUM_NMAP2 ), 0 ), ch );
        obj_to_char( create_object( get_obj_index( OBJ_VNUM_MAP_SM ), 0 ), ch );

        ch->pcdata->learned[get_weapon_sn( ch, FALSE )] = 40;

        char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
        do_outfit( ch, "" );
    }
    else if ( ch->in_room != NULL )
    {
        /* oyuncuların girişiyle aynı kural: kaydedildiği odaya döner */
        if ( cabal_area_check( ch ) )
        {
            int idx = IS_GOOD(ch) ? 0 : IS_EVIL(ch) ? 2 : 1;
            char_to_room( ch, get_room_index( hometown_table[ch->hometown].altar[idx] ) );
        }
        else
            char_to_room( ch, ch->in_room );
    }
    else
        char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );

    reset_char( ch );
    act( "$n mud'a giriş yaptı.", ch, NULL, NULL, TO_ROOM );
    wiznet( "$N gerçekliğe giriş yaptı.", ch, NULL, WIZ_LOGINS, 0, 0 );

    if ( ch->exp < ( exp_per_level( ch, ch->pcdata->points ) * ch->level ) )
        ch->exp = ch->level * exp_per_level( ch, ch->pcdata->points );

    if ( IS_QUESTOR(ch) && ch->pcdata->questmob == 0 )
    {
        ch->pcdata->nextquest = ch->pcdata->countdown;
        REMOVE_BIT( ch->act, PLR_QUESTOR );
    }
    if ( ch->pcdata->familya[ch->pcdata->race] < 75 )
        ch->pcdata->familya[ch->pcdata->race] = 75;
    REMOVE_BIT( ch->act, PLR_NO_EXP );
    REMOVE_BIT( ch->act, PLR_CHANGED_AFF );
    for ( i = 0; i < MAX_STATS; i++ )
    {
        if ( ch->perm_stat[i] > get_max_train( ch, i ) )
        {
            ch->train += ( ch->perm_stat[i] - get_max_train( ch, i ) );
            ch->perm_stat[i] = get_max_train( ch, i );
        }
    }
    ch->timer = 0;
    ch->logon = current_time;
    do_look( ch, "auto" );

    bot->ch          = ch;
    bot->login_time  = current_time;
    bot->logins++;
    bot->last_level  = ch->level;
    bot->last_room_vnum = ch->in_room != NULL ? ch->in_room->vnum : 0;
    bot->path_len = bot->path_pos = 0;
    bot->target_id = 0;
    bot->leader_id = 0;
    bot->pk_target_id = 0;
    bot->hunt_area = NULL;
    bot->town_tasks = 0;
    bot->inv_signature = -1;
    bot->next_think  = bot_pulse + number_range( 8, 30 );
    bot->next_chat   = bot_pulse + number_range( 240, 1200 );
    bot->next_kd     = bot_pulse + number_range( 600, 2400 );
    bot->next_social = bot_pulse + number_range( 200, 900 );
    bot->next_pk     = bot_pulse + number_range( 4800, 14400 );
    memset( bot->replies, 0, sizeof(bot->replies) );
    bot_set_state( bot, BOT_ST_IDLE );

    bot_brain_login( bot, fresh );

    snprintf( buf, sizeof(buf), "%s@bot baglandi. (seviye %d)", ch->name, ch->level );
    log_string( buf );
    bot_chat_event( bot, BOT_EV_LOGIN, NULL );
}

void bot_login( BOT_DATA *bot )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;
    bool found;

    if ( bot->ch != NULL || bot->disabled )
        return;
    if ( bot_name_taken( bot->name ) )          /* biri bu adla oyunda */
    {
        bot->next_login_try = current_time + 600;
        return;
    }

    d = new_descriptor();
    d->descriptor = -1;
    d->connected  = CON_PLAYING;
    d->host       = str_dup( "bot" );
    d->outsize    = 2000;
    d->outbuf     = alloc_mem( d->outsize );

    found = load_char_obj( d, bot->name );
    ch    = d->character;
    d->character = NULL;
    ch->desc = NULL;
    free_descriptor( d );

    if ( found && !ch->pcdata->bot_file )
    {
        bot_log( bot, "aynı adlı gerçek bir oyuncu dosyası var; bot devre dışı." );
        bot->disabled = TRUE;
        free_char( ch );
        return;
    }

    ch->pcdata->bot      = bot;
    ch->pcdata->bot_file = TRUE;
    if ( ch->pcdata->discord_id == NULL || ch->pcdata->discord_id[0] == '\0' )
    {
        char idbuf[32];
        long long h = 7;
        const char *p;

        /* her bot ayrı bir "kişi": aynı hosttan çoklu bağlantı kuralı için farklı kimlik */
        for ( p = bot->name; *p != '\0'; p++ )
            h = h * 131 + (unsigned char) *p;
        snprintf( idbuf, sizeof(idbuf), "9%017lld", h % 99999999999999999LL );
        free_string( ch->pcdata->discord_id );
        ch->pcdata->discord_id = str_dup( idbuf );
    }

    if ( !found )
        bot_apply_creation( bot, ch );

    bot_enter_world( bot, ch, !found );
}

void bot_logout( BOT_DATA *bot, bool force )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL )
        return;

    if ( force )
    {
        if ( ch->level > 1 )
            save_char_obj( ch );
        act( "$n oyundan ayrıldı.", ch, NULL, NULL, TO_ROOM );
        extract_char( ch, TRUE );
        iNumPlayers--;
    }
    else
    {
        bot_set_state( bot, BOT_ST_LOGOUT );
        do_quit( ch, "" );
    }

    if ( bot->ch == NULL )
    {
        char buf[MAX_STRING_LENGTH];
        bot->next_login_try = current_time + ( bot_in_hours( bot ) ? number_range( 600, 2400 ) : number_range( 1800, 7200 ) );
        snprintf( buf, sizeof(buf), "%s@bot oyundan ayrildi.", bot->name );
        log_string( buf );
    }
}

/* free_pcdata çağrılınca bot bağlantısını kopar */
void bot_pcdata_freed( PC_DATA *pcdata )
{
    BOT_DATA *bot;

    if ( pcdata == NULL || pcdata->bot == NULL )
        return;
    bot = pcdata->bot;
    pcdata->bot = NULL;
    if ( bot->ch != NULL && bot->ch->pcdata == pcdata )
    {
        bot->ch = NULL;
        bot_set_state( bot, BOT_ST_IDLE );
        bot->path_len = bot->path_pos = 0;
        if ( bot->next_login_try < current_time + 60 )
            bot->next_login_try = current_time + number_range( 300, 1800 );
    }
}

bool bot_name_taken( const char *name )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
        if ( IS_BOT(ch) && !str_cmp( ch->name, name ) )
            return TRUE;
    return FALSE;
}

int bot_online_count( void )
{
    BOT_DATA *bot;
    int n = 0;

    for ( bot = bot_list; bot != NULL; bot = bot->next )
        if ( bot->ch != NULL )
            n++;
    return n;
}

/*
 * "kim" ve benzeri listeler için: bağlı oyuncular + botlar.
 * dch: görünürlük denetimi yapılacak karakter, wch: listelenecek karakter.
 */
int bot_who_collect( CHAR_DATA **dch, CHAR_DATA **wch, int max )
{
    DESCRIPTOR_DATA *d;
    BOT_DATA *bot;
    int n = 0;

    for ( d = descriptor_list; d != NULL && n < max; d = d->next )
    {
        if ( d->connected != CON_PLAYING || d->character == NULL )
            continue;
        dch[n] = d->character;
        wch[n] = d->original != NULL ? d->original : d->character;
        n++;
    }
    for ( bot = bot_list; bot != NULL && n < max; bot = bot->next )
    {
        if ( bot->ch == NULL || bot->ch->in_room == NULL )
            continue;
        dch[n] = bot->ch;
        wch[n] = bot->ch;
        n++;
    }
    return n;
}

/* ---------------------------------------------------------------------
 * yol bulma (BFS)
 * ------------------------------------------------------------------ */
#define BOT_VNUM_MAX 32767
static unsigned int *bfs_stamp = NULL;
static int          *bfs_prev  = NULL;    /* önceki odanın vnum'u */
static signed char  *bfs_dir   = NULL;    /* önceki odadan gelinen yön */
static int          *bfs_queue = NULL;
static int          *bfs_dist  = NULL;    /* başlangıçtan uzaklık */
static unsigned int  bfs_cur_stamp = 0;

static bool bfs_alloc( void )
{
    if ( bfs_stamp != NULL )
        return TRUE;
    bfs_stamp = (unsigned int *) calloc( BOT_VNUM_MAX + 1, sizeof(unsigned int) );
    bfs_prev  = (int *) calloc( BOT_VNUM_MAX + 1, sizeof(int) );
    bfs_dir   = (signed char *) calloc( BOT_VNUM_MAX + 1, sizeof(signed char) );
    bfs_queue = (int *) calloc( BOT_VNUM_MAX + 1, sizeof(int) );
    bfs_dist  = (int *) calloc( BOT_VNUM_MAX + 1, sizeof(int) );
    if ( bfs_stamp == NULL || bfs_prev == NULL || bfs_dir == NULL || bfs_queue == NULL || bfs_dist == NULL )
    {
        free( bfs_stamp ); free( bfs_prev ); free( bfs_dir ); free( bfs_queue ); free( bfs_dist );
        bfs_stamp = NULL; bfs_prev = NULL; bfs_dir = NULL; bfs_queue = NULL; bfs_dist = NULL;
        bug( "bfs_alloc: bellek yok", 0 );
        return FALSE;
    }
    return TRUE;
}

/* next odasından room odasına geri dönen bir çıkış var mı? (tek yönlü tuzaklara girme) */
bool bot_exit_back( ROOM_INDEX_DATA *next, ROOM_INDEX_DATA *room )
{
    int d;

    for ( d = 0; d < 6; d++ )
        if ( next->exit[d] != NULL && next->exit[d]->u1.to_room == room )
            return TRUE;
    return FALSE;
}

bool bot_room_passable( CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool allow_cabal )
{
    int iClass, iGuild;

    if ( room == NULL || room->vnum < 0 )
        return FALSE;
    if ( !can_see_room( ch, room ) )
        return FALSE;
    if ( IS_SET( room->room_flags, ROOM_PRIVATE | ROOM_SOLITARY | ROOM_IMP_ONLY | ROOM_GODS_ONLY ) )
        return FALSE;
    if ( room->area != NULL && IS_SET( room->area->area_flag, AREA_CABAL ) )
    {
        /* kabal bölgesi: yalnızca kabal işi (PK/baskın) için; seviye aralığı kuralı uygulanmaz */
        if ( !allow_cabal )
            return FALSE;
    }
    /* seviyesinin çok üstündeki bölgelerden geçme (yolda ölmesin) */
    else if ( room->area != NULL && room->area->low_range > ch->level + 8 && !IS_IMMORTAL(ch) )
        return FALSE;
    if ( room->sector_type == SECT_AIR && !IS_AFFECTED( ch, AFF_FLYING ) )
        return FALSE;
    if ( IS_BOT(ch) && bot_room_avoided( ch->pcdata->bot, room ) )
        return FALSE;
    if ( room->sector_type == SECT_WATER_NOSWIM && !IS_AFFECTED( ch, AFF_FLYING ) )
    {
        OBJ_DATA *obj;
        bool boat = IS_SET( ch->act, PLR_GHOST );

        for ( obj = ch->carrying; obj != NULL && !boat; obj = obj->next_content )
            if ( obj->item_type == ITEM_BOAT )
                boat = TRUE;
        if ( !boat )
            return FALSE;
    }
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        for ( iGuild = 0; iGuild < MAX_GUILD; iGuild++ )
            if ( class_table[iClass].guild[iGuild] == room->vnum && iClass != ch->iclass )
                return FALSE;
    return TRUE;
}

/*
 * Kenar geçilebilir mi? Reddedilirse neden dizgisini döndürür, geçilebilirse NULL.
 * Hedef oda (o->to) geçilebilirlik kuralından muaftır (oraya varmak yeter).
 */
static const char *bfs_edge_reject( CHAR_DATA *ch, const struct bot_bfs *o, ROOM_INDEX_DATA *room,
                                    EXIT_DATA *pexit, ROOM_INDEX_DATA *next, ROOM_INDEX_DATA *start )
{
    if ( o->raw )
    {
        if ( o->block_closed && IS_SET( pexit->exit_info, EX_CLOSED ) )
            return "kapalı";
        return NULL;
    }
    if ( IS_SET( pexit->exit_info, EX_LOCKED ) && !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
        return "kilitli";
    if ( next == o->to )
        return NULL;
    if ( o->same_area && next->area != start->area )
        return "başka bölge";
    if ( !bot_room_passable( ch, next, o->allow_cabal ) )
        return "geçilemez";
    /* geri dönüşü olmayan çıkışlardan geçme; kabal karargâhları tek yönlü iniş
       olabilir (çıkış portalla), kabal işi için oraya girmeye izin verilir */
    if ( !bot_exit_back( next, room )
      && !( o->allow_cabal && next->area != NULL && IS_SET( next->area->area_flag, AREA_CABAL ) ) )
        return "geri dönüşsüz";
    return NULL;
}

/*
 * Genişlik öncelikli gezinti çekirdeği. o->to verilmişse en kısa yolun
 * uzunluğunu döndürür (dirs != NULL ise yön dizisini yazar; max'ı aşarsa -1),
 * yol yoksa -1. o->to NULL ise gezilen oda sayısını döndürür. Her durumda
 * o->visited ve o->first_dir doldurulur.
 */
int bot_bfs( CHAR_DATA *ch, ROOM_INDEX_DATA *from, struct bot_bfs *o, sh_int *dirs, int max )
{
    int head = 0, tail = 0, d, v;

    o->visited = 0;
    o->first_dir = -1;
    if ( from == NULL || ch == NULL || from->vnum < 0 || from->vnum > BOT_VNUM_MAX )
        return -1;
    if ( o->to != NULL && ( o->to->vnum < 0 || o->to->vnum > BOT_VNUM_MAX ) )
        return -1;
    if ( o->to == from )
        return 0;
    if ( !bfs_alloc() )
        return -1;
    if ( ++bfs_cur_stamp == 0 )
    {
        memset( bfs_stamp, 0, ( BOT_VNUM_MAX + 1 ) * sizeof(unsigned int) );
        bfs_cur_stamp = 1;
    }

    bfs_stamp[from->vnum] = bfs_cur_stamp;
    bfs_prev[from->vnum]  = -1;
    bfs_dist[from->vnum]  = 0;
    bfs_queue[tail++]     = from->vnum;

    while ( head < tail )
    {
        ROOM_INDEX_DATA *room = get_room_index( bfs_queue[head++] );
        int dist;

        if ( room == NULL )
            continue;
        o->visited++;
        dist = bfs_dist[room->vnum];
        if ( o->visit != NULL && !o->visit( room, dist, o->visit_ctx ) )
            break;
        if ( o->max_depth > 0 && dist >= o->max_depth )
            continue;
        for ( d = 0; d < 6; d++ )
        {
            EXIT_DATA *pexit = room->exit[d];
            ROOM_INDEX_DATA *next;
            const char *why;

            if ( pexit == NULL || ( next = pexit->u1.to_room ) == NULL )
                continue;
            if ( next->vnum < 0 || next->vnum > BOT_VNUM_MAX )
                continue;
            if ( bfs_stamp[next->vnum] == bfs_cur_stamp )
                continue;
            if ( ( why = bfs_edge_reject( ch, o, room, pexit, next, from ) ) != NULL )
            {
                bfs_stamp[next->vnum] = bfs_cur_stamp;      /* aynı sınır bir kez raporlanır */
                if ( o->reject != NULL )
                    o->reject( room, d, next, why, o->reject_ctx );
                continue;
            }
            bfs_stamp[next->vnum] = bfs_cur_stamp;
            bfs_prev[next->vnum]  = room->vnum;
            bfs_dir[next->vnum]   = (signed char) d;
            bfs_dist[next->vnum]  = dist + 1;
            if ( next == o->to )
            {
                int len = dist + 1;

                for ( v = next->vnum; bfs_prev[v] != from->vnum; v = bfs_prev[v] )
                    ;
                o->first_dir = bfs_dir[v];
                if ( dirs != NULL )
                {
                    int pos = len;

                    if ( len > max )
                        return -1;
                    for ( v = next->vnum; v != from->vnum && v >= 0; v = bfs_prev[v] )
                        dirs[--pos] = bfs_dir[v];
                }
                return len;
            }
            if ( o->max_rooms > 0 && tail >= o->max_rooms )
                continue;
            if ( tail <= BOT_VNUM_MAX )
                bfs_queue[tail++] = next->vnum;
        }
    }
    return o->to != NULL ? -1 : o->visited;
}

/*
 * from -> to arası en kısa yol; dirs'e yön dizisi yazar, uzunluğu döndürür.
 * Yol yoksa -1.
 */
int bot_find_path( CHAR_DATA *ch, ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to,
                   sh_int *dirs, int max, bool allow_cabal )
{
    struct bot_bfs o;

    if ( to == NULL )
        return -1;
    memset( &o, 0, sizeof(o) );
    o.to = to;
    o.allow_cabal = allow_cabal;
    return bot_bfs( ch, from, &o, dirs, max );
}

bool bot_set_travel( BOT_DATA *bot, int vnum, int after )
{
    ROOM_INDEX_DATA *to = get_room_index( vnum );
    int len;

    if ( bot->ch == NULL || bot->ch->in_room == NULL || to == NULL )
        return FALSE;
    len = bot_find_path( bot->ch, bot->ch->in_room, to, bot->path, BOT_MAX_PATH,
                         after == BOT_ST_PK || after == BOT_ST_RAID );
    if ( len < 0 )
        return FALSE;
    bot->path_len       = len;
    bot->path_pos       = 0;
    bot->target_vnum    = vnum;
    bot->after_travel   = after;
    bot->stuck          = 0;
    bot->travel_started = bot_pulse;
    bot_set_state( bot, BOT_ST_TRAVEL );
    return TRUE;
}

/* ---------------------------------------------------------------------
 * oyun kancaları
 * ------------------------------------------------------------------ */
void bot_hear( CHAR_DATA *listener, CHAR_DATA *speaker, int channel, const char *text )
{
    BOT_DATA *bot = bot_of( listener );

    if ( bot == NULL || speaker == NULL || speaker == listener || text == NULL )
        return;
    if ( bot_is_god( bot ) )
    {
        bot_god_hear( bot, speaker, channel, text );
        return;
    }
    if ( channel == BOT_CH_PRAY )
        return;
    if ( listener->position <= POS_SLEEPING && channel != BOT_CH_TELL )
        return;
    bot_chat_react( bot, speaker, channel, text );
}

void bot_on_death( CHAR_DATA *victim )
{
    BOT_DATA *bot = bot_of( victim );

    if ( bot == NULL )
        return;
    bot->deaths++;
    if ( bot->state == BOT_ST_RAID || bot->raid_cabal != CABAL_NONE )
        bot_raid_failed( bot );
    bot->death_room = victim->in_room != NULL ? victim->in_room->vnum : 0;
    bot->corpse_tries = 0;
    bot->target_id = 0;
    bot->pk_target_id = 0;
    bot->path_len = bot->path_pos = 0;
    bot_log( bot, "öldü (oda %d, seviye %d, son rakip %s).", bot->death_room, victim->level,
             bot->last_opp[0] != '\0' ? bot->last_opp : "?" );
}

void bot_on_kill( CHAR_DATA *killer, CHAR_DATA *victim )
{
    BOT_DATA *bot = bot_of( killer );

    if ( bot == NULL || victim == NULL )
        return;
    if ( IS_NPC(victim) )
    {
        bot->kills++;
        snprintf( bot->last_mob, sizeof(bot->last_mob), "%s", victim->short_descr != NULL ? victim->short_descr : "bir yaratık" );
        if ( bot_debug )
            bot_log( bot, "öldürdü: %s (lvl %d, yön %d) - bot lvl %d tp %d (son kesimden bu yana +%d)",
                     victim->short_descr, victim->level, victim->alignment, killer->level, killer->exp,
                     killer->exp - bot->last_kill_exp );
        bot->last_kill_exp = killer->exp;
        bot_note_kill( bot, victim );
        bot_chat_event( bot, BOT_EV_KILL, victim );
    }
    else
    {
        bot->pk_kills++;
        bot_chat_event( bot, BOT_EV_PK_KILL, victim );
    }
}

/* ---------------------------------------------------------------------
 * cevap kuyruğu
 * ------------------------------------------------------------------ */
void bot_queue_reply( BOT_DATA *bot, const char *to, int channel, int delay, const char *text )
{
    int i;

    for ( i = 0; i < BOT_MAX_REPLIES; i++ )
    {
        if ( bot->replies[i].when == 0 )
        {
            snprintf( bot->replies[i].to, BOT_NAME_LEN, "%s", to != NULL ? to : "" );
            bot->replies[i].channel = channel;
            bot->replies[i].when    = bot_pulse + UMAX( 1, delay );
            snprintf( bot->replies[i].text, sizeof(bot->replies[i].text), "%s", text );
            return;
        }
    }
}

static void bot_flush_replies( BOT_DATA *bot )
{
    int i;

    for ( i = 0; i < BOT_MAX_REPLIES; i++ )
    {
        struct bot_reply *r = &bot->replies[i];
        CHAR_DATA *to = NULL;

        if ( r->when == 0 || r->when > bot_pulse )
            continue;
        r->when = 0;
        if ( bot->ch == NULL )
            continue;
        if ( r->to[0] != '\0' )
        {
            CHAR_DATA *ch;
            for ( ch = char_list; ch != NULL; ch = ch->next )
                if ( !IS_NPC(ch) && !str_cmp( ch->name, r->to ) && ch->in_room != NULL )
                {
                    to = ch;
                    break;
                }
            if ( to == NULL && r->channel != BOT_CH_YELL )
                continue;
            if ( r->channel == BOT_CH_SAY && to != NULL && to->in_room != bot->ch->in_room )
            {
                /* konuşmak istediği kişi gitmiş: kd ile yolla */
                if ( bot_is_human( to ) )
                    r->channel = BOT_CH_TELL;
                else
                    continue;
            }
        }
        bot_talk( bot, r->channel, to, r->text );
        if ( bot->ch == NULL )
            return;
    }
}

/* ---------------------------------------------------------------------
 * zamanlayıcı
 * ------------------------------------------------------------------ */
static void bot_scheduler( void )
{
    BOT_DATA *bot;
    int online = bot_online_count();
    BOT_DATA *candidate = NULL;

    /* çıkışlar */
    for ( bot = bot_list; bot != NULL; bot = bot->next )
    {
        CHAR_DATA *ch = bot->ch;

        if ( ch == NULL )
            continue;
        if ( bot->session_end == 0 )
            bot->session_end = current_time + ( bot_in_hours( bot ) ? number_range( 2700, 10800 ) : number_range( 1200, 3600 ) );
        if ( current_time < bot->session_end )
            continue;
        if ( ch->fighting != NULL || ch->position < POS_RESTING || IS_SET( ch->act, PLR_GHOST ) )
            continue;
        if ( bot->state == BOT_ST_FOLLOW )       /* insanla geziyorsa kalır */
        {
            bot->session_end = current_time + 900;
            continue;
        }
        if ( online <= bot_min_online )
        {
            bot->session_end = current_time + number_range( 600, 1800 );
            continue;
        }
        if ( ch->last_fight_time != -1 && current_time - ch->last_fight_time < FIGHT_DELAY_TIME + 5 )
            continue;
        bot_chat_event( bot, BOT_EV_LOGOUT, NULL );
        bot_logout( bot, FALSE );
        if ( bot->ch == NULL )
        {
            bot->session_end = 0;
            online--;
        }
        else
            bot->session_end = current_time + 120;
        break;                                    /* tur başına bir çıkış */
    }

    /* girişler */
    for ( bot = bot_list; bot != NULL; bot = bot->next )
    {
        if ( bot->ch != NULL || bot->disabled )
            continue;
        if ( current_time < bot->next_login_try )
            continue;
        if ( candidate == NULL || bot->next_login_try < candidate->next_login_try )
            candidate = bot;
    }
    if ( candidate != NULL )
    {
        bool in_hours = bot_in_hours( candidate );
        bool go = FALSE;

        if ( online < bot_min_online )
            go = TRUE;
        else if ( online < bot_max_online && number_percent() < ( in_hours ? 70 : 20 ) )
            go = TRUE;

        if ( go )
        {
            bot_login( candidate );
            if ( candidate->ch != NULL )
                candidate->session_end = current_time + ( in_hours ? number_range( 2700, 10800 ) : number_range( 1200, 3600 ) );
            else
                candidate->next_login_try = current_time + 300;
        }
        else
            candidate->next_login_try = current_time + number_range( 120, 600 );
    }
}

/* yeni giren insanları fark et */
static void bot_watch_humans( void )
{
    DESCRIPTOR_DATA *d;
    long now_ids[BOT_SEEN_MAX];
    int now_count = 0, i, j;

    for ( d = descriptor_list; d != NULL && now_count < BOT_SEEN_MAX; d = d->next )
    {
        CHAR_DATA *wch;

        if ( d->connected != CON_PLAYING || d->character == NULL )
            continue;
        wch = d->original != NULL ? d->original : d->character;
        if ( IS_BOT(wch) || wch->in_room == NULL )
            continue;
        now_ids[now_count++] = wch->id;

        for ( i = 0; i < seen_count; i++ )
            if ( seen_humans[i] == wch->id )
                break;
        if ( i == seen_count && current_time - wch->logon < 120 && boot_pulse_scheduled )
        {
            /* yeni insan: birkaç bot selam versin */
            BOT_DATA *bot;
            int picked = 0;

            for ( bot = bot_list; bot != NULL && picked < 2; bot = bot->next )
            {
                if ( bot->ch == NULL || bot->ch->position < POS_RESTING )
                    continue;
                if ( bot_is_god( bot ) )
                {
                    bot_god_greet( bot, wch );
                    continue;
                }
                if ( number_percent() < 45 )
                {
                    bot_chat_event( bot, BOT_EV_HUMAN_LOGIN, wch );
                    picked++;
                }
            }
        }
    }
    for ( j = 0; j < now_count; j++ )
        seen_humans[j] = now_ids[j];
    seen_count = now_count;
}

void bot_update( void )
{
    BOT_DATA *bot;

    if ( !bots_enabled )
        return;

    bot_pulse++;

    if ( bot_pulse % 20 == 0 )
    {
        bot_scheduler();
        bot_watch_humans();
    }

    for ( bot = bot_list; bot != NULL; bot = bot->next )
    {
        CHAR_DATA *ch = bot->ch;

        if ( ch == NULL )
            continue;
        if ( !IS_VALID(ch) || ch->pcdata == NULL || ch->pcdata->bot != bot )
        {
            /* karakter beklenmedik biçimde gitmiş */
            bot->ch = NULL;
            bot_set_state( bot, BOT_ST_IDLE );
            continue;
        }

        ch->timer = 0;                               /* hiçliğe sürüklenmesin */
        if ( ch->daze > 0 )
            --ch->daze;
        if ( ch->wait > 0 )
        {
            --ch->wait;
            continue;
        }

        bot_flush_replies( bot );
        if ( bot->ch == NULL )
            continue;

        if ( bot_pulse >= bot->next_think && ch->in_room != NULL )
        {
            bot->next_think = bot_pulse + ( ch->fighting != NULL ? number_range( 3, 5 ) : number_range( 3, 6 ) );
            bot_think( bot );
        }
    }
}

/* ---------------------------------------------------------------------
 * ölümsüz komutu: botlar
 * ------------------------------------------------------------------ */
struct botlar_border
{
    CHAR_DATA * viewer;
    int         shown;
};

/* 'botlar yol' tanısı: BFS'in reddettiği kenarlar */
static void botlar_border_reject( ROOM_INDEX_DATA *room, int dir, ROOM_INDEX_DATA *next,
                                  const char *why, void *vctx )
{
    struct botlar_border *ctx = (struct botlar_border *) vctx;

    if ( ctx->shown++ < 15 )
        printf_to_char( ctx->viewer, "  sınır: %d -> %s -> %d (%s) sektör %d bayrak %ld: %s\n\r",
                        room->vnum, dir_name[dir], next->vnum, next->name, next->sector_type,
                        next->room_flags, why );
}

/* botlar: kadro özeti */
static void botlar_list( CHAR_DATA *ch )
{
    BOT_DATA *bot;
    char buf[MAX_INPUT_LENGTH];
    int kadro = 0, online = 0, total_lvl = 0;
    long total_exp = 0;

    for ( bot = bot_list; bot != NULL; bot = bot->next )
        kadro++;
    printf_to_char( ch, "{WÇevrimiçi %d / kadro %d  (en az %d, en çok %d, pulse %d){x\n\r",
                    bot_online_count(), kadro, bot_min_online, bot_max_online, bot_pulse );
    printf_to_char( ch, "{c%-12s %-4s %-6s %-16s %-5s %-5s %-24s %s{x\n\r",
                    "isim", "lvl", "sınıf", "durum", "sn", "yp%", "oda", "öldürme/ölüm/görev" );
    for ( bot = bot_list; bot != NULL; bot = bot->next )
    {
        CHAR_DATA *bch = bot->ch;

        if ( bch == NULL )
        {
            printf_to_char( ch, "%-12s %-4s %-6s %-16s %-5s %-5s %-24s %s\n\r", bot->name, "-",
                            class_table[bot->iclass].who_name,
                            bot->disabled ? "devre dışı" : "çevrimdışı", "", "", "", "" );
            continue;
        }
        snprintf( buf, sizeof(buf), "%d/%d/%d", bot->kills, bot->deaths, bot->quests );
        printf_to_char( ch, "%-12s %-4d %-6s %-16s %-5d %-5d %-24.24s %s\n\r",
                        bch->name, bch->level, class_table[bch->iclass].who_name,
                        bot_state_name( bot->state ), ( bot_pulse - bot->state_pulse ) / BOT_PULSE_SEC,
                        bot_pct( bch->hit, bch->max_hit ),
                        bch->in_room != NULL ? bch->in_room->name : "?", buf );
        total_exp += bch->exp;
        total_lvl += bch->level;
        online++;
    }
    printf_to_char( ch, "Toplam tp %ld, toplam seviye %d (%d çevrimiçi)\n\r", total_exp, total_lvl, online );
    send_to_char( "Kullanım: botlar <isim> | botlar bağla <isim> | botlar ayır <isim> | botlar av <isim> | botlar yol <isim> <oda> [kabal] | botlar debug\n\r", ch );
}

/* botlar bağla/ayır <isim> */
static void botlar_toggle( CHAR_DATA *ch, BOT_DATA *bot, bool login )
{
    if ( login )
    {
        if ( bot->ch != NULL )
        {
            send_to_char( "Zaten oyunda.\n\r", ch );
            return;
        }
        bot->disabled = FALSE;
        bot_login( bot );
        if ( bot->ch != NULL )
            bot->session_end = current_time + number_range( 2700, 7200 );
        send_to_char( bot->ch != NULL ? "Bot oyuna girdi.\n\r" : "Bot giremedi (günlüğe bakın).\n\r", ch );
        return;
    }
    if ( bot->ch == NULL )
    {
        send_to_char( "Zaten çevrimdışı.\n\r", ch );
        return;
    }
    bot_logout( bot, TRUE );
    bot->next_login_try = current_time + 3600;
    send_to_char( "Bot oyundan çıkarıldı.\n\r", ch );
}

/* botlar yol <isim> <oda> [kabal]: çıkışlar, yol ve yoksa BFS'in reddettiği sınırlar */
static void botlar_path( CHAR_DATA *ch, BOT_DATA *bot, char *argument )
{
    char where[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *to;
    sh_int dirs[BOT_MAX_PATH];
    int len, d;
    CHAR_DATA *walker = bot->ch;
    bool kabal;

    argument = one_argument( argument, where );
    kabal = argument[0] != '\0' && !str_prefix( argument, "kabal" );
    if ( walker == NULL || walker->in_room == NULL )
    {
        send_to_char( "Bot çevrimiçi değil.\n\r", ch );
        return;
    }
    if ( ( to = get_room_index( atoi( where ) ) ) == NULL )
    {
        send_to_char( "Öyle bir oda yok.\n\r", ch );
        return;
    }
    for ( d = 0; d < 6; d++ )
    {
        EXIT_DATA *pexit = walker->in_room->exit[d];
        if ( pexit == NULL || pexit->u1.to_room == NULL )
            continue;
        printf_to_char( ch, "%s -> %d: %s%s\n\r", dir_name[d], pexit->u1.to_room->vnum,
                        bot_room_passable( walker, pexit->u1.to_room, kabal ) ? "geçilebilir" : "GEÇİLEMEZ",
                        IS_SET( pexit->exit_info, EX_LOCKED ) ? " (kilitli)" : "" );
    }
    len = bot_find_path( walker, walker->in_room, to, dirs, BOT_MAX_PATH, kabal );
    printf_to_char( ch, "%d -> %d: yol %d\n\r", walker->in_room->vnum, to->vnum, len );
    if ( len < 0 )
    {
        /* aynı BFS ile erişilebilen bileşeni gez, reddedilen sınır odalarını yaz */
        struct bot_bfs o;
        struct botlar_border ctx;

        memset( &o, 0, sizeof(o) );
        ctx.viewer = ch;
        ctx.shown  = 0;
        o.allow_cabal = kabal;
        o.reject      = botlar_border_reject;
        o.reject_ctx  = &ctx;
        bot_bfs( walker, walker->in_room, &o, NULL, 0 );
        printf_to_char( ch, "  erişilebilen oda sayısı: %d\n\r", o.visited );
    }
    for ( d = 0; d < len && d < 30; d++ )
        printf_to_char( ch, "%s ", dir_name[dirs[d]] );
    send_to_char( "\n\r", ch );
}

/* botlar <isim>: durum ve istatistik */
static void botlar_show( CHAR_DATA *ch, BOT_DATA *bot )
{
    CHAR_DATA *bch = bot->ch;
    CHAR_DATA *revenge;

    printf_to_char( ch, "{W%s{x  ırk %s, sınıf %s, kişilik %d, saat %02d-%02d, giriş %d kez\n\r",
                    bot->name, race_table[bot->race].name[1], class_table[bot->iclass].name[1],
                    bot->kisilik, bot->hour_from, bot->hour_to, bot->logins );
    if ( bch == NULL )
    {
        printf_to_char( ch, "Çevrimdışı; sonraki giriş denemesi %lld sn sonra.\n\r",
                        (long long) ( bot->next_login_try - current_time ) );
        return;
    }
    printf_to_char( ch, "Durum %s (%d pulse), alt durum %d, yol %d/%d hedef %d, kasaba hedefi %d\n\r",
                    bot_state_name( bot->state ), bot_pulse - bot->state_pulse, bot->substate,
                    bot->path_pos, bot->path_len, bot->target_vnum, bot->town_target );
    printf_to_char( ch, "Oda %d (%s), bölge %s, av bölgesi %s\n\r",
                    bch->in_room != NULL ? bch->in_room->vnum : 0,
                    bch->in_room != NULL ? bch->in_room->name : "?",
                    bch->in_room != NULL ? bch->in_room->area->name : "?",
                    bot->hunt_area != NULL ? bot->hunt_area->name : "-" );
    printf_to_char( ch, "Tp %d (seviye başına %d)\n\r", bch->exp, exp_per_level( bch, bch->pcdata->points ) );
    printf_to_char( ch, "Yp %d/%d Mp %d/%d Zp %d/%d akçe %ld gp %d pratik %d eğitim %d\n\r",
                    bch->hit, bch->max_hit, bch->mana, bch->max_mana, bch->move, bch->max_move,
                    bch->silver, bch->pcdata->questpoints, bch->practice, bch->train );
    printf_to_char( ch, "Pozisyon %d, bekleme %d, sersemlik %d, etkiler %s, ışık %s, açlık %d, susuzluk %d\n\r",
                    bch->position, bch->wait, bch->daze, affect_bit_name( bch->affected_by ),
                    get_light_char( bch ) != NULL ? "var" : "YOK",
                    bch->pcdata->condition[COND_HUNGER], bch->pcdata->condition[COND_THIRST] );
    printf_to_char( ch, "Öldürme %d, ölüm %d, görev %d, pk %d, kasaba işleri %ld, son mob %s\n\r",
                    bot->kills, bot->deaths, bot->quests, bot->pk_kills, bot->town_tasks, bot->last_mob );
    revenge = bot_char_by_id( bot->revenge_id );
    printf_to_char( ch, "Kabal %s%s, katil hakkı %d, lider ataması %s, intikam %s, baskın %s/%d\n\r",
                    bch->cabal != CABAL_NONE ? cabal_table[bch->cabal].short_name : "-",
                    IS_SET( bch->act, PLR_CANINDUCT ) ? " (lider)" : "",
                    bch->pcdata->oyuncu_katli,
                    bot->leader_cabal > CABAL_NONE ? cabal_table[bot->leader_cabal].short_name : "-",
                    revenge != NULL ? revenge->name : "-",
                    bot->raid_cabal > CABAL_NONE ? cabal_table[bot->raid_cabal].short_name : "-", bot->raid_step );
    printf_to_char( ch, "Oturum bitişi %lld sn sonra.\n\r", (long long) ( bot->session_end - current_time ) );
}

void do_botlar( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], who[MAX_INPUT_LENGTH];
    BOT_DATA *bot;

    argument = one_argument( argument, arg );

    if ( !bots_enabled )
    {
        send_to_char( "Bot kadrosu yüklü değil (area/botlar.txt yok).\n\r", ch );
        return;
    }
    if ( arg[0] == '\0' )
    {
        botlar_list( ch );
        return;
    }
    if ( !str_cmp( arg, "debug" ) )
    {
        bot_debug = !bot_debug;
        printf_to_char( ch, "Bot hata ayıklama günlüğü %s.\n\r", bot_debug ? "açık" : "kapalı" );
        return;
    }

    /* alt komutlar bir bot adı alır; ad verilmemişse ilk sözcük bot adıdır */
    if ( !str_cmp( arg, "bağla" ) || !str_cmp( arg, "ayır" ) || !str_cmp( arg, "yol" ) || !str_cmp( arg, "av" ) )
        argument = one_argument( argument, who );
    else
        snprintf( who, sizeof(who), "%s", arg );
    if ( ( bot = bot_find( who ) ) == NULL )
    {
        send_to_char( "Öyle bir bot yok.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg, "bağla" ) )      botlar_toggle( ch, bot, TRUE );
    else if ( !str_cmp( arg, "ayır" ) )  botlar_toggle( ch, bot, FALSE );
    else if ( !str_cmp( arg, "yol" ) )   botlar_path( ch, bot, argument );
    else if ( !str_cmp( arg, "av" ) )    bot_debug_areas( ch, bot );
    else                                 botlar_show( ch, bot );
}
