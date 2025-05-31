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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"

/* language staff */
const struct translation_type translation_table[]=
{
        {"a",                   "e"},
        {"A",                   "E"},
        {"b",                   "c"},
        {"B",                   "C"},
        {"c",                   "d"},
        {"C",                   "D"},
        {"d",                   "f"},
        {"D",                   "F"},
        {"e",                   "i"},
        {"E",                   "I"},
        {"f",                   "g"},
        {"F",                   "G"},
        {"g",                   "h"},
        {"G",                   "H"},
        {"h",                   "j"},
        {"H",                   "J"},
        {"i",                   "o"},
        {"I",                   "O"},
        {"j",                   "k"},
        {"J",                   "K"},
        {"k",                   "l"},
        {"K",                   "L"},
        {"l",                   "m"},
        {"L",                   "M"},
        {"m",                   "n"},
        {"M",                   "N"},
        {"n",                   "p"},
        {"N",                   "P"},
        {"o",                   "u"},
        {"O",                   "U"},
        {"p",                   "q"},
        {"P",                   "Q"},
        {"q",                   "r"},
        {"Q",                   "R"},
        {"r",                   "s"},
        {"R",                   "S"},
        {"s",                   "t"},
        {"S",                   "T"},
        {"t",                   "v"},
        {"T",                   "V"},
        {"u",                   "y"},
        {"U",                   "Y"},
        {"v",                   "w"},
        {"V",                   "W"},
        {"w",                   "x"},
        {"W",                   "X"},
        {"x",                   "z"},
        {"X",                   "Z"},
        {"y",                   "a"},
        {"Y",                   "A"},
        {"z",                   "b"},
        {"Z",                   "B"},
        {"",            	"" }
};

const   struct  language_type       language_table      [ MAX_LANGUAGE ] =
{
  { "ortak",	LANG_COMMON	},
  { "insan",	LANG_HUMAN	},
  { "elf",	LANG_ELVISH	},
  { "cüce",	LANG_DWARVISH	},
  { "kuş",	LANG_BIRD	},
  { "kedi",	LANG_CAT	},
  { "asura",	LANG_ASURA	},
  { "sürüngen",	LANG_SURUNGEN	},
  { "astral",	LANG_ASTRAL	},
  { "dev",	LANG_GIANT	},
  { "ejderha",	LANG_DRAGON	},
  { "su",	LANG_WATER	},
  { "su",	LANG_WATER	},
  { "köpek",	LANG_DOG	},
};

/* item type list */
const struct item_type		item_table	[]	=
{
    {	ITEM_LIGHT,	"light"		},
    {	ITEM_SCROLL,	"scroll"	},
    {	ITEM_WAND,	"wand"		},
    {   ITEM_STAFF,	"staff"		},
    {   ITEM_WEAPON,	"weapon"	},
    {   ITEM_TREASURE,	"treasure"	},
    {   ITEM_ARMOR,	"armor"		},
    {	ITEM_POTION,	"potion"	},
    {	ITEM_CLOTHING,	"clothing"	},
    {   ITEM_FURNITURE,	"furniture"	},
    {	ITEM_TRASH,	"trash"		},
    {	ITEM_CONTAINER,	"container"	},
    {	ITEM_DRINK_CON, "drink"		},
    {	ITEM_KEY,	"key"		},
    {	ITEM_FOOD,	"food"		},
    {	ITEM_MONEY,	"money"		},
    {	ITEM_BOAT,	"boat"		},
    {	ITEM_CORPSE_NPC,"npc_corpse"	},
    {	ITEM_CORPSE_PC,	"pc_corpse"	},
    {   ITEM_FOUNTAIN,	"fountain"	},
    {	ITEM_PILL,	"pill"		},
    {	ITEM_PROTECT,	"protect"	},
    {	ITEM_MAP,	"map"		},
    {	ITEM_PORTAL,	"portal"	},
    {	ITEM_WARP_STONE,"warp_stone"	},
    {	ITEM_ROOM_KEY,	"room_key"	},
    {	ITEM_GEM,	"gem"		},
    {	ITEM_JEWELRY,	"jewelry"	},
    {   ITEM_JUKEBOX,	"jukebox"	},
    {   ITEM_TATTOO,	"tattoo"	},
    {   ITEM_MAYMUNCUK,	"maymuncuk"	},
    {   0,		NULL		}
};


/* weapon selection table */
const	struct	weapon_type	weapon_table	[]	=
{
   { "sword",	OBJ_VNUM_SCHOOL_SWORD,	WEAPON_SWORD,	&gsn_sword	},
   { "mace",	OBJ_VNUM_SCHOOL_MACE,	WEAPON_MACE,	&gsn_mace 	},
   { "dagger",	OBJ_VNUM_SCHOOL_DAGGER,	WEAPON_DAGGER,	&gsn_dagger	},
   { "axe",	OBJ_VNUM_SCHOOL_AXE,	WEAPON_AXE,	&gsn_axe	},
   { "staff",	OBJ_VNUM_SCHOOL_STAFF,	WEAPON_SPEAR,	&gsn_spear	},
   { "flail",	OBJ_VNUM_SCHOOL_FLAIL,	WEAPON_FLAIL,	&gsn_flail	},
   { "whip",	OBJ_VNUM_SCHOOL_WHIP,	WEAPON_WHIP,	&gsn_whip	},
   { "polearm",	OBJ_VNUM_SCHOOL_POLEARM,WEAPON_POLEARM,	&gsn_polearm	},
   { "bow",	OBJ_VNUM_SCHOOL_BOW,	WEAPON_BOW,	&gsn_bow	},
   { "arrow",	OBJ_VNUM_SCHOOL_POLEARM,WEAPON_ARROW,	&gsn_arrow	},
   { "lance",	OBJ_VNUM_SCHOOL_LANCE,	WEAPON_LANCE,	&gsn_lance	},
   { NULL,	0,				0,	NULL		}
};



/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",	WIZ_PREFIX,	IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L4 },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	IM },
   {	"spam",		WIZ_SPAM,	L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",	WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	IM },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {	NULL,		0,		0  }
};

/* attack table */
const 	struct attack_type	attack_table	[]		=
{
    { 	"none",		"vuruş",		-1		},  /*  0 */
    {	"slice",	"dilme", 	DAM_SLASH	},
    {   "stab",		"saplama",		DAM_PIERCE	},
    {	"slash",	"kesici",	DAM_SLASH	},
    {	"whip",		"kamçı",		DAM_SLASH	},
    {   "claw",		"pençe",		DAM_SLASH	},  /*  5 */
    {	"blast",	"patlama",	DAM_BASH	},
    {   "pound",	"çarpma",	DAM_BASH	},
    {	"crush",	"ezme",	DAM_BASH	},
    {   "grep",		"kesici",		DAM_SLASH	},
    {	"bite",		"ısırma",		DAM_PIERCE	},  /* 10 */
    {   "pierce",	"delici",	DAM_PIERCE	},
    {   "suction",	"emici",	DAM_BASH	},
    {	"beating",	"beating",	DAM_BASH	},
    {   "digestion",	"öğütücü",	DAM_ACID	},
    {	"charge",	"yüklenme",	DAM_BASH	},  /* 15 */
    { 	"slap",		"tokat",		DAM_BASH	},
    {	"punch",	"yumruk",	DAM_BASH	},
    {	"wrath",	"gazap",	DAM_ENERGY	},
    {	"magic",	"büyü",	DAM_ENERGY	},
    {   "divine",	"ilahi güç",	DAM_HOLY	},  /* 20 */
    {	"cleave",	"yarma",	DAM_SLASH	},
    {	"scratch",	"tırmalama",	DAM_PIERCE	},
    {   "peck",		"gaga",		DAM_PIERCE	},
    {   "peckb",	"gaga",		DAM_BASH	},
    {   "chop",		"chop",		DAM_SLASH	},  /* 25 */
    {   "sting",	"sokma",	DAM_PIERCE	},
    {   "smash",	 "smash",	DAM_BASH	},
    {   "shbite",	"şok ısırışı",DAM_LIGHTNING	},
    {	"flbite",	"alev ısırışı", DAM_FIRE	},
    {	"frbite",	"ayaz ısırışı", DAM_COLD	},  /* 30 */
    {	"acbite",	"asit ısırışı", 	DAM_ACID	},
    {	"chomp",	"çiğneme",	DAM_PIERCE	},
    {  	"drain",	"yaşam emici",	DAM_NEGATIVE	},
    {   "thrust",	"thrust",	DAM_PIERCE	},
    {   "slime",	"balçık",	DAM_ACID	},
    {	"shock",	"şok",	DAM_LIGHTNING	},
    {   "thwack",	"thwack",	DAM_BASH	},
    {   "flame",	"alev",	DAM_FIRE	},
    {   "chill",	"ayaz",	DAM_COLD	},
    {   NULL,		NULL,		0		}
};

