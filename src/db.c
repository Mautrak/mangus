/***************************************************************************
 *                                                                         *
 * Uzak Diyarlar açık kaynak Türkçe Mud projesidir.                        *
 * Oyun geliştirmesi Jai ve Maru tarafından yönetilmektedir.               *
 * Unutulmaması gerekenler: Nir, Kame, Randalin, Nyah, Sint                          *
 *                                                                         *
 * Github  : https://github.com/yelbuke/UzakDiyarlar                       *
 * Web     : http://www.uzakdiyarlar.net                                   *
 * Discord : https://discord.gg/kXyZzv                                     *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>

#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
#include "utf8.h"

void load_limited_objects( void );
static uint32_t rng_next( void );
#define RNG_MAX 0x7fffffffL


/* externals for counting purposes */
extern	OBJ_DATA	*obj_free;
extern	CHAR_DATA	*char_free;
extern  DESCRIPTOR_DATA *descriptor_free;
extern	PC_DATA		*pcdata_free;
extern  AFFECT_DATA	*affect_free;

/*
 * Globals.
 */
HELP_DATA *		help_first;
HELP_DATA *		help_last;

SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;

char			bug_buf		[2*MAX_INPUT_LENGTH];
CHAR_DATA *		char_list;
char *			help_greeting;
char			log_buf		[2*MAX_INPUT_LENGTH];
KILL_DATA		kill_table	[MAX_LEVEL];
OBJ_DATA *		object_list;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

AUCTION_DATA	*	auction;
ROOM_INDEX_DATA	*	top_affected_room;
int			reboot_counter;
int			time_sync;
int			max_newbies;
int			max_oldies;
int			iNumPlayers;
int     ikikat_tp;
int     ikikat_gp;



/*
 * for limited objects
 */
long                    total_levels;

sh_int			gsn_backstab;
sh_int			gsn_dodge;
sh_int			gsn_envenom;
sh_int			gsn_hide;
sh_int			gsn_peek;
sh_int			gsn_pick_lock;
sh_int			gsn_sneak;
sh_int			gsn_steal;

sh_int			gsn_disarm;
sh_int			gsn_enhanced_damage;
sh_int			gsn_kick;
sh_int			gsn_parry;
sh_int			gsn_rescue;
sh_int			gsn_second_attack;
sh_int			gsn_third_attack;

sh_int			gsn_blindness;
sh_int			gsn_charm_person;
sh_int			gsn_curse;
sh_int			gsn_remove_curse;
sh_int			gsn_invis;
sh_int			gsn_mass_invis;
sh_int			gsn_poison;
sh_int			gsn_plague;
sh_int			gsn_sleep;
sh_int			gsn_sanctuary;
sh_int			gsn_fly;
sh_int			gsn_death_protection;

sh_int  gsn_fourth_attack;
sh_int  gsn_dual_backstab;
sh_int  gsn_cleave;
sh_int  gsn_counter;
sh_int  gsn_ambush;
sh_int  gsn_camouflage;
sh_int  gsn_circle;
sh_int  gsn_nerve;
sh_int  gsn_endure;
sh_int  gsn_quiet_movement;
sh_int  gsn_herbs;
sh_int  gsn_tame;
sh_int  gsn_butcher;
sh_int  gsn_assassinate;
sh_int  gsn_wanted;
sh_int  gsn_caltraps;
sh_int  gsn_throw;
sh_int  gsn_strangle;
sh_int  gsn_blackjack;
sh_int  gsn_bloodthirst;
sh_int  gsn_spellbane;
sh_int  gsn_resistance;
sh_int  gsn_deathblow;
sh_int  gsn_doppelganger;
sh_int  gsn_fade;
sh_int  gsn_garble;
sh_int  gsn_confuse;
sh_int  gsn_mirror;
sh_int  gsn_track;
sh_int  gsn_vanish;
sh_int  gsn_chaos_blade;
sh_int  gsn_terangreal;
sh_int  gsn_dispel_evil;
sh_int  gsn_dispel_good;
sh_int  gsn_wrath;
sh_int  gsn_stalker;
sh_int  gsn_tesseract;
sh_int  gsn_randomizer;
sh_int  gsn_trophy;
sh_int  gsn_truesight;
sh_int  gsn_brew;
sh_int  gsn_flamestrike;
sh_int  gsn_shadowlife;
sh_int  gsn_ruler_badge;
sh_int  gsn_remove_badge;
sh_int  gsn_dragon_strength;
sh_int  gsn_dragon_breath;
sh_int  gsn_warcry;
sh_int  gsn_entangle;
sh_int  gsn_dragonsword;
sh_int  gsn_knight;
sh_int  gsn_guard;
sh_int  gsn_guard_call;
sh_int  gsn_love_potion;
sh_int  gsn_deafen;
sh_int  gsn_protective_shield;
sh_int  gsn_protection_heat_cold;
sh_int  gsn_trance;
sh_int  gsn_demon_summon;
sh_int  gsn_nightwalker;
sh_int  gsn_bear_call;
sh_int  gsn_squire;
sh_int  gsn_lightning_bolt;
sh_int  gsn_disperse;
sh_int  gsn_bless;
sh_int  gsn_weaken;
sh_int  gsn_haste;
sh_int  gsn_cure_critical;
sh_int  gsn_cure_serious;
sh_int  gsn_burning_hands;
sh_int  gsn_acid_blast;
sh_int  gsn_ray_of_truth;
sh_int  gsn_spell_craft;
sh_int  gsn_giant_strength;
sh_int  gsn_explode;
sh_int  gsn_acid_breath;
sh_int  gsn_fire_breath;
sh_int  gsn_frost_breath;
sh_int  gsn_gas_breath;
sh_int  gsn_lightning_breath;
sh_int	gsn_cure_light;
sh_int	gsn_magic_missile;
sh_int	gsn_demonfire;
sh_int	gsn_faerie_fire;
sh_int	gsn_shield;
sh_int	gsn_chill_touch;
sh_int	gsn_second_weapon;
sh_int	gsn_target;
sh_int	gsn_sand_storm;
sh_int	gsn_scream;
sh_int	gsn_enchant_sword;
sh_int	gsn_tiger_power;
sh_int	gsn_lion_call;
sh_int  gsn_hara_kiri;
sh_int  gsn_mouse;
sh_int  gsn_enhanced_armor;
sh_int  gsn_vampire;
sh_int  gsn_evolve_bear;
sh_int  gsn_vampiric_bite;
sh_int  gsn_light_res;
sh_int  gsn_blink;
sh_int  gsn_hunt;
sh_int  gsn_path_find;
sh_int  gsn_critical;
sh_int  gsn_detect_sneak;
sh_int  gsn_mend;
sh_int  gsn_shielding;
sh_int  gsn_blind_fighting;
sh_int  gsn_riding;
sh_int  gsn_smithing;
sh_int  gsn_swimming;
sh_int  gsn_move_camf;
sh_int  gsn_protection_heat;
sh_int  gsn_protection_cold;
sh_int  gsn_teleport;
sh_int  gsn_witch_curse;
sh_int	gsn_kassandra;
sh_int	gsn_sebat;
sh_int	gsn_matandra;
sh_int  gsn_armor_use;
sh_int	gsn_world_find;
sh_int  gsn_vampiric_touch;
sh_int  gsn_cure_poison;
sh_int  gsn_fire_shield;
sh_int  gsn_fear;
sh_int  gsn_settraps;
sh_int  gsn_mental_attack;
sh_int  gsn_secondary_attack;
sh_int  gsn_mortal_strike;
sh_int  gsn_shield_cleave;
sh_int  gsn_weapon_cleave;
sh_int  gsn_slow;
sh_int  gsn_imp_invis;
sh_int  gsn_camp;
sh_int  gsn_push;
sh_int  gsn_tail;
sh_int	gsn_power_stun;
sh_int	gsn_cabal_recall;
sh_int	gsn_escape;
sh_int	gsn_lay_hands;
sh_int	gsn_grip;
sh_int	gsn_concentrate;
sh_int	gsn_master_sword;
sh_int	gsn_master_hand;
sh_int	gsn_fifth_attack;
sh_int	gsn_area_attack;
sh_int	gsn_reserved;
sh_int	gsn_bandage;
sh_int	gsn_web;
sh_int	gsn_bow;
sh_int	gsn_bash_door;
sh_int	gsn_katana;
sh_int  gsn_bluefire;
sh_int	gsn_crush;
sh_int	gsn_perception;
sh_int	gsn_control_animal;
sh_int	gsn_sanctify_lands;
sh_int	gsn_deadly_venom;
sh_int	gsn_cursed_lands;
sh_int	gsn_lethargic_mist;
sh_int	gsn_black_death;
sh_int	gsn_mysterious_dream;
sh_int	gsn_sense_life;
sh_int	gsn_arrow;
sh_int	gsn_lance;
sh_int	gsn_evil_spirit;
sh_int	gsn_make_arrow;
sh_int	gsn_green_arrow;
sh_int	gsn_red_arrow;
sh_int	gsn_white_arrow;
sh_int	gsn_blue_arrow;
sh_int	gsn_make_bow;
sh_int	gsn_blindness_dust;
sh_int	gsn_poison_smoke;
sh_int	gsn_mastering_spell;
sh_int	gsn_judge;
sh_int  gsn_hand_block;
sh_int  gsn_cross_block;
sh_int	gsn_ground_strike;
sh_int	gsn_neckguard;
sh_int	gsn_lash;
sh_int	gsn_claw;
sh_int	gsn_absorb;
sh_int	gsn_earthfade;
sh_int	gsn_headguard;
sh_int	gsn_blackguard;
sh_int	gsn_mental_knife;

/* new_gsns */

sh_int  		gsn_axe;
sh_int  		gsn_dagger;
sh_int  		gsn_flail;
sh_int  		gsn_mace;
sh_int  		gsn_polearm;
sh_int			gsn_shield_block;
sh_int  		gsn_spear;
sh_int  		gsn_sword;
sh_int  		gsn_whip;

sh_int  		gsn_bash;
sh_int  		gsn_berserk;
sh_int  		gsn_dirt;
sh_int  		gsn_hand_to_hand;
sh_int  		gsn_trip;

sh_int  		gsn_fast_healing;
sh_int  		gsn_haggle;
sh_int  		gsn_lore;
sh_int  		gsn_meditation;

sh_int  		gsn_scrolls;
sh_int  		gsn_staves;
sh_int  		gsn_wands;
sh_int  		gsn_recall;
sh_int  		gsn_detect_hidden;
sh_int  gsn_pass_door;



/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];
char *			string_hash		[MAX_KEY_HASH];

AREA_DATA *		area_first;
AREA_DATA *		area_last;

char *			string_space;
char *			top_string;
char			str_empty	[1];

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int 			mobile_count = 0;
int			newmobs = 0;
int			newobjs = 0;


/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
#define			MAX_STRING	8000000
#define			MAX_PERM_BLOCK	150000
#define			MAX_MEM_LIST	11

void *			rgFreeList	[MAX_MEM_LIST];
const int		rgSizeList	[MAX_MEM_LIST]	=
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64
};

int			nAllocString;
int			sAllocString;
int			nAllocPerm;
int			sAllocPerm;



/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];
AREA_DATA		*Serarea;	/* currently read area */


/*
 * Local booting procedures.
*/
void    init_random_number_generator  ( void );
void	load_areadata	( FILE *fp );
void	load_helps	( FILE *fp );
void    load_omprogs    ( FILE *fp );
void 	load_new_mobiles	( FILE *fp );
void	load_old_obj	( FILE *fp );
void	load_new_old_obj	( FILE *fp );
void 	load_objects	( FILE *fp );
void 	load_new_objects	( FILE *fp );
void	load_resets	( FILE *fp );
void	load_rooms	( FILE *fp );
void	load_shops	( FILE *fp );
void 	load_socials	( FILE *fp );
void	load_specials	( FILE *fp );
void	load_bans	( void );
void    load_olimits    ( FILE *fp );
void    load_practicer  ( FILE *fp );
void    load_resetmsg   ( FILE *fp );
void    load_aflag	( FILE *fp );

void	fix_exits	( void );

void	reset_area	( AREA_DATA * pArea );

int	wear_convert	( int oldwear );

/*
 * Big mama top level function.
 */
