/*
 * bot.h - Otonom oyuncu botları.
 *
 * Botlar gerçek oyuncu karakterleridir (pfile, pcdata, seviye, yetenek);
 * tek farkları bir soketleri (descriptor) olmamasıdır. Her oyun pulse'ında
 * bot_update() çağrılır; botlar kararlarını dünya durumuna bakarak verir ve
 * sıradan oyuncu komutlarını interpret() üzerinden çalıştırır.
 */
#ifndef BOT_H
#define BOT_H

#define BOT_MAX_PATH      600
#define BOT_MAX_REPLIES   6
#define BOT_NAME_LEN      16
#define BOT_ROSTER_FILE   "botlar.txt"     /* area/ dizininde */
#define BOT_WHO_MAX       256
#define BOT_WEAR_FAIL_MAX 8
#define BOT_AVOID_MAX     4
#define BOT_SPAWN_MAX     24
#define BOT_AREA_MEM      16
#define BOT_SEENPC_MAX      8
#define BOT_PK_AREAS      4

/* zaman: bot_pulse saniyede BOT_PULSE_SEC kez artar */
#define BOT_PULSE_SEC     4
#define BOT_SEC(n)        ( BOT_PULSE_SEC * (n) )
#define BOT_MIN(n)        ( BOT_SEC(60) * (n) )
#define BOT_HOUR(n)       ( BOT_MIN(60) * (n) )

/*
 * Veteran kuralları (bkz. CLAUDE.md "Botlar"): yp/mana eşikleri yüzde,
 * kaçınma süreleri pulse cinsindendir.
 */
#define BOT_HP_FIGHT        70    /* bu yp altında dövüşe/PK'ya/baskına girilmez */
#define BOT_MANA_FIGHT      40    /* büyücü bu mana altında dövüşe girmez */
#define BOT_HP_REST         60    /* bu yp altında dinlenmeye gidilir */
#define BOT_HP_RESTED       92    /* dinlenme bu yp'ye dek sürer */
#define BOT_HP_FLEE         32    /* kaçış eşiği (savaşçı sınıflar) */
#define BOT_HP_FLEE_CASTER  40    /* kaçış eşiği (büyücü sınıflar) */
#define BOT_HP_LOSING       55    /* rakip sağlamken bu yp altında erken çıkış */
#define BOT_HP_PK_MIN       50    /* PK/baskın bu yp altında sürdürülmez */
#define BOT_HP_RAID_QUIT    35    /* baskından geri çekilme */
#define BOT_HP_IDLE_PK      85    /* boştayken PK/baskın planlaması için asgari yp */
#define BOT_AVOID_AREA      BOT_HOUR(3)   /* ölünen bölge (tek ölüm) */
#define BOT_AVOID_AREA_MULTI BOT_HOUR(12) /* ölünen bölge (2+ ölüm) */
#define BOT_AVOID_MOB       BOT_HOUR(6)   /* öldüren yaratık türü */
#define BOT_AVOID_ROOM      BOT_HOUR(3)   /* ölünen oda */
#define BOT_AVOID_SCOUT     BOT_MIN(20)   /* yolda görülen güçlü saldırganın odası */
#define BOT_RAID_FAIL_AVOID BOT_HOUR(12)  /* baskında ölüm: hedef kabal */
#define BOT_REST_MAX        BOT_MIN(14)   /* dinlenme en çok */
#define BOT_TRAVEL_MAX      BOT_MIN(12)   /* bir yolculuk en çok */

/* yüzde ve rastgele havuz seçimi (tüm bot dosyalarında ortak) */
#define BOT_PN(a)         ( (int) ( sizeof(a) / sizeof(a[0]) ) )
#define BOT_PICK(a)       bot_pick( (a), BOT_PN(a) )

static inline int bot_pct( int cur, int max )
{
    return max <= 0 ? 100 : cur * 100 / max;
}

static inline const char *bot_pick( const char **pool, int n )
{
    return pool[number_range( 0, n - 1 )];
}