/* race table */
const 	struct	race_type	race_table	[]		=
{
/*
    const char *	name[2];			// call name of the race
    const char 	who_name[8];
		sh_int	language;		// language
    bool	pc_race;		// can be chosen by pcs
    bool	humanoid;		// intelligent humanoid races
    sh_int	size;
    long	det;			// det bits for the race
    long	act;			// act bits for the race
    long	aff;			// aff bits for the race
    long	off;			// off bits for the race
    long	imm;			// imm bits for the race
    long  res;			// res bits for the race
    long	vuln;			// vuln bits for the race
    long	form;			// default form flag for the race
    long	parts;			// default parts for the race
		int		points;			// cost in exp of the race
		const char *	skills[5];		// bonus skills for the race
		sh_int 	stats[MAX_STATS];	// starting stats
*/
    { {"unique","eşsiz"},	"",	0, FALSE,FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,{ "" }, {0,0,0,0,0,0 } },

    { /* 1 */
	{"human","insan"},"İnsan",LANG_HUMAN,	TRUE,TRUE, 		SIZE_MEDIUM, 0,
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 0, { "identify", "steal" }, { 20, 21, 20, 21, 21 ,20}
    },

    { /* 2 */
	{"elf","elf"},"Elf",LANG_ELVISH,		TRUE,TRUE, 		SIZE_MEDIUM, 0,
	0,		AFF_INFRARED|AFF_SNEAK,	0,
	0,		RES_CHARM,	VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 250, { "lore" , "camouflage" }, { 20, 23, 20, 22, 18, 20 }
    },

    { /* 3 */
        {"arial","arial"},"Arial",LANG_BIRD,        TRUE,TRUE, 		SIZE_MEDIUM, 0,
        0,              AFF_FLYING,             0,
        0,              RES_MENTAL,              0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K, 250, { "fast healing" }, { 18, 23, 20, 24, 18 , 20}
    },

	{ /* 4  */
	{"felar","felar"},"Felar",LANG_CAT,	TRUE,TRUE, 		SIZE_MEDIUM, 0,
	0,		AFF_INFRARED,	OFF_TAIL,
	0,		RES_LIGHT|RES_COLD|RES_MENTAL,     VULN_FIRE|VULN_DROWNING,
	A|H|M|V,        A|C|D|E|F|H|J|K|Q|U|V, 250, {  "hand to hand","ambush", "berserk", "steal" }, { 21, 19, 20, 23, 21, 20}
	},

    {  /* 5 */
	{"trol","trol"},"Trol",LANG_TROLLISH,	TRUE,TRUE, 		SIZE_LARGE, 0,
	0,		AFF_REGENERATION|AFF_INFRARED,	OFF_BERSERK,
 	0,	RES_CHARM|RES_BASH,	VULN_FIRE|VULN_ACID,
	A|B|H|M|V,		A|B|C|D|E|F|G|H|I|J|K|U|V, 250, { "fast healing","trance" }, { 22, 19, 19, 21, 23, 20}
    },

	{ /* 6 */
	  {"duergar","duergar"},"Duerg",LANG_DWARVISH,        TRUE,TRUE, 		SIZE_MEDIUM, 0,
	  0,                AFF_INFRARED,   0,
	  0,                RES_MAGIC|RES_POISON|RES_DISEASE, VULN_DROWNING,
	  A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 250, { "berserk", "pick lock" }, {20, 20, 21, 19, 23, 20 }
	},

    { /* 7 */
        {"dark-elf","kara-elf"},"K-Elf",LANG_ELVISH,     TRUE,TRUE, 		SIZE_MEDIUM, 0, /* Fix L335: Added braces */
        0,              AFF_INFRARED|AFF_SNEAK, 0,
        0,              RES_CHARM,      VULN_IRON,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K, 250, { "lore" , "camouflage" }, { 21, 22, 20, 22, 18, 20 }
    },

    { /* 8 */
	{"dwarf","cüce"},"Cüce",LANG_DWARVISH,	TRUE,TRUE, 		SIZE_MEDIUM, 0,
	0,		AFF_INFRARED,	0,
	0,		RES_POISON|RES_DISEASE, VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 250, { "berserk" }, { 21, 20, 21, 18, 23, 20 }
    },

	{ /* 9 */
		{"gnome","gnom"},"",LANG_DWARVISH,        TRUE,TRUE, 		SIZE_MEDIUM, 0,
		0,              AFF_INFRARED,   0,
		0,              RES_POISON,     VULN_BASH,
		A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K, 250, { "identify" }, { 18, 23, 25, 20, 18 ,25}
	},

	{ /* 10 */
	{"githyanki","githyanki"},"Githy",LANG_ASTRAL,	TRUE,TRUE,  		SIZE_MEDIUM, 0,
	0,		AFF_INFRARED,	0,
	0, 		RES_MAGIC,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 250, { "sword" ,"dagger" }, {19, 22, 19, 23, 20 ,20 }
	},

        { /* 11 */
    	{"bear","ayı"},	"",	LANG_DOG, FALSE,FALSE, 		SIZE_LARGE, 0,
    	0,		0,		OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
    	0,		RES_BASH|RES_COLD,	0,
    	A|G|V,		A|B|C|D|E|F|H|J|K|U|V, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 12 */
    	{"cat","kedi"},"",LANG_CAT,		FALSE,FALSE, 		SIZE_SMALL, 0,
    	0,		/*AFF_DARK_VISION*/0,	OFF_FAST|OFF_DODGE,
    	0,		0,		0,
    	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 13 */
    	{"dog","köpek"},"",LANG_DOG,		FALSE,FALSE, 		SIZE_SMALL, 0,
    	0,		0,		OFF_FAST,
    	0,		0,		0,
    	A|G|V,		A|C|D|E|F|H|J|K|U|V, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 14 */
    	{"orc","ork"},"",LANG_CORA,		FALSE,TRUE, 		SIZE_MEDIUM, 0,
    	0,		AFF_INFRARED,	0,
    	0,		RES_DISEASE,	VULN_LIGHT,
    	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 15 */
    	{"pig","domuz"},"",	LANG_DOG,	FALSE,FALSE, 		SIZE_MEDIUM, 0,
    	0,		0,		0,
    	0,		0,		0,
    	A|G|V,	 	A|C|D|E|F|H|J|K, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        { /* 16 */
    	{"rabbit","tavşan"},"",LANG_DOG,	FALSE,FALSE, 		SIZE_SMALL, 0,
    	0,		0,		OFF_DODGE|OFF_FAST,
    	0,		0,		0,
    	A|G|V,		A|C|D|E|F|H|J|K, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 17 */
    	{"snake","sürüngen"},"",LANG_SURUNGEN,	FALSE,FALSE, 		SIZE_SMALL, 0,
    	0,		0,		0,
    	0,		RES_POISON,	VULN_COLD,
    	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 18 */
    	{"bird","kuş"},"",LANG_BIRD,	FALSE,FALSE, 		SIZE_TINY, 0,
    	0,		AFF_FLYING,		OFF_FAST|OFF_DODGE,
    	0,		0,		0,
    	A|G|W,		A|C|D|E|F|H|K|P, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 19 */
    	{"wolf","kurt"},"",LANG_DOG,		FALSE,FALSE, 		SIZE_MEDIUM, 0,
    	0,		/*AFF_DARK_VISION*/0,	OFF_FAST|OFF_DODGE,
    	0,		0,		0,
    	A|G|V,		A|C|D|E|F|J|K|Q|V, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 20 */
    	{"lion","aslan"},"",LANG_CAT,		FALSE,FALSE,		SIZE_MEDIUM, 0,
    	ACT_SENTINEL,	/*AFF_DARK_VISION*/0,	OFF_FAST|OFF_DODGE,
    	0,		RES_WEAPON,		0,
    	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 21 */
        {"fish","balık"},	"",LANG_WATER,		FALSE,FALSE,		SIZE_TINY, 0,
        0,		AFF_SWIM,	0,
        0,		0,		VULN_COLD|VULN_FIRE,
        A|H|Z,		A|D|E|F|J|K|O|X, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 22 */
        {"horse","at"},	"",LANG_DOG,FALSE, FALSE,		SIZE_LARGE, 0,
        0,		0,		OFF_FAST|OFF_DODGE,
        0,		0,		0,
        A|G|V,		A|C|D|E|F|J|K|Q, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        { /* 23 */
    	{"golem","golem"},"",LANG_HUMAN,	FALSE,TRUE, 		SIZE_LARGE, 0,
    	0,		0,		0,
    	IMM_POISON|IMM_DISEASE,		RES_WEAPON,	VULN_MENTAL|VULN_MAGIC,
    	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        { /* 24 */
        {"goblin","gulyabani"},"",LANG_CORA,	FALSE,TRUE, 		SIZE_MEDIUM, 0,
        0,		AFF_INFRARED,	0,
        0,		RES_DISEASE,	VULN_MAGIC,
        A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 25 */
        {"kobold","argun"},"",LANG_CORA,	FALSE,TRUE, 		SIZE_MEDIUM, 0,
        0,		AFF_INFRARED,	0,
        0,		RES_POISON,	VULN_MAGIC,
        A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Q, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 26 */
        {"modron","geme"},"",LANG_COMMON,	FALSE,FALSE, 		SIZE_MEDIUM, 0,
        0,		AFF_INFRARED,	ASSIST_RACE|ASSIST_ALIGN,
        IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
            RES_FIRE|RES_COLD|RES_ACID,	0,
        H,		A|B|C|G|H|J|K, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 27 */
        {"elemental","uşak"},"",LANG_ASTRAL,FALSE,TRUE,		SIZE_HUGE, 0,
        0,		0,		OFF_FADE,
        0,		RES_WEAPON|RES_SUMMON|RES_FIRE|RES_COLD,		0,
        A|H|Z,		A|D, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        {  /* 28 */
        {"ghoul","gul"},"",LANG_HUMAN,	FALSE,FALSE,		SIZE_MEDIUM, 0,
        0,		0,		0,
        IMM_DISEASE,	RES_CHARM,	VULN_MAGIC,
        A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        { /* 29 */
            {"dragon","ejderha"},"", LANG_DRAGON,	FALSE,TRUE,		SIZE_GARGANTUAN, 0,
    	0, 		AFF_INFRARED|AFF_FLYING,	0,
    	0,		RES_FIRE|RES_BASH|RES_CHARM,	VULN_PIERCE|VULN_COLD,
    	A|H|Z,		A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        { /* 30 */
    	{"giant","dev"},"",LANG_GIANT,	FALSE,TRUE, 		SIZE_GIANT, 0,
    	0,		0,		0,
    	0,		RES_FIRE|RES_COLD,	VULN_MENTAL|VULN_LIGHTNING,
    	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

        { /* 31 */
    	{"undead","hortlak"},"",LANG_ASTRAL,	FALSE,FALSE, SIZE_MEDIUM,
    	DETECT_INVIS|DETECT_HIDDEN|DETECT_GOOD|DETECT_EVIL,
    	0,		AFF_PASS_DOOR|AFF_INFRARED,	0,
    	IMM_DROWNING|IMM_DISEASE|IMM_POISON,	RES_NEGATIVE|RES_ENERGY,
    	VULN_SILVER|VULN_HOLY|VULN_LIGHT,
    	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, 0, { "" }, {20, 20, 20, 20, 20, 20 }
        },

    {  /* 32 */
    {"unique","eşsiz"},"",0,	FALSE,FALSE,		0, 0,
    0,		0,		0,
    0,		0,		0,
    0,		0, 0, { "" }, {20, 20, 20, 20, 20, 20 }
    },
	
};


/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{

    {
        {"invoker","gan"}, "Gan",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
        { 3018, 0, 0, 0, 0, 0 },  75,  18, 6,  60,  100, TRUE,
        0,{0,3,0,0,0,5},CR_ALL
    },
 
    {
		{"cleric","ermiş"}, "Erm",  STAT_WIS,  OBJ_VNUM_SCHOOL_SWORD,
		{ 3003, 9619, 9619, 9619, 9619, 9619},  75,  18, 2,  70, 80, TRUE,
		100,{1,2,2,0,0,5},CR_ALL
    },

    {
		{"thief","hırsız"}, "Hır",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
		{ 3028, 9639, 9639, 9639, 9639, 9639 },  75,  18,  -4, 100, 50, FALSE,
			0,{0,0,0,3,0,5},CR_ALL
    },

    {
		{"warrior","savaşçı"}, "Sav",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
		{ 3022, 9633, 9633, 9633, 9633, 9633 },  75,  18,  -10, 100,50,FALSE,
		0,{3,0,0,0,1,5},CR_ALL
    },

    {
		{"paladin","adbolar"}, "Adb",   STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
		{ 3063, 0, 0, 0, 0, 0 },  75,  18,  -6,  80, 70, TRUE,
		400,{2,0,1,0,2,5},CR_GOOD
    },

    {
		{"anti-paladin","kembolar"}, "Kmb",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
		{ 3061, 0, 0, 0, 0, 0 },  75,  18,  -6,  80, 70, TRUE,
		300,{2,1,0,1,1,5},CR_EVIL
    },

    {
		{"ninja","ninja"}, "Nin",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
		{ 3062, 0, 0, 0, 0, 0 },  75,  18,  -10,  90, 60, FALSE,
		300,{1,0,0,2,1,5},CR_ALL
    },

    {
		{"ranger","korucu"}, "Krc",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
		{ 6156, 0, 0, 0, 0, 0 },  75,  18,  -8,  90, 60, FALSE,
		200,{1,0,0,1,2,5},CR_ALL
    },

    {
        {"transmuter","biçimci"}, "Biç",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
        { 3073, 0, 0, 0, 0, 0 },  75,  18, 2,  60,  100, TRUE,
        0,{0,3,0,0,1,5},CR_ALL
    },

    {
        {"samurai","samuray"}, "Sam",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
        { 3075, 9633, 0, 0, 0, 0 },  75,  18,  -10,  100,60,FALSE,
        500,{2,1,1,0,1,5},CR_ALL
    },

	{
		{  "vampire","vampir"}, "Vam", STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
		{ 5395, 5396, 0, 0, 0, 0 },  75,  18, -6,  80,  70, TRUE,
		300,{2,2,0,0,1,5},CR_EVIL
	},

   {
        {"necromancer","karakam"}, "Kam",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
        { 5393, 5394, 0, 0, 0, 0 },  75,  18, 6,  60,  100, TRUE,
        200,{0,2,1,0,0,5},CR_EVIL
   },

   {
        {"elementalist","ögeci"}, "Öge",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
        { 9618, 0, 0, 0, 0, 0 },  75,  18, 6,  60,  100, TRUE,
        0,{0,3,0,0,0,5},CR_ALL
   }


};



/*
 * Titles. mage=100
 */
 const char *			title_table	[MAX_CLASS][MAX_LEVEL+1]=
 {
     {
       "Man",

       "Büyü Çırağı"		,
       "Büyü Öğrencisi"	,
        "Büyü Asistanı"	,
        "Büyü Araştıran"		,
        "Sihirbaz"	,

        "Büyü Katibi"	,
        "Falcı"		,
        "Arif"		,
        "İlüzyonist"		,
        "Tövbekar"		,

       "Duacı"		,
        "Duacı"		,
        "Büyü Yapıcı"		,
        "Büyü Yapıcı"		,
        "Hokkabaz"		,

        "Hokkabaz"		,
        "Büyücü"		,
        "Büyücü"		,
        "Oluşturan"     ,
        "Oluşturan"       ,

        "Alim"		,
        "Alim"		,
        "Büyü Ustası"			,
        "Büyü Ustası"			,
        "Büyü Alimi"		,

        "Büyü Alimi"		,
        "Savaşçı Arif"		,
        "Savaşçı Arif"		,
        "Durendiş"	,
        "Durendiş"	,

        "Kıdemli Büyücü"		,
        "Kıdemli Büyücü"		,
        "Büyük Büyücü"		,
        "Büyük Büyücü"	,
        "Yüce Büyücü"		,

        "Yüce Büyücü"	,
        "Golem Efendisi"		,
        "Golem Efendisi"	,
        "Yüce Golem Efendisi"		,
        "Yüce Golem Efendisi"	,

 	 "Taş Efendisi"	,
 	 "Taş Efendisi"	,
 	 "İksir Ustası"	,
 	 "İksir Ustası"	,
 	 "Parşömen Ustası"	,

 	 "Parşömen Ustası"	,
 	 "Sopa Ustası"	,
 	 "Sopa Ustası"		,
 	 "Değnek Ustası"	,
 	 "Değnek Ustası"		,

 	 "İblis Çağıran"	,
 	 "İblis Çağıran"		,
 	 "Yüce İblis Çağıran"	,
 	 "Yüce İblis Çağıran",
 	 "Ejderha Efendisi"		,

 	 "Ejderha Efendisi"		,
 	 "Ejderha Efendisi"	,
 	 "Ejderha Efendisi",
        "Karışım Efendisi",
        "Karışım Efendisi",

        "Büyülerin Efendisi",
        "Büyülerin Efendisi",
        "Büyüleme Efendisi",
        "Büyüleme Efendisi",
        "Çağrı Ustası",

        "Çağrı Ustası",
        "Büyük Kahin",
        "Büyük Kahin",
        "Değişim Ustası",
        "Değişim Ustası",

        "Fişek Efendisi",
        "Fişek Efendisi",
        "Ölülerin Efendisi",
        "Ölülerin Efendisi",
        "Simyacı",

        "Simyacı",
        "Bağ Ustası",
        "Bağ Ustası",
        "Söz Efendisi",
        "Söz Efendisi",

        "Geçmişe hükmeden",
        "Geçmişe hükmeden",
        "Bugüne hükmeden",
        "Bugüne hükmeden",
        "Geleceğe hükmeden",

        "Geleceğe hükmeden",
        "Baş Büyücü",
        "Baş Büyücü",
        "Bilinmeyen",
        "Bilinmeyen",

 	 "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"	},

     {
 	  "Man"			 ,

 	"İnanan"		 ,
 	"Yardımcı"		 ,
 	"Hizmetli"		 ,
 	"Çırak"			 ,
 	"Misyoner"		 ,

 	"Usta"		 ,
 	"Diyakoz"		 ,
 	"Önder"			 ,
 	"Üstat"		 ,
 	"Orta Elçi"			 ,

 	"Dindar"			 ,
 	"Dindar"			 ,
 	"Yükselen"		 ,
 	"Yükselen"			 ,
 	"Din Adamı"		 ,

 	"Din Adamı"			 ,
 	"Keşiş"			 ,
 	"Keşiş"			 ,
 	"İyileştiren"			 ,
 	"İyileştiren"		 ,

 	"Vaiz"		 ,
 	"Vaiz"		 ,
 	"Açıklayan"		 ,
 	"Açıklayan"			 ,
 	"Duyan"		 ,

 	"Duyan"		 ,
 	"Gören"	 ,
 	"Gören" ,
 	"Bilen"		 ,
 	"Bilen"		 ,

 	"Bilge"	 ,
 	"Bilge"		 ,
 	"Arif"		 ,
 	"Arif"		 ,
 	"Arif"	 ,

 	"Arif"		 ,
 	"İblis Katili"		 ,
 	"İblis Katili"			 ,
 	"Yüce İblis Katili"		 ,
 	"Yüce İblis Katili"		 ,

 	"Denizlerin Efendisi"		 ,
 	"Denizlerin Efendisi"		 ,
 	"Toprağın Efendisi" ,
 	"Toprağın Efendisi"		 ,
 	"Havanın Efendisi"		 ,

 	"Havanın Efendisi"		 ,
 	"Lokman Ruhu"	 ,
 	"Lokman Ruhu"	 ,
 	"Cennet Bekçisi"	 ,
 	"Cennet Bekçisi"	 ,

 	"Işıyan" ,
 	"Işıyan" ,
 	"Kutsal" ,
 	"Kutsal" ,
 	"İlahi" ,

 	"İlahi" ,
 	"Aziz" ,
 	"Aziz" ,
 	"Köklerin Efendisi" ,
 	"Köklerin Efendisi" ,

 	"Suyun Efendisi" ,
 	"Suyun Efendisi" ,
 	"Ateşin Efendisi" ,
 	"Ateşin Efendisi" ,
 	"Buzun Efendisi" ,

 	"Buzun Efendisi" ,
 	"Rüzgarın Efendisi" ,
 	"Rüzgarın Efendisi" ,
 	"Fırtına Efendisi" ,
 	"Fırtına Efendisi" ,

 	"Toprağa Hükmeden" ,
 	"Toprağa Hükmeden" ,
 	"Denize Hükmeden" ,
 	"Denize Hükmeden" ,
 	"Zamana Hükmeden" ,

 	"Zamana Hükmeden" ,
 	"Kutsal Ruh" ,
 	"Kutsal Ruh" ,
 	"Kutsal Bilge" ,
 	"Kutsal Bilge" ,

 	"Ölümsüz Elçisi"		 ,
 	"Ölümsüz Elçisi"	 ,
 	"Ölümsüz Elçisi"	 ,
 	"Ölümsüz Elçisi"	 ,
 	"Ölümsüz Elçisi"	 ,

 	"Ölümsüz Elçisi"	 ,
 	"Ölümsüz Elçisi"	 ,
 	"Ölümsüz Elçisi"	 ,
 	"Kutsanmış Olan" ,
 	"Kutsanmış Olan" ,

 	"Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"},

     {
 	  "Man"			 ,

 		  "Çocuk"			 ,
 		  "Sessiz Adım"		 ,
 		  "Aşıran"			 ,
 		  "Yan Kesici"		 ,
 		  "Süzülen"		 ,

 		 "Kıstıran"	 ,
 		  "Cepçi"	 ,
 		  "Kapkaççı"	 ,
 		  "Dolandırıcı"	 ,
 		  "Haydut"		 ,

 		 "Soyguncu" ,
 		  "Soyguncu" ,
 		  "Hilebaz"		 ,
 		  "Hilebaz"	 ,
 		  "Eşkiya"		 ,

 		 "Eşkiya"	 ,
 		  "Dessas"	 ,
 		  "Dessas"	 ,
 		  "Hırsız"	 ,
 		  "Hırsız"	 ,

 		  "Keskin Bıçak" ,
 		  "Keskin Bıçak" ,
 		  "Hızlı Bıçak"	 ,
 		  "Hızlı Bıçak"	 ,
 		  "Katil"		 ,

 		  "Katil"			 ,
 		  "Kanunsuz"		 ,
 		  "Kanunsuz"		 ,
 		  "Gırtlak Kesen"			 ,
 		  "Gırtlak Kesen"			 ,

 		  "İftiracı"	 ,
 		  "İftiracı"	 ,
 		  "Muhbir"	 ,
 		  "Muhbir"	 ,
 		  "Hafiye"	 ,

 		  "Hafiye"	 ,
 		  "Suikastçi" ,
 		  "Suikastçi" ,
 		  "Büyük Suikastçi" ,
 		  "Büyük Suikastçi" ,

 		 "Görüş Ustası"	 ,
 		  "Görüş Ustası"	 ,
 		  "Duyuş Ustası" ,
 		  "Duyuş Ustası" ,
 		  "Koku Ustası"	 ,

    		 "Koku Ustası"	 ,
 		  "Tat Ustası"	,
 		  "Tat Ustası"	 ,
 		  "Temas Ustası"	 ,
 		  "Temas Ustası"	 ,

 		  "Suçlu"		 ,
 		  "Suçlu"	 ,
 		  "Suç Ustası"	 ,
 		  "Suç Ustası" ,
 		  "Suç Üstadı",

 		  "Suç Üstadı"	 ,
 		  "Suç Efendisi"	 ,
 		  "Suç Efendisi"		 ,
 		  "Patron"			 ,
 		  "Patron"		 ,

 		  "Elebaşı" ,
 		  "Elebaşı" ,
 		  "Büyük Patron" ,
 		  "Büyük Patron" ,
 		  "Yeraltının Gölgesi" ,

 		  "Yeraltının Gölgesi" ,
 		  "Gölge" ,
 		  "Gölge" ,
 		  "Karanlığın Sesi" ,
 		  "Karanlığın Sesi" ,

 		  "Sessizliğin Efendisi" ,
 		  "Sessizliğin Efendisi" ,
 		  "Hile Efendisi" ,
 		  "Hile Efendisi" ,
 		  "Bıçak Efendisi" ,

 		"Bıçak Efendisi" ,
 		"Zehir Efendisi" ,
 		"Zehir Efendisi" ,
 		"Giz Efendisi" ,
 		"Giz Efendisi" ,

 	        "Hırsız Efendi" ,
 		"Hırsız Efendi" ,
 		"Suikastçi Efendi" ,
 		"Suikastçi Efendi" ,
 		"Görünmeyen" ,

 		"Görünmeyen" ,
 		"Hırsızların Efendisi" ,
 		"Hırsızların Efendisi" ,
 		"Suikastçilerin Efendisi" ,
 		"Suikastçilerin Efendisi" ,

 		"Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek" },

     {
           "Man"			 ,

 	  "Acemi"		 ,
 	  "Çaylak"		 ,
 	  "Nöbetçi"		 ,
 	  "Dövüşçü"		 ,
 	  "Asker"			 ,

 	  "Savaşan"		 ,
 	  "Kıdemli"			 ,
 	  "Kılıç Çeken"		 ,
 	  "Eskrimci"	 ,
 	  "Savaşçı"		 ,

 	  "Kahraman"		 ,
 	  "Kahraman"		 ,
 	  "Şanlı"		 ,
 	  "Şanlı"		 ,
 	  "Gri Kalkan"		 ,

 	  "Gri Kalkan"				 ,
 	  "Paralı Asker"					 ,
 	  "Paralı Asker"						 ,
 	  "Kılıç Ustası"				 ,
 	  "Kılıç Ustası"				 ,

 	  "Teğmen"						 ,
 	  "Teğmen"					 ,
 	  "Savunan"				 ,
 	  "Savunan"					 ,
 	  "Dragon"					 ,

 	  "Dragon"					 ,
 	  "Saygısız"						 ,
 	  "Saygısız"						 ,
 	  "Şövalye"					 ,
 	  "Şövalye"				 ,

 	  "Büyük Şövalye"				 ,
 	  "Büyük Şövalye"				 ,
 	  "Efendi Şövalye"					 ,
 	  "Efendi Şövalye"				 ,
 	  "Gladyatör"					 ,

 	  "Gladyatör"				 ,
 	  "Efendi Gladyatör"				 ,
 	  "Efendi Gladyatör"			 ,
 	  "İblis Katleden"					 ,
 	  "İblis Katleden"				 ,

 	  "Büyük İblis Katleden"		 ,
 	  "Büyük İblis Katleden"			 ,
 	  "Ejder Katili"				 ,
 	  "Ejder Katili"				 ,
 	  "Büyük Ejder Katili"		 ,

 	  "Büyük Ejder Katili"		 ,
 	  "Lord"						 ,
 	  "Lord"				 ,
 	  "Efendi"						 ,
 	  "Efendi"						 ,

 	  "Baron"			 ,
 	  "Baron"		 ,
 	  "Rüzgar Baron"			 ,
 	  "Rüzgar Baron"				 ,
 	  "Fırtına Baron"			 ,

 	  "Fırtına Baron"			 ,
 	  "Kasırga Baron"		 ,
 	  "Kasırga Baron"		 ,
 	  "Kudretli Baron"	 ,
 	  "Kudretli Baron"	 ,

 	  "Ateş Baron"   ,
 	  "Ateş Baron"   ,
 	  "Buz Baron"   ,
 	  "Buz Baron"   ,
 	  "Şimşek Baron"  ,

 	  "Şimşek Baron"  ,
 	  "Elementlerin Efendisi"   ,
 	  "Elementlerin Efendisi"   ,
 	  "Bakır Efendi"   ,
 	  "Bakır Efendi"  ,

 	  "Bronz Efendi"  ,
 	  "Bronz Efendi"   ,
 	  "Pirinç Efendi"  ,
 	  "Pirinç Efendi"   ,
 	  "Demir Efendi"   ,

 	  "Demir Efendi"   ,
 	  "Çelik Efendi"  ,
 	  "Çelik Efendi"   ,
 	  "Mitril Efendi"   ,
 	  "Mitril Efendi"  ,

 	  "Adamantit Efendi"   ,
 	  "Adamantit Efendi"  ,
 	  "Yüzbaşı"   ,
 	  "Yüzbaşı"  ,
 	  "General"   ,

 	  "General"  ,
 	  "Kara Mareşali"  ,
 	  "Kara Mareşali"  ,
 	  "Savaş Ustası"   ,
 	  "Savaş Ustası"  ,

 	  "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"     },

       {

         "Man"                ,

         "Paladin Yamağı"		  ,
         "Paladin Çırağı"			 ,
         "Kalkan"		   ,
         "Kalkan"		 ,
         "Kılıç"			 ,

         "Yay"			 ,
         "Taşıyıcı"		  ,
         "Atlı"			 ,
         "Bey Adayı"		 ,
         "Bey"		 ,

         "Piyade Neferi"   ,
         "Piyade Neferi"  ,
         "Mızrak Taşıyan" ,
         "Mızrak Taşıyan" ,
         "Yay Taşıyan"  ,

         "Yay Taşıyan"   ,
         "Kılıç Taşıyan"   ,
         "Kılıç Taşıyan"  ,
         "Onurlu"   ,
         "Onurlu"   ,

         "Soylu"   ,
         "Soylu"  ,
         "Güvenilir"   ,
         "Güvenilir"   ,
         "Doğrucu"   ,

         "Doğrucu"  ,
         "Yiğit"  ,
         "Yiğit"  ,
         "Paladin"   ,
         "Paladin"   ,

         "Görevci"  ,
         "Görevci"  ,
         "Saygın"   ,
         "Saygın"  ,
         "Savunan" ,

         "Savunan"  ,
         "Şanlı Şövalye"  ,
         "Şanlı Şövalye"   ,
         "Paladin Şövalye"  ,
         "Paladin Şövalye"   ,

         "Arna Adayı"  ,
         "Arna Adayı"   ,
         "Dindar Şövalye"   ,
         "Dindar Şövalye"  ,
         "Dindar Şövalye"  ,

         "Dindar Şövalye"  ,
         "Dindar Savunma"  ,
         "Dindar Savunma"   ,
         "Şövalye Arna"  ,
         "Şövalye Arna"   ,

         "Arna Çırağı"  ,
         "Arna Çırağı"   ,
         "Arna"   ,
         "Arna"  ,
         "Arna Teğmen"  ,

         "Arna Teğmen"   ,
         "Arna Yüzbaşı"  ,
         "Arna Yüzbaşı"   ,
         "Arna Albay"   ,
         "Arna Albay"   ,

         "Arna General"  ,
         "Arna General"  ,
         "Arna Kara Mareşali"   ,
         "Arna Kara Mareşali"   ,
         "Şifacı Şövalye"  ,

         "Şifacı Şövalye"   ,
         "Öc Alan"   ,
         "Öc Alan"  ,
         "Öncü"   ,
         "Öncü"   ,

         "Öncü Lideri"  ,
         "Öncü Lideri"   ,
         "Öncülerin Lideri"  ,
         "Öncülerin Lideri"  ,
         "Arna Lideri"   ,

         "Arna Lideri"  ,
         "Arnaların Lideri"   ,
         "Arnaların Lideri"   ,
         "Arnaların Şanlı Lideri"   ,
         "Arnaların Şanlı Lideri" ,

         "Arnaların Lordu"  ,
         "Arnaların Lordu"  ,
         "İnancın Çekici"   ,
         "İnancın Çekici"  ,
         "Kafirlerin Katili"   ,

         "Kafirlerin Katili"  ,
         "Dindar"   ,
         "Dindar"   ,
         "Kutsal Şövalye"  ,
         "Kutsal Şövalye"   ,

         "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"
      },
       {
         "Man"						 ,

         "Kirli"						 ,
         "Zorba"			 	 ,
         "Kaba"   ,
         "Cani"   ,
         "Cani"  ,

         "Yağmacı"   ,
         "Yokeden"  ,
         "Yakan"   ,
         "Kiralık Katil"   ,
         "Kanunsuz"  				 ,

         "Paralı Asker"   ,
         "Paralı Asker"   ,
         "Kara Kılıç"  ,
         "Kara Kılıç"   ,
         "Kızıl Kılıç"  ,

         "Kızıl Kılıç"   ,
         "Süzülen" ,
         "Süzülen"  ,
         "Gaddar"   ,
         "Gaddar" ,

         "Araklayan"   ,
         "Araklayan"   ,
         "Bilinmeyen"  ,
         "Bilinmeyen"   ,
         "Nefret Edilen"  ,

         "Nefret Edilen"   ,
         "Alçak"   ,
         "Alçak"  ,
         "Paladin Düşmanı"   ,
         "Paladin Düşmanı" ,

         "Şeytani Dövüşçü"  ,
         "Şeytani Dövüşçü"  ,
         "Haydut Şövalye"  ,
         "Haydut Şövalye"  ,
         "Şeytani Savunucu"   ,

         "Şeytani Savunucu"  ,
         "Kanlı"  ,
         "Kanlı"   ,
         "Kara Şövalye"   ,
         "Kara Şövalye"   ,

         "Kızıl Şövalye"  ,
         "Kızıl Şövalye"  ,
         "Büyük Şövalye"  ,
         "Büyük Şövalye"   ,
         "İnançsız Şövalye"   ,

         "İnançsız Şövalye"   ,
         "Acının Şövalyesi"   ,
         "Acının Şövalyesi"  ,
         "Lejyon Şövalyesi"   ,
         "Lejyon Şövalyesi"   ,

         "Lejyon Şövalyesi"   ,
         "Lejyon Şövalyesi"  ,
         "Lejyon Savunması"  ,
         "Lejyon Savunması"  ,
         "Lejyon Yüzbaşısı"   ,

         "Lejyon Yüzbaşısı"   ,
         "Lejyon Generali"   ,
         "Lejyon Generali"   ,
         "Lejyon Kara Mareşali" ,
         "Lejyon Kara Mareşali"   ,

         "Kıyamet Şövalyesi"  ,
         "Kıyamet Şövalyesi" ,
         "Işığı kesen"   ,
         "Işığı Kesen"   ,
         "Acının Efendisi"   ,

         "Acının Efendisi"   ,
         "Ölü Efendisi"   ,
         "Ölü Efendisi"   ,
         "Karanlık Şövalye"  ,
         "Karanlık Şövalye"   ,

         "Karanlık Lord"   ,
         "Karanlık Lord"   ,
         "Karanlık Kral"   ,
         "Karanlık Kral"   ,
         "Ölüm Habercisi"   ,

         "Ölüm Habercisi"   ,
         "Ölüm Yayan"   ,
         "Ölüm Yayan"   ,
         "Kıtlık Getiren"   ,
         "Kıtlık Getiren"   ,

         "Ölüm Getiren"   ,
         "Ölüm Getiren"   ,
         "Onursuz İnançsız"  ,
         "Onursuz İnançsız"  ,
         "Katleden"   ,

         "Katleden"   ,
         "Karanlık Tohum"   ,
         "Karanlık Tohum"  ,
         "Onursuz Şövalye"   ,
         "Onursuz Şövalye"  ,

         "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"	 },

       {
         "Man"  ,

         "Uke"   ,
         "Tori"   ,
         "Beyaz Kuşak"  ,
         "Sarı Kuşak"   ,
         "Yeşil Kuşak"   ,

         "Kahverengi Üç"   ,
         "Kahverengi İki" ,
         "Kahverengi Bir"  ,
         "Siyah Üç"   ,
         "Siyah İki"   ,

         "Okuiri"   ,
         "Okuiri"   ,
         "Shuto"   ,
         "Shuto"  ,
         "Uraken"   ,

         "Uraken"   ,
         "Shotei"   ,
         "Shotei"   ,
         "Tateken"   ,
         "Tateken"   ,

         "Seiken"   ,
         "Seiken"   ,
         "Ude"   ,
         "Ude"  ,
         "Empi"  ,

         "Empi"   ,
         "Josokutei"   ,
         "Josokutei"   ,
         "Kasokutei"   ,
         "Kasokutei"  ,

         "Atemi Waza Ustası"   ,
         "Atemi Waza Ustası"  ,
         "Kotegaeshi Ustası"   ,
         "Kotegaeshi Ustası"  ,
         "Kansetsuwaza Ustası"   ,

         "Kansetsuwaza Ustası"  ,
         "Taisabaki Ustası"   ,
         "Taisabaki Ustası"  ,
         "Kyusho Ustası"   ,
         "Kyusho Ustası"  ,

         "Kamae Öğrencisi"  ,
         "Kamae Öğrencisi"   ,
         "Kamae Ustası"  ,
         "Kamae Ustası"  ,
         "Ukemi Ustası"   ,

         "Ukemi Ustası"  ,
         "Mokuroku"   ,
         "Mokuroku"  ,
         "Ogoshi"  ,
         "Ogoshi"  ,

         "Ippon Seinage"   ,
         "Ippon Seinage"  ,
         "Koshi Garuma"   ,
         "Koshi Garuma"   ,
         "Sukuinage"   ,

         "Sukuinage"  ,
         "Osotogari"  ,
         "Osotogari"   ,
         "Uki Goshi" ,
         "Uki Goshi"  ,

         "Taiotoshi"   ,
         "Taiotoshi"   ,
         "Harai Goshi"   ,
         "Harai Goshi"   ,
         "Yama Arashi"   ,

         "Yama Arashi"   ,
         "Nage Waza Ustası"   ,
         "Nage Waza Ustası"   ,
         "Owazaya Giriş"  ,
         "Owazaya Giriş"   ,

         "Owaza Çırağı" ,
         "Owaza Çırağı"   ,
         "Owaza Öğrencisi"  ,
         "Owaza Öğrencisi"   ,
         "Owaza"   ,

         "Owaza"   ,
         "Owaza Ustası"  ,
         "Owaza Ustası"  ,
         "Menkyo"  ,
         "Menkyo"   ,

         "Sensei"   ,
         "Sensei"  ,
         "Shinan"   ,
         "Shinan"   ,
         "Shihan"   ,

         "Shihan"   ,
         "Kaiden"   ,
         "Kaiden"   ,
         "Miyama Ryu Ustası"  ,
         "Miyama Ryu Ustası" ,

         "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"
      },
      {

         "Man"	 ,

         "Yavru Kurt"   ,
         "Yavru Kurt"   ,
         "İzci"   ,
         "İzci"   ,
         "Korucu"  ,

         "Korucu"   ,
         "Tuzakçı"  ,
         "Tuzakçı"  ,
         "Avcı"  ,
         "Avcı"			 			 ,

         "İz Sürücü"   ,
         "İz Sürücü"  ,
         "Öncü" ,
         "Öncü"  ,
         "Yeşil Adam"   ,

         "Yeşil Adam"  ,
         "Okçu"   ,
         "Okçu"   ,
         "Ağaç Adam"   ,
         "Ağaç Adam"  ,

         "Ağaç Ustası"  ,
         "Ağaç Ustası"  ,
         "Kolcu Çırak"  ,
         "Kolcu Çırak"  ,
         "Kolcu Adayı"   ,

         "Kolcu Adayı"   ,
         "Kolcu Kalfası"   ,
         "Kolcu Kalfası"  ,
         "Kolcu"   ,
         "Kolcu"   ,

         "Kolcu"  ,
         "Kolcu"  ,
         "Kolcu"  ,
         "Kolcu"   ,
         "Kolcu"   ,

         "Kolcu"  ,
         "Kolcu Yüzbaşı"  ,
         "Kolcu Yüzbaşı"   ,
         "Büyük Korucu"  ,
         "Büyük Korucu"  ,

         "Korucu Yüzbaşı" ,
         "Korucu Yüzbaşı"   ,
         "Korucu General"   ,
         "Korucu General"   ,
         "Korucuların Efendisi"   ,

         "Korucuların Efendisi"   ,
         "Ormanın Şövalyesi"   ,
         "Ormanın Şövalyesi" ,
         "Ormanın Kılıcı"  ,
         "Ormanın Kılıcı"   ,

         "Orman Gardı"   ,
         "Orman Gardı"  ,
         "Orman Lordu"   ,
         "Orman Lordu"  ,
         "Orman Lordu"   ,

         "Orman Lordu"   ,
         "Orman Baronu"   ,
         "Orman Baronu"  ,
         "Ormanların Baronu"  ,
         "Ormanların Baronu"  ,

         "Avlayan"  ,
         "Avlayan"  ,
 	"Sakınan" ,
 	"Sakınan" ,
 	"Koruyan" ,

 	"Koruyan",
         "Kolcu Şövalye"   ,
         "Kolcu Şövalye" ,
         "Kolcu Şövalye"   ,
         "Kolcu Şövalye"  ,

         "Kolcu Lord"   ,
         "Kolcu Lord"  ,
         "Kolcu Lord"  ,
         "Kolcu Lord"  ,
         "Kolcu Baron"   ,

         "Kolcu Baron"  ,
         "Kolcu Baron"   ,
         "Kolcu Baron"   ,
 	"Gezgin" ,
 	"Gezgin" ,

         "Selvan",
 	"Selvan" ,
         "Ormanların Sesi"  ,
         "Ormanların Sesi"   ,
 	"Ormanların Sesi"   ,

         "İzleyen"   ,
         "İzleyen"   ,
 	"İzleyen"
 	"İzleyen"
 	"İzleyen"

         "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"
      },
       {
         "Man" ,

         "Büyü Çırağı"		 ,
         "Büyü Öğrencisi"	 ,
         "Büyü Asistanı"	 ,
         "Büyü Araştıran"		 ,
         "Sihirbaz"	 ,

         "Büyü Katibi"	 ,
         "Falcı"		 ,
         "Arif"		 ,
         "İlüzyonist"		 ,
         "Tövbekar"		 ,

         "Duacı"		 ,
         "Duacı"		 ,
         "Büyü Yapan"		 ,
         "Büyü Yapan"		 ,
         "Hokkabaz"		 ,

         "Hokkabaz" ,
         "Büyücü"	 ,
         "Büyücü"	 ,
 	"Oluşturan" ,
 	"Oluşturan" ,

         "Alim"		 ,
         "Alim"		 ,
         "Büyü Ustası"			 ,
         "Büyü Ustası"			 ,
         "Büyü Alimi"		 ,

         "Büyü Alimi"		 ,
         "Savaşçı Arif"		 ,
         "Savaşçı Arif"		 ,
         "Durendiş"	 ,
         "Durendiş"	 ,

         "Kıdemli Büyücü"		 ,
         "Kıdemli Büyücü"		 ,
         "Büyük Büyücü"		 ,
         "Büyük Büyücü"	 ,
         "Yüce Büyücü"		 ,

         "Yüce Büyücü"	 ,
         "Golem Efendisi"		 ,
         "Golem Efendisi"	 ,
         "Yüce Golem Efendisi"		 ,
         "Yüce Golem Efendisi"	 ,

 	  "Taş Efendisi"	 ,
 	  "Taş Efendisi"	 ,
 	  "İksir Ustası"	 ,
 	  "İksir Ustası"	 ,
 	  "Parşömen Ustası"	 ,

 	  "Parşömen Ustası"	 ,
 	  "Değnek Ustası"	 ,
 	  "Değnek Ustası"		 ,
 	  "Değnek Ustası"	 ,
 	  "Değnek Ustası"		 ,

 	  "İblis Çağıran"	 ,
 	  "İblis Çağıran"		 ,
 	  "Yüce İblis Çağıran"	 ,
 	  "Yüce İblis Çağıran" ,
 	  "Ejderha Efendisi"		 ,

 	  "Ejderha Efendisi"		 ,
 	  "Ejderha Efendisi"	 ,
 	  "Ejderha Efendisi" ,
         "Karışım Efendisi" ,
         "Karışım Efendisi" ,

         "Büyülerin Efendisi" ,
         "Büyülerin Efendisi" ,
         "Büyüleme Efendisi" ,
         "Büyüleme Efendisi" ,
         "Çağrı Ustası" ,

         "Çağrı Ustası" ,
         "Büyük Kahin" ,
         "Büyük Kahin" ,
         "Değişim Ustası" ,
         "Değişim Ustası" ,

         "Fişek Efendisi" ,
         "Fişek Efendisi" ,
         "Ölülerin Efendisi" ,
         "Ölülerin Efendisi" ,
         "Simyacı" ,

         "Simyacı" ,
         "Bağ Ustası" ,
         "Bağ Ustası" ,
         "Söz Efendisi" ,
         "Söz Efendisi" ,

         "Geçmişe hükmeden" ,
         "Geçmişe hükmeden" ,
         "Bugüne hükmeden" ,
         "Bugüne hükmeden" ,
         "Geleceğe hükmeden" ,

         "Geleceğe hükmeden" ,
         "Baş Büyücü" ,
         "Baş Büyücü" ,
         "Bilinmeyen" ,
         "Bilinmeyen" ,

 	  "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"
      },
      {

           "Man"		 ,

 	  "Acemi"	 ,
 	  "Çaylak"	 ,
 	  "Nöbetçi" ,
 	  "Dövüşçü" ,
 	  "Asker"		 ,

 	  "Savaşan"	 ,
 	  "Kıdemli"		 ,
 	  "Kılıç Çeken"	 ,
 	  "Eskrimci"	 ,
 	  "Savaşçı"		 ,

 	  "Kahraman"	 ,
 	  "Kahraman"	 ,
 	  "Şanlı"	 ,
 	  "Şanlı"	 ,
 	  "Gri Kalkan"	 ,

 	  "Gri Kalkan"		 ,
 	  "Paralı Asker"		 ,
 	  "Paralı Asker"		 ,
 	  "Kılıç Ustası"	 ,
 	  "Kılıç Ustası"	 ,

 	  "Teğmen"		 ,
 	  "Teğmen"		 ,
 	  "Savunan"	 ,
 	  "Savunan"	 ,
 	  "Dragon"		 ,

 	  "Dragon"			 ,
 	  "Saygısız"		 ,
 	  "Saygısız"	 ,
 	  "Şövalye"		 ,
 	  "Şövalye"	 ,

 	  "Büyük Şövalye"				 ,
 	  "Büyük Şövalye"				 ,
 	  "Efendi Şövalye"					 ,
 	  "Efendi Şövalye"				 ,
 	  "Gladyatör"					 ,

 	  "Gladyatör"				 ,
 	  "Efendi Gladyatör"				 ,
 	  "Efendi Gladyatör"			 ,
 	  "İblis Katleden"					 ,
 	  "İblis Katleden"				 ,

 	  "Büyük İblis Katleden"		 ,
 	  "Büyük İblis Katleden"			 ,
 	  "Ejder Katili"				 ,
 	  "Ejder Katili"				 ,
 	  "Büyük Ejder Katili"		 ,

 	  "Büyük Ejder Katili"		 ,
 	  "Lord"						 ,
 	  "Lord"				 ,
 	  "Efendi"						 ,
 	  "Efendi"						 ,

 	  "Baron"			 ,
 	  "Baron"		 ,
 	  "Rüzgar Baron"			 ,
 	  "Rüzgar Baron"				 ,
 	  "Fırtına Baron"			 ,

 	  "Fırtına Baron"			 ,
 	  "Kasırga Baron"		 ,
 	  "Kasırga Baron"		 ,
 	  "Kudretli Baron"  ,
 	  "Kudretli Baron" ,

 	  "Ateş Baron"   ,
 	  "Ateş Baron"   ,
 	  "Buz Baron"   ,
 	  "Buz Baron"   ,
 	  "Şimşek Baron"  ,

 	  "Şimşek Baron"  ,
 	  "Elementlerin Efendisi"   ,
 	  "Elementlerin Efendisi"   ,
 	  "Bakır Efendi"   ,
 	  "Bakır Efendi"  ,

 	  "Bronz Efendi"  ,
 	  "Bronz Efendi"   ,
 	  "Pirinç Efendi"  ,
 	  "Pirinç Efendi"   ,
 	  "Demir Efendi"   ,

 	  "Demir Efendi"   ,
 	  "Çelik Efendi"  ,
 	  "Çelik Efendi"   ,
 	  "Mitril Efendi"   ,
 	  "Mitril Efendi"  ,

 	  "Adamantit Efendi"   ,
 	  "Adamantit Efendi"  ,
 	  "Yüzbaşı"   ,
 	  "Yüzbaşı"  ,
 	  "General"   ,

 	  "General"  ,
 	  "Kara Mareşali"  ,
 	  "Kara Mareşali"  ,
 	  "Savaş Ustası"   ,
 	  "Savaş Ustası"  ,

 	  "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"
      },
       {
         "Man" ,

         "Büyü Çırağı"		 ,
         "Büyü Öğrencisi"	 ,
         "Büyü Asistanı"	 ,
         "Büyü Araştıran"		 ,
         "Sihirbaz"	 ,

         "Büyü Katibi"	 ,
         "Falcı"		 ,
         "Arif"		 ,
         "İlüzyonist"		 ,
         "Tövbekar"		 ,

         "Duacı"		 ,
         "Duacı"		 ,
         "Büyü Yapan"		 ,
         "Büyü Yapan"		 ,
         "Hokkabaz"		 ,

         "Hokkabaz"		 ,
         "Büyücü"		 ,
         "Büyücü"		 ,
 	"Oluşturan"    ,
 	"Oluşturan" ,

         "Alim"	 ,
         "Alim"	 ,
         "Büyü Ustası"		 ,
         "Büyü Ustası"		 ,
         "Büyü Alimi"	 ,

         "Büyü Alimi"		 ,
         "Savaşçı Arif"		 ,
         "Savaşçı Arif"		 ,
         "Durendiş"	 ,
         "Durendiş"	 ,

         "Kıdemli Büyücü"		 ,
         "Kıdemli Büyücü"		 ,
         "Büyük Büyücü"		 ,
         "Büyük Büyücü"	 ,
         "Yüce Büyücü"		 ,

         "Yüce Büyücü"	 ,
         "Golem Efendisi"		 ,
         "Golem Efendisi"	 ,
         "Yüce Golem Efendisi"		 ,
         "Yüce Golem Efendisi"	 ,

 	  "Taş Efendisi"	 ,
 	  "Taş Efendisi"	 ,
 	  "İksir Ustası"	 ,
 	  "İksir Ustası"	 ,
 	  "Parşömen Ustası"	 ,

 	  "Parşömen Ustası"	 ,
 	  "Değnek Ustası"	 ,
 	  "Değnek Ustası"		 ,
 	  "Değnek Ustası"	 ,
 	  "Değnek Ustası"		 ,

 	  "İblis Çağıran"	 ,
 	  "İblis Çağıran"		 ,
 	  "Yüce İblis Çağıran"	 ,
 	  "Yüce İblis Çağıran" ,
 	  "Ejderha Efendisi"		 ,

 	  "Ejderha Efendisi"		 ,
 	  "Ejderha Efendisi"	 ,
 	  "Ejderha Efendisi" ,
         "Karışım Efendisi" ,
         "Karışım Efendisi" ,

         "Büyülerin Efendisi" ,
         "Büyülerin Efendisi" ,
         "Büyüleme Efendisi" ,
         "Büyüleme Efendisi" ,
         "Çağrı Ustası" ,

         "Çağrı Ustası" ,
         "Büyük Kahin" ,
         "Büyük Kahin" ,
         "Değişim Ustası" ,
         "Değişim Ustası" ,

         "Fişek Efendisi" ,
         "Fişek Efendisi" ,
         "Ölülerin Efendisi" ,
         "Ölülerin Efendisi" ,
         "Simyacı" ,

         "Simyacı" ,
         "Bağ Ustası" ,
         "Bağ Ustası" ,
         "Söz Efendisi" ,
         "Söz Efendisi" ,

         "Geçmişe hükmeden" ,
         "Geçmişe hükmeden" ,
         "Bugüne hükmeden" ,
         "Bugüne hükmeden" ,
         "Geleceğe hükmeden" ,

         "Geleceğe hükmeden" ,
         "Baş Büyücü" ,
         "Baş Büyücü" ,
         "Bilinmeyen" ,
         "Bilinmeyen" ,

 	  "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"
      },
       {

         "Man" ,

         "Büyü Çırağı"		 ,
         "Büyü Öğrencisi"	 ,
         "Büyü Asistanı"	 ,
         "Büyü Araştıran"		 ,
         "Sihirbaz"	 ,

         "Büyü Katibi"	 ,
         "Falcı"		 ,
         "Arif"		 ,
         "İlüzyonist"		 ,
         "Tövbekar"		 ,

         "Duacı"		 ,
         "Duacı"		 ,
         "Büyü Yapan"		 ,
         "Büyü Yapan"		 ,
         "Hokkabaz"		 ,

         "Hokkabaz" ,
         "Büyücü"	 ,
         "Büyücü"	 ,
 	"Oluşturan" ,
 	"Oluşturan" ,

         "Alim"		 ,
         "Alim"		 ,
         "Büyü Ustası"			 ,
         "Büyü Ustası"			 ,
         "Büyü Alimi"		 ,

         "Büyü Alimi"		 ,
         "Savaşçı Arif"		 ,
         "Savaşçı Arif"		 ,
         "Durendiş"	 ,
         "Durendiş"	 ,

         "Kıdemli Büyücü"		 ,
         "Kıdemli Büyücü"		 ,
         "Büyük Büyücü"		 ,
         "Büyük Büyücü"	 ,
         "Yüce Büyücü"		 ,

         "Yüce Büyücü"	 ,
         "Golem Efendisi"		 ,
         "Golem Efendisi"	 ,
         "Yüce Golem Efendisi"		 ,
         "Yüce Golem Efendisi"	 ,

 	  "Taş Efendisi"	 ,
 	  "Taş Efendisi"	 ,
 	  "İksir Ustası"	 ,
 	  "İksir Ustası"	 ,
 	  "Parşömen Ustası"	 ,

 	  "Parşömen Ustası"	 ,
 	  "Değnek Ustası"	 ,
 	  "Değnek Ustası"		 ,
 	  "Değnek Ustası"	 ,
 	  "Değnek Ustası"		 ,

 	  "İblis Çağıran"	 ,
 	  "İblis Çağıran"		 ,
 	  "Yüce İblis Çağıran"	 ,
 	  "Yüce İblis Çağıran" ,
 	  "Ejderha Efendisi"		 ,

 	  "Ejderha Efendisi"		 ,
 	  "Ejderha Efendisi"	 ,
 	  "Ejderha Efendisi" ,
         "Karışım Efendisi" ,
         "Karışım Efendisi" ,

         "Büyülerin Efendisi" ,
         "Büyülerin Efendisi" ,
         "Büyüleme Efendisi" ,
         "Büyüleme Efendisi" ,
         "Çağrı Ustası" ,

         "Çağrı Ustası" ,
         "Büyük Kahin" ,
         "Büyük Kahin" ,
         "Değişim Ustası" ,
         "Değişim Ustası" ,

         "Fişek Efendisi" ,
         "Fişek Efendisi" ,
         "Ölülerin Efendisi" ,
         "Ölülerin Efendisi" ,
         "Simyacı" ,

         "Simyacı" ,
         "Bağ Ustası" ,
         "Bağ Ustası" ,
         "Söz Efendisi" ,
         "Söz Efendisi" ,

         "Geçmişe hükmeden" ,
         "Geçmişe hükmeden" ,
         "Bugüne hükmeden" ,
         "Bugüne hükmeden" ,
         "Geleceğe hükmeden" ,

         "Geleceğe hükmeden" ,
         "Baş Büyücü" ,
         "Baş Büyücü" ,
         "Bilinmeyen" ,
         "Bilinmeyen" ,

 	  "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"
      },
      {

         "Man" ,

         "Büyü Çırağı"		 ,
         "Büyü Öğrencisi"	 ,
         "Büyü Asistanı"	 ,
         "Büyü Araştıran"		 ,
         "Sihirbaz"	 ,

         "Büyü Katibi"	 ,
         "Falcı"		 ,
         "Arif"		 ,
         "İlüzyonist"		 ,
         "Tövbekar"		 ,

         "Duacı"		 ,
         "Duacı"		 ,
         "Büyü Yapan"		 ,
         "Büyü Yapan"		 ,
         "Hokkabaz"		 ,

         "Hokkabaz" ,
         "Büyücü"	 ,
         "Büyücü"	 ,
 	"Oluşturan" ,
 	"Oluşturan" ,

         "Alim"		 ,
         "Alim"		 ,
         "Büyü Ustası"			 ,
         "Büyü Ustası"			 ,
         "Büyü Alimi"		 ,

         "Büyü Alimi"		 ,
         "Savaşçı Arif"		 ,
         "Savaşçı Arif"		 ,
         "Durendiş"	 ,
         "Durendiş"	 ,

         "Kıdemli Büyücü"		 ,
         "Kıdemli Büyücü"		 ,
         "Büyük Büyücü"		 ,
         "Büyük Büyücü"	 ,
         "Yüce Büyücü"		 ,

         "Yüce Büyücü"	 ,
         "Golem Efendisi"		 ,
         "Golem Efendisi"	 ,
         "Yüce Golem Efendisi"		 ,
         "Yüce Golem Efendisi"	 ,

 	  "Taş Efendisi"	 ,
 	  "Taş Efendisi"	 ,
 	  "İksir Ustası"	 ,
 	  "İksir Ustası"	 ,
 	  "Parşömen Ustası"	 ,

 	  "Parşömen Ustası"	 ,
 	  "Değnek Ustası"	 ,
 	  "Değnek Ustası"		 ,
 	  "Değnek Ustası"	 ,
 	  "Değnek Ustası"		 ,

 	  "İblis Çağıran"	 ,
 	  "İblis Çağıran"		 ,
 	  "Yüce İblis Çağıran"	 ,
 	  "Yüce İblis Çağıran" ,
 	  "Ejderha Efendisi"		 ,

 	  "Ejderha Efendisi"		 ,
 	  "Ejderha Efendisi"	 ,
 	  "Ejderha Efendisi" ,
         "Karışım Efendisi" ,
         "Karışım Efendisi" ,

         "Büyülerin Efendisi" ,
         "Büyülerin Efendisi" ,
         "Büyüleme Efendisi" ,
         "Büyüleme Efendisi" ,
         "Çağrı Ustası" ,

         "Çağrı Ustası" ,
         "Büyük Kahin" ,
         "Büyük Kahin" ,
         "Değişim Ustası" ,
         "Değişim Ustası" ,

         "Fişek Efendisi" ,
         "Fişek Efendisi" ,
         "Ölülerin Efendisi" ,
         "Ölülerin Efendisi" ,
         "Simyacı" ,

         "Simyacı" ,
         "Bağ Ustası" ,
         "Bağ Ustası" ,
         "Söz Efendisi" ,
         "Söz Efendisi" ,

         "Geçmişe hükmeden" ,
         "Geçmişe hükmeden" ,
         "Bugüne hükmeden" ,
         "Bugüne hükmeden" ,
         "Geleceğe hükmeden" ,

         "Geleceğe hükmeden" ,
         "Baş Büyücü" ,
         "Baş Büyücü" ,
         "Bilinmeyen" ,
         "Bilinmeyen" ,

 	 "Kahraman"		,
 	 "Melek"		,
 	 "Melek"		,
 	 "Elçi"		,
 	 "Ölümsüz"		,
 	"Yüce"		,
 	"Yüce",
 	"Yüce",
 	"Yüce",
 	 "Tek"
     }
 ,
   };

const struct color_type color_table[] = /*  Last entry should be NULL   */
{
  {"Black", CLR_BLACK},
  {"Green", CLR_GREEN},
  {"Brown", CLR_BROWN},
  {"Yellow", CLR_YELLOW},
  {"Red", CLR_RED},
  {"Blue", CLR_BLUE},
  {"Magenta", CLR_MAGENTA},
  {"Cyan", CLR_CYAN},
  {"White", CLR_WHITE},
  {NULL, NULL}
};

/* God's Name, name of religion, tattoo vnum  */
const struct religion_type religion_table [] =
{
  { "", "None", "", 0 },
  { "Kamenilik", "Kameniler", "Kame", OBJ_VNUM_TATTOO_KAME },
  { "Niryanilik", "Niryaniler", "Nir", OBJ_VNUM_TATTOO_NIR },
  { "Nyahilik", "Nyahiler", "Nyah", OBJ_VNUM_TATTOO_NYAH },
  { "Sintiyanlık", "Sintiyanlar", "Sint", OBJ_VNUM_TATTOO_SINT },
};

const struct ethos_type ethos_table [] =
{
  { "Null" },
  { "Tüze" },
  { "Yansız" },
  { "Kaos" }
};

/*  altar good neut evil, recall good neut evil, pit good neut evil */
const struct hometown_type hometown_table [] =
{
  { "Selenge", {3070, 3054, 3072}, {3068, 3001, 3071}, {3069,3054,3072} },
  { "Yeni Thalos", {9605, 9605, 9605}, {9609,9609,9609}, {9609,9609,9609} },
  { "Titan", {18127,18127,18127},{18126,18126,18126},{18127,18127,18127} },
  { "Yeni Ofcol", {669, 669, 669},      {698, 698, 698},   {669, 669, 669} },
  { "Eski Selenge",{5386, 5386,5386}, {5379,5379, 5379}, {5386,5386,5386} },
};

/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[26]		=
{
    { -5, -4,    0 },  /* 0  */
    { -5, -4,  100 },  /* 1  */
    { -3, -2,  200 },
    { -3, -1,  200 },  /* 3  */
    { -2, -1,  200 },
    { -2, -1,  200 },  /* 5  */
    { -1,  0,  300 },
    { -1,  0,  400 },
    {  0,  0,  500 },
    {  0,  0,  600 },
    {  0,  0,  700 }, /* 10  */
    {  0,  0,  800 },
    {  0,  0,  900 },
    {  0,  0, 1000 }, /* 13  */
    {  0,  1, 1200 },
    {  1,  1, 1400 }, /* 15  */
    {  1,  2, 1700 },
    {  2,  3, 2000 },
    {  2,  3, 2400 }, /* 18  */
    {  3,  4, 2800 },
    {  3,  5, 3300 }, /* 20  */
    {  4,  6, 3800 },
    {  4,  6, 4400 },
    {  5,  7, 5000 },
    {  5,  8, 5700 },
    {  6,  9, 6400 }  /* 25   */
};



const	struct	int_app_type	int_app		[26]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 }	/* 25 */
};



const	struct	wis_app_type	wis_app		[26]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },	/* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },	/* 15 */
    { 2 },
    { 2 },
    { 3 },	/* 18 */
    { 3 },
    { 3 },	/* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 }	/* 25 */
};



const	struct	dex_app_type	dex_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};


const	struct	con_app_type	con_app		[26]		=
{
    {  0, 20 },   /*  0 */
    {  1, 25 },   /*  1 */
    {  1, 30 },
    {  2, 35 },	  /*  3 */
    {  3, 40 },
    {  4, 45 },   /*  5 */
    {  5, 50 },
    {  6, 55 },
    {  7, 60 },
    {  8, 65 },
    {  9, 70 },   /* 10 */
    { 10, 75 },
    { 11, 80 },
    { 12, 85 },
    { 13, 88 },
    { 14, 90 },   /* 15 */
    { 15, 95 },
    { 16, 97 },
    { 17, 99 },   /* 18 */
    { 18, 99 },
    { 19, 99 },   /* 20 */
    { 20, 99 },
    { 21, 99 },
    { 22, 99 },
    { 23, 99 },
    { 24, 99 }    /* 25 */
};

const	struct	cha_app_type	cha_app		[26]		=
{
    { 10.0/20.0 },   /* 0 */
    { 10.0/20.0 },   /* 1 */
    { 10.0/20.0 },
    { 11.0/20.0 },
    { 11.0/20.0 },
    { 11.0/20.0 },   /* 5 */
    { 12.0/20.0 },
    { 12.0/20.0 },
    { 13.0/20.0 },
    { 13.0/20.0 },
    { 14.0/20.0 },   /* 10 */
    { 15.0/20.0 },
    { 16.0/20.0 },
    { 17.0/20.0 },
    { 18.0/20.0 },
    { 19.0/20.0 },   /* 15 */
    { 20.0/20.0 },
    { 21.0/20.0 },
    { 22.0/20.0 },
    { 23.0/20.0 },
    { 24.0/20.0 },   /* 20 */
    { 26.0/20.0 },
    { 28.0/20.0 },
    { 30.0/20.0 },
    { 32.0/20.0 },
    { 35.0/20.0 },    /* 25 */
};


/*
 * Liquid properties.
 * Used in world.obj.
 */
 const	struct	liq_type	liq_table	[]	=
 {
 /*    name			color	proof, full, thirst, food, ssize */
     { "water","su",			"clear",	{   0, 1, 10, 0, 16 }	},
     { "beer","bira",			"amber",	{  12, 1,  8, 1, 12 }	},
     { "red wine","kırmızı şarap",		"burgundy",	{  30, 1,  8, 1,  5 }	},
     { "ale","bira",			"brown",	{  15, 1,  8, 1, 12 }	},
     { "dark ale","siyah bira",		"dark",		{  16, 1,  8, 1, 12 }	},

     { "whisky","viski",			"golden",	{ 120, 1,  5, 0,  2 }	},
     { "lemonade","limonata",		"pink",		{   0, 1,  9, 2, 12 }	},
     { "firebreather","ateşnefesi",		"boiling",	{ 190, 0,  4, 0,  2 }	},
     { "local specialty","yerel içki",	"clear",	{ 151, 1,  3, 0,  2 }	},
     { "slime mold juice","balçık suyu",	"green",	{   0, 2, -8, 1,  2 }	},

     { "milk","süt",			"white",	{   0, 2,  9, 3, 12 }	},
     { "tea","çay",			"tan",		{   0, 1,  8, 0,  6 }	},
     { "coffee",	"kahve",		"black",	{   0, 1,  8, 0,  6 }	},
     { "blood","kan",			"red",		{   0, 2, -1, 2,  6 }	},
     { "salt water","tuzlu su",		"clear",	{   0, 1, -2, 0,  1 }	},

     { "coke","kola",			"brown",	{   0, 2,  9, 2, 12 }	},
     { "root beer","kök birası",		"brown",	{   0, 2,  9, 2, 12 }   },
     { "elvish wine","yel şarabı",		"green",	{  35, 2,  8, 1,  5 }   },
     { "white wine","beyaz şarap",		"golden",	{  28, 1,  8, 1,  5 }   },
     { "champagne","şampanya",		"golden",	{  32, 1,  8, 1,  5 }   },

     { "mead","bal likörü",			"honey-colored",{  34, 2,  8, 2, 12 }   },
     { "rose wine","gül şarabı",		"pink",		{  26, 1,  8, 1,  5 }	},
     { "benedictine wine","kutsal şarap",	"burgundy",	{  40, 1,  8, 1,  5 }   },
     { "vodka","votka",			"clear",	{ 130, 1,  5, 0,  2 }   },
     { "cranberry juice","kızılcık suyu",	"red",		{   0, 1,  9, 2, 12 }	},

     { "orange juice","portakal suyu",		"orange",	{   0, 2,  9, 3, 12 }   },
     { "absinthe","rakı",		"clear",	{ 200, 1,  4, 0,  2 }	},
     { "brandy","brendi",			"golden",	{  80, 1,  5, 0,  4 }	},
     { "aquavit","kimyon likörü",		"clear",	{ 140, 1,  5, 0,  2 }	},
     { "schnapps","patates likörü",		"clear",	{  90, 1,  5, 0,  2 }   },

     { "icewine","buzbağ",		"purple",	{  50, 2,  6, 1,  5 }	},
     { "amontillado","kiraz likörü",		"burgundy",	{  35, 2,  8, 1,  5 }	},
     { "sherry","şeri",			"red",		{  38, 2,  7, 1,  5 }   },
     { "framboise","çilek likörü",		"red",		{  50, 1,  7, 1,  5 }   },
     { "rum","rom",			"amber",	{ 151, 1,  4, 0,  2 }	},

     { "cordial","şeftali likörü",		"clear",	{ 100, 1,  5, 0,  2 }   },
     { NULL,NULL,			NULL,		{   0, 0,  0, 0,  0 }	}
 };



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/*
 * Magic spells.
 */

    {
	{"reserved","reserved"},	{100,100,100,100, 100,100,100,100,100 ,100,100,100,100},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,			TAR_IGNORE,		POS_STANDING,
	&gsn_reserved,			SLOT( 0),	 0,	 0,
	"",			"",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"absorb","soğurma"},	{ 67, 93, 93, 93, 93, 93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_absorb,		TAR_CHAR_SELF,		POS_STANDING,
	&gsn_absorb,		SLOT(707),	100,	12,
  "",		"Çevrendeki enerji alanı yokoluyor!",
	"$pG çevresindeki enerji alanı yokoluyor.",
 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"acetum primus","aketum primus"},	{ 34,93,93,93, 93,93,93,93,93,93,93,93,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_acetum_primus,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(654),	20,	12,
	"aketum primus",	"!Aketum Primus!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"accid arrow","asit oku"},		{ 48, 93, 93, 93, 93, 93,93,93,48,93,93,48,48},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_acid_arrow,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(644),	20,	12,
	"asit oku",		"!Asit Oku!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"acid blast","asit patlaması"},		{ 63, 93, 93, 93, 93,93,93,93,63,93,93,63,63},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_acid_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_acid_blast,	SLOT(70),	40,	12,
	"asit",		"!Asit!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"acute vision","güçlü görüş"},         {  93,93,93,93, 93, 93, 93, 7 ,93,93,93,93,93},
        { 3,  2,  1,  1, 1, 1, 2, 1, 1, 1,1,1,1},
        spell_acute_vision,        TAR_CHAR_SELF,          POS_STANDING,
        NULL,                    SLOT(514),        10,       0,
	"",         "Görüşün sönükleşti.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"adamantite golem","adament golem"},{93,93,93,93,93,93,93,93,93,93,93,71,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_adamantite_golem,	TAR_IGNORE,	POS_STANDING,
	NULL,		SLOT(665),	500,		30,
	"",	"Yeni golemler yapmak için yeterli gücü topladın.","",
	CABAL_NONE , 	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"aid","ilk yardım"},			{ 93, 53,93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_aid,        TAR_CHAR_DEFENSIVE,          POS_FIGHTING,
	NULL,		SLOT(680),	100,		12,
	"",	"Daha fazla kişiyi iyileştirebilirsin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"amnesia","amnezi"},         {  93, 93, 93,93,93,93,93,93,93,93,93,93,93 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_amnesia,      TAR_CHAR_OFFENSIVE,       POS_STANDING,
	NULL,       SLOT(538),        100,       12,
	"", "!amnesia!", "",	CABAL_NONE , RACE_NONE,ALIGN_NONE, GROUP_NONE
    },

    {
	{"animate dead","ölü canlandırma"},			{ 93, 93, 93, 93,93,93,93,93,93,93,42,38,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_animate_dead,	TAR_OBJ_CHAR_OFF,	POS_STANDING,
	NULL,		SLOT(581),	50,	12,
	"",	"Yeni ölüleri canlandırmak için gerekli enerjiyi topladın.", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"animate object","nesne canlandırma"},	{ 70, 93, 93, 93, 93, 93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_animate_object,	TAR_OBJ_CHAR_OFF,	POS_STANDING,
	NULL,		SLOT(709),	50,	12,
	"",		"Yeni nesneleri canlandırmak için gerekli enerjiyi topladın.", "",
 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"armor","zırh"},		{  13,1,93,93, 12,93,13,93 , 93, 10,13,13,13},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_armor,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 1),	 5,	12,
	"",			"Zırhının zayıfladığını hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"assist","destek"},      {  93, 93, 93, 93,93,93,93,93,93,93,93,43,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_assist,        TAR_CHAR_DEFENSIVE,          POS_FIGHTING,
        NULL,             SLOT(670),        100,       12,
	"", "Yeniden destek olabilirsin.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"astral walk","yıldız yürüyüşü"},		{31,93,93,93,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_astral_walk,	TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(622),	80,	12,
	"",			"!Astral Walk!",	"",CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"attract other","kişi etkileme"},	{ 93, 93, 93, 93,93,93,93,93,93,93,30,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_attract_other,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	NULL,		SLOT(580),	5,	12,
	"",	"Efendinin seni terkettiğini hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BEGUILING
    },

    {
	{"bark skin","ağaç deri"},        {  93, 93, 93, 93, 93, 93, 93, 22 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_bark_skin,        TAR_CHAR_SELF,          POS_STANDING,
        NULL,             SLOT(515),        40,       0,
	"", "Ağaç derin yokoluyor.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
		{"black death","kara ölüm"},    { 93, 93, 93, 93,93, 64, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_black_death,	TAR_IGNORE,	POS_STANDING,
	&gsn_black_death,	SLOT(677),	200,	24,
	"",		"!black death!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"blade barrier","bıçak duvarı"},	{93,60,93,93,93,93,93,93,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_blade_barrier,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(679),	40,	12,
	"bıçak duvarı",	"!Blade Barrier!",	"",CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ATTACK
    },

    {
	{"bless","kutsama"},		{ 93,  10, 93, 93,14,93,93,93,93,12,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_bless,		TAR_OBJ_CHAR_DEF,	POS_STANDING,
	&gsn_bless,		SLOT( 3),	 5,	12,
  "",			"Kutsallığını yitiriyorsun.",
	"$pG kutsal aylası yokoluyor.", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"bless weapon","silah kutsama"},{  93, 55, 93, 93,55,93,93,93,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_bless_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(637),	100,	24,
	"",			"!Bless Weapon!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ENCHANTMENT
    },

    {
	{"blindness","körlük"},	{  93,  14, 93, 93,93,14,16,93, 20,93,20,19,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_blindness,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_blindness,		SLOT( 4),	 5,	12,
	"",			"Yeniden görebiliyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"bluefire","mavi alev"},	{  93,37,93,93,93,93,93,93,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_bluefire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_bluefire,			SLOT(660),	20,	12,
	"mavi alev",		"!Mavi Alev!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ATTACK
    },

    {
	{"burning hands","yanan eller"},	{  14,93,93,93, 93, 93, 93, 93 ,14,93,14,14,14},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_burning_hands,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_burning_hands,	SLOT( 5),	15,	12,
  "yanan eller",	"!Burning Hands!", 	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"call lightning","şimşek çağrısı"},	{ 93, 44, 93, 93, 41,93,50, 93 , 93,93,93,93,33},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_call_lightning,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT( 6),	15,	12,
  "şimşek çağrısı",	"!Call Lightning!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEATHER
    },

    {
    {"calm","yatıştırma"},	{ 93, 26, 93, 93, 63, 93, 93, 93 ,93,60,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_calm,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(509),	30,	12,
  "",			"Sakinliğini yitiriyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"cancellation","iptal"},	{ 24, 26, 93, 93,93,93,93,93 , 28,93,93,28,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cancellation,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(507),	20,	12,
	"",			"!cancellation!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"cause critical","kritik zarar"},	{  93,  15, 93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1,1, 1,1,1,1},
	spell_cause_critical,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(63),	20,	12,
  "kritik zarar",		"!Cause Critical!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HARMFUL
    },

    {
	{"cause light","hafif zarar"},		{ 93,  2, 93, 93,93,93,93,93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cause_light,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(62),	15,	12,
  "hafif zarar",		"!Cause Light!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HARMFUL
    },

    {
	{"cause serious","ciddi zarar"},	{  93,  8, 93, 93,93,93,93,93, 93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cause_serious,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(64),	17,	12,
  "ciddi zarar",		"!Cause Serious!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HARMFUL
    },

    {
	{"caustic font","yakıcı su"},{ 93, 93, 93, 93, 93, 93, 93,93,41,93,93,93,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_caustic_font,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(653),	20,	12,
  "yakıcı su",	"!caustic font!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"chain lightning","şimşek zinciri"},		{ 33, 93, 93, 93,93,93,93,93, 33 ,93,33,33,33},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_chain_lightning,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(500),	25,	12,
  "şimşek",		"!Chain Lightning!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"charm person","teshir"},		{ 93, 93, 93, 93,93,38,93,93, 93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_charm_person,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	SLOT( 7),	 5,	12,
  "",		"Kendine güvenin artıyor.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_BEGUILING
    },

    {
	{"chromatic orb","krom küre"},	{ 93, 93, 93, 93, 93,93,93,93,63,93,93,93,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_chromatic_orb,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(714),	50,	12,
  "krom küre",		"!Chromatic Orb!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"control undead","hortlak kontrolü"},      {  93, 93,93,93,93,93,93,93,93,93,93,57,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_control_undead,   TAR_CHAR_OFFENSIVE,          POS_STANDING,
        NULL,             SLOT(669),        20,       12,
        "", "Kendine güvenin artıyor.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"chill touch","ayaz"},		{  5, 93, 93, 93,93,6,93,93 ,5,93,93,7,8 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_chill_touch,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_chill_touch,		SLOT( 8),	15,	12,
  "ayaz",	"Üşümen azalıyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"colour spray","renk spreyi"},		{  22, 93, 93, 93,93,93,93,93 , 22,93,25,93,22},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_colour_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(10),	15,	12,
  "renk spreyi",		"!Colour Spray!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"continual light","daimi ışık"},		{  11,  5, 93, 93,93,93,93,93 , 11,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_continual_light,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(57),	 7,	12,
	"",			"!Continual Light!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_CREATION
    },

    {
	{"control weather","hava kontrolü"},	{ 93,21,93,93, 93,93,11,33, 30,93,24,24,20},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_control_weather,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(11),	25,	12,
	"",			"!Control Weather!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEATHER
    },

    {
	{"corruption","çürüme"},      {  93, 93, 93, 93, 93,93,93,93,93,93,93,63,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_corruption,    TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL,             SLOT(671),        20,       12,
        "çürüme", "Kendini sağlıklı hissediyorsun.", "",CABAL_NONE,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"create food","mantar"},		{ 15, 8, 93, 93,3,93,93,93 , 93,93,93,12,13},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_create_food,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(12),	 5,	12,
	"",			"!Create Food!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_CREATION
    },

    {
	{"create rose","gül"},		{ 26, 93, 93, 93, 20, 93, 93, 93,93,93,26,26,26},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_create_rose,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(511),	30, 	12,
	"",			"!Create Rose!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_CREATION
    },

    {
	{"create spring","pınar"},		{ 24, 27, 93, 93,93,93,93,93 ,93,93,93,31,31},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_create_spring,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(80),	20,	12,
	"",			"!Create Spring!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_CREATION
    },

    {
	{"create water","su yaratma"},	{ 8,  3, 93, 93,4,93,93,93 ,93,93,93,11,8},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_create_water,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(13),	 5,	12,
	"",			"!Create Water!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_CREATION
    },

    {
	{"cure blindness","körlük tedavi"},		{ 93,11,93,93, 7,93,9,13, 93,20,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(14),	 5,	12,
	"",			"!Cure Blindness!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_CURATIVE
    },

    {
	{"cure critical","kritik tedavi"},	{93,  18, 93, 93,18,93,25,93,93,25,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cure_critical,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	&gsn_cure_critical,	SLOT(15),	20,	12,
	"",			"!Cure Critical!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"cure disease","hastalık tedavi"},		{  93, 19, 93, 93,33,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cure_disease,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(501),	20,	12,
	"",			"!Cure Disease!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_CURATIVE
    },

    {
	{"cure light","hafif tedavi"},	{ 93,  4, 93, 93,5,11,9,93,93,10,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cure_light,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	&gsn_cure_light,	SLOT(16),	10,	12,
	"",			"!Cure Light!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"poison","zehir"},		{  93,  22, 93,93,93,15,93,93,27,93,93,23,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_poison,		TAR_OBJ_CHAR_OFF,	POS_FIGHTING,
	&gsn_poison,		SLOT(33),	10,	12,
  "zehir",		"Üzerindeki zehir kuruyor.",
	"$p üzerindeki zehir kuruyor.", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"cure poison","zehir tedavi"},			{ 93,  23, 93,93,33,93,93,93,93,35,93,93,23 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_cure_poison,		SLOT(43),	 5,	12,
	"",			"!Cure Poison!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_CURATIVE
    },

    {
	{"cure serious","ciddi tedavi"},	{ 93,  10, 93, 93,10,93,18,93,93,18,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cure_serious,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	&gsn_cure_serious,	SLOT(61),	15,	12,
	"",			"!Cure Serious!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"curse","lanet"},		{ 93, 33, 93,93,93,18,93,93,31,93,44,34,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_curse,		TAR_OBJ_CHAR_OFF,	POS_FIGHTING,
	&gsn_curse,		SLOT(17),	20,	12,
  "lanet",		"Üzerindeki lanet çürüyor.",
	"$p üzerindeki lanet çürüyor.", CABAL_NONE , RACE_NONE,
	ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"cursed lands","lanetli topraklar"},    { 93, 41, 93, 93,93, 93, 93, 93 ,93,93,93,64,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_cursed_lands,	TAR_IGNORE,	POS_STANDING,
	&gsn_cursed_lands,	SLOT(675),	200,	24,
	"",		"!cursed lands!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"deadly venom","ölüm zehri"},   { 93, 93, 93, 93,93, 58, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_deadly_venom,	TAR_IGNORE,	POS_STANDING,
	&gsn_deadly_venom,		SLOT(674),	200,	24,
	"",		"!deadly venom!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"deafen","sağırlık"},       {  93,93,93,93,93,42,93,93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_deafen,      TAR_CHAR_OFFENSIVE,   POS_FIGHTING,
        &gsn_deafen,       SLOT(570),        40,       12,
        "sağırlık", "Kulaklarındaki çınlama sonunda yokoluyor.", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"demonfire","iblis ateşi"},		{  93, 37, 93, 93,93,93,93,93,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_demonfire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_demonfire,			SLOT(505),	20,	12,
  "iblis ateşi",		"!Demonfire!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ATTACK
    },

    {
	{"desert fist","çöl yumruğu"},		{ 93, 58, 93,93,93,93,93,93,93,93,93,93,55 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_desert_fist, 	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(681),	50,	12,
  "çöl yumruğu",	"!desert fist!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE,GROUP_NONE
    },

    {
	{"detect evil","kem saptama"},		{ 12,  11, 93, 93,3,93,93,93 ,12,93,93,93,13},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_detect_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(18),	 5,	12,
  "",			"Kem saptama sönüyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"detect good","iyi saptama"},         { 12,  11, 93, 93,3,93,93,93 ,12,93,13,13,13},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_detect_good,      TAR_CHAR_SELF,          POS_STANDING,
        NULL,                   SLOT(513),        5,     12,
        "",                     "İyiyi saptama sönüyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"detect hide","saklıyı saptama"},	{ 93, 93, 10, 93, 93, 93, 10, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_CHAR_SELF,		POS_STANDING,
	&gsn_detect_hidden,	SLOT(0),	 5,	12,
  "",			"Saklıyı saptama sönüyor.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"detect invis","görünmezi saptama"},		{  5,  17, 93, 93,23,13,93,93 ,10,93,23,13,8},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_detect_invis,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(19),	 5,	12,
  "",			"Görünmezi saptama sönüyor.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"detect magic","büyü saptama"},	{  8,  12, 93, 93,93,93,93,93,6,30,93,15,12},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_detect_magic,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(20),	 5,	12,
  "",			"Büyü saptama sönüyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"detect poison","zehir saptama"},	{ 35,  39, 93, 93,93,93,93,93,35 ,39,31,21,7},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_detect_poison,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(21),	 5,	12,
	"",			"!Detect Poison!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
{"detect undead","hortlak saptama"},	{ 93, 93,93, 93,93,93,93,93 ,93,93,93,13,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_detect_undead,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		SLOT(594),	5,	12,
  "",		"Hortlak saptama sönüyor.", "",
	CABAL_NONE, 	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"disenchant armor","zırh alçalt"},	{ 27, 93, 93, 93, 93, 93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_disenchant_armor,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(705),	50,	24,
	"",		"!disenchant armor!",	"", 	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"disenchant weapon","silah alçalt"},		{ 34, 93, 93, 93, 93, 93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_disenchant_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(706),	50,	24,
	"",		"!disenchant weapon!",	"", 	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    { /* Ani ölüm */
	{"disintegrate","ufala"},       {  75,93,93,93,93,93,93,93,93,93 ,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_disintegrate,      TAR_CHAR_OFFENSIVE,   POS_STANDING,
        NULL,                    SLOT(574),    100,       18,
        "ince ışık ışını", "!disintegrate!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    { /* Ani ölüm */
	{"arz yutağı","arz yutağı"},       {  93,93,93,93,93,93,93,93,93,93 ,93,93,75},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_arz_yutagi,      TAR_CHAR_OFFENSIVE,   POS_STANDING,
        NULL,                    SLOT(574),    100,       18,
        "arz yutağı", "!arz yutağı!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"dismantle","parçala"},	{ 93, 93,93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		SLOT(621),	200,	24,
	"",	"!621!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"dispel evil","kem defet"},		{ 93, 27, 93, 93,35,93,93,93,55 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_dispel_evil,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(22),	15,	12,
  "kem defet",		"!Dispel Evil!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ATTACK
    },

    {
	{"dispel good","iyilik defet"},          {   93,  27,  93,  93,93,30,93,93,57,93,32,32,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_dispel_good,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(512),      15,     12,
        "iyilik defet",          "!Dispel Good!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ATTACK
    },

    {
	{"dispel magic","büyü defet"},	{ 36, 43, 93, 93,93,93,93,93 ,41,93,24,24,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_dispel_magic,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(59),	15,	12,
	"",			"!Dispel Magic!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"disruption","nefes kesme"},	{ 93, 93, 93, 93, 93, 93, 93,93,93,93,93,40,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_disruption,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(648),	20,	12,
  "nefes kesme",	"!disruption!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"dragon breath","ejderha nefesi"}, {93,93,93,93,93,93,93,93,93,93,93,93,93},
	{1,1,1,1,1,1,1,1,1, 1,1,1,1},
        spell_dragon_breath,  TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_dragon_breath,  SLOT(563),   75,   12,
        "ejderha nefesi", "!dragon breath!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"dragon skin","ejderha derisi"},	{ 10, 10,10, 10,10,10,10,10 ,10,10,10,10,10},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_dragon_skin,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(612),	50,	24,
  "",	"Derin yumuşuyor.",	"",
	CABAL_NONE , RACE_TOP, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"dragon strength","ejderha gücü"}, {93,93,93,93,93,93,93,93,93,93,93,93,93},
	{1,1,1,1,1,1,1,1, 1, 1,1,1,1},
        spell_dragon_strength, TAR_CHAR_SELF, POS_FIGHTING,
        &gsn_dragon_strength, SLOT(562),    75,    12,
        "", "Ejderha gücünün seni terkettiğini hissediyorsun.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"dragons breath","ejderha nefesi"},	{ 61, 93, 93, 93, 93, 93, 93, 93 ,61,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_dragons_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(576),	200,	24,
  "ejderha nefesi",	"Sağlığına kavuşuyorsun.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"drain","kurut"},		{ 31, 93, 93, 93, 93, 93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_drain,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(704),	5,	12,
	"",		"!drain!",	"", 	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"earthfade","arz soluşu"},		{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,73},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_earthfade,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	&gsn_earthfade,			SLOT(702),	100,	12,
  "",		"Doğal formuna dönüyorsun.",	"",
 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"earthmaw","fay"},		{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,67},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_earthmaw,		TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(703),	30,	12,
	"earthmaw",		"!earthmaw!",	"", 	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"earthquake","deprem"},	{ 93,  19, 93, 93,93,93,93,93,93,93,93,93,19 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_earthquake,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(23),	15,	12,
  "deprem",		"!Deprem!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ATTACK
    },

    {
	{"elemental sphere","uşak küresi"},	{ 93, 93,93,93, 93,93,93,93 ,93,93,93,93,29},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_elemental_sphere,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(719),	75,	12,
  "",	"Koruyucu uşak küresi yokoluyor.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"enchant armor","zırh yükselt"},	{  26, 25, 93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_enchant_armor,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(510),	100,	24,
	"",			"!Enchant Armor!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ENCHANTMENT
    },

    {
	{"enchant weapon","silah yükselt"},{  33, 25, 93, 93,93,93,93,93,17,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_enchant_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(24),	100,	24,
	"",			"!Enchant Weapon!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ENCHANTMENT
    },

    {
	{"energy drain","yaşam kurut"},		{ 38,42,93,93,93,26,93,93,93,93,93,45,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(25),	35,	12,
  "yaşam kurut",		"!Energy Drain!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"enhanced armor","gelişmiş zırh"},	{ 48, 33, 93,93,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1},
	spell_enhanced_armor, 	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	&gsn_enhanced_armor,	SLOT(583),	20,	12,
  "",	"Korunmasız hissediyorsun.", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"enlarge","genişleme"},		{  93,93,93,93, 93,93,93,93, 38,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_enlarge,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(711),	 50,	12,
  "",	"Doğal boyutlarına dönüyorsun.",	"", CABAL_NONE,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"etheral fist","ruhani yumruk"},		{ 93,93,93,93, 93, 50,93,93,93,93,93,93,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_etheral_fist,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(645),	20,	12,
  "ruhani yumruk",		"!Etheral Fist!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"evolve bear","ayıya dönüşme"},	{   93, 93, 93, 93,93,93,93,65,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	        spell_null,      TAR_IGNORE,       POS_STANDING,
	&gsn_evolve_bear,       SLOT(0),        100,       12,
    "",	"Doğallaşıyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"faerie fire","cin ateşi"},		{ 11,  6, 93, 93,93,93,8,93 ,15 ,93,93,93,11},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_faerie_fire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_faerie_fire,		SLOT(72),	 5,	12,
  "cin ateşi",		"Pembe aylandan kurtuluyorsun.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_WEATHER
    },

    {
	{"faerie fog","cin sisi"},	{ 24, 31, 93, 93,93,93,93,93 ,27,93,93,93,24},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_faerie_fog,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(73),	12,	12,
  "cin sisi",		"!Faerie Fog!",		"", CABAL_NONE ,
	RACE_NONE,ALIGN_NONE, GROUP_WEATHER
    },

    {
	{"farsight","uzak görüş"},	{  22, 93, 93, 93,93,93,93,93,22,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_farsight,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(521),	20,	12,
  "uzak görüş",		"!Farsight!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"fear","korku"},			{   93, 93, 93, 93,93,93,93,93,93 ,93,93,73,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_fear,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_fear,	SLOT(598),	50,	12,
  "",		"Daha cesur hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ILLUSION
    },

    {
	{"fire and ice","ateş ve buz"},		{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,65},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_fire_and_ice,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(699),	40,	12,
  "ateş ve buz",		"!fire and ice!",	"", 	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"fireball","alev topu"},		{55,93,93,93,93,56,93,93,56,93,93,93,55},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_fireball,		TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(26),	25,	12,
  "alev topu",		"!Fireball!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"fireproof","ateş geçirmez"},		{ 25, 16, 93, 93, 20, 93,93,93,21,93,93,42,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_fireproof,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(523),	10,	12,
  "",		"",	"$p üzerindeki koruyucu ayla yokoluyor.", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ENCHANTMENT
    },

    {
	{"firestream","alev akımı"},	{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,35},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_firestream,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(692),	20,	12,
	"",		"",	"", 	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"fire shield","alev kalkanı"},			{   93, 93, 93, 93,93,93,93,93,93 ,93,93,93,68},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_fire_shield,	TAR_IGNORE,	POS_STANDING,
	&gsn_fire_shield,		SLOT(601),	200,	24,
	"",			"!fire shield!",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"flamestrike","alev saldırısı"},			{93,42,93,93,32,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_flamestrike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(65),	20,	12,
  "alev saldırısı",		"!Flamestrike!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ATTACK
    },

    {
	{"fly","uçma"},				{ 19, 30, 93,93,93,93,93,93,19,93,43,23,21},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_fly,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_fly,			SLOT(56),	10,	18,
  "",		"Yere iniyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"floating disc","asılı disk"},		{ 4, 10, 93, 93, 93, 93, 93, 93 ,4,93,93,93,4},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_floating_disc,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(522),	40,	24,
	"",			"!Floating disc!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"forcecage","güç kafesi"},			{ 29, 93,93,93, 93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_forcecage,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(717),	75,	12,
  "",	"Çevrendeki güç kafesi yokoluyor.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"frenzy","cinnet"},               {  93, 34, 93, 93,93,93,93,93,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
        NULL,                   SLOT(504),      30,     24,
        "",                     "Cinnetin geçiyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"frostbolt","ayaz topu"},		{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,42},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_frostbolt,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(695),	20,	12,
	"ayaz topu",		"!frostbolt!",	"", 	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"fumble","beceriksizlik"},		{ 93,93,93,93, 93,93,93,93 ,36,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_fumble,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(712), 	25,	18,
  "",		"Hızlanıyor ve eski gücüne kavuşuyorsun!",	"",
	CABAL_NONE, RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"galvanic whip","enerji kırbacı"},	{ 28, 93, 93, 93, 93, 93,93,93,93,93,93,93,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_galvanic_whip,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(655),	20,	12,
  "enerji kırbacı",	"!galvanic whip!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"gate","geçit"},			{93,63,93,93,93,93,93,93,27,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_gate,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(83),	80,	12,
	"",			"!Gate!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"giant strength","dev gücü"},{  20, 39, 93, 93,93,93,93,93, 30,93,93,28,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_giant_strength,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_giant_strength,		SLOT(39),	20,	12,
  "",			"Gücün azalıyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ENHANCEMENT
    },

    {
	{"grounding","topraklama"},		{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,53},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_grounding,	TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,			SLOT(700),	50,	12,
  "",		"Topraklamanı yitiriyorsun.",	"",
 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"group defense","grup savunma"},	{   93, 35, 93,93,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_group_defense,		TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(586),	100,	36,
  "",			"Korunmasız hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"group heal","grup şifa"},			{  93,65,93,93,75,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_group_healing,	TAR_CHAR_DEFENSIVE, POS_FIGHTING,
	NULL,			SLOT(642),	500,	24,
	"",			"!Group Heal!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"hallucination","sanrı"},	{   93, 93, 93, 93,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_hallucination,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,		SLOT(606),	200,	12,
  "",			"Büyüye karşı yine savunmasızsın.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_ILLUSION
    },

    {
	{"hand of undead","hortlak eli"},	{ 93, 93, 93,93,93,93,93,93,93,93,93,44,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_hand_of_undead,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(647),	20,	24,
  "hortlak eli",	"!hand of undead!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"harm","zarar"},			{  93, 26, 93, 93,93,29,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_harm,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(27),	35,	12,
  "zarar",		"!Harm!",	"", 	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HARMFUL
    },

    {
	{"haste","hız"},	{ 24, 93, 93,93,93,93,93,93,24,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_haste,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	&gsn_haste,		SLOT(502),	30,	12,
  "",			"Yavaşladığını hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ENHANCEMENT
    },

    {
	{"heal","şifa"},		{  93, 28, 93, 93,29,93,93,93 ,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_heal,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(28),	50,	12,
	"",			"!Heal!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"healing light","şifa ışığı"},    { 93, 41, 93, 93,93, 93, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_healing_light,	TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(613),	200,	24,
  "",			"Bir odayı yeniden ışığınla doldurabilirsin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"heat metal","metali ısıtma"},		{ 93,26,93,93, 93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_heat_metal,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(516), 	25,	18,
  "metali ısıtma",		"!Heat Metal!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"helical flow","sarmal akım"},		{93,93,93,93,93,93,93,93,93,93,93,93,43},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_helical_flow,		TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(661),	80,	12,
	"",			"!Helical Flow!",	"",CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"hellfire","cehennem ateşi"},     {  93, 93, 93, 93, 93, 93, 45, 93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_hellfire,      TAR_CHAR_OFFENSIVE,          POS_FIGHTING,
        NULL,       SLOT(520),        20,       12,
        "cehennem ateşi", "!hellfire!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ATTACK
    },

    {
	{"holy aura","kutsal ayla"},{ 93, 93,93,93,62,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_holy_aura,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(617),	75,	12,
  "",	"Kutsal aylan yokoluyor.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"holy fury","kutsal hiddet"},	{ 93, 93,93,93,59,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_holy_fury,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(682),	50,	24,
  "",	"Artık daha merhametlisin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"holy word","kutsal söz"},		{ 93,48,93,93,38,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_holy_word,	TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(506), 	200,	24,
  "kutsal söz",		"!Holy Word!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"hurricane","kasırga"},	{  93, 93, 93, 93, 93,53,93,93,93,93,93,65,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_hurricane,	TAR_IGNORE,	POS_FIGHTING,
	NULL,		SLOT(672),	200,	24,
  "kasırga",	"!Hurricane!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"hydroblast","su patlaması"},	{  93,93,93,93, 93,93,93,46, 93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_hydroblast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(684),	50,	12,
  "su patlaması",	"!Hydroblast!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"iceball","buztopu"},        {  93, 93, 93, 93, 93,58,93,93,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_iceball,     TAR_IGNORE,     POS_FIGHTING,
        NULL,                   SLOT(513),        25,       12,
        "buztopu",         "!Iceball!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"identify","tanımlama"},		{ 18, 36, 93, 93,93,93,93,93 ,25,93,93,93,18},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_identify,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(53),	12,	24,
	"",			"!Identify!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"improved detect","gelişmiş saptama"},	{ 40, 93, 93, 93, 93,93,93, 93 ,40,93,93,40,40},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_improved_detection,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		SLOT(626),	 20,	12,
  "",			"Artık çevrende olup bitenden daha az haberdarsın.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"improved invis","gelişmiş görünmezlik"},	{  43, 93, 93, 93,93,93,93,93,43,93,93,93,43 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_improved_invis,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_imp_invis,		SLOT(627),	 20,	12,
  "",			"Görünmezliğin yitiyor.",
	"$p görünürleşiyor.", CABAL_NONE , RACE_NONE,ALIGN_NONE, GROUP_ILLUSION
    },

    {
	{"infravision","ısı görüşü"},		{  17,  25, 93, 93,93,93,5,93,18,93,93,21,17},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_infravision,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(77),	 5,	18,
  "",			"Artık karanlıkta göremeyeceksin.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ENHANCEMENT
    },

    {
	{"insanity","delilik"},	{ 93, 93,93, 93,93,93,93,93 ,93,93,93,59,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_insanity,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	NULL,		SLOT(616),	100,	24,
  "",	"Sakinleştiğini hissediyorsun.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"inspire","ilham"},		{   93, 49, 93,93,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_inspire,		TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(587),	75,	24,
  "",			"İlhamın azalıyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"invisibility","görünmezlik"},		{  10, 93, 93, 93,93,9,93,93,13,93,93,93,10 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_invis,		TAR_OBJ_CHAR_DEF,	POS_STANDING,
	&gsn_invis,		SLOT(29),	 5,	12,
	"",			"Artık görünmez değilsin.",
	"$p görünür oluyor.", CABAL_NONE , RACE_NONE,ALIGN_NONE, GROUP_ILLUSION
    },

    {
{"iron body","demir gövde"},	{ 93, 93,93,93, 93,93,93,93 ,29,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_iron_body,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(718),	75,	12,
  "",	"Derin yumuşuyor.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"iron golem","demir golem"},		{93,93,93,93,93,93,93,93,93,93,93,52,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_iron_golem,	TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(664),	400,		24,
  "",	"Yeni golemler yapmak için yeterli gücü topladın.","",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"knock","taktak"},			{   56,93,93,93,93,93,93,93,64,93,93,62,46},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_knock,	TAR_IGNORE,	POS_STANDING,
	NULL,		SLOT(603),	20,	24,
	"",			"!knock!",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"know alignment","yönelim saptama"},	{  22,  13,93,93,1,5,93,93,29,93,93,22,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_know_alignment,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(58),	 9,	12,
	"",			"!Know Alignment!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"lesser golem","küçük golem"},		{93,93,93,93,93,93,93,93,93,93,93,25,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_lesser_golem,	TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(662),	200,		12,
  "",	"Yeni golemler yapmak için yeterli gücü topladın.","",
	CABAL_NONE , 	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"lethargic mist","uyuşukluk sisi"},   { 52, 93,93,93,93, 62, 93,93,56,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_lethargic_mist,	TAR_IGNORE,	POS_STANDING,
	&gsn_lethargic_mist,	SLOT(676),	200,	24,
	"",		"!lethargic mist!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"light arrow", "ışık oku"},	{  93, 93, 93, 93,64,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_light_arrow,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(683),	40,	12,
	"ışık oku",	"!light arrow!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"lightning bolt","yıldırım"},	{  23, 93, 93, 93,93,13,93,93 ,23,93,93,23,23},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_lightning_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_lightning_bolt,		SLOT(30),	15,	12,
  "yıldırım",	"!Lightning Bolt!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"lightning shield","şimşek kalkanı"},	{ 41, 93,93, 93,93,93,93,93 ,93,93,93,41,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_lightning_shield,	TAR_IGNORE,	POS_STANDING,
	NULL,		SLOT(614),	150,	24,
  "şimşek kalkanı",	"Bir odayı yeniden kalkanlayabilirsin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"link","bağ"},	{   31, 93, 93, 93,93,93,93,93,33 ,93,93,33,33},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_link,		TAR_CHAR_DEFENSIVE,		POS_STANDING,
	NULL,		SLOT(588),	125,	18,
	"",			"!link!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MEDITATION
    },

    {
	{"lion help","aslan yardımı"},	{   93, 93, 93, 93,93,93,93,63,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_lion_help,	TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(595),	100,	12,
  "",	"Yeniden aslan yardımına başvurabilirsin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"locate object","nesne bulma"},	{   30, 30, 93, 93,19,93,14,93,29,93,93,93,31},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_locate_object,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(31),	20,	18,
	"",			"!Locate Object!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"love potion","aşk iksiri"},           { 93, 93, 93,93,93,93,93,93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_love_potion, TAR_CHAR_SELF,       POS_STANDING,
        &gsn_love_potion,  SLOT(666),        10,       0,
        "", "Gözlerindeki rüya sönüyor.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"magic jar","büyülü şişe"},		{   93, 93, 93, 93,93,93,93,93,93 ,93,93,68,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_magic_jar,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(596),	20,	12,
	"",			"!magic jar!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BEGUILING
    },

    {
	{"magic missile","fişek"},	{ 1, 93, 93, 93,93,1,93,93 ,1,93,93,2,1},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_magic_missile,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_magic_missile,		SLOT(32),	15,	12,
	"fişek",	"!Magic Missile!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"magic resistance","büyü direnci"},	{   68, 93, 93, 93,93,93,93,93,72 ,93,93,69,71},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_magic_resistance,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,		SLOT(605),	200,	24,
  "",			"Büyüye karşı yeniden savunmasızsın.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"magnetic trust","manyetizma"},{ 93, 93, 93, 93, 93, 93,93,93,34,93,93,93,34},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_magnetic_trust,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(656),	20,	12,
  "manyetizma",	"!magnetic trust!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"mass healing","toplu şifa"},	{ 93, 54, 93, 93,93,93,93,93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mass_healing,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(508),	100,	36,
	"",			"!Mass Healing!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"mass invis","toplu görünmezlik"},		{ 43, 47, 93, 93,93,93,93,93 ,42,93,93,93,46},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mass_invis,	TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	SLOT(69),	20,	24,
  "",			"Görünmezliğin yitiyor.",		"", CABAL_NONE ,
	RACE_NONE,ALIGN_NONE, GROUP_ILLUSION
    },

    {
	{"mass sanctuary","toplu takdis"},{   93, 62, 93, 93,93,93,93,93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mass_sanctuary,	TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(589),	200,	24,
  "",			"Beyaz auran yokoluyor.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"master healing","yüksek tedavi"},	{  93,58,93,93,70,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_master_heal,	TAR_CHAR_DEFENSIVE,POS_FIGHTING,
	NULL,			SLOT(641),	300,	12,
	"",			"!Master Heal!",	"",CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"meld into stone","taşa dönüşme"},	{ 93, 93,93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_meld_into_stone,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(584),	12,	18,
  "",	"Derindeki taşlar kumlaşarak dökülüyor.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"mend","onar"},	{   50, 93, 93, 93,93,93,93,93,50 ,93,93,93,50},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mend,		TAR_OBJ_INV,		POS_STANDING,
	&gsn_mend,		SLOT(590),	150,	24,
	"",			"!mend!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ENCHANTMENT
    },

    {
	{"mind light","zihin ışığı"},	{ 93, 83,93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mind_light,	TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(82),	200,	24,
  "",	"Artık yeni odaları hızlandırabilirsin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"mind wrack","zihin yosunu"},{ 93, 93, 93, 93, 93, 27, 93,93,93,93,93,93,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mind_wrack,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(650),	20,	12,
  "zihin yosunu",	"!mind wrack!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"mind wrench","zihin bükümü"},	{ 93, 93, 93, 93, 93, 40, 93,93,93,93,93,93,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mind_wrench,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(651),	20,	12,
  "zihin bükümü",	"!mind wrench!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"mist walk","gizemli yürüyüş"},		{  93,93,93,93,93,93,93,93,93,93,47,47,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mist_walk,	TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(658),	80,	12,
	"",			"!Mist Walk!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"mummify","mumyala"},	{ 93, 93, 93, 93,93,93,93,93,93,93,93,44,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mummify,	TAR_OBJ_CHAR_OFF,	POS_STANDING,
	NULL,		SLOT(715),	50,	12,
  "",	"Yeni ölülere hayat verebilirsin.", 	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"mysterious dream","gizemli düş"}, 	{ 93, 93,93,93,93,35,93,93,49,93,93,27,93},
		{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_mysterious_dream,	TAR_IGNORE,	POS_STANDING,
	&gsn_mysterious_dream,	SLOT(678),	200,	24,
	"",		"!mysterous dream!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_BEGUILING
    },

    {
	{"nexus","neksus"},     { 93,93,93,93,93,93,93,93 ,60,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_nexus,            TAR_IGNORE,             POS_STANDING,
        NULL,                   SLOT(520),       150,   36,
	"",                     "!Nexus!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"pass door","saydamlık"},		{  24, 32, 93, 93,93,93,93,93 ,24,24,93,27,24},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_pass_door,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_pass_door,			SLOT(74),	20,	12,
  "",			"Cisimleşiyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"plague","veba"},		{  93, 93, 93, 93,93,46,93,93,33,93,93,36,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_plague,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_plague,		SLOT(503),	20,	12,
  "veba",		"Hastalığın çürüyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"polymorph","binbir şekil"},      { 93, 93, 93, 93,93, 93,93,93,73,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_polymorph,	TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(639),	250,	24,
  "",			"Doğal ırkına dönüyorsun.","",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"portal","köprü"},                { 54, 93, 93, 93, 93,93,93,93,40,93,93,43,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_portal,           TAR_IGNORE,             POS_STANDING,
        NULL,                   SLOT(519),       100,     24,
	"",                     "!Portal!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"protection cold","soğuktan korunma"},{   93, 44, 93, 93, 93,93,93,25, 93,93,93,50,44},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_protection_cold,	TAR_CHAR_SELF,	POS_STANDING,
	&gsn_protection_cold,		SLOT(600),	5,	12,
  "",			"Daha az korunmasız hissediyorsun.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"protection evil","kemden korunma"},	{ 93,  9, 93, 93,11,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_protection_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(34), 	5,	12,
  "",			"Daha az korunmasız hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"protection good","iyiden korunma"},{ 93,9,93,93,93,13,93,93,93,93,27,17,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_protection_good,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(666), 	5,	12,
  "",			"Daha az korunmasız hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"protection heat","sıcaktan korunma"},	{ 93, 39, 93, 93,93,93,93,93, 93,93,93,48,39},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_protection_heat,	TAR_CHAR_SELF,	POS_STANDING,
	&gsn_protection_heat,		SLOT(599),	5,	12,
  "",			"Daha az korunmasız hissediyorsun.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"protection negative","negatiften korunma"},{   93, 93,93,93,93,93,93,93,93,93,93,15,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_protection_negative,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,		SLOT(636),	20,	12,
  "",		"Kendi saldırılarına karşı daha az korunmasız hissediyorsun.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"protective shield","koruma kalkanı"},     { 26,38,93,93, 93,93,93,93 ,26,93,93,18,26},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_protective_shield,      TAR_CHAR_SELF,   POS_STANDING,
        &gsn_protective_shield,       SLOT(572),        70,       12,
        "", "Kalkanın sönüyor.", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    { /* Ani ölüm */
	{"power word kill","ölüm sözcüğü"},		{   93, 93, 93, 93,93,93,93,93,93 ,93,93,78,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_power_kill,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	NULL,		SLOT(604),	200,	18,
  "ölüm sözcüğü",		"Dayanıklılığın artıyor.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"power word stun","bayıltma sözcüğü"},{ 93, 93,93, 93,93, 62,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_power_stun,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_power_stun,		SLOT(625),	200,	24,
  "",	"Hareket edebilirsin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"quantum spike","enerji dikeni"},	{ 41, 93, 93, 93, 93, 93,93,93,93,93,93,93,41},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_quantum_spike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(657),	20,	12,
  "enerji dikeni",	"!quantum spike!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"ranger staff","korucu asası"},          {  93, 93, 93, 93, 93, 93, 93, 32 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_ranger_staff,      TAR_IGNORE,          POS_FIGHTING,
        NULL,             SLOT(519),        75,       0,
	"", "!ranger staff!", "", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"ray of truth","ilahi enerji"},        { 93, 37, 93, 93, 45,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_ray_of_truth,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_ray_of_truth,      SLOT(518),      20,     12,
        "ilahi enerji",         "!Ray of Truth!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"recharge","şarj"},		{ 29,93,93,93, 93, 93, 93, 93, 29,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_recharge,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(517),	60,	24,
	"",			"!Recharge!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ENCHANTMENT
    },

    {
	{"refresh","yenileme"},		{  93,7, 93,93, 93,93,93,93, 93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_refresh,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(81),	12,	18,
  "yenileme",		"!Refresh!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"reincarnation","dirilme"},    {  93, 93, 93, 93, 93,93,93,93,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_IGNORE,          POS_STANDING,
        NULL,             SLOT(668),        0,       0,
	"", "!!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"remove curse","lanet kaldır"},	{  93, 32, 93, 93,13,93,93,93,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_remove_curse,	TAR_OBJ_CHAR_DEF,	POS_STANDING,
	&gsn_remove_curse,	SLOT(35),	 5,	12,
	"",			"!Remove Curse!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_CURATIVE
    },

    {
	{"remove fear","korku kaldır"},		{ 93, 34, 93,93,41,93,93,93,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_remove_fear, 	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(582),	5,	12,
	"",	"!Remove Fear!", "", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"remove tattoo","dövme kaldır"},    { 93, 93, 93,93,93,93,93,93,93,93,93,93,93 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_remove_tattoo, TAR_CHAR_DEFENSIVE,       POS_STANDING,
        NULL,       SLOT(552),        10,       0,
	"", 	"!remove tattoo!", 	"",
        CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"resilience","esneklik"},	{  93,93, 93, 93,56,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_resilience,	TAR_CHAR_DEFENSIVE, POS_STANDING,
	NULL,			SLOT( 638),	 50,	12,
  "",	"Emici saldırılara karşı koruman azalıyor.", "",
	CABAL_NONE ,RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"restoring light","iyileştiren ışık"},   { 93, 71, 93,93,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_restoring_light,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(643),	50,	24,
	"",			"!restoring light!","",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"sanctify lands","kutsal topraklar"},  { 93, 41, 93, 93,53, 93, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_sanctify_lands,	TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(673),	200,	24,
	"",		"!sanctify lands!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"sanctuary","takdis"},			{ 93, 29, 93, 93,27,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_sanctuary,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_sanctuary,		SLOT(36),	75,	12,
  "",			"Beyaz auran yokoluyor.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"sand storm","kum fırtınası"},	{ 93, 93, 93, 93,93,93,93,93,93,93,93,93,51 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_sand_storm,	TAR_IGNORE,	POS_FIGHTING,
	&gsn_sand_storm,		SLOT(577),	200,	24,
  "kum fırtınası",	"Gözlerindeki kum dökülüyor.","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"scream","çığlık"},		{ 52, 93, 93,93,93,93,93,93,48,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_scream,	TAR_IGNORE,	POS_FIGHTING,
	&gsn_scream,		SLOT(578),	200,	24,
  "çığlık",	"Yeniden duyabiliyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"severity force","arz kırımı"},	{ 93, 45,93,93,93, 93, 93, 93,93,93 ,93,93,41},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_severity_force,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(622),	20,	12,
  "arz kırımı",		"!severity force!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"shield","kalkan"},	{ 20, 35, 93, 93,93,93,19,93,93 ,93,20,20,20},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_shield,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_shield,		SLOT(67),	12,	18,
  "",	"Güç kalkanın parlayarak yokoluyor.", 	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"shielding","kalkanlama"},		{   93, 93, 93, 93,93,93,93,93,53 ,93,93,53,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_shielding,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_shielding,		SLOT(591),	250,	12,
  "",			"Gerçek KaynakZ uzaklaştığını hissediyorsun.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"shocking grasp","şok kavrayışı"},		{ 8, 93, 93, 93,93,93,93,93 ,9,93,93,93,8},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_shocking_grasp,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(53),	15,	12,
  "şok kavrayışı",	"!Shocking Grasp!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"shocking trap","şokçu tuzak"},	{ 37, 93,93, 93,93,93,93,93 ,40,93,93,37,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_shocking_trap,	TAR_IGNORE,	POS_STANDING,
	NULL,		SLOT(615),	150,	24,
  "şokçu tuzak",	"Daha fazla odayı şokla doldurabilirsin.","",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"sleep","uyku"},	{ 93, 93, 93, 93,93,25,93,93,39,93,93,39,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_sleep,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_sleep,		SLOT(38),	15,	12,
  "",			"Yorgunluğun geçiyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BEGUILING
    },

    {
	{"slow","yavaşlatma"},       { 28,38,93,93, 93, 28, 93, 93 ,23,93,28,29,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_slow,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_slow,                   SLOT(515),      30,     12,
        "",                     "Hızlandığını hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"soften","yumuşatma"},		{ 93, 93, 93, 93,93,93,93,93 ,58,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_soften,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(713),	 75,	12,
  "yumuşatma",	"Derin sertleşiyor.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_WEATHER
    },

    {
	{"solar flight","güneş uçuşu"},		{93,35,93,93,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_solar_flight,	TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(659),	80,	12,
	"",			"!Solar Flight!",	"",CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"sonic resonance","ses direnci"},	{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,28,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_sonic_resonance,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(649),	20,	12,
  "ses direnci",	"!sonic resonance!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"soul bind","ruh bağı"},		{ 93, 93, 93, 93,93,39,93,93, 93,93,93,54,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_soul_bind,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	NULL,		SLOT(716),	 5,	12,
  "",		"Kendine güvenin artıyor.",	"",
	CABAL_NONE, RACE_NONE, ALIGN_NONE, GROUP_BEGUILING
    },

    {
	{"spectral furor","hayali öfke"},	{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,35,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_spectral_furor,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(646),	20,	12,
  "hayali öfke",	"!spectral furor!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"stone golem","taş golem"},		{93,93,93,93,93,93,93,93,93,93,93,39,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_stone_golem,	TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(663),	300,		18,
  "",	"Yeni golemler yapmak için yeterli gücü topladın.","",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"stone skin","taş deri"},	{ 25, 36, 93,93,93,93,35,93,93,93,60,30,25},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_stone_skin,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(66),	12,	18,
  "",			"Derin yumuşuyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"suffocate","boğma"},    {  93,93,93,93, 93,93,93,93, 48,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_suffocate,    TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL,             SLOT(714),        50,       12,
        "boğma",     "Yeniden soluyabilirsin.", "",CABAL_NONE,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"sulfurus spray","sülfür spreyi"},	{ 93, 93, 93, 93, 93,93,93,93,28,93,93,93,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_sulfurus_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(652),	20,	12,
  "sülfür spreyi",	"!sulfurus spray!", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"summon","çağrı"},		{  23, 22, 93, 93,51,50,93,93,24,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_summon,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(40),	50,	12,
	"",			"!Çağrı!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"summon air elemental","hava uşağı"},  { 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,42},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_summon_air_elm,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(696),	50,	12,
  "",	"Hava uşağı çağırabilecek gücü kazanıyorsun.",
	"", 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"summon earth elemental","toprak uşağı"},{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,38},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_summon_earth_elm,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(693),	50,	12,
  "",	"Toprak uşağı çağırabilecek gücü kazanıyorsun.",
	"", 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"summon fire elemental","ateş uşağı"},{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,58},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_summon_fire_elm,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(697),	50,	12,
  "",	"Ateş uşağı çağırabilecek gücü kazanıyorsun.",
	"", 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"summon lightning elemental","enerji uşağı"},{ 93,93,93,93,93, 93,93,93,93,93,93,93,68},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_summon_light_elm,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(710),	50,	12,
  "", "Enerji uşağı çağırabilecek gücü kazanıyorsun.",
	"", 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"summon water elemental","su uşağı"},{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,48},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_summon_water_elm,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(698),	50,	12,
  "",	"Su uşağı çağırabilecek gücü kazanıyorsun.",
	"", 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"summon shadow","gölge çağrısı"},{ 93, 93,93,93,93,93,93,93,93,93,93,50,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_summon_shadow,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		SLOT(620),	200,	24,
  "",	"Yeni gölgeler çağırabilirsin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"superior heal","yüksek şifa"},{  93, 38, 93,93,43,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_super_heal,	TAR_CHAR_DEFENSIVE,POS_FIGHTING,
	NULL,			SLOT(640),	100,	12,
	"",			"!Super Heal!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_HEALING
    },

    {
	{"tattoo","dövme"},    { 93, 93, 93,93,93,93,93,93,93,93,93,93,93},
	{1,1,1,1,1,1,1,1,1, 1,1,1,1},
        spell_tattoo, TAR_CHAR_DEFENSIVE,       POS_STANDING,
        NULL,       SLOT(551),        10,       0,
	"", "!tattoo!", "",CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"teleport","nakil"},		{  22, 32, 93, 93,93,7,93,93 ,23,93,93,16,16},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_teleport,		TAR_CHAR_SELF,		POS_FIGHTING,
	&gsn_teleport,	 		SLOT( 2),	35,	12,
	"",			"!Teleport!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

    {
	{"transfer object","nesne nakli"},	{ 62, 93, 93, 93, 93, 93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_transfer_object,	TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(708),	40,	12,
	"",		"!transfer object!", "",
 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"tsunami","tsunami"},{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,62},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_tsunami,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(701),	50,	12,
	"raging tidal wave",	"!tsunami!",	"", 	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"turn","devir"},		{   93, 50, 93, 93,60,93,93,93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_turn,	TAR_IGNORE,	POS_FIGHTING,
	NULL,		SLOT(597),	50,	12,
  "",	"Devir büyüsünü yeniden kullanabilirsin.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"vampiric blast","yarasa öfkesi"},	{ 93, 93, 93, 93, 93, 93, 93, 93,93,93 ,38,93,93},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_vam_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(611),	20,	12,
  "yarasa öfkesi",	"!Vampiric Blast!", "",CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"ventriloquate","vantrolog"},	{   1, 93, 93, 93,93,93,93,93, 1,93,93,93,3},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_ventriloquate,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(41),	 5,	12,
	"",			"!Ventriloquate!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_ILLUSION
    },

    {
	{"web","ağ"},		{   93, 93, 93, 93,93,93,93,93,69 ,93,93,58,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_web,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_web,		SLOT(592),	50,	12,
	"",		"Seni saran ağlar eriyor.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_ATTACK
    },

    {
	{"windwall","rüzgar duvarı"},		{ 93, 93, 93, 93, 93, 93,93,93,93,93,93,93,47},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_windwall,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(694),	20,	12,
  "rüzgar duvarı",		"Gözlerin iyileşiyor.",	"",
 	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_COMBAT
    },

    {
	{"witch curse","cadı laneti"},	{   93, 93, 93, 93,93,93,93,93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_witch_curse,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_witch_curse,		SLOT(602),	150,	24,
  "",			"Dayanıklılığın artıyor.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"wrath","gazap"},         { 93, 93, 93,93,34,93,93,93,93,93,93,93,93 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_wrath, TAR_CHAR_OFFENSIVE,       POS_STANDING,
	&gsn_wrath,       SLOT(553),        20,       12,
  "gazap", "Lanet yokoluyor.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BENEDICTIONS
    },

    {
	{"weaken","zayıflatma"},	{   93, 24, 93, 93,93,93,93,93 ,19,93,16,29,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_weaken,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_weaken,		SLOT(68),	20,	12,
  "zayıflatma",		"Güçlendiğini hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MALADICTIONS
    },

    {
	{"word of recall","anımsama sözcüğü"},		{ 32, 28, 93, 93,32,93,29,93 ,32,38,10,10,32},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_word_of_recall,	TAR_CHAR_SELF,		POS_RESTING,
	NULL,			SLOT(42),	 5,	12,
	"",			"!Word of Recall!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_TRANSPORTATION
    },

/*
 * Dragon breath
 */

    {
	{"acid breath","asit nefesi"},		{  67, 67, 67, 67,67,67,67,67 ,67,67,67,67,67},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_acid_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_acid_breath,		SLOT(200),	100,	24,
  "asit nefesi",	"!Acid Breath!",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"desert heat","çöl sıcağı"},		{ 69, 69, 69, 69,69,69,69,69,69,69,69,69,69 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(629),	200,	24,
  "çöl sıcağı","Gözlerindeki duman yokoluyor.",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"fire breath","ateş nefesi"},	{ 69, 69, 69, 69,69,69,69,69,69,69,69,69,69 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_fire_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_fire_breath,		SLOT(201),	200,	24,
  "ateş nefesi",	"Gözlerindeki duman yokoluyor.",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"frost breath","buz nefesi"},			{ 61,61,61,61,61,61,61,61,61,61,61,61,61  },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_frost_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_frost_breath,		SLOT(202),	125,	24,
  "buz nefesi",	"!Frost Breath!",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"gas breath","gaz nefesi"},		{ 70,70,70,70,70,70,70,70,70,70,70,70,70  },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_gas_breath,	TAR_IGNORE,		POS_FIGHTING,
	&gsn_gas_breath,	SLOT(203),	175,	24,
  "gaz nefesi",		"!Gas Breath!",		"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"lightning breath","şimşek nefesi"},	{ 64,64,64,64,64,64,64,64,64,64,64,64,64  },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_lightning_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_lightning_breath,		SLOT(204),	150,	24,
  "şimşek nefesi",	"!Lightning Breath!",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"lightning stroke","çelik darbe"},		{ 69, 69, 69, 69,69,69,69,69,69,69,69,69,69 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(632),	200,	24,
  "çelik darbe",	"!lightning stroke!",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
   {"luck bonus","şans bonusu"},{ 67, 67, 67, 67,67,67,67,67,67,67,67,67,67 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT( 630),	 20,	12,
  "",		"Büyüye karşı zırhının azaldığını hissediyorsun.",	"", CABAL_NONE,
	RACE_TOP, ALIGN_NONE, GROUP_PROTECTIVE
    },

    {
	{"paralyzation","felç"},	{ 69, 69, 69, 69,69,69,69,69,69,69,69,69,69 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_IGNORE,	POS_FIGHTING,
	NULL,		SLOT(631),	200,	24,
  "felç",	"Yeniden hareket edebiliyorsun.",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"repulsion","repulsion"},		{ 65, 65, 65, 65,65,65,65,65,65,65,65,65,65 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(633),	200,	24,
	"repulsion",	"!repulsion!",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"sleep gas","uyku gazı"},	{ 67, 67, 67, 67,67,67,67,67,67,67,67,67,67 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_IGNORE,	POS_FIGHTING,
	NULL,		SLOT(628),	200,	24,
  "uyku gazı",	"Kendini kurumuş hissediyorsun.",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

    {
	{"slow gas","slow gas"},{ 69, 69, 69, 69,69,69,69,69,69,69,69,69,69 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(634),	200,	24,
  "slow gas",	"Eski hızını kazanıyorsun.",	"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_DRACONIAN
    },

/*
 * Skills for mobiles. (general purpose and high explosive from
 * Glop/Erkenbrand
 */
    {
	{"crush","baskı"},	{93,93,93,93,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_crush,            	SLOT( 0),       0,      18,
        "baskı",                 "!crush!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"general purpose","genel amaç"},       { 93, 93, 93, 93, 93, 93, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(401),      0,      12,
	"genel amaç", "!General Purpose Ammo!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"high explosive","yüksek patlayıcı"},       { 93, 93, 93, 93, 93, 93, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(402),      0,      12,
        "yüksek patlayıcı",  "!High Explosive Ammo!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"tail","kuyruk"},		{  53, 53,53, 53,53,53,53,53,53 ,53,53,53,53},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_tail,            	SLOT( 0),       0,      18,
        "kuyruk",                 "!Tail!",		"", CABAL_NONE ,
	RACE_TOP, ALIGN_NONE, GROUP_FIGHTMASTER
    },

/* combat and weapons skills */

    {
	{"arrow","ok"},		{ 93, 93,93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_IGNORE,	POS_STANDING,
	&gsn_arrow,	SLOT(0),	0,	0,
  "ok",	"!arrow!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"axe","balta"},		{  93, 93, 93,  1,93,1,93,1 ,93 ,93,93,93,93},
	{ 2, 2, 1, 1,1,1,1,1,1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_axe,            	SLOT( 0),       0,      0,
	"",                     "!Axe!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"bow","yay"},{ 93, 93,93, 44,93,93,93,35 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_IGNORE,	POS_STANDING,
	&gsn_bow,		SLOT(0),	0,	4,
  "yay",		"!bow!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"dagger","hançer"},             { 1,  1,  1,  1,1,1,1,1 ,1 ,1,1,1,1},
	{ 2, 2, 1, 1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dagger,            SLOT( 0),       0,      0,
	"",                     "!Dagger!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"flail","döven"},		{ 93,  1, 93,  1,93,1,93,93 ,93,93,93,93,1},
	{ 2, 2, 1, 1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_flail,            	SLOT( 0),       0,      0,
	"",                     "!Flail!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"lance","kargı"},	{ 33,33,33,33,33,33,33,33,33,33,33,33,33},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_IGNORE,	POS_STANDING,
	&gsn_lance,	SLOT(0),	0,	0,
	"lance",	"!lance!",	"",
	CABAL_KNIGHT , RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"mace","topuz"},			{  93,  1,  1,  1,1,1,93,93 ,93 ,1,93,93,93},
	{ 2, 2, 1, 1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_mace,            	SLOT( 0),       0,      0,
	"",                     "!Mace!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"polearm","teber"},		{ 93, 93, 93,  1,1,1,93,93,93 ,93,93,93,93},
	{ 2, 2, 1, 1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_polearm,           SLOT( 0),       0,      0,
	"",                     "!Polearm!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"shield block","kalkan bloğu"},		{ 93,17,7,1,1,1,12,10,93 ,10,93,93,93},
	{ 2, 1, 1, 1,1,1,1,1,1,1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_shield_block,	SLOT(0),	0,	0,
	"",			"!Shield!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DEFENSIVE
    },

    {
	{"spear","mızrak"},			{  93,  93,  1,  1,93,1,1,1 ,93,1,8,93,93 },
	{ 2, 2, 1, 1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_spear,            	SLOT( 0),       0,      12,
        "mızrak",                     "!Spear!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"sword","kılıç"},		{  93, 1,  1,  1,1,1,1,1,93 ,1,1,93,93},
	{ 2, 2, 1, 1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_sword,            	SLOT( 0),       0,      0,
	"",                     "!sword!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"whip","kırbaç"},		{  1, 1,  93,  1,1,1,93,1 ,1,93,1,1,1},
	{ 2, 2, 1, 1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_whip,            	SLOT( 0),       0,      0,
	"",                     "!Whip!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"second weapon","ikinci silah"},	{  93,93,93,23,48,48,93,23,93,24,93,93,93},
	{ 2, 2, 1, 1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_second_weapon,            	SLOT( 0),       0,      0,
	"",	"!second weapon!",	"",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"ambush","pusu"},         {  93, 93, 93, 93, 93, 93, 93, 17,93 ,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_CHAR_OFFENSIVE,          POS_STANDING,
        &gsn_ambush,                    SLOT(0),        0,       12,
        "pusu", "!Ambush!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"area attack","alan saldırısı"},      {93, 93, 93, 73,93,93,93,93,93,69,93,93,93},
	{1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_area_attack,     SLOT(0),        0,      0,
	"",         "!Area Attack!", 	"",	CABAL_NONE,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    { /* Ani ölüm */
	{"assassinate","suikast"},       {  93, 93, 93, 93, 93, 93, 65, 93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,          POS_STANDING,
        &gsn_assassinate,       SLOT(0),        0,       18,
        "suikast", "!assassinate!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"backstab","ardılan"},          {  93, 93,  11, 93,93,93,93,93 ,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_backstab,          SLOT( 0),        0,     12,
        "ardılan",             "!Backstab!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"bash","omuz"},		{  93,93,93,1, 6,1,1,10 ,93 ,93,11,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_bash,            	SLOT( 0),       0,      18,
        "omuz",                 "!Bash!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"bash door","kapıkırma"},	{  93, 93, 93,  1,6,1,1,93 ,93,93,11,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_bash_door,        	SLOT( 0),       0,      18,
        "kapıkırma",                 "!Bash Door!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"bearcall","ayı çağırma"},      {  93, 93, 93, 93, 93, 93, 93, 31,93,93,93,93,93 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,		TAR_IGNORE,          POS_FIGHTING,
        &gsn_bear_call,		SLOT(518),	50,       0,
        "", "Yeni ayılarla başa çıkabilirsin.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"berserk","vahşileşme"},		{ 93, 93, 93, 18,93,93,93,93 ,93,20,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_berserk,        	SLOT( 0),       0,      24,
        "",                     "Nabzının yavaşladığını hissediyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"blackguard","karagard"},{  93, 93, 93, 93, 93, 93, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_CHAR_OFFENSIVE,   POS_STANDING,
	&gsn_blackguard,		SLOT(0),    0,       12,
  "karagard",		"Karagardın yokoluyor.","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"blackjack","copla"},	{  93, 93, 30, 93, 93, 93, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_CHAR_OFFENSIVE,   POS_STANDING,
	&gsn_blackjack,		SLOT(0),    0,       8,
  "copla",		"Kafandaki ağrı geçiyor.","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"blind fighting","kör dövüşü"},	{   93, 93, 93, 13,32,93,11,43,93 ,23,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_blind_fighting,		SLOT(0),	0,	0,
	"",			"!blind fighting!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"blindness dust","körlük tozu"},       {  93, 93, 93, 93, 93, 93, 28, 93,93,93,93,93,93 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,   		TAR_IGNORE,             POS_FIGHTING,
        &gsn_blindness_dust,    SLOT(0),        20,       18,
	"",                     "!blindness dust!", "",	 CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"blink","zayi"},            {  1, 93,  93, 93,93,93,93,93 ,1,93,93,1,1},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_blink,             SLOT( 0),        0,     0,
	"",                     "!Blink!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"butcher","kasap"},       {  93, 93, 93, 93, 93, 93, 93, 13 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_IGNORE,          POS_STANDING,
        &gsn_butcher,             SLOT(0),        0,       0,
	"", "!butcher!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"caltraps","çivi"},        {  93, 93, 93, 93, 93, 93, 18, 93 ,93,36,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,          POS_FIGHTING,
        &gsn_caltraps,       SLOT(0),        0,       18,
        "çivi", "Bacaklarındaki acı sönüyor.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"camouflage","kamuflaj"},       {  93, 93, 93, 93, 93, 93, 93, 4 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_IGNORE,           POS_STANDING,
        &gsn_camouflage,         SLOT(0),        0,       24,
	"",         "!Camouflage!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"camouflage move","kamufle hareket"},	{ 93, 93, 93, 93, 93,93,93,23, 93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_move_camf,		SLOT(0),	0,	12,
	"",		"!move camouflaged!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"camp","kamp"},			{  93, 93, 93,  93,93,93,93,48 ,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_camp,            	SLOT( 0),       0,      24,
        "kamp",          "Yeni kamplar kurabilirsin.","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"circle","çevrel"},          {  93, 93, 7, 93, 93, 93, 93, 93,93 ,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_CHAR_OFFENSIVE,          POS_FIGHTING,
        &gsn_circle,             SLOT(0),        0,       18,
        "çevrel", "!Circle!", "",		CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"control animal","hayvan kontrolü"},	{ 93, 93, 93, 93,93,93,93,93,93,93,43,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_control_animal,	SLOT( 0),	 5,	12,
  "",			"Kendine güvenin artıyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_BEGUILING
    },

    {
	{"cleave","yarma"},         {  93, 93, 93, 93, 93, 55, 93, 93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,     TAR_CHAR_OFFENSIVE,     POS_STANDING,
        &gsn_cleave,         SLOT(0),        0,       24,
        "yarma",         "!Cleave!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"concentrate","konsantrasyon"},        { 93, 93,93,59,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_concentrate,        SLOT( 0),        0,     18,
        "",      "Yeni dövüşlere konsantre olabilirsin.",	"",CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"counter","kontra"},     {  93, 93, 93, 28, 35, 93, 93, 93 ,93,28,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_IGNORE,           POS_STANDING,
        &gsn_counter,         SLOT(0),        0,       0,
	"",         "!Counter!", "",		CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"critical strike","kritik vuruş"},	{   93, 93, 93, 93,93,93,58,93,93 ,62,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_critical,			SLOT(0),	0,	18,
	"",			"!critical strike!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"cross block","çapraz blok"},       {  93,93,93,31,56,56,93,31,93,32,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_cross_block,       SLOT( 0),        0,     0,
	"",                     "!cross block!",	"", CABAL_NONE ,
	RACE_NONE,ALIGN_NONE, GROUP_DEFENSIVE
    },

    {
	{"detect hidden","saklıyı saptama"},	{ 93, 93, 93, 93, 93, 93, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_detect_hidden,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		SLOT(44),	 5,	12,
  "",			"Saklıyı saptaman sönüyor.",
	"", CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"detect sneak","süzüleni saptama"},		{   93, 93, 93, 93,93,93,93,93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_CHAR_SELF,		POS_STANDING,
	&gsn_detect_sneak,		SLOT(0),	20,	18,
	"",			"!detect sneak!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DETECTION
    },

    {
	{"dirt kicking","toz atma"},	{ 93, 93,  3,  8,93,3,3,5,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_dirt,		SLOT( 0),	0,	12,
  "toz atma",		"Gözlerindeki toz dökülüyor.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"disarm","silahsızlandır"},              {  93, 93, 28, 20,20,21,23,21,93,21,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_disarm,            SLOT( 0),        0,     18,
	"",                     "!Disarm!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"dodge","sıyrılma"},           {  33,93,1,13, 21,21,1,9, 93,15,8,93,27},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dodge,             SLOT( 0),        0,     0,
	"",                     "!Dodge!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DEFENSIVE
    },

   {
	{"dual backstab","çift ardılan"},       {  93, 93, 38, 93, 93, 93, 93, 93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_CHAR_OFFENSIVE,           POS_STANDING,
        &gsn_dual_backstab,       SLOT(0),        0,       0,
        "çift ardılan",      "!dual backstab!", "",		CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"enchant sword","kılıç yücelt"},		{ 93, 93, 93, 93,93,93,93,93,93,42,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_OBJ_INV,		POS_STANDING,
	&gsn_enchant_sword,	SLOT(0),	100,	24,
	"",	"!Enchant sword!",	"", CABAL_NONE , RACE_NONE,
	ALIGN_NONE, GROUP_NONE
    },

    {
	{"endure","dayanım"},          {  93, 93, 93, 93, 93, 93, 20, 93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_CHAR_SELF,          POS_FIGHTING,
        &gsn_endure,             SLOT(0),        0,       24,
        "", "Büyüye karşı dayanımın azalıyor.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"enhanced damage","yüksek zarar"},      {  93, 33, 25,  3,6,3,16,15 ,93,8,14,93,93},
	{ 1,  2,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_enhanced_damage,   SLOT( 0),        0,     0,
	"",                     "!Enhanced Damage!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"entangle","sarmaşık"},        {  93,93,93,93,93,93,93,26 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_entangle,      TAR_CHAR_OFFENSIVE,   POS_FIGHTING,
        &gsn_entangle,       SLOT(568),        40,       12,
        "sarmaşık", "Sarmaşıklardan kurtulduğunu hissediyorsun.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"envenom","sem"},		{ 93, 93, 15, 93, 93, 93, 93, 93,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,	  	POS_RESTING,
	&gsn_envenom,		SLOT(0),	0,	12,
	"",			"!Envenom!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"escape","sıvış"},		{93,93,45,93,93,93,55,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_escape,		SLOT( 0),	0,	24,
	"",	"!escape!","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"explode","patlat"},      {  93, 93, 93, 93, 93, 93, 93, 93 ,93,46,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_CHAR_OFFENSIVE,          POS_FIGHTING,
        &gsn_explode,       SLOT(0),        100,       24,
        "patlat", "Gözlerindeki duman yokoluyor.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
{"ground strike","zemin vuruşu"},	{ 93,93,93,93, 93,93,93,52, 93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_ground_strike,	SLOT(0),	0,	18,
	"",			"!ground strike!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"hand block","el bloğu"},            { 93,93,93,93,93,93,22,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_hand_block,             SLOT( 0),        0,     0,
	"",                     "!hand block!",		"", CABAL_NONE ,
	RACE_NONE,ALIGN_NONE, GROUP_DEFENSIVE
    },

    {
	{"hand to hand","çıplak el"},		{ 45,  30, 18, 9,11,18,3,24,45,12,19,45,35 },
	{ 2,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_hand_to_hand,	SLOT( 0),	0,	0,
	"",			"!Hand to Hand!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"harakiri","harakiri"},	{ 93, 93, 93, 93,93,93,93,93,93,25,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null, 	TAR_IGNORE,	POS_FIGHTING,
	&gsn_hara_kiri,		SLOT(0),	50,	12,
  "",	"Yaşamaya yeniden başladığını hissediyorsun.", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"headguard","kafakoruma"},	{  93, 93, 93, 93, 93, 93, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_CHAR_OFFENSIVE,   POS_STANDING,
	&gsn_headguard,		SLOT(0),    0,       12,
  "kafakoruma",		"Kafakoruman yokoluyor.","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"herbs","bitki"},       {  93, 93, 93, 93, 93, 93, 93, 18 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_CHAR_DEFENSIVE,          POS_STANDING,
        &gsn_herbs,             SLOT(0),        0,       30,
        "", "Yine bitkilerle uğraşabilirsin.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"kick","tekme"},           {  93, 12, 14,  5,93,9,1,8 ,93,2,4,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_kick,              SLOT( 0),        0,     12,
        "tekme",                 "!Tekme!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"lash","kırbaçla"},			{ 48,48,93,93, 48,93,48,93, 48,93,48,48,48},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_lash,            	SLOT( 0),       0,      4,
        "kırbaç",                 "!Lash!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"light resistance","ışık direnci"},	{  93,  93, 93, 93,93,93,93,93,93,93,50,93,93},
	{ 1,  1,  2,  2, 1, 1, 1, 1,1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_light_res,		SLOT( 0),	0,	0,
	"",			"Light Resistance","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"lion call","aslan çağırma"},	{ 93, 93, 93, 93,93,93,93,55,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null, 	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	&gsn_lion_call,		SLOT(0),	50,	12,
	"",		"!lion call!", 		"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"make arrow","ok yapma"},		{ 93, 93, 93,93,93,93,93,37,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_make_arrow,	SLOT(0),	 50,	24,
	"",		"!make arrow!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"make bow","yay yapma"},		{ 93, 93, 93,93,93,93,93,36,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_make_bow,		SLOT(0),	 200,	24,
	"",		"!make bow!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"mental attack","zihin saldırısı"},{ 93, 93,93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_mental_attack,		SLOT(0),	200,	24,
	"",	"!mental attack!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"neckguard","boyunkoruma"},	{  93, 93, 93, 93, 93, 93, 93, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_CHAR_OFFENSIVE,   POS_STANDING,
	&gsn_neckguard,		SLOT(0),    0,       12,
  "boyun koruma",		"Boyun koruman yokoluyor.","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"nerve","sinir"},       {  93, 93, 93, 93, 93, 93, 29, 93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_CHAR_OFFENSIVE,          POS_FIGHTING,
        &gsn_nerve,             SLOT(0),        0,       18,
        "", "Sinirlerin iyileşiyor.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"parry","karşılama"},              { 93,25,19,1, 3,5,93,3, 23,1,22,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_parry,             SLOT( 0),        0,     0,
	"",                     "!Parry!",		"", CABAL_NONE ,
	RACE_NONE,ALIGN_NONE, GROUP_DEFENSIVE
    },

    {
	{"perception","algı"},  {  93, 93, 23, 93, 93,93,93,93,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_IGNORE,          POS_STANDING,
        &gsn_perception,             SLOT(0),        0,       0,
	"", "!perception!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"push","itme"},			{  93, 93, 53,  93,93,93,93,93 ,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_push,            	SLOT( 0),       0,      18,
        "itme",          "!push!","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"rescue","kurtar"},               {  93, 93, 93,  1,1,93,93,93,93,23,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_rescue,            SLOT( 0),        0,     12,
	"",                     "!Rescue!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_DEFENSIVE
    },

    {
	{"sense life","yaşam sezme"},{ 93, 93,93, 93,93,93,93,93 ,93,93,13,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_sense_life,	SLOT(623),	20,	12,
  "",	"Yaşam sezme gücünü yitiriyorsun.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"settraps","tuzak"},	{ 93, 93, 59, 93,93,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_settraps,		SLOT(0),	200,	24,
  "tuzak",	"Yeni tuzaklar kurabilirsin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"shield cleave","kalkan yarma"},{ 93, 93, 33, 26, 29, 29, 31, 35, 93, 26, 39, 93, 93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_shield_cleave,		SLOT(0),	200,	24,
	"",	"!shield cleave!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"smithing","demirci"},		{   93, 93, 93, 50,93,93,93,93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_smithing,			SLOT(0),	10,	18,
	"",			"!smithing!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"spell craft","büyü tanıma"},			{ 10,30,93,93, 93,93,93,93, 10,93,93,10,10 },
	{ 1,  1,  1,  1, 1, 1, 2, 2, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_spell_craft,	SLOT(0),    0,       0,
  "büyü tanıma",		"!spell craft!","", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"strangle","bayıltma"},          {  93,  93,  93,  93,93,93,26,93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_CHAR_OFFENSIVE,          POS_STANDING,
        &gsn_strangle,       SLOT(0),        0,       8,
        "bayıltma", "Boynunu yeniden hareket ettirebiliyorsun.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"swimming","yüzme"},	{   93, 93, 93, 93,93,93,93,93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_swimming,		SLOT(0),	0,	0,
	"",			"!swimming!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"target","hedef"},                 {  93, 93, 93,  93,93,93,93,93 ,93,40,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_target,              SLOT( 0),        0,     12,
        "hedef",                 "!Kick!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"throw","fırlatma"},        {  93, 93, 93, 93, 93, 93, 25, 93 ,93,37,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_CHAR_OFFENSIVE,       POS_FIGHTING,
        &gsn_throw,       SLOT(0),        0,       18,
        "fırlatma", "!throw!", "",	CABAL_NONE , RACE_NONE,ALIGN_NONE, GROUP_NONE
    },

    {
	{"tiger power","kaplan gücü"},		{ 93,93,93,93, 93,93,93,61, 93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null, 	TAR_IGNORE,	POS_FIGHTING,
	&gsn_tiger_power,	SLOT(0),	0,	12,
  "",	"Kaplanlarının kaçtığını hissediyorsun.", "", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"track","iz"},       {  93, 93, 93,93,93,93,93,39,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,       POS_STANDING,
        &gsn_track,       SLOT(0),        0,       12,
	"", "!track!", "",CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"trip","çelme"},		{  93, 93,  3, 25,93,10,8,93 ,93,20,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_trip,		SLOT( 0),	0,	12,
  "çelme",			"!Trip!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"vampire","vampir"},			{ 93, 93, 93, 93,93,93,93,93,93,93,10,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_IGNORE,		POS_STANDING,
	&gsn_vampire,		SLOT(0),	100,	12,
  "",	"Doğallaşıyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"vampiric bite","vampir ısırığı"},          {  93, 93,  93, 93,93,93,93,93 ,93,93,10,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_vampiric_bite,          SLOT( 0),        0,     12,
        "vampir ısırığı",             "!vampiric bite!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"vampiric touch","vampir teması"},      {  93,  93,  93,  93,93,93,93,93 ,93,93,15,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,      TAR_CHAR_OFFENSIVE,          POS_STANDING,
	&gsn_vampiric_touch,       SLOT(0),        0,       12,
  "vampir teması", "Kabuslardan uyanıyorsun.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"vanish","yokol"},        {  93, 93, 93, 93, 93, 93, 18, 93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_CHAR_SELF,          POS_FIGHTING,
        &gsn_vanish,       SLOT(521),        25,       18,
	"", "!vanish!", "", 	CABAL_NONE , RACE_NONE,ALIGN_NONE, GROUP_NONE
    },

    {
	{"warcry","nara"},       {  93, 93, 93,31,93,93,93,35,93 ,30,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,       POS_FIGHTING,
        &gsn_warcry,       SLOT(0),        30,       12,
        "", "Naranın etkisi yokoluyor.", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"weapon cleave","silah yarma"}, { 93, 93, 37, 29, 34, 34, 35, 38, 93, 29, 44, 93, 93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_weapon_cleave,	SLOT(0),	200,	24,
	"",	"!weapon cleave!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
       { "second attack", "ikili hamle"},        {  34, 28, 18,  10,13,14,15,13,25,10,12,34,34},
	{ 2,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_second_attack,     SLOT( 0),        0,     0,
	"",                     "!Second Attack!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"secondary attack","ikincil hamle"},	{93,93,93,52, 61,61,93,58 ,93,55,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_secondary_attack,	SLOT(0),	200,	24,
	"",	"!secondary attack!",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"third attack","üçlü hamle"},        { 93, 93,36,19,27,26,26,45,93,20,32,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_third_attack,      SLOT( 0),        0,     0,
	"",                     "!Third Attack!",	"", CABAL_NONE ,
	RACE_NONE,ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"fourth attack","dörtlü hamle"},       {93,93,93,30, 93,93,48,93, 93,93,93,93,93},
	{1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_fourth_attack,     SLOT(0),        0,      0,
	"",                     "!Fourth Attack!", 	"",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"fifth attack","beşli hamle"},         {93, 93, 93, 52, 93, 93, 93,93,93,93,93,93,93},
	{1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_fifth_attack,     SLOT(0),        0,      0,
	"",          "!Fifth Attack!", 	"",	CABAL_NONE,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

/* non-combat skills */

    {
	{"blue arrow","mavi ok"},			{ 93, 93, 93,93,93,93,93,51,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_blue_arrow,	SLOT(0),	 50,	12,
	"",		"!blue arrow!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"fast healing","hızlı iyileşme"},	{  15, 9, 16,  6,7,7,9,6 ,15 ,3,9,9,9},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_fast_healing,	SLOT( 0),	0,	0,
	"",			"!Fast Healing!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MEDITATION
    },

    {
	{"green arrow","yeşil ok"},			{ 93, 93, 93,93,93,93,93,40,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_green_arrow,	SLOT(0),	 50,	12,
	"",		"!green arrow!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"grip","kavrama"},            { 93, 93, 93,43,93,93,93,93,93,50,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_grip,            SLOT( 0),        0,     18,
	"",                     "!Grip!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"haggle","pazarlık"},		{  7, 18,  1, 14,12,8,17,15 ,7 ,19,5,5,5},
	{ 2,  2,  1,  2, 2, 2, 1, 2, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_haggle,		SLOT( 0),	0,	0,
	"",			"!Haggle!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"hide","saklan"},		{  93, 93,  1,  93,93,93,10,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hide,		SLOT( 0),	 0,	18,
	"",			"!Hide!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"katana","katana"},	{ 93, 93, 93, 93,93,93,93,93,93,72,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_OBJ_INV,		POS_STANDING,
	&gsn_katana,		SLOT(0),	100,	24,
  "",		"Yeni bir katana yapabilirsin.",	"",
	CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"lay hands","şifalıeller"},		{93,93,93,93,63,93,93,93,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_lay_hands,		SLOT( 0),	0,	12,
  "",	"Yeniden masum iyileştirebilirsin.",	"",CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"lore","irfan"},		{  93,93,13,21,19,20,19,18,93,22,20,93,93},
	{ 2,  2,  2,  2, 2, 2, 2, 2, 2, 2,2,2,2},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_lore,		SLOT( 0),	0,	36,
	"",			"!Lore!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"mastering pound","vuruş ustalığı"},	{ 93,93,93,93,93,93,61,93,93,93,93,93,93},
	{ 2,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_master_hand,	SLOT( 0),	0,	0,
	"",			"!Master Hand!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_FIGHTMASTER
    },

    {
	{"mastering sword","kılıç ustalığı"},	{ 93,93,93,93,93,93,93,93,93,61,93,93,93},
	{ 2, 2, 1, 1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_master_sword,           SLOT( 0),       0,      0,
	"",                     "!master sword!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WEAPONSMASTER
    },

    {
	{"meditation","meditasyon"},		{  6,  6, 15, 15,14,15,14,16,6,16,6,6,6},
	{ 1,  1,  2,  2, 1, 1, 1, 1,1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_meditation,	SLOT( 0),	0,	0,
	"",			"Meditation",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_MEDITATION
    },

    {
	{"peek","dikiz"},			{  93, 93,  1, 93,93,93,93,93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_peek,		SLOT( 0),	 0,	 0,
	"",			"!Peek!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"pick lock","maymuncuk"},		{ 93, 93,  17, 93,93,93,34,93,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pick_lock,		SLOT( 0),	 0,	12,
	"",			"!Pick!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"poison smoke","zehir bulutu"},      {  93, 93, 93, 93, 93, 93, 30, 93,93,93,93,93,93 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,     	TAR_IGNORE,            POS_FIGHTING,
        &gsn_poison_smoke,      SLOT(0),        20, 	18,
	"",                     "!poison smoke!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"quiet movement","sessiz yürüyüş"},      {  93, 93, 93, 93, 93, 93, 93, 15 ,93,93,10,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_IGNORE,          POS_STANDING,
        &gsn_quiet_movement,             SLOT(0),        0,       0,
	"", "!quiet movement!", "",	CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"recall","anımsa"},{  1,  1,  1,  1,1,1,1,1 ,1,1,1,1,1},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_recall,		SLOT( 0),	0,	24,
	"",			"!Recall!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"red arrow","kızıl ok"},		{ 93, 93, 93,93,93,93,93,44,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_red_arrow,	SLOT(0),	 50,	12,
	"",		"!red arrow!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"sneak","süzül"},	{  93, 93, 4, 93, 93, 93, 18, 93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,		SLOT( 0),	 0,	12,
  "",			"Artık süzülmüyorsun.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"steal","arakla"},			{  93, 93, 12, 93, 93, 93, 93, 93,93 ,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_steal,		SLOT( 0),	 0,	12,
	"",			"!Steal!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"scrolls","parşömen"},		{  1,  1,  1,  93,93,1,93,1 , 1,6,1,1,1},
	{ 1,  1,  2,  2, 2, 2, 2, 2, 2, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_scrolls,		SLOT( 0),	0,	12,
	"",			"!Scrolls!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WIZARD
    },

    {
	{"staves","tılsımlar"},		{  1,  1,  93,  93,93,93,1,1 ,1,1,1,1,1},
	{ 1,  1,  2,  2, 2, 2, 2, 2, 2, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_staves,		SLOT( 0),	0,	12,
	"",			"!Staves!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WIZARD
    },

    {
	{"tame","uysallaştır"},        {  93, 93, 93, 93, 93, 93, 93, 39 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,        TAR_CHAR_DEFENSIVE,          POS_FIGHTING,
        &gsn_tame,             SLOT(0),        0,       24,
	"", "!tame!", "",CABAL_NONE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"trance","trans"},       {  14,15,93,93,93,93,93,93,14 ,93,21,21,21},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,   POS_SLEEPING,
        &gsn_trance,       SLOT(0),        0,       0,
	"", "", "",	CABAL_NONE , RACE_NONE,ALIGN_NONE, GROUP_MEDITATION
    },

    {
	{"white arrow","beyaz ok"},			{ 93, 93, 93,93,93,93,93,49,93,93,93,93,93 },
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_white_arrow,	SLOT(0),	 50,	12,
	"",		"!white arrow!",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"wands","değnekler"},	{  1,  1,  93,  93,93,1,93,93 ,1,5,1,1,1},
	{ 1,  1,  2,  2, 2, 2, 2, 2, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_wands,		SLOT( 0),	0,	12,
	"",			"!Wands!",		"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_WIZARD
    },

/* cabals */

    {
	{"mortal strike","ölümcül atak"},	{ 63,63,63,63,63,63,63,63,63,63,63,63,63},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_mortal_strike,		SLOT(0),	200,	24,
  "ölümcül atak",	"!mortal strike!",	"",
	CABAL_BATTLE , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"disgrace","gözden düşür"},{  33, 33, 33, 33, 33,33,33,33,33,33,33,33,33},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_disgrace,	TAR_CHAR_OFFENSIVE,		POS_STANDING,
	NULL,		SLOT(619),	200,	12,
  "",	"Onurunu kazandığını hissediyorsun.",	"",
	CABAL_CHAOS , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"evil spirit","kem ruh"},	{  33, 33, 33, 33, 33,33,33,33,33,33,33,33,33},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_evil_spirit,	TAR_IGNORE,		POS_STANDING,
	&gsn_evil_spirit,		SLOT(618),	800,	36,
  "kem ruh",	"Vücudunla ruhun yeniden bir oluyor.",	"",
	CABAL_INVADER , RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"ruler aura","tüze aurası"},     {  33, 33, 33, 33, 33,33,33,33,33,33,33,33,33},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_ruler_aura,    TAR_CHAR_SELF,          POS_STANDING,
        NULL,             SLOT(667),        20,       12,
        "", "Tüze auran yokoluyor.", 	"",	CABAL_RULER ,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"sword of justice","adalet kılıcı"},	{48,48,48,48, 48,48,48,48,48,48,48,48,48},
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_sword_of_justice,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(686),	50,	12,
  "adalet kılıcı",	"!sword of justice!", "", CABAL_RULER ,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"bandage","sargı"},       {23, 23, 23, 23,23,23,23,23,23,23,23,23,23},
	{1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_bandage,     SLOT(0),        0,      0,
	"",        "Daha az sağlıklı hissediyorsun.", 	"",	CABAL_BATTLE,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"cabal recall","kabal anımsa"},	{ 10,10,10,10,10,10,10,10,10,10,10,10,10},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_cabal_recall,		SLOT( 0),	0,	24,
  "",	"Anımsama için yeniden dua edebilirsin.","", CABAL_BATTLE ,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {"wanted","aranıyor"},                {  1,  1,  1,  1, 1, 1, 1, 1 ,1,1,1,1,1},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_DEAD,
        &gsn_wanted,                   SLOT(0),        0,       0,
        "",                     "!Wanted!", "",		CABAL_RULER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {"judge","yargılama"},               {  1,  1,  1,  1, 1, 1, 1, 1 ,1,1,1,1,1},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,             TAR_IGNORE,             POS_DEAD,
        &gsn_judge,              SLOT(0),        0,       0,
        "",                     "!Judge!", "",		CABAL_RULER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {"bloodthirst","kana susama"},          {  15, 15, 15, 15, 15, 15, 15, 15 ,15,15,15,15,15},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_CHAR_SELF,       POS_FIGHTING,
        &gsn_bloodthirst,       SLOT(0),        0,       12,
        "", "Artık kana susamıyorsun.", "",	CABAL_BATTLE,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {"spellbane","büyüyıkımı"},       { 10,10,10,10,10,10,10,10,10,10,10,10,10 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_CHAR_SELF,       POS_STANDING,
        &gsn_spellbane,       SLOT(0),        0,       12,
        "büyüyıkımı", "Büyüye karşı direncin azalıyor.", "",	CABAL_BATTLE,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {"resistance","direnç"},      {  12, 12, 12,12,12,12,12,12 ,12,12,12,12,12},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_CHAR_SELF,       POS_FIGHTING,
        &gsn_resistance,       SLOT(0),        0,       24,
        "", "Sertliğin yokoluyor.", "",	CABAL_BATTLE,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
       { "deathblow","ölüm rüzgarı"},          {  20,20,20,20,20,20,20,20 ,20,20,20,20,20},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,       POS_STANDING,
        &gsn_deathblow,       SLOT(0),        0,       0,
        "", "!deathblow!", "",	CABAL_BATTLE,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {"transform", "dönüşüm"},          {  10, 10, 10,10,10,10,10,10 ,10,10,10,10,10},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_transform,      TAR_CHAR_SELF,       POS_STANDING,
        NULL,       SLOT(522),        100,       24,
        "", "Sağlığın azalıyor.", "",	CABAL_SHALAFI,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"mental knife","zihin bıçağı"},        {  15, 15, 15,93,15,15,15,15,15,15,15,15,15 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_mental_knife,      TAR_CHAR_OFFENSIVE,       POS_FIGHTING,
        &gsn_mental_knife,       SLOT(524),        35,       12,
        "zihin bıçağı", "Zihnindeki acı yokoluyor.", "",CABAL_SHALAFI,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"demon summon","iblis çağrısı"},         {  25, 25, 25,93,25,25,25,25 ,25,25,25,25,25},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_demon_summon,      TAR_CHAR_SELF,       POS_FIGHTING,
        &gsn_demon_summon,       SLOT(525),        100,       12,
        "", "Çağrı gücünün geri geldiğini hissediyorsun.", "", CABAL_SHALAFI,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"scourge","ceza kırbacı"},     {  30, 30, 30,93,30,30,30,30,30,30,30,30,30 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_scourge,      TAR_IGNORE,       POS_FIGHTING,
        NULL,       SLOT(526),        50,       18,
        "ceza kırbacı", "!scourge!", "",CABAL_SHALAFI,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {"manacles","kelepçe"},        {  20, 20, 20,20,20,20,20,20 ,20,20,20,20,20},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_manacles,      TAR_CHAR_OFFENSIVE,       POS_FIGHTING,
        NULL,       SLOT(528),        75,       12,
        "", "Zincirlerin yokoluyor.", "", 	CABAL_RULER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
      {"shield of ruler","tüze kalkanı"},        {  10, 10, 10,10,10,10,10,10 ,10,10,10,10,10},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_shield_ruler,      TAR_IGNORE,       POS_FIGHTING,
        NULL,       SLOT(529),        100,       12,
        "", "!shield!", "",	CABAL_RULER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
      {"guard","savun"},    {  22,22,22,22,22,22,22,22 ,22,22,22,22,22},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,       POS_STANDING,
        &gsn_guard,       SLOT(0),        0,       12,
        "", "", "",	CABAL_KNIGHT,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
      {"guard call","muhafız çağrısı"},          {  15, 15, 15,15,15,15,15,15 ,15,15,15,15,15},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_guard_call,      TAR_IGNORE,       POS_FIGHTING,
	&gsn_guard_call,       SLOT(530),        75,       12,
  "", 	"Yeniden muhafız çağırabilirsin.", 	"",	CABAL_RULER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
          {"nightwalker","solucan"},        {  25, 25, 25,25,25,25,25,25 ,25,25,25,25,25},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_nightwalker,      TAR_IGNORE,       POS_FIGHTING,
	&gsn_nightwalker,       SLOT(531),        75,       12,
  "", "Çağrı gücünün geri geldiğini hissediyorsun.", "",CABAL_INVADER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
          {"eyes of intrigue","entrika"},          {  13, 13, 13,13,13,13,13,13,13,13,13,13,13 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_eyes,      TAR_IGNORE,       POS_FIGHTING,
	NULL,       SLOT(532),        75,       12,
        "", "!eyes of intrigue!", "",	CABAL_INVADER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {"fade", "soluş"},       {  20, 20, 20,20,20,20,20,20 ,20,20,20,20,20},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,       POS_STANDING,
        &gsn_fade,       SLOT(0),        0,       24,
        "", "!fade!", "",	CABAL_INVADER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
       { "shadow cloak","gölge pelerini"},        {  10, 10, 10,10,10,10,10,10,10,10,10,10,10 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_shadow_cloak,      TAR_CHAR_DEFENSIVE,       POS_STANDING,
	NULL,       SLOT(533),        10,       12,
  "", "Gölgeler seni korumuyor.", "",	CABAL_INVADER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
      {"nightfall", "alacakaranlık"},     {  16, 16, 16,16,16,16,16,16,16,16,16,16,16 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_nightfall,      TAR_IGNORE,       POS_STANDING,
	NULL,       	SLOT(534),        50,       12,
  "", "Işıkları kontrol edemiyorsun.", "",	CABAL_INVADER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {"aura of chaos","kaos aurası"},    {  16, 16, 16,16,16,16,16,16,16,16,16,16,16 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_aura_of_chaos,      TAR_CHAR_DEFENSIVE,       POS_STANDING,
	NULL,       	SLOT(720),        20,       12,
  "", "Karmaşa tanrısı artık seni korumuyor.", 	"",
	CABAL_CHAOS, RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
       { "garble","afoni"},        {  10, 10, 10,10,10,10,10,10 ,10,10,10,10,10},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_garble,      TAR_CHAR_DEFENSIVE,       POS_FIGHTING,
	&gsn_garble,       SLOT(535),        30,       12,
  "", "Dilin normale dönüyor.", "",	CABAL_CHAOS,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
         { "mirror","ayna"},         { 15, 15, 15,15,15,15,15,15,15,15,15,15,15 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_mirror,      TAR_CHAR_DEFENSIVE,       POS_STANDING,
	&gsn_mirror,       SLOT(536),        40,       12,
  "", "Yokolma hissini yaşıyorsun.", "",	CABAL_CHAOS,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"confuse","şaşırtma"},        {  20, 20, 20,20,20,20,20,20 ,20,20,20,20,20},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_confuse,      TAR_CHAR_OFFENSIVE,       POS_FIGHTING,
	&gsn_confuse,       SLOT(537),        20,       12,
  "", "Şaşkınlığın azalıyor.", "",	CABAL_CHAOS,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"doppelganger","dönüşüm"},        {  25, 25, 25,25,25,25,25,25 ,25,25,25,25,25},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_doppelganger,      TAR_CHAR_DEFENSIVE,       POS_STANDING,
	&gsn_doppelganger,       SLOT(527),        75,       12,
  "", "Doğal formuna döndün.", "",	CABAL_CHAOS,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
       {   "chaos blade","yatağan"},      {  23, 23, 23,23,23,23,23,23,23,23,23,23,23 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_chaos_blade,      TAR_IGNORE,       POS_STANDING,
        &gsn_chaos_blade,       SLOT(550),        60,       12,
        "", "!chaos blade!", "",	CABAL_CHAOS,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

   {
          {    "stalker","avcı"},    {  93, 93, 93,93,93,93,93,93 ,93,93,93,93,93},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_stalker,      TAR_IGNORE,       POS_STANDING,
        &gsn_stalker,       SLOT(554),        100,       12,
        "", "Yeni bir avcı çağırabilirsin.", "",
	CABAL_RULER, 	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
           {   "random","rasgele"},          {  28, 28, 28,28,28,28,28,28 ,28,28,28,28,28},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_randomizer,      TAR_IGNORE,       POS_STANDING,
        &gsn_randomizer,       SLOT(555),        200,       24,
        "", "Yeniden rasgeleyi kullanabilirsin.", "", CABAL_CHAOS,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"tesseract","öte boyut"},          {  29, 29, 29,93,29,29,29,29 ,29,29,29,29,29},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_tesseract,      TAR_IGNORE,       POS_STANDING,
        &gsn_tesseract,       SLOT(556),        150,       12,
        "", "!tesseract!", "",	CABAL_SHALAFI,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
       {   "trophy","andaç"},       {  23, 23, 23,23,23,23,23,23 ,23,23,23,23,23},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,       POS_STANDING,
        &gsn_trophy,       SLOT(0),        30,       12,
        "", "Yeni bir andaçın hayalini kuruyorsun.", "",	CABAL_BATTLE,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
         {   "truesight","keskingörüş"},         {  25, 25, 25,25,25,25,25,25 ,25,25,25,25,25},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,       POS_STANDING,
        &gsn_truesight,       SLOT(0),        50,       12,
        "", "Gözlerin keskinliğini yitiriyor.", "",	CABAL_BATTLE,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {  "brew","mayacı"},     {  23, 23, 23,23,23,23,23,23,23 ,23,23,23,23},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_brew,      TAR_OBJ_INV,       POS_STANDING,
        &gsn_brew,       SLOT(557),        25,       12,
        "", "Yeniden mayalamaya başlayabilirsin.", "",
	CABAL_SHALAFI,	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {  "shadowlife","gölgeyaşam"},         {  30, 30, 30,30,30,30,30,30 ,30,30,30,30,30},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_shadowlife,      TAR_CHAR_OFFENSIVE,       POS_STANDING,
        &gsn_shadowlife,       SLOT(558),        80,       12,
        "", "Kendini daha gölgeli hisssediyorsun.", "",	CABAL_INVADER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
         {   "ruler badge", "tüze nişanı"},        {  25, 25, 25,25,25,25,25,25 ,25,25,25,25,25},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_ruler_badge,      TAR_CHAR_SELF,   POS_STANDING,
        &gsn_ruler_badge,       SLOT(560),        50,       12,
        "", "!tüze nişanı!", "",	CABAL_RULER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
         {   "remove badge","nişan kaldır"},        {  28, 28, 28,28,28,28,28,28,28,28,28,28,28 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_remove_badge,      TAR_CHAR_SELF,   POS_STANDING,
        &gsn_remove_badge,       SLOT(561),        100,       12,
        "", "!remove badge!", "",	CABAL_RULER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
      {  "golden aura","altın ayla"},         {  10,10,10,10,10,10,10,10,10,10,10,10,10 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_golden_aura,      TAR_IGNORE,   POS_STANDING,
        NULL,       SLOT(564),        25,       12,
        "", "Altın aylanın yokolduğunu hissediyorsun.", "",	CABAL_KNIGHT,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
        {  "dragonplate", "ejderha zırhı"},       {  15,15,15,15,15,15,15,15 ,15,15,15,15,15},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_dragonplate,      TAR_IGNORE,   POS_STANDING,
        NULL,       SLOT(565),        60,       12,
        "", "", "",	CABAL_KNIGHT,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
     {   "squire","bey"},       {  25, 25, 25,25,25,25,25,25 ,25,25,25,25,25},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_squire,      TAR_IGNORE,   POS_STANDING,
        &gsn_squire,       SLOT(566),        100,       12,
        "", "Yeni bir beyi düşünmeye başlıyorsun.", "",	CABAL_KNIGHT,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
     {   "dragonsword", "ejderha kılıcı"},          {  25,25,25,25,25,25,25,25 ,25,25,25,25,25},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_dragonsword,      TAR_IGNORE,   POS_STANDING,
        &gsn_dragonsword,       SLOT(567),        70,       12,
	"", "", "",	CABAL_KNIGHT,	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
         {   "holy armor", "kutsal zırh"},         {  10,10,10,10,10,10,10,10,10 ,10,10,10,10},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_holy_armor,      TAR_CHAR_SELF,   POS_RESTING,
        &gsn_knight,       SLOT(569),        20,       12,
        "", "Zarara karşı daha az korumalısın.", "",	CABAL_KNIGHT,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },


    {
      {  "disperse","dağıtma"},       {  20,20,20,20,20,20,20,20,20,20,20,20,20 },
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_disperse,      TAR_IGNORE,   	POS_FIGHTING,
        &gsn_disperse,       SLOT(573),        100,       24,
        "", "Yeniden dağıtabilirsin.", "",CABAL_CHAOS,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"hunt","avlama"},        {  13, 13, 13,13,13,13,13,13 ,13,13,13,13,13},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,       POS_STANDING,
        &gsn_hunt,       SLOT(0),        0,       6,
	"", 	"!hunt!", 	"",	CABAL_HUNTER,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"find object","nesne ara"},	{   25, 25, 25, 25,25,25,25,25,25 ,25,25,25,25},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_find_object,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(585),	20,	18,
	"",			"!Find Object!",	"", CABAL_HUNTER ,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"path find","yol bulma"},		{  13,13 ,13,13,13,13,13,13 ,13 ,13,13,13,13},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_path_find,		SLOT( 0),	0,	0,
	"",			"!endur!",	"", CABAL_HUNTER ,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"riding","binicilik"},		{   30, 30, 30, 30,30,30,30,30,30 ,30,30,30,30},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_riding,			SLOT(0),	0,	6,
	"",			"!riding!",	"", CABAL_KNIGHT ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"wolf","kurt"},		{   20, 20,20, 20,20,20,20,20,20 ,20,20,20,20},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_wolf,	TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(593),	100,	12,
  "",	"Daha çok kurdu kontrol edebileceğini hissediyorsun.",	"", CABAL_HUNTER ,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"wolf spirit","kurt ruhu"},	{   28, 28, 28, 28,28,28,28,28,28 ,28,28,28,28},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_wolf_spirit,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(685),	50,	12,
  "",	"Damarlarındaki kan tekrar eskisi gibi akmaya başladı.",	"",
	CABAL_HUNTER , RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"armor use","zırh kullanımı"},		{ 17,17,17,17,17,17,17,17,17 ,17,17,17,17},
	{ 2, 1, 1, 1,1,1,1,1,1,1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_armor_use,	SLOT(0),	0,	0,
	"",			"!Armor Use!",		"", CABAL_HUNTER ,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"world find","diyar bulma"},	{  53,53 ,53,53,53,53,53,53 ,53 ,53,53,53,53},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_world_find,	SLOT( 0),	0,	0,
	"",			"!world find!",	"", CABAL_HUNTER ,
	RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"take revenge","intikam"},{  53,53 ,53,53,53,53,53,53 ,53 ,53,53,53,53},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_take_revenge,	TAR_IGNORE,		POS_STANDING,
	NULL,		SLOT(624),	50,		12,
	"",	"!take revenge!",	"",
	CABAL_HUNTER , RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
	{"mastering spell","büyü ustalığı"},	{37,37,37,37,37,37,37,37,37,37,37,37,37},
	{ 1,  1,  1,  1, 1, 1, 2, 2, 1, 1,1,1,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_mastering_spell,	SLOT(0),    0,       0,
  "büyü ustalığı",		"!mastering spell!","",
	CABAL_SHALAFI, RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
       { "guard dogs","muhafız köpekler"},        {  15, 15, 15,15,15,15,15,15 ,15,15,15,15,15},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_guard_dogs,      TAR_IGNORE,       POS_FIGHTING,
	NULL,       SLOT(687),        100,       12,
        "", 	"", 	"",
	CABAL_LIONS, RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
  {  "eyes of tiger","kaplan gözleri"},        {  25, 25, 25,25,25,25,25,25 ,25,25,25,25,25},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_eyes_of_tiger,      TAR_IGNORE,       POS_FIGHTING,
	NULL,       SLOT(688),        20,       12,
        "", 	"", 	"",
	CABAL_LIONS, RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
    {  "lion shield","aslan kalkanı"},       {  24, 24, 24,24,24,24,24,24 ,24,24,24,24,24},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_lion_shield,      TAR_IGNORE,       POS_FIGHTING,
	NULL,       SLOT(689),        200,       12,
        "", 	"", 	"",
	CABAL_LIONS, RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
      {  "evolve lion","aslana dönüşme"},        {  18, 18, 18,18,18,18,18,18 ,18,18,18,18,18},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_evolve_lion,      TAR_IGNORE,       POS_FIGHTING,
	NULL,       SLOT(690),        50,       12,
        "", 	"", 	"",
	CABAL_LIONS, RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
{   "claw","pençe"},      {  20, 20, 20,20,20,20,20,20 ,20,20,20,20,20},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_null,      TAR_IGNORE,       POS_FIGHTING,
	&gsn_claw,       SLOT(0),        50,       24,
  "pençe", 	"", 	"",
	CABAL_LIONS, RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

    {
  {  "prevent","intikam bağı"},   {  35, 35, 35,35,35,35,35,35 ,35,35,35,35,35},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
        spell_prevent,      TAR_IGNORE,       POS_FIGHTING,
	NULL,       SLOT(691),        75,       12,
        "", 	"", 	"",
	CABAL_LIONS, RACE_NONE, ALIGN_NONE, GROUP_CABAL
    },

/* object spells */

    {
	{"terangreal","uyuşturma"},		{ 93,  93, 93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_terangreal,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_terangreal,	SLOT(607),	 5,	12,
  "uyuşturma",		"Artık uyanıksın.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"kassandra","kasandra"},	{ 93,  93, 93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_kassandra,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_kassandra,	SLOT(608),	 5,	12,
  "",		"Kendini yine iyileştirebilirsin.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"sebat","sebat"},		{ 93,  93, 93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_sebat,		TAR_CHAR_SELF,		POS_STANDING,
	&gsn_sebat,		SLOT(609),	 5,	12,
  "",			"Kendini yine koruyabilirsin.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    },

    {
	{"matandra","matandra"},	{ 93,  93, 93, 93,93,93,93,93 ,93,93,93,93,93},
	{ 1,  1,  1,  1, 1, 1, 1, 1, 1, 1,1,1,1},
	spell_matandra,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_matandra,		SLOT(610),	 5,	12,
  "matandra",	"Kasandrayı tekrar kullanabilirsin.",	"", CABAL_NONE ,
	RACE_NONE, ALIGN_NONE, GROUP_NONE
    }

};