void boot_db( void )
{
    char buf[MAX_STRING_LENGTH];

    /*
     * Init some data space stuff.
     */
    {
	if ( ( string_space = (char *)calloc( 1, MAX_STRING ) ) == NULL )
	{
	    bug( "Boot_db: can't alloc %d string space.", MAX_STRING );
	    exit( 1 );
	}
	top_string	= string_space;
	fBootDb		= TRUE;
    }

    /*
     * Init random number generator.
     */
    {
        init_random_number_generator( );
    }

    ikikat_tp = 0;
    ikikat_gp = 0;

    ud_data_read();

    /*
     * Set time and weather.
     */
    {
	time_info.year  = 1;
	time_info.month = 1;
	time_info.day   = 1;
	time_info.hour  = 0;
	game_time_update();

	     if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
	else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
	else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
	else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
	else                            weather_info.sunlight = SUN_DARK;

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if ( time_info.month >= 7 && time_info.month <=12 )
	    weather_info.mmhg += number_range( 1, 50 );
	else
	    weather_info.mmhg += number_range( 1, 80 );

	     if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
	else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
	else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
	else                                  weather_info.sky = SKY_CLOUDLESS;

    }

	/* auction */

	auction = (AUCTION_DATA *) malloc (sizeof(AUCTION_DATA));
	if (auction == NULL)
	 {
	  bug("malloc'ing AUCTION_DATA didn't give %d bytes",(int) sizeof(AUCTION_DATA));
	  exit(1);
	 }

        auction->item = NULL; /* nothing is being sold */

	/* room_affect_data */
	top_affected_room = NULL;

	/* reboot counter */
	reboot_counter	= -1;	/* no default reboot */
	time_sync	= 0;	/* time_sync is not set */

	max_newbies	= MAX_NEWBIES;
	max_oldies	= MAX_OLDIES;
	iNumPlayers	= 0;

    /*
     * Assign gsn's for skills which have them.
     */
    {
	int sn;

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].pgsn != NULL )
		*skill_table[sn].pgsn = sn;
	}
    }

    /*
     * Read in all the area files.
     */
    {
	FILE *fpList;

	if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
	{
	    perror( AREA_LIST );
	    exit( 1 );
	}

	for ( ; ; )
	{
	    snprintf( strArea, sizeof(strArea), "%s", fread_word( fpList ) );
	    if ( strArea[0] == '$' )
		break;

	    if ( strArea[0] == '-' )
	    {
		fpArea = stdin;
	    }
	    else
	    {
		if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
		{
		    perror( strArea );
		    exit( 1 );
		}
	    }

	    for ( ; ; )
	    {
		static const struct { const char *name; void (*fn)( FILE *fp ); }
		section_table[] =
		{
		    { "AREADATA",	load_areadata	},
		    { "HELPS",		load_helps	},
		    { "NEW_MOBILES",	load_new_mobiles },
		    { "OBJOLD",		load_old_obj	},
		    { "NEW_OBJOLD",	load_new_old_obj },
		    { "OBJECTS",	load_objects	},
		    { "NEW_OBJECTS",	load_new_objects },
		    { "RESETS",		load_resets	},
		    { "ROOMS",		load_rooms	},
		    { "SHOPS",		load_shops	},
		    { "SOCIALS",	load_socials	},
		    { "OMPROGS",	load_omprogs	},
		    { "OLIMITS",	load_olimits	},
		    { "SPECIALS",	load_specials	},
		    { "PRACTICERS",	load_practicer	},
		    { "RESETMESSAGE",	load_resetmsg	},
		    { "FLAG",		load_aflag	},
		};
		char *word;
		size_t i;

		if ( fread_letter( fpArea ) != '#' )
		{
		    bug( "Boot_db: # not found.", 0 );
		    exit( 1 );
		}

		word = fread_word( fpArea );
		if ( word[0] == '$' )
		    break;

		for ( i = 0; i < sizeof(section_table) / sizeof(section_table[0]); i++ )
		    if ( !str_cmp( word, section_table[i].name ) )
		    {
			section_table[i].fn( fpArea );
			break;
		    }

		if ( i == sizeof(section_table) / sizeof(section_table[0]) )
		{
		    bug( "Boot_db: bad section name.", 0 );
		    exit( 1 );
		}
	    }

	    if ( fpArea != stdin )
		fclose( fpArea );
	    fpArea = NULL;
	}
	fclose( fpList );
	Serarea = NULL;
    }

    {
      fix_exits( );
      load_limited_objects();
      snprintf(buf, sizeof(buf),"Total non-immortal levels > 5: %li",total_levels);
      log_string(buf);

      fBootDb	= FALSE;
      area_update( );
      load_bans();
    }

    return;
}

// Bolge bilgilerini biraz daha detaylandiralim.

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void load_areadata( FILE *fp )
{
	AREA_DATA *pArea;
    char *word;
	bool fMatch;

	pArea		= (AREA_DATA *)alloc_perm( sizeof(*pArea) );

	pArea->reset_first	= NULL;
    pArea->reset_last	= NULL;
	pArea->age		= 15;
    pArea->nplayer	= 0;
    pArea->empty	= FALSE;
    pArea->count	= 0;
    pArea->resetmsg	= NULL;
    pArea->area_flag	= 0;
	pArea->language	= NULL;
	pArea->translator	= NULL;
	pArea->path	= NULL;
  pArea->yonelim_iyi	= 33;
  pArea->yonelim_yansiz	= 34;
  pArea->yonelim_kem	= 33;

    if ( area_first == NULL )
		area_first = pArea;
    if ( area_last  != NULL )
		area_last->next = pArea;
    area_last	= pArea;
    pArea->next	= NULL;
    Serarea = pArea;
	top_area++;

	while (true)
	{
		word   = (char *)(feof( fp ) ? "END" : fread_word( fp ));
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
			case 'B':
				KEY( "Builder",		pArea->writer,		fread_string( fp ) );
			break;
			case 'E':
				if ( !str_cmp( word, "END" ) )
				{
					return;
				}
			break;
			case 'F':
				KEY( "Filename",		pArea->file_name,		fread_string( fp ) );
			break;
			case 'H':
				KEY( "Highlevel",		pArea->high_range,		fread_number( fp ) );
			break;
			case 'L':
				KEY( "Language",		pArea->language,		fread_string( fp ) );
				KEY( "Lowlevel",		pArea->low_range,		fread_number( fp ) );
			break;
			case 'M':
				KEY( "Minvnum",		pArea->min_vnum,		fread_number( fp ) );
				KEY( "Maxvnum",		pArea->max_vnum,		fread_number( fp ) );
			break;
			case 'N':
				KEY( "Name",		pArea->name,		fread_string( fp ) );
			break;
			case 'P':
				KEY( "Path",		pArea->path,		fread_string( fp ) );
			break;
			case 'T':
				KEY( "Translator",		pArea->translator,		fread_string( fp ) );
			break;
      case 'Y':
				KEY( "Yonelim_iyi",		pArea->yonelim_iyi,		fread_number( fp ) );
        KEY( "Yonelim_yansiz",		pArea->yonelim_yansiz,		fread_number( fp ) );
        KEY( "Yonelim_kem",		pArea->yonelim_kem,		fread_number( fp ) );
			break;
		}

		if ( !fMatch )
		{
			bug( "Fread_areadata: no match.", 0 );
			fread_to_eol( fp );
			return;
		}
	}
}



/*
 * Snarf a help section.
 */
void load_helps( FILE *fp )
{
    HELP_DATA *pHelp;

    for ( ; ; )
    {
	pHelp		= (HELP_DATA *)alloc_perm( sizeof(*pHelp) );
	pHelp->level	= fread_number( fp );
	pHelp->keyword	= fread_string( fp );
	if ( pHelp->keyword[0] == '$' )
	    break;
	pHelp->text	= fread_string( fp );

	if ( !str_cmp( pHelp->keyword, "merhaba" ) )
	    help_greeting = pHelp->text;

	if ( help_first == NULL )
	    help_first = pHelp;
	if ( help_last  != NULL )
	    help_last->next = pHelp;

	help_last	= pHelp;
	pHelp->next	= NULL;
	top_help++;
    }

    return;
}

/*
 * Snarf an obj section.  old style (OBJOLD sabit, NEW_OBJOLD rastgele eşya).
 */
static void load_old_obj_section( FILE *fp, bool random_object )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( ; ; )
    {
	int vnum;

	if ( ( vnum = read_obj_vnum( fp, "Load_old_objects" ) ) == 0 )
	    break;

	pObjIndex			= (OBJ_INDEX_DATA *)alloc_perm( sizeof(*pObjIndex) );
	pObjIndex->vnum			= vnum;
	pObjIndex->random_object	= random_object;
	pObjIndex->new_format		= FALSE;
	pObjIndex->reset_num	 	= 0;
	pObjIndex->name			= fread_string( fp );
	pObjIndex->short_descr		= fread_string( fp );
	pObjIndex->description		= fread_string( fp );
	/* Action description */	  fread_string( fp );

	pObjIndex->short_descr	= first_case( pObjIndex->short_descr, FALSE );
	pObjIndex->description	= first_case( pObjIndex->description, TRUE );
	pObjIndex->material		= str_dup("");

	pObjIndex->item_type		= fread_number( fp );
	pObjIndex->extra_flags		= fread_flag( fp );
	pObjIndex->wear_flags		= fread_flag( fp );
	pObjIndex->value[0]		= fread_number( fp );
	pObjIndex->value[1]		= fread_number( fp );
	pObjIndex->value[2]		= fread_number( fp );
	pObjIndex->value[3]		= fread_number( fp );
	pObjIndex->value[4]		= 0;
	pObjIndex->level		= 0;
	pObjIndex->condition 		= 100;
	pObjIndex->weight		= fread_number( fp );
	pObjIndex->cost			= fread_number( fp );	/* Unused */
	/* Cost per day */		  fread_number( fp );
	pObjIndex->limit		= -1;
	pObjIndex->oprogs		= NULL;

	if (pObjIndex->item_type == ITEM_WEAPON)
	{
	    if (is_name("two",pObjIndex->name)
	    ||  is_name("two-handed",pObjIndex->name)
	    ||  is_name("claymore",pObjIndex->name)
      ||  is_name("iki-el",pObjIndex->name)
      ||  is_name("ikiel",pObjIndex->name)
      ||  is_name("çift-el",pObjIndex->name))
		SET_BIT(pObjIndex->value[4],WEAPON_TWO_HANDS);
	}

	load_obj_affects( fp, pObjIndex, 20 /* RT temp fix */, FALSE, TRUE );

        /* fix armors */
        if (pObjIndex->item_type == ITEM_ARMOR)
        {
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[1];
        }

	/*
	 * Translate spell "slot numbers" to internal "skill numbers."
	 */
	switch ( pObjIndex->item_type )
	{
	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
	    pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
	    pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	    pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
	    break;

	case ITEM_STAFF:
	case ITEM_WAND:
	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	    break;
	}

	hash_insert_obj( pObjIndex );
    }

    return;
}

void load_old_obj( FILE *fp )
{
    load_old_obj_section( fp, FALSE );
}

void load_new_old_obj( FILE *fp )
{
    load_old_obj_section( fp, TRUE );
}


/*
 * Snarf a reset section.
 */