/* durumlar */
#define BOT_ST_IDLE       0
#define BOT_ST_TRAVEL     1
#define BOT_ST_HUNT       2
#define BOT_ST_REST       3
#define BOT_ST_TOWN       4
#define BOT_ST_QUEST      5
#define BOT_ST_CORPSE     6
#define BOT_ST_FOLLOW     7
#define BOT_ST_PK         8
#define BOT_ST_LOGOUT     9
#define BOT_ST_MEET       10
#define BOT_ST_RAID       11

/* kişilikler */
#define BOT_K_SAKIN       0
#define BOT_K_GEVEZE      1
#define BOT_K_AGRESIF     2
#define BOT_K_YARDIMSEVER 3
#define BOT_K_ESPRILI     4
#define BOT_K_GIZEMLI     5
#define BOT_K_ACEMI       6
#define BOT_K_BILGE       7
#define BOT_K_MAX         8

/* duyma kanalları */
#define BOT_CH_SAY        0
#define BOT_CH_TELL       1
#define BOT_CH_YELL       2
#define BOT_CH_GTELL      3
#define BOT_CH_CABAL      4
#define BOT_CH_SOCIAL     5
#define BOT_CH_EMOTE      6
#define BOT_CH_KDG        7
#define BOT_CH_PRAY       8
#define BOT_CH_IMM        9

/* olaylar */
#define BOT_EV_LEVEL      0
#define BOT_EV_DEATH      1
#define BOT_EV_KILL       2
#define BOT_EV_LOGIN      3
#define BOT_EV_QUEST_DONE 4
#define BOT_EV_QUEST_GET  5
#define BOT_EV_LOOT       6
#define BOT_EV_FLEE       7
#define BOT_EV_CABAL      8
#define BOT_EV_PK_KILL    9
#define BOT_EV_HUMAN_LOGIN 10
#define BOT_EV_HUMAN_ROOM 11
#define BOT_EV_LOGOUT     12
#define BOT_EV_PK_TAUNT   13
#define BOT_EV_MAX        14

/* kasaba işleri (bit) */
#define BOT_TOWN_SELL       (A)
#define BOT_TOWN_PRACTICE   (B)
#define BOT_TOWN_TRAIN      (C)
#define BOT_TOWN_POTION     (D)
#define BOT_TOWN_FOOD       (E)
#define BOT_TOWN_DRINK      (F)
#define BOT_TOWN_LIGHT      (G)
#define BOT_TOWN_HEAL       (H)
#define BOT_TOWN_QUEST_GET  (I)
#define BOT_TOWN_QUEST_DONE (J)
#define BOT_TOWN_QUEST_BUY  (K)
#define BOT_TOWN_FOUNTAIN   (L)
#define BOT_TOWN_UPGRADE    (M)

/*
 * Oda grafiğinde genişlik öncelikli gezinti (tek çekirdek: yol bulma, yakın oda
 * taraması, avcı yaratık takibi ve tanı çıktısı). Bkz. bot_bfs().
 */
typedef bool BOT_BFS_VISIT ( ROOM_INDEX_DATA *room, int dist, void *ctx );
typedef void BOT_BFS_REJECT( ROOM_INDEX_DATA *room, int dir, ROOM_INDEX_DATA *next,
                             const char *why, void *ctx );

struct bot_bfs
{
    ROOM_INDEX_DATA *to;          /* hedef oda; NULL ise gezinti */
    int         max_depth;        /* 0: sınırsız */
    int         max_rooms;        /* 0: sınırsız (kuyruğa alınan oda sayısı) */
    bool        same_area;        /* başlangıç bölgesinin dışına açılma */
    bool        allow_cabal;      /* bot_room_passable: kabal bölgesine gir */
    bool        raw;              /* yaratık takibi: geçilebilirlik/kilit kuralı yok */
    bool        block_closed;     /* raw: kapalı kapıdan geçme */
    BOT_BFS_VISIT  *visit;        /* her oda için (başlangıç dahil); FALSE: dur */
    void *      visit_ctx;
    BOT_BFS_REJECT *reject;       /* reddedilen kenar (tanı) */
    void *      reject_ctx;
    /* sonuç */
    int         visited;          /* gezilen oda sayısı */
    int         first_dir;        /* hedefe ilk adım (-1: yok) */
};

struct bot_area_mem
{
    AREA_DATA * area;
    int         kills;
    int         deaths;
    int         last_pulse;
};

struct bot_reply
{
    char        to[BOT_NAME_LEN];
    int         channel;
    int         when;                  /* bot_pulse cinsinden */
    char        text[MAX_INPUT_LENGTH];
};

struct bot_data
{
    BOT_DATA *  next;

    /* kadro dosyasından */
    char        name[BOT_NAME_LEN];
    int         race;
    int         iclass;
    int         sex;
    int         align;
    int         ethos;
    int         kisilik;
    int         hour_from;
    int         hour_to;
    bool        turkce;
    bool        kucuk_harf;
    char *      lakap;
    bool        disabled;
    bool        pk_istekli;
    int         leader_cabal;      /* kadro: tanrıların atadığı liderlik */
    int         god_level;         /* kadro: !tanrı <bot> <seviye> (0 = ölümlü) */

    /* çalışma zamanı */
    CHAR_DATA * ch;
    int         state;
    int         substate;
    int         state_pulse;
    int         next_think;
    time_t      login_time;
    time_t      session_end;
    time_t      next_login_try;
    int         logins;
    int         last_level;
    int         busy_until;            /* komut arası bekleme (pulse) */

    /* seyahat */
    int         target_vnum;
    int         after_travel;
    sh_int      path[BOT_MAX_PATH];
    int         path_len;
    int         path_pos;
    int         stuck;
    int         last_room_vnum;
    int         travel_started;

    /* av */
    AREA_DATA * hunt_area;
    int         hunt_area_pulse;
    int         hunt_fail;
    int         nopath_pulse;
    long        target_id;
    int         kills;
    int         last_kill_exp;
    char        last_opp[96];
    int         last_opp_vnum;
    int         deaths;
    int         quests;
    int         pk_kills;
    int         fled_pulse;
    int         last_fight_pulse;
    int         rest_reason;

    /* görev */
    int         quest_stage;
    int         quest_tries;
    int         quest_room_hint;

    /* ceset */
    int         death_room;
    int         corpse_tries;

    /* kasaba */
    long        town_tasks;
    int         town_step;
    int         town_target;           /* hedef oda vnum */
    long        town_keeper_id;
    int         last_town_pulse;
    int         practice_block_until;
    int         town_retry[16];
    int         inv_signature;
    int         wear_fail[BOT_WEAR_FAIL_MAX];
    int         wear_fail_pos;
    int         avoid_vnum[BOT_AVOID_MAX];
    int         avoid_until[BOT_AVOID_MAX];
    int         avoid_pos;
    int         avoid_room[BOT_AVOID_MAX];   /* tehlikeli/ölümcül odalar */
    int         avoid_room_until[BOT_AVOID_MAX];
    int         avoid_room_pos;
    /* algı hafızası */
    int         spawn_vnum[BOT_SPAWN_MAX];
    int         spawn_seen[BOT_SPAWN_MAX];
    int         spawn_visit[BOT_SPAWN_MAX];
    int         spawn_pos;
    unsigned char *visited;
    AREA_DATA * visited_area;
    struct bot_area_mem area_mem[BOT_AREA_MEM];

    /* sohbet */
    struct bot_reply replies[BOT_MAX_REPLIES];
    int         next_chat;
    int         next_kd;
    int         next_social;
    int         last_bot_talk;
    char        last_speaker[BOT_NAME_LEN];
    char        last_mob[80];
    char        last_item[80];
    char        last_human[BOT_NAME_LEN];
    int         greeted_pulse;