void load_resets( FILE *fp )
{
    RESET_DATA *pReset;

    if ( area_last == NULL )
    {
	bug( "Load_resets: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	ROOM_INDEX_DATA *pRoomIndex;
	EXIT_DATA *pexit;
	char letter;
	OBJ_INDEX_DATA *temp_index;

	if ( ( letter = fread_letter( fp ) ) == 'S' )
	    break;

	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}

	pReset		= (RESET_DATA *)alloc_perm( sizeof(*pReset) );
	pReset->command	= letter;
	/* if_flag */	  fread_number( fp );
	pReset->arg1	= fread_number( fp );
	pReset->arg2	= fread_number( fp );
	pReset->arg3	= (letter == 'G' || letter == 'R')
			    ? 0 : fread_number( fp );
	pReset->arg4	= (letter == 'P' || letter == 'M')
			    ? fread_number(fp) : 0;
			  fread_to_eol( fp );

	/*
	 * Validate parameters.
	 * We're calling the index functions for the side effect.
	 */
	switch ( letter )
	{
	default:
	    bug( "Load_resets: bad command '%c'.", letter );
	    exit( 1 );
	    break;

	case 'M':
	    get_mob_index  ( pReset->arg1 );
	    get_room_index ( pReset->arg3 );
	    break;

	case 'O':
	    temp_index = get_obj_index  ( pReset->arg1 );
	    temp_index->reset_num++;
	    get_room_index ( pReset->arg3 );
	    break;

	case 'P':
	    temp_index = get_obj_index  ( pReset->arg1 );
	    temp_index->reset_num++;
	    get_obj_index  ( pReset->arg3 );
	    break;

	case 'G':
	case 'E':
	    temp_index = get_obj_index  ( pReset->arg1 );
	    temp_index->reset_num++;
	    break;

	case 'D':
	    pRoomIndex = get_room_index( pReset->arg1 );

	    if ( pReset->arg2 < 0
	    ||   pReset->arg2 >= MAX_DIR
	    || ( pexit = pRoomIndex->exit[pReset->arg2] ) == NULL
	    || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
	    {
		bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
		exit( 1 );
	    }

	    if ( pReset->arg3 < 0 || pReset->arg3 > 2 )
	    {
		bug( "Load_resets: 'D': bad 'locks': %d.", pReset->arg3 );
		exit( 1 );
	    }

	    break;

	case 'R':
	    pRoomIndex		= get_room_index( pReset->arg1 );

	    if ( pReset->arg2 < 0 || pReset->arg2 > MAX_DIR )
	    {
		bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
		exit( 1 );
	    }

	    break;
	}

	if ( area_last->reset_first == NULL )
	    area_last->reset_first	= pReset;
	if ( area_last->reset_last  != NULL )
	    area_last->reset_last->next	= pReset;

	area_last->reset_last	= pReset;
	pReset->next		= NULL;
	top_reset++;
    }

    return;
}



/*
 * Snarf a room section.
 */
void load_rooms( FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( area_last == NULL )
    {
	bug( "Load_rooms: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	int vnum;
	char letter;
	int door;

	if ( ( vnum = read_room_vnum( fp, "Load_rooms" ) ) == 0 )
	    break;

	pRoomIndex			= (ROOM_INDEX_DATA *)alloc_perm( sizeof(*pRoomIndex) );
	pRoomIndex->owner		= str_dup("");
	pRoomIndex->people		= NULL;
	pRoomIndex->contents		= NULL;
	pRoomIndex->extra_descr		= NULL;
        pRoomIndex->history             = NULL;
	pRoomIndex->area		= area_last;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );
	/* Area number */		  fread_number( fp );
	pRoomIndex->room_flags		= fread_flag( fp );

  	if ( 3000 <= vnum && vnum < 3400)
	   SET_BIT(pRoomIndex->room_flags,ROOM_LAW);

	pRoomIndex->sector_type		= fread_number( fp );
	pRoomIndex->light		= 0;
	for ( door = 0; door < MAX_DIR; door++ )
	    pRoomIndex->exit[door] = NULL;

	/* defaults */
	pRoomIndex->heal_rate = 100;
	pRoomIndex->mana_rate = 100;
	pRoomIndex->affected = NULL;
	pRoomIndex->affected_by = 0;
	pRoomIndex->aff_next = NULL;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' )
		break;

	    if ( letter == 'H') /* healing room */
		pRoomIndex->heal_rate = fread_number(fp);

	    else if ( letter == 'M') /* mana room */
		pRoomIndex->mana_rate = fread_number(fp);

	    else if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		int locks;

		door = fread_number( fp );
		if ( door < 0 || door >= MAX_DIR )
		{
		    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
		    exit( 1 );
		}

		pexit			= (EXIT_DATA *)alloc_perm( sizeof(*pexit) );
		pexit->description	= fread_string( fp );
		pexit->keyword		= fread_string( fp );
		pexit->exit_info	= 0;
		locks			= fread_number( fp );
		pexit->key		= fread_number( fp );
		pexit->u1.vnum		= fread_number( fp );

		switch ( locks )
		{
		case 1: pexit->exit_info = EX_ISDOOR;                break;
		case 2: pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
		case 3: pexit->exit_info = EX_ISDOOR | EX_NOPASS;    break;
		case 4: pexit->exit_info = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
			break;
		case 5: pexit->exit_info = EX_NOFLEE;                break;
		}

		pRoomIndex->exit[door]	= pexit;
		pRoomIndex->old_exit[door] = pexit;
		top_exit++;
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		ed			= (EXTRA_DESCR_DATA *)alloc_perm( sizeof(*ed) );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pRoomIndex->extra_descr;
		pRoomIndex->extra_descr	= ed;
		top_ed++;
	    }

	    else if (letter == 'O')
	    {
		if (pRoomIndex->owner[0] != '\0')
		{
		    bug("Load_rooms: duplicate owner.",0);
		    exit(1);
		}

		pRoomIndex->owner = fread_string(fp);
	    }

	    else
	    {
		bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
		exit( 1 );
	    }
	}

	hash_insert_room( pRoomIndex );
    }

    return;
}



/*
 * Snarf a shop section.
 */
void load_shops( FILE *fp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iTrade;

	pShop			= (SHOP_DATA *)alloc_perm( sizeof(*pShop) );
	pShop->keeper		= fread_number( fp );
	if ( pShop->keeper == 0 )
	    break;
	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	    pShop->buy_type[iTrade]	= fread_number( fp );
	pShop->profit_buy	= fread_number( fp );
	pShop->profit_sell	= fread_number( fp );
	pShop->open_hour	= fread_number( fp );
	pShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index( pShop->keeper );
	pMobIndex->pShop	= pShop;

	if ( shop_first == NULL )
	    shop_first = pShop;
	if ( shop_last  != NULL )
	    shop_last->next = pShop;

	shop_last	= pShop;
	pShop->next	= NULL;
	top_shop++;
    }

    return;
}


/*
 * Snarf spec proc declarations.
 */
static void special_directive( FILE *fp, char letter )
{
    MOB_INDEX_DATA *pMobIndex;

    (void) letter;	/* yalnızca 'M' */
    pMobIndex		= get_mob_index	( fread_number ( fp ) );
    pMobIndex->spec_fun	= spec_lookup	( fread_word   ( fp ) );
    if ( pMobIndex->spec_fun == 0 )
    {
	bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
	exit( 1 );
    }
}

void load_specials( FILE *fp )
{
    load_directives( fp, "Load_specials", "M", special_directive );
}


/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    extern const sh_int rev_dir [];
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
    int iHash;
    int door;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex != NULL;
	      pRoomIndex  = pRoomIndex->next )
	{
	    bool fexit;

	    fexit = FALSE;
	    for ( door = 0; door < MAX_DIR; door++ )
	    {
		if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
		{
		    if ( pexit->u1.vnum <= 0
		    || get_room_index(pexit->u1.vnum) == NULL)
			pexit->u1.to_room = NULL;
		    else
		    {
		   	fexit = TRUE;
			pexit->u1.to_room = get_room_index( pexit->u1.vnum );
		    }
		}
	    }
	    if (!fexit)
		SET_BIT(pRoomIndex->room_flags,ROOM_NO_MOB);
	}
    }

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex != NULL;
	      pRoomIndex  = pRoomIndex->next )
	{
	    for ( door = 0; door < MAX_DIR; door++ )
	    {
		if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
		&&   ( to_room   = pexit->u1.to_room            ) != NULL
		&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
		&&   pexit_rev->u1.to_room != pRoomIndex
		&&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
		{
		    bugf( "Fix_exits: %d:%d -> %d:%d -> %d.",
			pRoomIndex->vnum, door,
			to_room->vnum,    rev_dir[door],
			(pexit_rev->u1.to_room == NULL)
			    ? 0 : pexit_rev->u1.to_room->vnum );
		}
	    }
	}
    }

    return;
}



/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {

	if ( ++pArea->age < 3 )
	    continue;

	/*
	 * Check age and reset.
	 * Note: Mud School resets every 3 minutes (not 15).
	 */
	if ( (!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
	||    pArea->age >= 31)
	{
	    ROOM_INDEX_DATA *pRoomIndex;

	    reset_area( pArea );
      snprintf(buf, sizeof(buf),"%s bölgesi reset'lendi.",pArea->name);
	    wiznet(buf,NULL,NULL,WIZ_RESETS,0,0);

	    if (pArea->resetmsg)
	    	 snprintf(buf, sizeof(buf),"%s\n\r",pArea->resetmsg);
	    else
      snprintf(buf, sizeof(buf),"Yaşamın yeni seslerini duyuyorsun...\n\r");

            for ( d = descriptor_list; d != NULL; d = d->next )
	       {
             	 if ( d->connected == CON_PLAYING
              	    &&   IS_AWAKE(d->character )
		    &&   d->character->in_room )
		    if ( d->character->in_room->area == pArea )
			send_to_char( buf, d->character );
	       }

	    /* Bu odaları içeren bölgeler daha sık (3 dakikada) reset'lenir. */
	    {
		static const int fast_reset_vnum[] = { 200, 210, 220, 230,
						       ROOM_VNUM_SCHOOL };
		size_t i;

		pArea->age = number_range( 0, 3 );
		for ( i = 0; i < sizeof(fast_reset_vnum) / sizeof(fast_reset_vnum[0]); i++ )
		{
		    pRoomIndex = get_room_index( fast_reset_vnum[i] );
		    if ( pRoomIndex != NULL && pArea == pRoomIndex->area )
			pArea->age = 15 - 2;
		}
	    }
	    pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
	    if ( ( pRoomIndex == NULL || pArea != pRoomIndex->area )
	    &&   pArea->nplayer == 0 )
		pArea->empty = TRUE;
	}
    }

    return;
}



/*
 * Reset one area.
 */