    /* takip / grup */
    long        leader_id;
    long        meet_id;
    int         meet_until;
    int         hold_until;        /* konuşmak için durdu (odada kal) */
    int         fill_block_until;  /* çeşmede doldurma başarısızsa bekle */
    int         fill_vnum;
    int         fill_amount;
    /* kabal yaşamı */
    int         cabal_ask_pulse;
    long        induct_id;
    int         induct_pulse;
    long        revenge_id;
    int         revenge_pulse;
    int         help_call_pulse;
    AREA_DATA * pk_areas[BOT_PK_AREAS];
    int         pk_area_n;
    int         pk_area_i;
    long        seen_id[BOT_SEENPC_MAX];
    int         seen_vnum[BOT_SEENPC_MAX];
    int         seen_pulse[BOT_SEENPC_MAX];
    int         raid_cabal;
    long        raid_leader_id;
    int         raid_pulse;
    int         raid_step;
    int         opp_pk_pulse;      /* son fırsat PK kararı */
    int         pk_tries;          /* dövüş başlamayan saldırı denemeleri */
    int         next_raid_check;   /* baskın değerlendirme zamanı (PK'dan bağımsız) */
    /* tanrı botu */
    int         god_next_act;
    int         god_next_gecho;
    long        god_pending_id;
    int         god_pending_kind;
    int         god_pending_pulse;
    int         god_last_restore;
    int         follow_since;
    int         follow_until;
    int         leader_last_action;
    int         group_offer_pulse;
    long        invite_id;
    int         invite_pulse;

    /* PK / kabal */
    long        pk_target_id;
    int         pk_until;
    int         next_pk;
    int         cabal_check_pulse;
};

extern BOT_DATA *   bot_list;
extern bool         bots_enabled;
extern int          bot_pulse;
extern int          bot_min_online;
extern int          bot_max_online;
extern bool         bot_debug;

/* bot.c */
void    bot_boot            ( void );
void    bot_update          ( void );
void    bot_login           ( BOT_DATA *bot );
void    bot_logout          ( BOT_DATA *bot, bool force );
void    bot_hear            ( CHAR_DATA *listener, CHAR_DATA *speaker, int channel, const char *text );
void    bot_on_death        ( CHAR_DATA *victim );
void    bot_on_kill         ( CHAR_DATA *killer, CHAR_DATA *victim );
void    bot_pcdata_freed    ( PC_DATA *pcdata );
bool    bot_name_taken      ( const char *name );
int     bot_online_count    ( void );
int     bot_who_collect     ( CHAR_DATA **dch, CHAR_DATA **wch, int max );
BOT_DATA *bot_of            ( CHAR_DATA *ch );
BOT_DATA *bot_find          ( const char *name );
void    bot_cmd             ( BOT_DATA *bot, const char *fmt, ... );
void    bot_log             ( BOT_DATA *bot, const char *fmt, ... );
void    bot_set_state       ( BOT_DATA *bot, int state );
const char *bot_state_name  ( int state );
void    bot_queue_reply     ( BOT_DATA *bot, const char *to, int channel, int delay, const char *text );
int     bot_find_path       ( CHAR_DATA *ch, ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to,
                              sh_int *dirs, int max, bool allow_cabal );
int     bot_bfs             ( CHAR_DATA *ch, ROOM_INDEX_DATA *from, struct bot_bfs *o,
                              sh_int *dirs, int max );
bool    bot_room_passable   ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool allow_cabal );
bool    bot_exit_back       ( ROOM_INDEX_DATA *next, ROOM_INDEX_DATA *room );
bool    bot_set_travel      ( BOT_DATA *bot, int vnum, int after );
CHAR_DATA *bot_char_by_id   ( long id );
CHAR_DATA *bot_room_char_by_id ( ROOM_INDEX_DATA *room, long id );
bool    bot_is_human        ( CHAR_DATA *ch );
CHAR_DATA *bot_random_human ( void );
void    bot_char_keyword    ( CHAR_DATA *viewer, CHAR_DATA *victim, char *buf, size_t size );
void    bot_obj_keyword     ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *list, char *buf, size_t size );
int     bot_hour            ( void );