void reset_area( AREA_DATA *pArea )
{
  RESET_DATA *pReset;
  CHAR_DATA *mob;
  bool last;
  int level;
  int i;
  ROOM_INDEX_DATA *room;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *ch;

  if ( weather_info.sky == SKY_RAINING )
  {
    for ( d = descriptor_list; d!=NULL; d=d->next)
    {
      if ( d->connected != CON_PLAYING )
      {
        continue;
      }
      ch = ( d->original != NULL ) ? d->original : d->character;
      if ( (ch->in_room->area == pArea) &&
        ( get_skill(ch, gsn_track)>50) &&
        ( !IS_SET(ch->in_room->room_flags, ROOM_INDOORS) ) )
      {
        send_to_char("Yağmur izleri temizliyor.\n\r", ch );
      }
    }
    for (i=pArea->min_vnum; i<=pArea->max_vnum; i++)
    {
      room = get_room_index(i);
      if (room == NULL)
      {
        continue;
      }
      if (IS_SET(room->room_flags, ROOM_INDOORS))
      {
        continue;
      }
      room_record( "erased", room, -1 );
      if (number_percent() < 50)
      {
        room_record( "erased", room, -1 );
      }
    }
  }
    mob 	= NULL;
    last	= TRUE;
    level	= 0;
    for ( pReset = pArea->reset_first; pReset != NULL; pReset = pReset->next )
    {
	ROOM_INDEX_DATA *pRoomIndex;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_INDEX_DATA *pObjToIndex;
	OBJ_INDEX_DATA *cabal_item;
	EXIT_DATA *pexit;
	OBJ_DATA *obj;
	OBJ_DATA *obj_to;
	int count, limit, ci_vnum=0;

	switch ( pReset->command )
	{
	default:
	    bug( "Reset_area: bad command %c.", pReset->command );
	    break;

	case 'M':
	    if ( ( pMobIndex = get_mob_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'M': bad vnum %d.", pReset->arg1 );
		continue;
	    }

	    if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
	    {
		bug( "Reset_area: 'M': bad room vnum %d.", pReset->arg3 );
		continue;
	    }

	    if ( pMobIndex->count >= pReset->arg2 )
	    {
		last = FALSE;
		break;
	    }

	    count = 0;
	    for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
		if (mob->pIndexData == pMobIndex)
		{
		    count++;
		    if (count >= pReset->arg4)
		    {
		    	last = FALSE;
		    	break;
		    }
		}

	    if (count >= pReset->arg4)
		break;

	    mob = create_mobile( pMobIndex , pRoomIndex->area);

	    /*
	     * Check for pet shop.
	     */
	    {
		ROOM_INDEX_DATA *pRoomIndexPrev;
		pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
		if ( pRoomIndexPrev != NULL
		&&   IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP) )
		    SET_BIT(mob->act, ACT_PET);
	    }

	    /* set area */
	    mob->zone = pRoomIndex->area;

	    char_to_room( mob, pRoomIndex );
	    level = URANGE( 0, mob->level - 2, LEVEL_HERO - 1 );
	    last  = TRUE;
	    break;

	case 'O':
	    if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'O': bad vnum %d.", pReset->arg1 );
		continue;
	    }

	    if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
	    {
		bug( "Reset_area: 'O': bad room vnum %d.", pReset->arg3 );
		continue;
	    }

	    if ( pArea->nplayer > 0
	    ||   count_obj_list( pObjIndex, pRoomIndex->contents ) > 0 )
	    {
		last = FALSE;
		break;
	    }

	    /* Kabal sunağı/tahtı, kabal eşyası dünyadayken yeniden konmaz. */
	    {
		static const struct { int altar_vnum; int cabal; } altar_cabal[] =
		{
		    { OBJ_VNUM_RULER_STAND,	CABAL_RULER	},
		    { OBJ_VNUM_INVADER_SKULL,	CABAL_INVADER	},
		    { OBJ_VNUM_SHALAFI_ALTAR,	CABAL_SHALAFI	},
		    { OBJ_VNUM_CHAOS_ALTAR,	CABAL_CHAOS	},
		    { OBJ_VNUM_KNIGHT_ALTAR,	CABAL_KNIGHT	},
		    { OBJ_VNUM_LIONS_ALTAR,	CABAL_LIONS	},
		    { OBJ_VNUM_BATTLE_THRONE,	CABAL_BATTLE	},
		    { OBJ_VNUM_HUNTER_ALTAR,	CABAL_HUNTER	},
		};
		size_t ai;

		for ( ai = 0; ai < sizeof(altar_cabal) / sizeof(altar_cabal[0]); ai++ )
		    if ( pObjIndex->vnum == altar_cabal[ai].altar_vnum )
			ci_vnum = cabal_table[altar_cabal[ai].cabal].obj_vnum;
	    }

	    if ( ci_vnum != 0
	    &&   ( cabal_item = get_obj_index( ci_vnum ) ) != NULL
	    &&   cabal_item->count > 0 )
	     {
		last = FALSE;
		break;
	     }

            if ( ( pObjIndex->limit != -1 ) &&
                 ( pObjIndex->count >= pObjIndex->limit ) )
              {
                last = FALSE;
                break;
              }

	    obj       = create_object( pObjIndex, UMIN(number_fuzzy(level),
						       LEVEL_HERO - 1) );
	    obj->cost = 0;
	    obj_to_room( obj, pRoomIndex );
	    last = TRUE;
	    break;

	case 'P':
	    if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'P': bad vnum %d.", pReset->arg1 );
		continue;
	    }

	    if ( ( pObjToIndex = get_obj_index( pReset->arg3 ) ) == NULL )
	    {
		bug( "Reset_area: 'P': bad vnum %d.", pReset->arg3 );
		continue;
	    }

            if (pReset->arg2 > 50) 		/* old format */
                limit = 6;
            else if (pReset->arg2 == -1) 	/* no limit */
                limit = 999;
            else
                limit = pReset->arg2;

	    if (pArea->nplayer > 0
	    || (obj_to = get_obj_type( pObjToIndex ) ) == NULL
	    || (obj_to->in_room == NULL && !last)
	    || ( pObjIndex->count >= limit && number_range(0,4) != 0)
	    || (count = count_obj_list(pObjIndex,obj_to->contains))
		> pReset->arg4 )
	    {
		last = FALSE;
		break;
	    }

            if ( ( pObjIndex->limit != -1               ) &&
                 ( pObjIndex->count >= pObjIndex->limit ) )
              {
                last = FALSE;
                dump_to_scr("Reseting area: [P] OBJ limit reached\n\r");
                break;
              }

	    while (count < pReset->arg4)
	    {
	        obj = create_object( pObjIndex, number_fuzzy(obj_to->level) );
	    	obj_to_obj( obj, obj_to );
		count++;
		if (pObjIndex->count >= limit)
		    break;
	    }
	    /* fix object lock state! */
	    obj_to->value[1] = obj_to->pIndexData->value[1];
	    last = TRUE;
	    break;

	case 'G':
	case 'E':
	    if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1 );
		continue;
	    }

	    if ( !last )
		break;

	    if ( mob == NULL )
	    {
		bug( "Reset_area: 'E' or 'G': null mob for vnum %d.",
		    pReset->arg1 );
		last = FALSE;
		break;
	    }

	    if ( mob->pIndexData->pShop != NULL )
	    {
		int olevel = 0,i,j;

		if (!pObjIndex->new_format)
		    switch ( pObjIndex->item_type )
		{
		case ITEM_PILL:
		case ITEM_POTION:
		case ITEM_SCROLL:
		    olevel = MAX_LEVEL - 7;
		    for (i = 1; i < 5; i++)
		    {
			if (pObjIndex->value[i] > 0)
			{
		    	    for (j = 0; j < MAX_CLASS; j++)
			    {
				olevel = UMIN(olevel,
				         skill_table[pObjIndex->value[i]].
						     skill_level[j]);
			    }
			}
		    }

		    olevel = UMAX(0,(olevel * 3 / 4) - 2);
		    break;
		case ITEM_WAND:		olevel = number_range( 10, 20 ); break;
		case ITEM_STAFF:	olevel = number_range( 15, 25 ); break;
		case ITEM_ARMOR:	olevel = number_range(  5, 15 ); break;
		case ITEM_WEAPON:	olevel = number_range(  5, 15 ); break;
		case ITEM_TREASURE:	olevel = number_range( 10, 20 ); break;
		}

		obj = create_object( pObjIndex, olevel );
		SET_BIT( obj->extra_flags, ITEM_INVENTORY );
	    }

	    else
	    {
                if ( (pObjIndex->limit == -1)  ||
                  (pObjIndex->count < pObjIndex->limit) )
                  obj=create_object(pObjIndex,UMIN(number_fuzzy(level),
                                                   LEVEL_HERO - 1) );
                else break;

	    }

	    obj_to_char( obj, mob );
	    if ( pReset->command == 'E' )
	     {
	      int iWear =  wear_convert( pReset->arg3 );
	      if ( iWear != WEAR_NONE)	equip_char( mob, obj, iWear );
	     }
	    last = TRUE;
	    break;

	case 'D':
	    if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'D': bad vnum %d.", pReset->arg1 );
		continue;
	    }

	    if ( ( pexit = pRoomIndex->exit[pReset->arg2] ) == NULL )
		break;

	    switch ( pReset->arg3 )
	    {
	    case 0:
		REMOVE_BIT( pexit->exit_info, EX_CLOSED );
		REMOVE_BIT( pexit->exit_info, EX_LOCKED );
		break;

	    case 1:
		SET_BIT(    pexit->exit_info, EX_CLOSED );
		REMOVE_BIT( pexit->exit_info, EX_LOCKED );
		break;

	    case 2:
		SET_BIT(    pexit->exit_info, EX_CLOSED );
		SET_BIT(    pexit->exit_info, EX_LOCKED );
		break;
	    }

	    last = TRUE;
	    break;

	case 'R':
	    if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'R': bad vnum %d.", pReset->arg1 );
		continue;
	    }

	    {
		int d0;
		int d1;

		for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
		{
		    d1                   = number_range( d0, pReset->arg2-1 );
		    pexit                = pRoomIndex->exit[d0];
		    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
		    pRoomIndex->exit[d1] = pexit;
		}
	    }
	    break;
	}
    }


  return;
}