/* bot_brain.c */
void    bot_think           ( BOT_DATA *bot );
void    bot_brain_boot      ( void );
void    bot_brain_login     ( BOT_DATA *bot, bool fresh );
bool    bot_wants_group_with( BOT_DATA *bot, CHAR_DATA *other );
void    bot_start_follow    ( BOT_DATA *bot, CHAR_DATA *leader );
void    bot_offer_meeting   ( BOT_DATA *bot, CHAR_DATA *other );
void    bot_stop_follow     ( BOT_DATA *bot, bool say );
bool    bot_in_group        ( CHAR_DATA *ch );
const char *bot_area_name   ( BOT_DATA *bot );
void    bot_note_loot       ( BOT_DATA *bot );
void    bot_note_kill       ( BOT_DATA *bot, CHAR_DATA *victim );
bool    bot_room_avoided    ( BOT_DATA *bot, ROOM_INDEX_DATA *room );
void    bot_avoid_room      ( BOT_DATA *bot, ROOM_INDEX_DATA *room, int pulses );
/* bot_brain.c dışa açılanlar */
void    bot_attack          ( BOT_DATA *bot, CHAR_DATA *victim );
bool    bot_cast_buffs      ( BOT_DATA *bot );
ROOM_INDEX_DATA *area_entry_room( BOT_DATA *bot, AREA_DATA *area );
bool    bot_escape_pocket   ( BOT_DATA *bot );
/* bot_war.c */
bool    bot_cabal_fits      ( CHAR_DATA *ch, int cabal );
int     bot_choose_cabal    ( CHAR_DATA *ch );
bool    bot_leader_handle   ( BOT_DATA *leader, CHAR_DATA *speaker, int channel );
bool    bot_war_opportunity ( BOT_DATA *bot );
void    bot_war_attacked    ( BOT_DATA *bot, CHAR_DATA *attacker );
bool    bot_war_help        ( BOT_DATA *bot, CHAR_DATA *speaker, const char *text );
void    bot_war_note_room   ( BOT_DATA *bot );
void    bot_pk              ( BOT_DATA *bot );
void    bot_raid            ( BOT_DATA *bot );
bool    bot_war_goal        ( BOT_DATA *bot );
void    bot_war_tick        ( BOT_DATA *bot );
void    bot_cabal_alarm     ( int cabal, CHAR_DATA *thief );
bool    bot_raid_scout      ( BOT_DATA *bot, ROOM_INDEX_DATA *next );
/* bot_god.c */
bool    bot_is_god          ( BOT_DATA *bot );
void    bot_god_enter       ( BOT_DATA *bot, CHAR_DATA *ch, bool fresh );
void    bot_god_think       ( BOT_DATA *bot );
void    bot_god_hear        ( BOT_DATA *bot, CHAR_DATA *speaker, int channel, const char *text );
void    bot_god_greet       ( BOT_DATA *bot, CHAR_DATA *human );
void    bot_raid_failed     ( BOT_DATA *bot );
void    bot_after_death     ( BOT_DATA *bot );
void    bot_debug_areas     ( CHAR_DATA *viewer, BOT_DATA *bot );
int     bot_sell_candidates ( BOT_DATA *bot );
int     bot_count_potions   ( CHAR_DATA *ch );

/* bot_chat.c */
void    bot_chat_boot       ( void );
void    bot_chat_idle       ( BOT_DATA *bot );
void    bot_chat_react      ( BOT_DATA *bot, CHAR_DATA *speaker, int channel, const char *text );
void    bot_chat_event      ( BOT_DATA *bot, int event, CHAR_DATA *other );
void    bot_style           ( BOT_DATA *bot, const char *in, char *out, size_t size );
void    bot_style_ch        ( BOT_DATA *bot, const char *in, char *out, size_t size, bool ic );
void    bot_fill            ( BOT_DATA *bot, const char *tmpl, CHAR_DATA *other, char *out, size_t size );
void    bot_fill_ch         ( BOT_DATA *bot, const char *tmpl, CHAR_DATA *other, char *out, size_t size, bool ic );
void    bot_talk            ( BOT_DATA *bot, int channel, CHAR_DATA *to, const char *text );
/* Grup 08 */
void    bot_lower_ascii     ( char *s );
CHAR_DATA *bot_human_immortal( void );

#endif /* BOT_H */