/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex , AREA_DATA *	pArea)
{
    CHAR_DATA *mob;
    AFFECT_DATA af;
    int i;

    mobile_count++;

    if ( pMobIndex == NULL )
    {
      bug( "Create_mobile: NULL pMobIndex.", 0 );
      exit( 1 );
    }

    mob = new_char();

    mob->pIndexData	= pMobIndex;

    mob->name		= pMobIndex->player_name;
    mob->id		= get_mob_id();
    mob->short_descr	= pMobIndex->short_descr;
    mob->long_descr	= pMobIndex->long_descr;
    mob->description	= pMobIndex->description;
    mob->spec_fun	= pMobIndex->spec_fun;
    mob->prompt		= NULL;
    mob->progtypes	= pMobIndex->progtypes;
    mob->riding		= FALSE;
    mob->mount		= NULL;
    mob->hunting	= NULL;
    mob->endur		= 0;
    mob->in_mind	= NULL;
    mob->cabal		= CABAL_NONE;
    mob->iclass		= CLASS_CLERIC;

  mob->act 		= pMobIndex->act | ACT_IS_NPC;
  mob->comm		= COMM_NOCHANNELS|COMM_NOSHOUT|COMM_NOTELL;
  mob->affected_by	= pMobIndex->affected_by;
  mob->detection		= pMobIndex->detection;

  /*
   * Yonelim ayarlama
   */
  if ( pArea == NULL )
  {
    mob->alignment		= number_range(-1000,1000);
  }
  else
  {
    i = number_range(1,100);
    if (i < pArea->yonelim_iyi)
    {
      mob->alignment = number_range(350,1000);
    }
    else if (i < ( pArea->yonelim_iyi + pArea->yonelim_yansiz ) )
    {
      mob->alignment = number_range(-349,349);
    }
    else
    {
      mob->alignment = number_range(-1000,-350);
    }
  }
  /*
   * Yonelim ayarlama bitti
   */

  mob->level		= pMobIndex->level;
  /* Seviye atanmadan hesaplanıyordu (her yaratık 0 gümüşle doğuyordu). */
  mob->silver		= number_range(mob->level*3,mob->level*20);
  mob->hitroll		= hitroll_damroll_hesapla(pMobIndex->level);
  mob->damroll		= hitroll_damroll_hesapla(pMobIndex->level);
  mob->max_hit		= number_range( yp_tablo[ pMobIndex->level ].min_yp , yp_tablo[ pMobIndex->level ].max_yp );
  mob->hit		= mob->max_hit;
  mob->max_mana		= number_range( yp_tablo[ pMobIndex->level ].min_yp , yp_tablo[ pMobIndex->level ].max_yp );
  mob->mana		= mob->max_mana;
  mob->damage[DICE_NUMBER]= damage_dice_0(pMobIndex->level);
  mob->damage[DICE_TYPE]	= damage_dice_1(pMobIndex->level);
  mob->damage[DICE_BONUS]	= damage_dice_2(pMobIndex->level);
  mob->dam_type		= dam_type_dice();
  mob->status		= 0;
  
  /* mob icin din ayarlama: %40 dinsiz, kalanı 1-4 arasında rastgele */
  mob->religion		= number_percent() < 40 ? 0 : number_range(1,4);

  if (mob->dam_type == 0)
    switch(number_range(1,3))
    {
    case (1): mob->dam_type = 3;        break;  /* slash */
    case (2): mob->dam_type = 7;        break;  /* pound */
    case (3): mob->dam_type = 11;       break;  /* pierce */
    }
  for (i = 0; i < 4; i++)
    mob->armor[i]	= ac_dice(i,pMobIndex->level);

  /* Büyüyle çağrılan/özel yaratıklar alan dosyasındaki ırkını korur. */
  {
    static const int fixed_race_vnum[] =
    {
      MOB_VNUM_ELM_EARTH, MOB_VNUM_ELM_AIR, MOB_VNUM_ELM_FIRE,
      MOB_VNUM_ELM_WATER, MOB_VNUM_ELM_LIGHT, MOB_VNUM_WEAPON,
      MOB_VNUM_ARMOR, MOB_VNUM_DEMON, MOB_VNUM_UNDEAD,
      MOB_VNUM_LION, MOB_VNUM_WOLF, MOB_VNUM_LESSER_GOLEM,
      MOB_VNUM_STONE_GOLEM, MOB_VNUM_IRON_GOLEM, MOB_VNUM_ADAMANTITE_GOLEM,
      MOB_VNUM_HUNTER, MOB_VNUM_SUM_SHADOW, MOB_VNUM_DOG,
    };
    size_t fi;
    bool fixed_race = FALSE;

    for ( fi = 0; fi < sizeof(fixed_race_vnum) / sizeof(fixed_race_vnum[0]); fi++ )
      if ( pMobIndex->vnum == fixed_race_vnum[fi] )
        fixed_race = TRUE;

  if ( fixed_race )
      {
        mob->race		= pMobIndex->race;
      }
  else
    {
        if( (mob->spec_fun != spec_lookup( "spec_questmaster" )) || IS_SET(mob->act, ACT_MAFYA) || IS_SET(mob->act, ACT_TRAIN) ||
            IS_SET(mob->act, ACT_PRACTICE) || IS_SET(mob->act, ACT_CLERIC) || IS_SET(mob->act, ACT_MAGE) ||
            IS_SET(mob->act, ACT_THIEF) || IS_SET(mob->act, ACT_WARRIOR) || IS_SET(mob->act, ACT_IS_HEALER) ||
            IS_SET(mob->act, ACT_GAIN) )
            {
                mob->race		= race_dice(mob->level, TRUE);
            }
        else
        {
            mob->race		= race_dice(mob->level, FALSE);
        }
    }
  }

  mob->off_flags		= race_table[mob->race].off;
  mob->imm_flags		= race_table[mob->race].imm;
  mob->res_flags		= race_table[mob->race].res;
  mob->vuln_flags		= race_table[mob->race].vuln;

  mob->start_pos		= position_dice();
  mob->default_pos		= position_dice();
  mob->sex				= sex_dice();

  mob->form		= race_table[mob->race].form;
  mob->parts		= race_table[mob->race].parts;
  mob->size		= race_table[mob->race].size;
  mob->material		= str_dup("none");
  mob->extracted		= FALSE;
  mob = mob_assign_perm_stats(mob);

  /* let's get some spell action */
  if (IS_AFFECTED(mob,AFF_SANCTUARY))
  {
    af.where	 = TO_AFFECTS;
    af.type      = skill_lookup("sanctuary");
    af.level     = mob->level;
    af.duration  = -1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( mob, &af );
  }

  if (IS_AFFECTED(mob,AFF_HASTE))
  {
    af.where	 = TO_AFFECTS;
    af.type      = skill_lookup("haste");
    af.level     = mob->level;
    af.duration  = -1;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (mob->level >= 18) + (mob->level >= 25) + (mob->level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( mob, &af );
  }

  if (IS_AFFECTED(mob,AFF_PROTECT_EVIL))
  {
    af.where	 = TO_AFFECTS;
    af.type	 = skill_lookup("protection evil");
    af.level	 = mob->level;
    af.duration	 = -1;
    af.location	 = APPLY_SAVES;
    af.modifier	 = -1;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(mob,&af);
  }

  if (IS_AFFECTED(mob,AFF_PROTECT_GOOD))
  {
    af.where	 = TO_AFFECTS;
    af.type      = skill_lookup("protection good");
    af.level     = mob->level;
    af.duration  = -1;
    af.location  = APPLY_SAVES;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char(mob,&af);
  }

  mob->position = mob->start_pos;

  /* link the mob to the world list */
  mob->next		= char_list;
  char_list		= mob;
  pMobIndex->count++;
  return mob;
}

CHAR_DATA * mob_assign_perm_stats(CHAR_DATA* mob)
{
  int i;
  for (i = 0; i < MAX_STATS; i ++)
        mob->perm_stat[i] = UMIN(25,11 + mob->level/4);

  if (IS_SET(mob->act,ACT_WARRIOR))
  {
      mob->perm_stat[STAT_STR] += 3;
      mob->perm_stat[STAT_INT] -= 1;
      mob->perm_stat[STAT_CON] += 2;
  }

  if (IS_SET(mob->act,ACT_THIEF))
  {
      mob->perm_stat[STAT_DEX] += 3;
      mob->perm_stat[STAT_INT] += 1;
      mob->perm_stat[STAT_WIS] -= 1;
  }

  if (IS_SET(mob->act,ACT_CLERIC))
  {
      mob->perm_stat[STAT_WIS] += 3;
      mob->perm_stat[STAT_DEX] -= 1;
      mob->perm_stat[STAT_STR] += 1;
  }

  if (IS_SET(mob->act,ACT_MAGE))
  {
      mob->perm_stat[STAT_INT] += 3;
      mob->perm_stat[STAT_STR] -= 1;
      mob->perm_stat[STAT_DEX] += 1;
  }

  if (IS_SET(mob->off_flags,OFF_FAST))
      mob->perm_stat[STAT_DEX] += 2;

  return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;

    if ( parent == NULL || clone == NULL || !IS_NPC(parent))
	return;

    /* start fixing values */
    clone->name 	= str_dup(parent->name);
    clone->short_descr	= str_dup(parent->short_descr);
    clone->long_descr	= str_dup(parent->long_descr);
    clone->description	= str_dup(parent->description);
    clone->sex		= parent->sex;
    clone->iclass	= parent->iclass;
    clone->race		= parent->race;
    clone->level	= parent->level;
    clone->trust	= 0;
    clone->timer	= parent->timer;
    clone->wait		= parent->wait;
    clone->hit		= parent->hit;
    clone->max_hit	= parent->max_hit;
    clone->mana		= parent->mana;
    clone->max_mana	= parent->max_mana;
    clone->move		= parent->move;
    clone->max_move	= parent->max_move;
    clone->silver	= parent->silver;
    clone->exp		= parent->exp;
    clone->act		= parent->act;
    clone->comm		= parent->comm;
    clone->imm_flags	= parent->imm_flags;
    clone->res_flags	= parent->res_flags;
    clone->vuln_flags	= parent->vuln_flags;
    clone->invis_level	= parent->invis_level;
    clone->affected_by	= parent->affected_by;
    clone->detection	= parent->detection;
    clone->position	= parent->position;
    clone->practice	= parent->practice;
    clone->train	= parent->train;
    clone->saving_throw	= parent->saving_throw;
    clone->alignment	= parent->alignment;
    clone->hitroll	= parent->hitroll;
    clone->damroll	= parent->damroll;
    clone->wimpy	= parent->wimpy;
    clone->form		= parent->form;
    clone->parts	= parent->parts;
    clone->size		= parent->size;
    clone->material	= str_dup(parent->material);
    clone->extracted	= parent->extracted;
    clone->off_flags	= parent->off_flags;
    clone->dam_type	= parent->dam_type;
    clone->start_pos	= parent->start_pos;
    clone->default_pos	= parent->default_pos;
    clone->spec_fun	= parent->spec_fun;
    clone->progtypes	= parent->progtypes;
    clone->status	= parent->status;
    clone->hunting	= NULL;
    clone->endur	= 0;
    clone->in_mind	= NULL;
    clone->cabal	= CABAL_NONE;

    for (i = 0; i < 4; i++)
    	clone->armor[i]	= parent->armor[i];

    for (i = 0; i < MAX_STATS; i++)
    {
	clone->perm_stat[i]	= parent->perm_stat[i];
	clone->mod_stat[i]	= parent->mod_stat[i];
    }

    for (i = 0; i < 3; i++)
	clone->damage[i]	= parent->damage[i];

    /* now add the affects */
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_char(clone,paf);

}



/*
 * Create an object with modifying the count
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
  return create_object_org(pObjIndex,level,TRUE);
}

/*
 * for player load/quit
 * Create an object and do not modify the count
 */
OBJ_DATA *create_object_nocount(OBJ_INDEX_DATA *pObjIndex, int level )
{
  return create_object_org(pObjIndex,level,FALSE);
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object_org( OBJ_INDEX_DATA *pObjIndex, int level, bool Count )
{

    OBJ_DATA *obj;
    int i;


    if ( pObjIndex == NULL )
    {
	bug( "Create_object: NULL pObjIndex.", 0 );
	exit( 1 );
    }

    obj = new_obj();

    obj->pIndexData	= pObjIndex;
    obj->in_room	= NULL;
    obj->enchanted	= FALSE;
    obj->kasada_duruyor	= FALSE;

    for (i=1;i < MAX_CABAL;i++)
    {
      if (pObjIndex->vnum == cabal_table[i].obj_vnum)
      {
        cabal_table[i].obj_ptr = obj;
        break;
      }
    }

    if ( pObjIndex->new_format )
 	obj->level = pObjIndex->level;
    else
	obj->level		= UMAX(0,level);
    obj->wear_loc	= -1;


    obj->name		= pObjIndex->name;
    obj->short_descr	= pObjIndex->short_descr;
    obj->description	= pObjIndex->description;
    obj->item_type	= pObjIndex->item_type;
    obj->extra_flags	= pObjIndex->extra_flags;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->value[0]	= pObjIndex->value[0];
    obj->value[1]	= pObjIndex->value[1];
    obj->value[2]	= pObjIndex->value[2];
    obj->value[3]	= pObjIndex->value[3];
    obj->value[4]	= pObjIndex->value[4];
    obj->weight		= pObjIndex->weight;
    obj->extracted	= FALSE;
    obj->progtypes	= pObjIndex->progtypes;
    obj->from           = str_dup(""); /* used with body parts */
    obj->pit            = OBJ_VNUM_PIT; /* default for corpse decaying */
    obj->altar          = ROOM_VNUM_ALTAR; /* default for corpses */
    obj->condition	= pObjIndex->condition;
    obj->creation_time  = current_time;

    if (level == -1 || pObjIndex->new_format)
	obj->cost	= pObjIndex->cost;
    else
    	obj->cost	= number_fuzzy( 10 )
			* number_fuzzy( level ) * number_fuzzy( level );

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
        default:
        bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
        break;

        case ITEM_LIGHT:
        if (obj->value[2] == 999)
            obj->value[2] = -1;
        break;

        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_MAYMUNCUK:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_PORTAL:
        if (!pObjIndex->new_format)
            obj->cost /= 5;
        break;

        case ITEM_TREASURE:
        case ITEM_WARP_STONE:
        case ITEM_ROOM_KEY:
        case ITEM_GEM:
        case ITEM_JEWELRY:
        case ITEM_TATTOO:
        break;

        case ITEM_JUKEBOX:
        for (i = 0; i < 5; i++)
        obj->value[i] = -1;
        break;

        case ITEM_SCROLL:
        if (level != -1 && !pObjIndex->new_format)
            obj->value[0]	= number_fuzzy( obj->value[0] );
        break;

        case ITEM_WAND:
        case ITEM_STAFF:
        if (level != -1 && !pObjIndex->new_format)
        {
            obj->value[0]	= number_fuzzy( obj->value[0] );
            obj->value[1]	= number_fuzzy( obj->value[1] );
            obj->value[2]	= obj->value[1];
        }
        if (!pObjIndex->new_format)
            obj->cost *= 2;
        break;

        case ITEM_WEAPON:
        if (level != -1 && !pObjIndex->new_format)
        {
            obj->value[1] = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
            obj->value[2] = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
        }
        break;

        case ITEM_ARMOR:
        if (level != -1 && !pObjIndex->new_format)
        {
            obj->value[0]	= number_fuzzy( level / 5 + 3 );
            obj->value[1]	= number_fuzzy( level / 5 + 3 );
            obj->value[2]	= number_fuzzy( level / 5 + 3 );
        }
        break;

        case ITEM_POTION:
        case ITEM_PILL:
        if (level != -1 && !pObjIndex->new_format)
            obj->value[0] = number_fuzzy( number_fuzzy( obj->value[0] ) );
        break;

        case ITEM_MONEY:
        if (!pObjIndex->new_format)
            obj->value[0]	= obj->cost;
        break;
    }
	
	if(Count == TRUE && obj->pIndexData->random_object)
	{
		obj_random_paf(obj);
        obj->condition	    = obj_random_condition();
        obj_random_material(obj);
        obj->cost           = obj_random_cost(obj->level);
        obj->weight         = obj_random_weight(obj->pIndexData->vnum, obj->item_type, obj->value[0], find_material_index(obj->material), obj->wear_flags);
        obj->extra_flags    = obj_random_extra_flag();

        switch ( obj->item_type )
        {
            case ITEM_WEAPON:
                obj->value[0]	= number_range(1,12)-1;			// weapon type
                obj->value[1]	= UMAX(1,number_range(level/11,level/9)+3);				// number of dice
                obj->value[2]	= UMAX(1,number_range(level/8,level/6));					// number of dice, each dice has
                /* Hançer delici, kılıç kesici bir vuruş türü alır (en çok 100 deneme). */
                {
                    int want_dam = obj->value[0] == WEAPON_DAGGER ? DAM_PIERCE
                                 : obj->value[0] == WEAPON_SWORD  ? DAM_SLASH : -1;

                    for ( i = 0; i < 100; i++ )
                    {
                        obj->value[3] = number_range(1,40)-1;
                        if ( want_dam < 0 || attack_table[obj->value[3]].damage == want_dam )
                            break;
                    }
                }
                obj->value[4]   = obj_random_weapon_flag();
                obj_random_name(obj);
            break;
            case ITEM_ARMOR:
                obj->value[0]	= number_range( UMAX(1,(int)((obj->level+4)/5)) , UMAX(1,(int)((obj->level+3)/2)) );	// armor vs. pierce
                obj->value[1]	= number_range( UMAX(1,(int)((obj->level+4)/5)) , UMAX(1,(int)((obj->level+3)/2)) );	// armor vs. bash
                obj->value[2]	= number_range( UMAX(1,(int)((obj->level+4)/5)) , UMAX(1,(int)((obj->level+3)/2)) );	// armor vs. slash
                obj->value[3]	= number_range( UMAX(1,(int)((obj->level+4)/5)) , UMAX(1,(int)((obj->level+3)/2)) );	// armor vs. exotic weapons
                obj->value[4]	= 0;										// unused
            break;
            case ITEM_WAND:
            case ITEM_STAFF:
                obj->value[0]	= (number_percent()<95)?(obj->level):(number_range(5,90)); // spell level
                obj->value[1]	= number_range( UMAX(1,(int)((obj->level+4)/5)) , UMAX(1,(int)((obj->level+3)/2)) );	// maximum number of charges
                obj->value[2]	= obj->value[1];	// current number of charges
                obj->value[3]	= skill_lookup(obj_random_wand_potion_spell());
                obj->value[4]	= 0;					// unused
            break;
            case ITEM_POTION:
            case ITEM_PILL:
            case ITEM_SCROLL:
                if(obj->value[2] == 0)
                {
                    if(number_range(1,100) == 1)
                    {
                        obj->value[2] = skill_lookup(obj_random_wand_potion_spell());
                    }
                }
                if(obj->value[3] == 0)
                {
                    if(number_range(1,100) == 1)
                    {
                        obj->value[3] = skill_lookup(obj_random_wand_potion_spell());
                    }
                }
                if(obj->value[4] == 0)
                {
                    if(number_range(1,100) == 1)
                    {
                        obj->value[4] = skill_lookup(obj_random_wand_potion_spell());
                    }
                }
                obj->cost = number_range( UMAX(1,obj->level) , UMAX(2,obj->level*3) );
            break;
            case ITEM_LIGHT:
                obj->value[0]	= 0;								// unused
                obj->value[1]	= 0;								// unused
                obj->value[2]	= (number_percent()<20)?-1:number_range(5,500); // light duration in game hours (-1 unlimited)
                obj->value[3]	= 0;								// unused
                obj->value[4]	= 0;								// unused
            break;
            case ITEM_FOOD:
                obj->value[0]	= number_range(5,40);				// number of game hours the food will keep the person who eats it full
                obj->value[1]	= number_range(5,70);				// number of hours it will keep the person from getting hungry
                obj->value[2]	= 0;								// unused
                obj->value[3]	= (number_percent()<95)?0:1;		// nonpoisoned:0,poisoned:1
                obj->value[4]	= 0;								// unused
            break;
        }
	}
    else
    {
        obj->material	= str_dup(pObjIndex->material);
    }

    obj->next		= object_list;
    object_list		= obj;
    if ( Count )
      pObjIndex->count++;
    return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;
    EXTRA_DESCR_DATA *ed,*ed_new;

    if (parent == NULL || clone == NULL)
	return;

    /* start fixing the object */
    clone->name 	= str_dup(parent->name);
    clone->short_descr 	= str_dup(parent->short_descr);
    clone->description	= str_dup(parent->description);
    clone->item_type	= parent->item_type;
    clone->extra_flags	= parent->extra_flags;
    clone->wear_flags	= parent->wear_flags;
    clone->weight	= parent->weight;
    clone->cost		= parent->cost;
    clone->level	= parent->level;
    clone->condition	= parent->condition;
    clone->material	= str_dup(parent->material);
    clone->timer	= parent->timer;
    clone->from         = parent->from;
    clone->extracted    = parent->extracted;
    clone->pit          = parent->pit;
    clone->altar        = parent->altar;

    for (i = 0;  i < 5; i ++)
	clone->value[i]	= parent->value[i];

    /* affects */
    clone->enchanted	= parent->enchanted;

    for (paf = parent->affected; paf != NULL; paf = paf->next)
	affect_to_obj(clone,paf);

    /* extended desc */
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next)
    {
        ed_new                  = new_extra_descr();
        ed_new->keyword    	= str_dup( ed->keyword);
        ed_new->description     = str_dup( ed->description );
        ed_new->next           	= clone->extra_descr;
        clone->extra_descr  	= ed_new;
    }

}




/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != NULL; ed = ed->next )
    {
	if ( is_name( (char *) name, ed->keyword ) )
	    return ed->description;
    }
    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex = find_mob_index( vnum );

    if ( pMobIndex == NULL && fBootDb )
    {
	bug( "Get_mob_index: bad vnum %d.", vnum );
	exit( 1 );
    }

    return pMobIndex;
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex = find_obj_index( vnum );

    if ( pObjIndex == NULL && fBootDb )
    {
	bug( "Get_obj_index: bad vnum %d.", vnum );
	exit( 1 );
    }

    return pObjIndex;
}



/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex = find_room_index( vnum );

    if ( pRoomIndex == NULL && fBootDb )
    {
	bug( "Get_room_index: bad vnum %d.", vnum );
	exit( 1 );
    }

    return pRoomIndex;
}



/*
 * Salt hash araması: açılışta bile bulunamayınca NULL döner (bug/exit yok).
 * Yükleyicilerdeki fBootDb aç/kapa dansının yerine.
 */
MOB_INDEX_DATA *find_mob_index( int vnum )
{
    MOB_INDEX_DATA *p;

    for ( p = mob_index_hash[vnum % MAX_KEY_HASH]; p != NULL; p = p->next )
	if ( p->vnum == vnum )
	    return p;
    return NULL;
}

OBJ_INDEX_DATA *find_obj_index( int vnum )
{
    OBJ_INDEX_DATA *p;

    for ( p = obj_index_hash[vnum % MAX_KEY_HASH]; p != NULL; p = p->next )
	if ( p->vnum == vnum )
	    return p;
    return NULL;
}

ROOM_INDEX_DATA *find_room_index( int vnum )
{
    ROOM_INDEX_DATA *p;

    for ( p = room_index_hash[vnum % MAX_KEY_HASH]; p != NULL; p = p->next )
	if ( p->vnum == vnum )
	    return p;
    return NULL;
}

/*
 * Bölüm kaydı başlığı: '#' ve vnum okur; 0 bölüm sonu demektir.
 */
static int read_vnum_header( FILE *fp, const char *who )
{
    if ( fread_letter( fp ) != '#' )
    {
	bugf( "%s: # not found.", who );
	exit( 1 );
    }
    return fread_number( fp );
}

int read_mob_vnum( FILE *fp, const char *who )
{
    int vnum = read_vnum_header( fp, who );

    if ( vnum != 0 && find_mob_index( vnum ) != NULL )
    {
	bugf( "%s: vnum %d duplicated.", who, vnum );
	exit( 1 );
    }
    return vnum;
}

int read_obj_vnum( FILE *fp, const char *who )
{
    int vnum = read_vnum_header( fp, who );

    if ( vnum != 0 && find_obj_index( vnum ) != NULL )
    {
	bugf( "%s: vnum %d duplicated.", who, vnum );
	exit( 1 );
    }
    return vnum;
}

int read_room_vnum( FILE *fp, const char *who )
{
    int vnum = read_vnum_header( fp, who );

    if ( vnum != 0 && find_room_index( vnum ) != NULL )
    {
	bugf( "%s: vnum %d duplicated.", who, vnum );
	exit( 1 );
    }
    return vnum;
}

void hash_insert_mob( MOB_INDEX_DATA *pMobIndex )
{
    int iHash = pMobIndex->vnum % MAX_KEY_HASH;

    pMobIndex->next	= mob_index_hash[iHash];
    mob_index_hash[iHash] = pMobIndex;
    top_mob_index++;
}

void hash_insert_obj( OBJ_INDEX_DATA *pObjIndex )
{
    int iHash = pObjIndex->vnum % MAX_KEY_HASH;

    pObjIndex->next	= obj_index_hash[iHash];
    obj_index_hash[iHash] = pObjIndex;
    top_obj_index++;
}

void hash_insert_room( ROOM_INDEX_DATA *pRoomIndex )
{
    int iHash = pRoomIndex->vnum % MAX_KEY_HASH;

    pRoomIndex->next	= room_index_hash[iHash];
    room_index_hash[iHash] = pRoomIndex;
    top_room++;
}

/*
 * Bir eşya kaydının kuyruğu: 'A' (etki), allow_f ise 'F' (bayrak etkisi) ve
 * 'E' (ek açıklama) satırları. keep FALSE ise etkiler okunup atılır (rastgele
 * eşyalar kendi etkilerini üretir), açıklamalar yine saklanır.
 */
void load_obj_affects( FILE *fp, OBJ_INDEX_DATA *pObjIndex, int paf_level,
		       bool allow_f, bool keep )
{
    for ( ; ; )
    {
	char letter = fread_letter( fp );

	if ( letter == 'A' || ( letter == 'F' && allow_f ) )
	{
	    AFFECT_DATA discard;
	    AFFECT_DATA *paf = keep ? (AFFECT_DATA *)alloc_perm( sizeof(*paf) )
				    : &discard;

	    paf->where = TO_OBJECT;
	    if ( letter == 'F' )
	    {
		switch ( fread_letter( fp ) )
		{
		case 'A': paf->where = TO_AFFECTS;	break;
		case 'I': paf->where = TO_IMMUNE;	break;
		case 'R': paf->where = TO_RESIST;	break;
		case 'V': paf->where = TO_VULN;		break;
		case 'D': paf->where = TO_DETECTS;	break;
		default:
		    if ( keep )
		    {
			bug( "Load_objects: Bad where on flag set.", 0 );
			exit( 1 );
		    }
		    break;
		}
	    }
	    paf->type		= -1;
	    paf->level		= paf_level;
	    paf->duration	= -1;
	    paf->location	= fread_number( fp );
	    paf->modifier	= fread_number( fp );
	    paf->bitvector	= letter == 'F' ? fread_flag( fp ) : 0;

	    if ( keep )
	    {
		paf->next		= pObjIndex->affected;
		pObjIndex->affected	= paf;
		top_affect++;
	    }
	}
	else if ( letter == 'E' )
	{
	    EXTRA_DESCR_DATA *ed;

	    ed			= (EXTRA_DESCR_DATA *)alloc_perm( sizeof(*ed) );
	    ed->keyword		= fread_string( fp );
	    ed->description	= fread_string( fp );
	    ed->next		= pObjIndex->extra_descr;
	    pObjIndex->extra_descr = ed;
	    top_ed++;
	}
	else
	{
	    ungetc( letter, fp );
	    break;
	}
    }
}

/*
 * "harf ..." satırlarından oluşan bölüm (SPECIALS, PRACTICERS, OMPROGS,
 * OLIMITS): '*' yorum, 'S' son; letters içindeki harfler fn'e verilir,
 * her satırın kalanı atlanır.
 */
void load_directives( FILE *fp, const char *who, const char *letters,
		      void (*fn)( FILE *fp, char letter ) )
{
    for ( ; ; )
    {
	char letter = fread_letter( fp );

	if ( letter == 'S' )
	    return;

	if ( letter != '*' )
	{
	    if ( letter == '\0' || strchr( letters, letter ) == NULL )
	    {
		bugf( "%s: letter '%c' not *%sS.", who, letter, letters );
		exit( 1 );
	    }
	    fn( fp, letter );
	}

	fread_to_eol( fp );
    }
}



/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    int c;

    do
    {
	c = getc( fp );
    }
    while ( c != EOF && isspace( c ) );

    return (char) c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    long long number;
    bool sign;
    int c;

    do
    {
	c = getc( fp );
    }
    while ( c != EOF && isspace( c ) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( c == EOF || !isdigit( c ) )
    {
	bug( "Fread_number: bad format.", 0 );
	exit( 1 );
    }

    while ( c != EOF && isdigit( c ) )
    {
	number = number * 10 + c - '0';
	if ( number > INT_MAX )
	{
	    bug( "Fread_number: number too large.", 0 );
	    exit( 1 );
	}
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' && c != EOF )
	ungetc( c, fp );

    return (int) number;
}

long fread_flag( FILE *fp)
{
    long number;
    int c;
    bool negative = FALSE;

    do
    {
	c = getc( fp );
    }
    while ( c != EOF && isspace( c ) );

    if (c == '-')
    {
	negative = TRUE;
	c = getc(fp);
    }

    number = 0;

    if ( c == EOF || !isdigit( c ) )
    {
	while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
	{
	    number += flag_convert( (char) c );
	    c = getc(fp);
	}
    }

    while ( c != EOF && isdigit( c ) )
    {
	number = number * 10 + c - '0';
	c = getc(fp);
    }

    if (c == '|')
	number += fread_flag(fp);

    else if  ( c != ' ' && c != EOF )
	ungetc(c,fp);

    if (negative)
	return -1 * number;

    return number;
}

/*
 * Bayrak harfi -> bit: A..Z = 2^0..2^25, a..z = 2^26..2^51.
 * long'a sığmayan bit (Win64'te long 32 bit) 0 döner ve günlüğe yazılır.
 */
long flag_convert(char letter )
{
    int n;

    if ('A' <= letter && letter <= 'Z')
	n = letter - 'A';
    else if ('a' <= letter && letter <= 'z')
	n = 26 + letter - 'a';
    else
	return 0;

    if ( n >= (int) ( sizeof(long) * CHAR_BIT - 1 ) )
    {
	bug( "Flag_convert: bit %d does not fit in long.", n );
	return 0;
    }

    return (long) ( 1UL << n );
}



/*
 * Dizgi alanının sonu; ham okuma bu adrese dayanınca durulur.
 */
#define STRING_SPACE_END	( &string_space[MAX_STRING] )

static void fread_string_overflow( void )
{
    bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
    exit( 1 );
}

/*
 * top_string + sizeof(char *) adresinden başlayan, '\0' ile biten ham
 * dizgiyi bitirir: eski Latin-5 girdiyi UTF-8'e çevirir, uzunluk karmasında
 * arar; açılışta yeni dizgiyi alanda kalıcı kılar, sonrasında str_dup ile
 * kopyalar. fread_string ve fread_string_eol'un ortak kuyruğu.
 */
static char *intern_read_string( void )
{
    union
    {
	char *	pc;
	char	rgc[sizeof(char *)];
    } u1;
    size_t ic;
    int iHash;
    char *pHash;
    char *pHashPrev;
    char *pString;
    char *plast;
    char *pStart = top_string + sizeof(char *);
    size_t need = strlen( pStart ) + 1;

    /*
     * Latin-5 dönüşümünde her yüksek bayt bir bayt büyür; alan yetmiyorsa
     * sessizce kesmek yerine dur.
     */
    if ( !utf8_valid( pStart ) )
    {
	const unsigned char *p;

	for ( p = (const unsigned char *) pStart; *p != '\0'; p++ )
	    if ( *p >= 0x80 )
		need++;
	if ( pStart + need > STRING_SPACE_END )
	    fread_string_overflow( );
	utf8_from_latin5( pStart, need );
    }

    plast = pStart + strlen( pStart ) + 1;

    iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
    for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
    {
	for ( ic = 0; ic < sizeof(char *); ic++ )
	    u1.rgc[ic] = pHash[ic];
	pHashPrev = u1.pc;
	pHash    += sizeof(char *);

	if ( pStart[0] == pHash[0]
	&&   !strcmp( pStart + 1, pHash + 1 ) )
	    return pHash;
    }

    if ( fBootDb )
    {
	pString		= top_string;
	top_string	= plast;
	u1.pc		= string_hash[iHash];
	for ( ic = 0; ic < sizeof(char *); ic++ )
	    pString[ic] = u1.rgc[ic];
	string_hash[iHash]	= pString;

	nAllocString += 1;
	sAllocString += top_string - pString;
	return pString + sizeof(char *);
    }

    return str_dup( pStart );
}

/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp )
{
    char *plast;
    int c;

    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
	fread_string_overflow( );

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	c = getc( fp );
    }
    while ( c != EOF && isspace( c ) );

    if ( c == '~' )
	return &str_empty[0];

    for ( ;; )
    {
	if ( c == EOF )
	{
	    bug( "Fread_string: EOF", 0 );
	    return &str_empty[0];
	}

	if ( plast >= STRING_SPACE_END - 2 )
	    fread_string_overflow( );

	switch ( c )
	{
	default:
	    *plast++ = (char) c;
	    break;

	case '\n':
	    *plast++ = '\n';
	    *plast++ = '\r';
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return intern_read_string( );
	}

	c = getc( fp );
    }
}

/*
 * Satır sonuna kadar dizgi okur (sosyaller). '\n'/'\r' ile biter, '~' yok.
 */
char *fread_string_eol( FILE *fp )
{
    char *plast;
    int c;

    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
	fread_string_overflow( );

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	c = getc( fp );
    }
    while ( c != EOF && isspace( c ) );

    if ( c == '\n' )
	return &str_empty[0];

    for ( ;; )
    {
	if ( c == EOF )
	{
	    bug( "Fread_string_eol  EOF", 0 );
	    exit( 1 );
	}

	if ( plast >= STRING_SPACE_END - 1 )
	    fread_string_overflow( );

	if ( c == '\n' || c == '\r' )
	{
	    *plast = '\0';
	    return intern_read_string( );
	}

	*plast++ = (char) c;
	c = getc( fp );
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    int c;

    do
    {
	c = getc( fp );
    }
    while ( c != EOF && c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    if ( c != EOF )
	ungetc( c, fp );
    return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    int cEnd;
    int c;

    do
    {
	cEnd = getc( fp );
    }
    while ( cEnd != EOF && isspace( cEnd ) );

    if ( cEnd == EOF )
    {
	bug( "Fread_word: EOF.", 0 );
	exit( 1 );
    }

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = (char) cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	c = getc( fp );
	if ( c == EOF || ( cEnd == ' ' ? isspace( c ) : c == cEnd ) )
	{
	    if ( cEnd == ' ' && c != EOF )
		ungetc( c, fp );
	    *pword = '\0';
	    return word;
	}
	*pword = (char) c;
    }

    bug( "Fread_word: word too long.", 0 );
    exit( 1 );
    return NULL;
}

/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem( int sMem )
{
    void *pMem;
    intptr_t *magic;
    int iList;

    sMem += sizeof(*magic);

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
        if ( sMem <= rgSizeList[iList] )
            break;
    }

    if ( iList == MAX_MEM_LIST )
    {
        bug( "Alloc_mem: size %d too large.", sMem );
        exit( 1 );
    }

    if ( rgFreeList[iList] == NULL )
    {
        pMem              = alloc_perm( rgSizeList[iList] );
    }
    else
    {
        pMem              = rgFreeList[iList];
        rgFreeList[iList] = * ((void **) rgFreeList[iList]);
    }

    magic = (intptr_t *) pMem;
    *magic = MAGIC_NUM;
    pMem = (char *) pMem + sizeof(*magic);

    return pMem;
}



/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem( void *pMem, int sMem )
{
    int iList;
    intptr_t *magic;

    pMem = (char *) pMem - sizeof(*magic);
    magic = (intptr_t *) pMem;

    if (*magic != MAGIC_NUM)
    {
        bug("Attempt to recyle invalid memory of size %d.",sMem);
        bugf("%s", (char*) pMem + sizeof(*magic));
        return;
    }

    *magic = 0;
    sMem += sizeof(*magic);

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
        if ( sMem <= rgSizeList[iList] )
            break;
    }

    if ( iList == MAX_MEM_LIST )
    {
        bug( "Free_mem: size %d too large.", sMem );
        exit( 1 );
    }

    * ((void **) pMem) = rgFreeList[iList];
    rgFreeList[iList]  = pMem;

    return;
}


/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm( int sMem )
{
    static char *pMemPerm;
    static int iMemPerm;
    void *pMem;
    const int align = (int) _Alignof(max_align_t);

    /* Win64'te long 4 bayt: hizayı her türü kapsayan max_align_t belirler. */
    sMem = ( sMem + align - 1 ) / align * align;
    if ( sMem > MAX_PERM_BLOCK )
    {
	bug( "Alloc_perm: %d too large.", sMem );
	exit( 1 );
    }

    if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
	iMemPerm = 0;
	if ( ( pMemPerm = (char *)calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
	{
	    perror( "Alloc_perm" );
	    exit( 1 );
	}
    }

    pMem        = pMemPerm + iMemPerm;
    iMemPerm   += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
    return pMem;
}



/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup( const char *str )
{
    char *str_new;

    if ( str[0] == '\0' )
	return &str_empty[0];

    if ( str >= string_space && str < top_string )
	return (char *) str;

    str_new = (char *)alloc_mem( strlen(str) + 1 );
    strcpy( str_new, str );
    return str_new;
}



/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string( char *pstr )
{
    if ( pstr == NULL
    ||   pstr == &str_empty[0]
    || ( pstr >= string_space && pstr < top_string ) )
	return;

    free_mem( pstr, strlen(pstr) + 1 );
    return;
}


void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    printf_to_char(ch,"Bölgeler:\n\r\n\r");
    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
        printf_to_char(ch,"[{W%2d %3d{x] {c%*s{x - {c%s{x\n\r",pArea->low_range,pArea->high_range,utf8_width(pArea->name, 25), pArea->name,pArea->path);
    }

    return;
}



void do_memory( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    snprintf(buf, sizeof(buf), "Affects %5d\n\r", top_affect    ); send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "Areas   %5d\n\r", top_area      ); send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "ExDes   %5d\n\r", top_ed        ); send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "Exits   %5d\n\r", top_exit      ); send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "Helps   %5d\n\r", top_help      ); send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "Socials %5d\n\r", social_count  ); send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "Mobs    %5d(%d new format)\n\r", top_mob_index,newmobs );
    send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "(in use)%5d\n\r", mobile_count  ); send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "Objs    %5d(%d new format)\n\r", top_obj_index,newobjs );
    send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "Resets  %5d\n\r", top_reset     ); send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "Rooms   %5d\n\r", top_room      ); send_to_char( buf, ch );
    snprintf(buf, sizeof(buf), "Shops   %5d\n\r", top_shop      ); send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Strings %5d strings of %7d bytes (max %d).\n\r",
	nAllocString, sAllocString, MAX_STRING );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Perms   %5d blocks  of %7d bytes.\n\r",
	nAllocPerm, sAllocPerm );
    send_to_char( buf, ch );

    return;
}

/*
 * Çalışma dizininde (area/) döküm dosyası açar; açılamazsa günlüğe yazar.
 */
static FILE *dump_open( const char *name )
{
    FILE *fp;

    if ( ( fp = fopen( name, "w" ) ) == NULL )
	bugf( "Do_dump: %s açılamadı.", name );
    return fp;
}

void do_dump( CHAR_DATA *ch, char *argument )
{
    int count,count2,num_pcs,aff_count;
    CHAR_DATA *fch;
    MOB_INDEX_DATA *pMobIndex;
    PC_DATA *pc;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *room;
    EXIT_DATA *exit;
    DESCRIPTOR_DATA *d;
    AFFECT_DATA *af;
    FILE *fp;
    int vnum,nMatch = 0;

    /* open file */
    fclose(fpReserve);

    if ( ( fp = dump_open( "mem.dmp" ) ) != NULL )
    {
	/* report use of data structures */

	num_pcs = 0;
	aff_count = 0;

	/* mobile prototypes */
	fprintf(fp,"MobProt	%4d (%8d bytes)\n",
	    top_mob_index, (int)(top_mob_index * (sizeof(*pMobIndex))));

	/* mobs */
	count = 0;  count2 = 0;
	for (fch = char_list; fch != NULL; fch = fch->next)
	{
	    count++;
	    if (fch->pcdata != NULL)
		num_pcs++;
	    for (af = fch->affected; af != NULL; af = af->next)
		aff_count++;
	}
	for (fch = char_free; fch != NULL; fch = fch->next)
	    count2++;

	fprintf(fp,"Mobs	%4d (%8d bytes), %2d free (%d bytes)\n",
	    count, (int)(count * (sizeof(*fch))), count2, (int)(count2 * (sizeof(*fch))));

	/* pcdata */
	count = 0;
	for (pc = pcdata_free; pc != NULL; pc = pc->next)
	    count++;

	fprintf(fp,"Pcdata	%4d (%8d bytes), %2d free (%d bytes)\n",
	    num_pcs, (int)(num_pcs * (sizeof(*pc))), count, (int)(count * (sizeof(*pc))));

	/* descriptors */
	count = 0; count2 = 0;
	for (d = descriptor_list; d != NULL; d = d->next)
	    count++;
	for (d= descriptor_free; d != NULL; d = d->next)
	    count2++;

	fprintf(fp, "Descs	%4d (%8d bytes), %2d free (%d bytes)\n",
	    count, (int)(count * (sizeof(*d))), count2, (int)(count2 * (sizeof(*d))));

	/* object prototypes */
	for ( vnum = 0; nMatch < top_obj_index; vnum++ )
	    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	    {
		for (af = pObjIndex->affected; af != NULL; af = af->next)
		    aff_count++;
		nMatch++;
	    }

	fprintf(fp,"ObjProt	%4d (%8d bytes)\n",
	    top_obj_index, (int)(top_obj_index * (sizeof(*pObjIndex))));


	/* objects */
	count = 0;  count2 = 0;
	for (obj = object_list; obj != NULL; obj = obj->next)
	{
	    count++;
	    for (af = obj->affected; af != NULL; af = af->next)
		aff_count++;
	}
	for (obj = obj_free; obj != NULL; obj = obj->next)
	    count2++;

	fprintf(fp,"Objs	%4d (%8d bytes), %2d free (%d bytes)\n",
	    count, (int)(count * (sizeof(*obj))), count2, (int)(count2 * (sizeof(*obj))));

	/* affects */
	count = 0;
	for (af = affect_free; af != NULL; af = af->next)
	    count++;

	fprintf(fp,"Affects	%4d (%8d bytes), %2d free (%d bytes)\n",
	    aff_count, (int)(aff_count * (sizeof(*af))), count, (int)(count * (sizeof(*af))));

	/* rooms */
	fprintf(fp,"Rooms	%4d (%8d bytes)\n",
	    top_room, (int)(top_room * (sizeof(*room))));

	 /* exits */
	fprintf(fp,"Exits	%4d (%8d bytes)\n",
	    top_exit, (int)(top_exit * (sizeof(*exit))));

	fclose(fp);
    }

    /* start printing out mobile data */
    if ( ( fp = dump_open( "mob.dmp" ) ) != NULL )
    {
	fprintf(fp,"\nMobile Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_mob_index; vnum++)
	    if ((pMobIndex = get_mob_index(vnum)) != NULL)
	    {
		nMatch++;
		fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
		    pMobIndex->vnum,pMobIndex->count,
		    pMobIndex->killed,pMobIndex->short_descr);
	    }
	fclose(fp);
    }

    /* start printing out object data */
    if ( ( fp = dump_open( "obj.dmp" ) ) != NULL )
    {
	fprintf(fp,"\nObject Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
	    if ((pObjIndex = get_obj_index(vnum)) != NULL)
	    {
		nMatch++;
		fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
		    pObjIndex->vnum,pObjIndex->count,
		    pObjIndex->reset_num,pObjIndex->short_descr);
	    }

	/* close file */
	fclose(fp);
    }

    fpReserve = fopen( NULL_FILE, "r" );
}



/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_range(0,3) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Mangus'taki rastgele sayi uretiminin temel yeri.
 * Modula ile yapilan hesaplamanin problemini
 * asmaya calisiyoruz.
 */
int number_range( int from, int to )
{
  int x;
  int fark;

  fark = abs(to - from);

  if( fark == 0 )
  {
    return from;
  }

  do {
    x = rng_next() & RNG_MAX;
  } while (x >= (RNG_MAX - (RNG_MAX % (fark+1))));

  x %= (fark+1);

  return x + from;
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
    return number_range(1,100);
}



/*
 * Generate a random door.
 */
int number_door( void )
{
    return number_range(0,5);
}

/*
 * Taşınabilir xorshift128 üreteci (rand()/random() farklılıklarından bağımsız).
 */
static uint32_t rng_state[4] = { 0x12345678u, 0x9abcdef0u, 0xdeadbeefu, 0xcafebabeu };

static uint32_t rng_next( void )
{
    uint32_t t = rng_state[3];
    uint32_t s = rng_state[0];

    rng_state[3] = rng_state[2];
    rng_state[2] = rng_state[1];
    rng_state[1] = s;
    t ^= t << 11;
    t ^= t >> 8;
    return rng_state[0] = t ^ s ^ ( s >> 19 );
}

void init_random_number_generator( void )
{
    uint64_t z = (uint64_t) time( NULL ) ^ ( (uint64_t) platform_pid( ) << 32 );
    int i;

    for ( i = 0; i < 4; i++ )
    {
	z += 0x9e3779b97f4a7c15ULL;
	z = ( z ^ ( z >> 30 ) ) * 0xbf58476d1ce4e5b9ULL;
	z = ( z ^ ( z >> 27 ) ) * 0x94d049bb133111ebULL;
	rng_state[i] = (uint32_t) ( z ^ ( z >> 31 ) ) | 1u;
    }
}

/*
 * Roll some dice.
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }

    return;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
	bug( "Str_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Str_cmp: null bstr.", 0 );
	return TRUE;
    }

    return utf8_str_cmp( astr, bstr );
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
	bug( "Strn_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Strn_cmp: null bstr.", 0 );
	return TRUE;
    }

    return utf8_str_prefix( astr, bstr );
}






/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}



/*
 * İlk harfi büyütür ya da küçültür. UTF-8'de harfin bayt uzunluğu
 * değişirse (i/İ, ı/I) yerinde değiştirilemez; yeni dizgi döndürülür.
 */
char *first_case( char *str, bool upper )
{
    static char buf[MAX_STRING_LENGTH];
    uint32_t cp, cp2;
    int len, len2;
    char enc[4];

    if ( str == NULL || ( len = utf8_decode( str, &cp ) ) <= 0 )
	return str;
    cp2 = upper ? utf8_toupper_cp( cp ) : utf8_tolower_cp( cp );
    if ( cp2 == cp )
	return str;
    len2 = utf8_encode( cp2, enc );
    if ( len2 == len )
    {
	memcpy( str, enc, (size_t) len );
	return str;
    }
    if ( strlen( str ) + 2 > sizeof(buf) )
	return str;
    memcpy( buf, enc, (size_t) len2 );
    strcpy( buf + len2, str + len );
    return str_dup( buf );
}

/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    size_t o = 0;
    bool first = TRUE;
    uint32_t cp;
    int len, n;
    char enc[4];

    /*
     * ASCII harfler eski davranışı korur (oyuncu dosya adları buna dayanır:
     * 'i' -> 'I'); Türkçe harfler UTF-8 kurallarıyla dönüştürülür.
     */
    while ( ( len = utf8_decode( str, &cp ) ) > 0 )
    {
	str += len;
	if ( cp < 0x80 )
	    cp = first ? (uint32_t) UPPER( (int) cp ) : (uint32_t) LOWER( (int) cp );
	else if ( cp != UTF8_REPLACEMENT )
	    cp = first ? utf8_toupper_cp( cp ) : utf8_tolower_cp( cp );
	n = utf8_encode( cp, enc );
	if ( o + (size_t) n + 1 >= sizeof(strcap) )
	    break;
	memcpy( strcap + o, enc, (size_t) n );
	o += (size_t) n;
	first = FALSE;
    }
    strcap[o] = '\0';
    return strcap;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;

    if ( IS_NPC(ch) || str[0] == '\0' )
	return;

    fclose( fpReserve );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	send_to_char( "Dosya açılamadı!\n\r", ch );
    }
    else
    {
	fprintf( fp, "[%5d] %s: %s\n",
	    ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * bug()'ın tek int parametresiyle güvenle biçimlenebilecek dizgi: hiç
 * dönüşüm yok ya da yalnızca bir tane, o da tam sayı alan (%d %i %c %u %x,
 * isteğe bağlı bayrak/genişlik/'l'). Başka her şey veri sayılıp olduğu gibi
 * yazılır; böylece oda adı/oyuncu verisi taşıyan tamponlar biçim olamaz.
 */
static bool bug_format_ok( const char *str )
{
    int conversions = 0;

    for ( ; *str != '\0'; str++ )
    {
	if ( *str != '%' )
	    continue;
	str++;
	if ( *str == '%' )
	    continue;
	if ( ++conversions > 1 )
	    return FALSE;
	while ( *str == '-' || *str == '+' || *str == ' ' || *str == '#'
	     || *str == '0' || *str == '.' || isdigit( (unsigned char) *str ) )
	    str++;
	if ( *str == 'l' )
	    str++;
	if ( *str == '\0' || strchr( "dicuxX", *str ) == NULL )
	    return FALSE;
    }

    return TRUE;
}

/*
 * Reports a bug.
 */
void bug( const char *str, int param )
{
    char buf[MAX_STRING_LENGTH];
    char msg[MAX_STRING_LENGTH];

    if ( fpArea != NULL )
    {
	int iLine;
	long iChar;

	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
	    int c;

	    iChar = ftell( fpArea );
	    fseek( fpArea, 0, 0 );
	    for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	    {
		while ( ( c = getc( fpArea ) ) != '\n' && c != EOF )
		    ;
		if ( c == EOF )
		    break;
	    }
	    fseek( fpArea, iChar, 0 );
	}

	snprintf(buf, sizeof(buf), "[*****] FILE: %s LINE: %d", strArea, iLine );
	log_string( buf );
    }

    if ( str == NULL )
	str = "(null)";

    if ( bug_format_ok( str ) )
	snprintf( msg, sizeof(msg), str, param );
    else
	snprintf( msg, sizeof(msg), "%s", str );

    snprintf( buf, sizeof(buf), "[*****] BUG: %s", msg );
    log_string( buf );

    return;
}



/*
 * Writes a string to the log.
 */
void log_string( const char *str )
{
    char *strtime;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    fprintf( stderr, "%s :: %s\n", strtime, str );
    return;
}



/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}


static void olimit_directive( FILE *fp, char letter )
{
  int vnum;
  int limit;
  OBJ_INDEX_DATA *pIndex;

  (void) letter;	/* yalnızca 'O' */
  vnum = fread_number(fp);
  limit = fread_number(fp);
  if ( (pIndex = get_obj_index(vnum)) == NULL)
    {
      bug("Load_olimits: bad vnum %d",vnum);
      exit(1);
    }
  pIndex->limit = limit;
}

void load_olimits(FILE *fp)
{
  load_directives( fp, "Load_olimits", "O", olimit_directive );
}


/*
 * Add the objects in players not logged on to object count
 */
void load_limited_objects()
{
  struct dirent *dp;

  int i;
  DIR *dirp;
  FILE *pfile;
  OBJ_INDEX_DATA *pIndex;
  char letter;
  char *word;
  char buf[MAX_INPUT_LENGTH];
  bool fReadLevel;
  int vnum;
  int tplayed;

  total_levels = 0;

  if ( (dirp = opendir(PLAYER_DIR)) == NULL)
  {
    bug("Load_limited_objects: unable to open player directory.",0);
    exit(1);
  }

  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp) )
  {
    if (strlen(dp->d_name) >= 3)
    {
      snprintf(buf, sizeof(buf), "%s/%s",PLAYER_DIR, dp->d_name);
      fReadLevel = FALSE;
      tplayed = 0;
      snprintf(log_buf, sizeof(log_buf),"[%s] okunacak.\n\r",buf);
      dump_to_scr(log_buf);
      if ( (pfile = fopen(buf, "r")) == NULL)
      {
        bug("Load_limited_objects: Can't open player file.",0);
      }
      else
      {
        /* fread_letter char döner: EOF'u feof ile yakala (unsigned char'lı
           platformda (char)EOF != EOF olup döngü hiç bitmiyordu). */
        for (letter = fread_letter(pfile); !feof(pfile); letter = fread_letter(pfile) )
        {
          if (letter == 'L')
          {
            if (!fReadLevel)
            {
              word = fread_word(pfile);

              if (!str_cmp(word, "evl") || !str_cmp(word,"ev") || !str_cmp(word, "evel"))
              {
                i = fread_number(pfile);
                fReadLevel = TRUE;
                total_levels += UMAX(0,i - 5);
                snprintf(log_buf, sizeof(log_buf),"[%s]'s file +: %d\n\r",buf, UMAX(0,i-5));
                dump_to_scr(log_buf);
                continue;
              }
            }
          }
          else if (letter == 'P')
          {
            word = fread_word(pfile);

            if (!str_cmp(word, "layLog") )
            {
              int d, t;
              int today = parse_date( current_time );

              fread_number(pfile);	/* read the version */
              while (1)
              {
                if ( (d = fread_number(pfile)) < 0 )
                {
                  break;
                }
                t = fread_number(pfile);
                if ( today > 14 )
                {
                  if (d <= today && d > (today - 14))
                  {
                    tplayed += t;
                  }
                }
                else
                {
                  if ( d < today )
                  {
                    d+= 365;
                  }
                  if (d<=(today + 365) && d>(today + 351))
                  {
                    tplayed += t;
                  }
                }
              }
            }
          }
          else if (letter == '#')
          {
            word = fread_word(pfile);
            if (!str_cmp(word, "O") || !str_cmp(word, "OBJECT"))
            {
              if ( tplayed < MIN_TIME_LIMIT )
              {
                snprintf(log_buf, sizeof(log_buf),"Discarding the player %s for limited equipments!.\n",buf);
                dump_to_scr( log_buf );
                break;
              }
              fread_word(pfile);
              vnum = fread_number(pfile);
              if ( (pIndex = find_obj_index(vnum)) != NULL)
              {
                pIndex->count++;
                if( pIndex->limit != -1 )
                {
                    snprintf(log_buf, sizeof(log_buf),"**** Limit eq in player file: %d.\n",vnum);
                    dump_to_scr( log_buf );
                }
              }
            }
          }
          else fread_to_eol(pfile);
        }
        fclose(pfile);
      }
    }
  }
  closedir(dirp);
}

/*
 * Given a name, return the appropriate prac fun.
 */
long prac_lookup( const char *name )
{
   int i;

   for ( i = 0; prac_table[i].name != NULL; i++)
   {
        if (utf8_first_eq(name, prac_table[i].name)
        &&  !str_prefix( name,prac_table[i].name))
            return (1 << prac_table[i].number);
   }

    return 0;
}

/*
 * Snarf can prac declarations.
 */
static void practicer_directive( FILE *fp, char letter )
{
    MOB_INDEX_DATA *pMobIndex;

    (void) letter;	/* yalnızca 'M' */
    pMobIndex	= get_mob_index	( fread_number ( fp ) );
    SET_BIT(pMobIndex->practicer,prac_lookup( fread_word(fp) ) );
    if ( pMobIndex->practicer == 0 )
    {
	bug( "Load_practicers: 'M': vnum %d.", pMobIndex->vnum );
	exit( 1 );
    }
}

void load_practicer( FILE *fp )
{
    load_directives( fp, "Load_practicers", "M", practicer_directive );
}


void load_resetmsg( FILE *fp )
{
    Serarea->resetmsg	= fread_string(fp);
    return;
}

void load_aflag( FILE *fp )
{
    Serarea->area_flag	= fread_flag(fp);
    return;
}
