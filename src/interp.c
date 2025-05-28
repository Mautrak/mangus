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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "turkish_morphology.h"

#undef IMMORTALS_LOGS

bool	check_social	args( ( CHAR_DATA *ch, char *command,char *argument ) );


/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2



/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;

#ifdef IMMORTALS_LOGS
/*
 * immortals log file
 */
FILE				*imm_log;
#endif

/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
  { "aşağı",		do_down,	POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "batı",		do_west,	POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "doğu",		do_east,	POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "güney",		do_south,	POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "kuzey",		do_north,	POS_STANDING,    0,  LOG_NEVER, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "yukarı",		do_up,		POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { ",",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, 0, CMD_GHOST },
  { ";",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, 0, CMD_GHOST },
  { ":",		do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "'",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, 0, CMD_GHOST },
  { "aç",		do_open,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "acıbana",		do_outfit,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST},
  { "advance",	do_advance,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "affrooms",	do_affrooms,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "al",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "allow",		do_allow,	POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "andaç",         do_trophy,      POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "anımsa",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "arakla",		do_steal,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
  { "aranıyor",         do_wanted,      POS_STANDING,    0,  LOG_ALWAYS, 1,0 },
  { "ardılan",	do_backstab,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
  { "aslançağır",       do_lion_call,   POS_FIGHTING,    0,  LOG_NORMAL, 1,0},
  { "at",             do_at,          POS_DEAD,       L6,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST  },
  { "avla",           do_hunt,        POS_STANDING,    0,  LOG_NORMAL,1,0 },
  { "ayı",       do_evolve_bear,   POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "ayıçağır",       do_bear_call,   POS_FIGHTING,    0,  LOG_NORMAL, 1,0},
  { "ayrı",		do_qui,		POS_DEAD,	 0,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "ayrıl",		do_quit,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "ayril",	do_quit,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "bak",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "bakiye",	do_balance,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
  { "ban",		do_ban,		POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "bayılt",       do_strangle,    POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "bileşik",	do_combine,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "biniş",          do_mount,       POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "bitki",          do_herbs,       POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "bırak",		do_drop,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "bölge",		do_areas,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "bug",		do_bug,		POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "büyü",		do_cast,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0},
  { "büyüler",		do_spells,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "büyüyıkımı",      do_spellbane,   POS_FIGHTING,    0,  LOG_NORMAL, 0,0 },
  { "cabal_scan",	do_cabal_scan,	POS_STANDING,    0,  LOG_NEVER, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "cevapla",		do_kdcevapla,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "çağrıyok",	do_nosummon,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "çarp",		do_zap,		POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "çelme",		do_trip,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "çevrel",         do_circle,      POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "çivi",       do_caltraps,    POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "çıkar",		do_remove,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "çıkışlar",		do_exits,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "clear",		do_clear,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "clone",		do_clone,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "cls",		do_clear,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "çöp",           do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL, 0,0 },
  { "copla",	do_blackjack,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
  { "count",		do_count,	POS_SLEEPING,	HE,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "dağıt",		do_split,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "dayanım",         do_endure,      POS_STANDING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "değer",		do_value,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "deny",		do_deny,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "dilek",	do_dilek,       POS_RESTING,     0,  LOG_NORMAL, 1 , CMD_KEEP_HIDE | CMD_GHOST},
  { "dinlen",		do_rest,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "direnç",     do_resistance,  POS_FIGHTING,    0,  LOG_NORMAL, 0,0 },
  { "diril",     do_diril,  POS_RESTING,    0,  LOG_NORMAL, 1,CMD_GHOST},
  { "disconnect",	do_disconnect,	POS_DEAD,	L3,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "discord",	do_discord,	POS_DEAD,	0,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "doğra",        do_butcher,     POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "dök",		do_pour,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_KEEP_HIDE },
  { "dokun",		do_vtouch,	POS_STANDING,	 0,  LOG_NORMAL, 0,0 },
  { "doldur",		do_fill,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "dönüş",          do_human,       POS_STANDING,    0,  LOG_NORMAL,1,0 },
  { "dua",           do_pray,        POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "dump",		do_dump,	POS_DEAD,	ML,  LOG_ALWAYS, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "duygu",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "duyuru",		do_duyuru,	POS_DEAD,	IM,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "eğitim",		do_train,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "ekipman",	do_equipment,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "emret",		do_order,	POS_RESTING,	 0,  LOG_NORMAL, 1,0},
  { "envanter",	do_inventory,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "eniyi",	do_eniyi,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "exlist",		do_exlist,	POS_DEAD,	L1,  LOG_ALWAYS, 1,CMD_KEEP_HIDE|CMD_GHOST },
  { "ücret",	do_estimate,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "etkiler",	do_affects_col,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "familya",	do_familya,	POS_DEAD,	 0,  LOG_NORMAL, 1,CMD_KEEP_HIDE|CMD_GHOST},
  { "find",		do_find,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "fırlat",          do_throw,       POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "flag",		do_flag,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "force",		do_force,	POS_DEAD,	L6,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "freeze",		do_freeze,	POS_DEAD,	L7,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "gir", 		do_enter, 	POS_STANDING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "giy",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "goto",           do_goto,        POS_DEAD,       L8,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "göm",		do_bury,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
  { "görev",          do_quest,       POS_RESTING,     0,  LOG_NORMAL, 1 , CMD_GHOST},
  { "görün",	do_visible,	POS_SLEEPING,	 0,  LOG_NORMAL, 1,0 },
  { "görünüm",	do_description,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "göster",		do_show,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "grant",		do_grant,	POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "grup",          do_group,       POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "grupliste",          do_glist,       POS_DEAD,        0,  LOG_NEVER,  1, 0},
  { "ganlat",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "günce",		do_worth,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "harakiri",           do_hara,        POS_STANDING,    0,  LOG_NORMAL,1,0 },
  { "havadurumu",	do_weather,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "haykır",		do_yell,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "hedef",		do_target,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "holylight",	do_holylight,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "iç",		do_drink,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "ikikat",	do_ikikat,	POS_DEAD,	L6,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "immtalk",	do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "incele",	do_examine,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "incognito",	do_incognito,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "induct",		do_induct,	POS_DEAD,	 0,  LOG_ALWAYS, 1,0 },
  { "iniş",       do_dismount,    POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "intiha",		do_delet,	POS_DEAD,	 0,  LOG_ALWAYS, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "intihar",		do_delete,	POS_STANDING,	 0,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "invis",		do_invis,	POS_DEAD,	IM,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "iptal",	do_nocancel,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "irfan",           do_lore,        POS_RESTING,     0,  LOG_NORMAL, 1,0 },
  { "isimlendir",		do_restring,		POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "iste",		do_iste,		POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "itekle",		do_push,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
  { "ititle",		do_ititle,	POS_DEAD,	IM,  LOG_NORMAL, 1,CMD_KEEP_HIDE|CMD_GHOST },
  { "iyileştir",		do_heal,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "ısır",		do_vbite,	POS_STANDING,	 0,  LOG_NORMAL, 0,0 },
  { "kaç",		do_flee,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "kalk",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL, 1,CMD_GHOST},
  { "kalk",		do_wake,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "kalkan",		do_shield,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "kamp",		do_camp,  	POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "kamuflaj",     do_camouflage,  POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "kanallar",       do_channels,    POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST},
  { "kanasusama",	do_bloodthirst,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "kanımsa",	do_crecall,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "kapat",		do_close,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "kapıkır",	do_bash_door,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "kaplan",		do_tiger,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "kasa",	        do_kasa,	POS_STANDING,	 0,  LOG_NORMAL, 1,CMD_GHOST},
  { "katana",		do_katana, 	POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "katle",		do_murde,	POS_FIGHTING,	 0,  LOG_NORMAL, 0,0 },
  { "katlet",		do_murder,	POS_FIGHTING,	 0,  LOG_ALWAYS, 1,0 },
  { "kaydet",		do_save,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "kaz",		do_dig,  	POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "kazan",		do_gain,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
  { "kd",		do_kd,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  /*{ "kdg",	do_kdg,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },*/
  { "keskingörüş",      do_truesight,   POS_FIGHTING,    0,  LOG_NORMAL, 0,0 },
  { "kilitaç",         do_unlock,      POS_RESTING,     0,  LOG_NORMAL, 1,CMD_GHOST},
  { "kilitle",		do_lock,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "kim",		do_who_col,		POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "kimdir",		do_whois,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "kirala",		do_rent,	POS_DEAD,	 0,  LOG_NORMAL, 0,0 },
  { "kırbaçla",		do_lash,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "kısayo",		do_alia,	POS_DEAD,	 0,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "kısayol",		do_alias,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "kısayolkaldır",	do_unalias,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "kıyasla",	do_compare,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "kk",             do_cb,          POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_GHOST },
  { "kompakt",	do_compact,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "komutlar",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "konsantrasyon",	do_concentrate,	POS_STANDING,	 0,  LOG_NORMAL, 1, 0 },
  { "kontrol",       	do_control,	POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "korkaklık",		do_wimpy,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "körlüktozu",	do_blindness_dust,POS_FIGHTING,	 0,  LOG_ALWAYS, 1,0 },
  { "koş",		do_run,		POS_STANDING,	ML,  LOG_NEVER, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "koy",		do_put,		POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "kullan",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST},
  { "kurban",	do_sacrifice,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "kurtar",		do_rescue,	POS_FIGHTING,	 0,  LOG_NORMAL, 0,0 },
  { "kuyruk",		do_tail,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "laka",		do_titl,	POS_DEAD,	 0,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "lakap",		do_titl,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST }, /* Changed do_title to do_titl */
  { "limited",	do_limited,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "lisan",		do_speak,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "liste",		do_list,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_KEEP_HIDE|CMD_GHOST },
  { "load",		do_load,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "log",		do_log,		POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST  },
  { "lookup",		do_slookup,	POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "maximum",	do_maximum,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "maymuncuk",		do_pick,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "memory",		do_memory,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "mezat",        do_auction,     POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_GHOST  },
  { "mobstat",		do_mobstat,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "mwhere",		do_mwhere,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "nara",         do_warcry,      POS_FIGHTING,    0,  LOG_NORMAL, 1,0},
  { "nerede",		do_where,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "newlock",	do_newlock,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "noaffect",	do_noaffect,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "nochannels",	do_nochannels,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "noemote",	do_noemote,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "noshout",	do_noshout,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "notell",		do_notell,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "notitle",	do_notitle,	POS_DEAD,	L7,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "objlist",	do_objlist,	POS_DEAD,	ML,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "odaetki",	do_raffects,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "öğren",		do_learn,	POS_STANDING,	 0,  LOG_NORMAL, 1,0},
  { "öğret",		do_teach,	POS_STANDING,	 0,  LOG_NORMAL, 1,0},
  { "oku",		do_recite,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "öldür",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0},
  { "omuz",		do_bash,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "onar",	do_smithing,	POS_RESTING,	 0,  LOG_NORMAL, 1,0},
  { "otoliste",	do_autolist,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "otoakçe",	do_autoakce,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "otoçıkış",	do_autoexit,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "otodağıt",	do_autosplit,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "otodestek",	do_autoassist,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "otokurban",	do_autosac,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "otoyağma",	do_autoloot,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "otur",		do_sit,		POS_SLEEPING,    0,  LOG_NORMAL, 1,CMD_GHOST},
  { "owhere",		do_owhere,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "özet",		do_brief,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "paraçek",	do_withdraw,	POS_STANDING,    0,  LOG_NORMAL, 1,CMD_GHOST},
  { "parayatır",	do_deposit,	POS_STANDING,	 0,  LOG_NORMAL, 1,CMD_GHOST},
  { "patlat",	do_explode, 	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "peace",		do_peace,	POS_DEAD,	L5,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "pençe",		do_claw,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0},
  { "permban",	do_permban,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "pmote",		do_pmote,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "poofin",		do_bamfin,	POS_DEAD,	L8,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "poofout",	do_bamfout,	POS_DEAD,	L8,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "popularity",	do_popularity,	POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "poz",		do_pose,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  //{ "pracnew",        do_pracnew,	POS_SLEEPING,    ML, LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "pratik",       do_practice,	POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "prefi",		do_prefi,	POS_DEAD,	IM,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "prefix",		do_prefix,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "protect",	do_protect,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "purge",		do_purge,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "pusu",         do_ambush,      POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "rapor",		do_report,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "read",		do_read,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "reboo",		do_reboo,	POS_DEAD,	L1,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "reboot",		do_reboot,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "rename",		do_rename,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "restore",	do_restore,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "return",         do_return,      POS_DEAD,       L6,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "rezil",	do_dishonor,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "rica",        do_request,     POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "sağır",		do_deaf,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "saklan",		do_hide,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "saldır",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 0,0},
  { "sapta",         do_detect_hidden,POS_RESTING,    0,  LOG_NORMAL, 1,0},
  { "sargı",        do_bandage,     POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "sat",		do_sell,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "satınal",		do_buy,		POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST},
  { "savun",          do_guard,       POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "savur",	do_brandish,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "sem",	do_envenom,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "sessiz",		do_quiet,	POS_SLEEPING, 	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "set",		do_set,		POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "sez",          do_sense,       POS_RESTING,     0,  LOG_NORMAL, 1,0},
  { "shutdow",	do_shutdow,	POS_DEAD,	L1,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "shutdown",	do_shutdown,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST},
  { "silah",		do_weapon,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "silahsızlandır",		do_disarm,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "sinir",          do_nerve,       POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "sıvış",		do_escape,	POS_FIGHTING,	 0,  LOG_NORMAL,1,0 },
  { "skor",		do_score,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "skrol",		do_scroll,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "sla",		do_sla,		POS_DEAD,	L3,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST },
  { "slay",		do_slay,	POS_DEAD,	L3,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "slook",		do_slook,	POS_SLEEPING,	 0,  LOG_NORMAL, 1,CMD_KEEP_HIDE|CMD_GHOST},
  { "smite",		do_smite,	POS_DEAD,	L7,  LOG_ALWAYS, 1,0 },
  { "smote",		do_smote,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "snoop",		do_snoop,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST}, /* Removed redundant .extra = 0 */
  { "sockets",	do_sockets,	POS_DEAD,	L4,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST}, /* Added missing extra initializer */
  { "sockets",        do_sockets,	POS_DEAD,       L4,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST},
  { "soluş",		do_fade,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "sosyaller",	do_socials,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "söyle",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "stat",		do_stat,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "string",		do_string,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "suflör",		do_prompt,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "suikast",    do_assassinate, POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "sürükle",		do_drag,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
  { "süzül",		do_sneak,	POS_STANDING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "switch",		do_switch,	POS_DEAD,	L6,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "syetenek",		do_slist,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "şifalıeller",	do_layhands,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "şifre",	do_password,	POS_DEAD,	 0,  LOG_NEVER,  1, CMD_KEEP_HIDE|CMD_GHOST },
  { "take",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_KEEP_HIDE },
  { "takip",		do_follow,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "takipyok",	do_nofollow,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "talep",         do_demand,      POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "tamir",		do_repair,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "tanımla",	do_identify,	POS_STANDING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "tara",           do_scan,        POS_RESTING,     0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "tart",	do_consider,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
  { "tekme",		do_kick,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "tekrarla",		do_replay,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_GHOST },
  { "teleport",	do_transfer,    POS_DEAD,	L7,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "tick",		do_tick,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "toz",		do_dirt,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "track",          do_track,       POS_STANDING,    0,  LOG_NORMAL, 0,0 },
  { "transfer",	do_transfer,	POS_DEAD,	L7,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "trust",		do_trust,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "tut",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST},
  { "tuzak",	do_settraps,	POS_STANDING,	 0,  LOG_NORMAL, 1,0},
  { "typo",		do_typo,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "uç",		do_fly,		POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "uysallaştır",           do_tame,        POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "uyu",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "vahşileş",	do_berserk,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
  { "vampir",	do_vampire,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "ver",		do_give,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "violate",	do_violate,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "vlist",		do_vlist,	POS_DEAD,	L1,  LOG_ALWAYS, 1,CMD_KEEP_HIDE|CMD_GHOST},
  { "vnum",		do_vnum,	POS_DEAD,	L4,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "vur",          do_shoot,       POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "wizhelp",	do_wizhelp,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "wizinvis",	do_invis,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "wizlock",	do_wizlock,	POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "wiznet",		do_wiznet,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "yağmayok",		do_noloot,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "yap",           do_make,        POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "yardım",		do_help,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "yargıla",          do_judge,       POS_RESTING,     0,  LOG_ALWAYS, 1,CMD_KEEP_HIDE },
  { "yarma",         do_cleave,      POS_STANDING,    0,  LOG_NORMAL, 1,0 },
  { "ye",		do_eat,		POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "yeniyaşa",		do_remor,	POS_STANDING,	 0,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "yeniyaşam",		do_remort,	POS_DEAD,	 0,  LOG_ALWAYS, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "yetenekler",		do_skills,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "yokol",		do_vanish,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
  { "yudumla",		do_quaff,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
  { "yükselt",	do_enchant, 	POS_RESTING,     0,  LOG_NORMAL, 1,0 },
  { "zaman",		do_time,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE|CMD_GHOST },
  { "zar",	do_zar,	POS_STANDING,	 0,  LOG_NORMAL, 1,CMD_GHOST },
  { "zayi",		do_blink,	POS_FIGHTING,    0,  LOG_NORMAL, 1,CMD_KEEP_HIDE },
  { "zehir",		do_poison_smoke,POS_FIGHTING,	 0,  LOG_ALWAYS, 1,0 },
  { "",		0,		POS_DEAD,	 0,  LOG_NORMAL, 0, CMD_KEEP_HIDE|CMD_GHOST }
};




/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument, bool is_order )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
#ifdef IMMORTALS_LOGS
    char buf[MAX_INPUT_LENGTH];
    char *strtime;
#endif
    int cmd;
    int trust;
    bool found;

    /*
     * Strip leading spaces.
     */
    smash_tilde(argument);
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
      send_to_char( "{RTamamen donmuşsun!{x\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     * also no spaces needed after punctuation.
     */
    strcpy( logline, argument );

#ifdef IMMORTALS_LOGS
    if (IS_IMMORTAL(ch))
	{
	if ( (imm_log = fopen(IMM_LOG_FILE,"a+")) == NULL )
	   {
	    bug("cannot open imm_log_file",0);
	   }
	 else
	 {
	  strtime = (char *) malloc(100);
	  strtime = ctime( &current_time);
	  strtime[strlen(strtime) -1] = '\0';
	  sprintf(buf,"%s :[%s]:%s\n", strtime,ch->name,logline);
	  fprintf(imm_log,buf);
	  fclose(imm_log);
	  free(strtime);
	 }
	}
#endif

    /* Removed checks for multi-byte Turkish characters in single quotes */
    if ( argument[0] == '\'' || argument[0] == ';' || argument[0] == ':' || argument[0] == ',' )
    {
 command[0] = argument[0];
 command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    char lower_command[MAX_INPUT_LENGTH];
    convert_to_turkish_lowercase_utf8(command, lower_command, MAX_INPUT_LENGTH);

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( lower_command[0] == cmd_table[cmd].name[0]
	&&   cmd_table[cmd].level <= trust
	&&   !str_prefix_turkish( lower_command, cmd_table[cmd].name ) )
	{
         /*
          * Implement charmed mobs commands.
          */
         if ( !is_order && IS_AFFECTED(ch,AFF_CHARM) )
          {
            send_to_char(  "Pek sevgili efendini terketmek istemiyorsun.\n\r", ch );
	   return;
          }

          if ( IS_AFFECTED(ch,AFF_STUN) &&
		!(cmd_table[cmd].extra & CMD_KEEP_HIDE) )
	  {
      send_to_char("Bunu yapamayacak kadar baygınsın.\n\r",ch);
	   return;
	  }
          /* Come out of hiding for most commands */
          if ( IS_AFFECTED(ch, AFF_HIDE) && !IS_NPC(ch)
              && !(cmd_table[cmd].extra & CMD_KEEP_HIDE) )
          {
              REMOVE_BIT(ch->affected_by, AFF_HIDE);
              send_to_char("Gölgelerden çıkıyorsun.\n\r", ch);
              act("$n gölgelerden çıkıyor.", ch, NULL, NULL, TO_ROOM);
          }

          if ( IS_AFFECTED(ch, AFF_FADE) && !IS_NPC(ch)
              && !(cmd_table[cmd].extra & CMD_KEEP_HIDE) )
          {
              REMOVE_BIT(ch->affected_by, AFF_FADE);
              send_to_char("Gölgelerden çıkıyorsun.\n\r", ch);
              act("$n gölgelerden çıkıyor.", ch, NULL, NULL, TO_ROOM);
          }

          if ( IS_AFFECTED(ch, AFF_IMP_INVIS) && !IS_NPC(ch)
              && (cmd_table[cmd].position == POS_FIGHTING) )
          {
	      affect_strip(ch,gsn_imp_invis);
              REMOVE_BIT(ch->affected_by, AFF_IMP_INVIS);
              send_to_char("Görünür oluyorsun.\n\r", ch);
              act("$n Görünür oluyor.", ch, NULL, NULL, TO_ROOM);
          }

          if ( CAN_DETECT(ch, ADET_EARTHFADE) && !IS_NPC(ch)
              && (cmd_table[cmd].position == POS_FIGHTING) )
          {
	      affect_strip(ch,gsn_earthfade);
              REMOVE_BIT(ch->detection, ADET_EARTHFADE);
	      WAIT_STATE(ch, (PULSE_VIOLENCE / 2) );
        send_to_char("Doğal formuna dönüyorsun.\n\r", ch);
        act("$n şekilleniyor.", ch, NULL, NULL, TO_ROOM);
          }

          /* prevent ghosts from doing a bunch of commands */
          if (IS_SET(ch->act, PLR_GHOST) && !IS_NPC(ch)
              && !(cmd_table[cmd].extra & CMD_GHOST) )
            continue;

	    found = TRUE;
	    break;
	}
    }

    /*
     * Log and snoop.
     */
    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );

    if ( ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||   cmd_table[cmd].log == LOG_ALWAYS ) && logline[0] != '\0' &&
	 logline[0] != '\n' )
    {
	sprintf( log_buf, "Log %s: %s", ch->name, logline );
	wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	log_string( log_buf );
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "# ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */
	if ( !check_social( ch, command, argument ) )  {
    send_to_char( "Hı?\n\r", ch );
	    return;
	}
	else
	  return;
    }

    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
  send_to_char( "Yatmaya devam et; sen ÖLÜsün.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
  send_to_char( "Bunu yapamayacak kan kaybettin.\n\r", ch );
	    break;

	case POS_STUNNED:
  send_to_char( "Bunu yapamayacak kadar yaralısın.\n\r", ch );
	    break;

	case POS_SLEEPING:
  send_to_char( "Rüyalarında mı?\n\r", ch );
	    break;

	case POS_RESTING:
  send_to_char( "Haayır... Çok rahatlamış durumdasın...\n\r", ch);
	    break;

	case POS_SITTING:
  send_to_char("Önce ayağa kalkmalısın.\n\r",ch);
	    break;

	case POS_FIGHTING:
  send_to_char( "Olmaz!  Hala dövüşüyorsun!\n\r", ch);
	    break;

	}
	return;
    }

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument );

    tail_chain( );
    return;
}



bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;
    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix_turkish( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
      send_to_char("Sen bir antisosyalsin!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
    send_to_char( "Yatmaya devam et; sen ÖLÜsün.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
    send_to_char("Bunu yapamayacak kan kaybettin.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
    send_to_char( "Bunu yapamayacak kadar yaralısın.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
      send_to_char( "Rüyalarında mı?\n\r", ch );
	return TRUE;

    }

    if ( IS_AFFECTED( ch, AFF_HIDE )  )  {
      REMOVE_BIT( ch->affected_by, AFF_HIDE );
      send_to_char( "Gölgelerden çıkıyorsun.\n\r", ch);
      act( "$n gölgelerden çıkıyor.", ch, NULL, NULL, TO_ROOM);
    }

    if ( IS_AFFECTED( ch, AFF_FADE )  )  {
      REMOVE_BIT( ch->affected_by, AFF_FADE );
      send_to_char("Gölgelerden çıkıyorsun.\n\r", ch);
      act("$n gölgelerden çıkıyor.", ch, NULL, NULL, TO_ROOM);
    }

    if ( IS_AFFECTED(ch, AFF_IMP_INVIS) && !IS_NPC(ch)
        && (cmd_table[cmd].position == POS_FIGHTING) )
    {
      affect_strip(ch,gsn_imp_invis);
      REMOVE_BIT(ch->affected_by, AFF_IMP_INVIS);
      send_to_char("Görünmeye başlıyorsun.\n\r", ch);
      act("$n görünmeye başlıyor.", ch, NULL, NULL, TO_ROOM);
    }

    if ( CAN_DETECT(ch, ADET_EARTHFADE) && !IS_NPC(ch)
        && (cmd_table[cmd].position == POS_FIGHTING) )
    {
      affect_strip(ch,gsn_earthfade);
      REMOVE_BIT(ch->detection, ADET_EARTHFADE);
      WAIT_STATE(ch, (PULSE_VIOLENCE / 2));
      send_to_char("Doğal formuna dönüyorsun.\n\r", ch);
      act("$n şekilleniyor.", ch, NULL, NULL, TO_ROOM);
    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
      send_to_char( "Burada değil.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim)
	&&   victim->desc == NULL)
	{
	    switch ( number_range(0,15) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
      act( "$n $M tokatlıyor.",  victim, NULL, ch, TO_NOTVICT );
  		act( "Sen $M tokatlıyorsun.",  victim, NULL, ch, TO_CHAR    );
  		act( "$n seni tokatlıyor.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{

    if ( *arg == '\0' )
        return FALSE;

    if ( *arg == '+' || *arg == '-' )
        arg++;

    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }

    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}

/*
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    (void)argument; /* Mark as unused */
    char buf[MAX_STRING_LENGTH];
    char output[4 * MAX_STRING_LENGTH];
    int letter;
    int cmd;
    int col;

    col = 0;
    output[0] = '\0';

    for(letter = 'a'; letter <= 'z'; letter++)
    {
      for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
      {
        if ( cmd_table[cmd].name[0] == letter
	&&   cmd_table[cmd].level <  LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch )
	&&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    strcat( output, buf );
	    if ( ++col % 6 == 0 )
		strcat(output, "\n\r" );
	}
      }
    }

    if ( col % 6 != 0 )
	strcat( output, "\n\r" );

    page_to_char( output, ch );
    return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    (void)argument; /* Mark as unused */
    char buf[MAX_STRING_LENGTH];
    char output[4 * MAX_STRING_LENGTH];
    char letter;
    int cmd;
    int col;

    col = 0;
    output[0] = '\0';

    for(letter = 'a'; letter <= 'z'; letter++)
    {
      for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
      {
        if ( cmd_table[cmd].name[0] == letter
	&&   cmd_table[cmd].level >= LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch )
        &&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    strcat(output, buf);
	    if ( ++col % 6 == 0 )
		strcat( output, "\n\r");
	}
      }
    }

    if ( col % 6 != 0 )
	strcat(output, "\n\r");

    page_to_char(output, ch);
    return;
}


void do_reture( CHAR_DATA *ch, char *argument)
{
  (void)argument; /* Mark as unused */
  send_to_char("Tamam.\n\r",ch);
  return;
}

/*********** alias.c **************/

/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d, char *argument)
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH],prefix[MAX_INPUT_LENGTH],name[MAX_INPUT_LENGTH];
    char *point;
    int alias;

    ch = d->original ? d->original : d->character;

    /* check for prefix */
    if (ch->prefix[0] != '\0' && str_prefix_turkish("prefix",argument))
    {
	if (strlen(ch->prefix) + strlen(argument) > MAX_INPUT_LENGTH)
	    send_to_char("Line to long, prefix not processed.\r\n",ch);
	else
	{
	    sprintf(prefix,"%s %s",ch->prefix,argument);
	    argument = prefix;
	}
    }

    if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL
    ||	!str_prefix_turkish("kısayol",argument) || !str_prefix_turkish("kısayolkaldır",argument)
    ||  !str_prefix_turkish("prefix",argument))
    {
	interpret(d->character,argument, FALSE);
	return;
    }

    strcpy(buf,argument);

    for (alias = 0; alias < MAX_ALIAS; alias++)	 /* go through the aliases */
    {
	if (ch->pcdata->alias[alias] == NULL)
	    break;

	if (!str_prefix_turkish(ch->pcdata->alias[alias],argument))
	{
	    point = one_argument(argument,name);
	    if (!strcmp(ch->pcdata->alias[alias],name))
	    {
		buf[0] = '\0';
		strcat(buf,ch->pcdata->alias_sub[alias]);
		strcat(buf," ");
		strcat(buf,point);
		break;
	    }
	    if (strlen(buf) > MAX_INPUT_LENGTH)
	    {
		send_to_char("Alias substitution too long. Truncated.\r\n",ch);
		buf[MAX_INPUT_LENGTH -1] = '\0';
	    }
	}
    }
    interpret(d->character,buf, FALSE);
}

void do_alia(CHAR_DATA *ch, char *argument)
{
  (void)argument; /* Mark as unused */
  send_to_char("Üzgünüm, KISAYOL komutu tam girilmeli.\n\r",ch);
    return;
}

void do_alias(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    int pos;

    smash_tilde( argument );

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    argument = one_argument(argument,arg);


    if (arg[0] == '\0')
    {

	if (rch->pcdata->alias[0] == NULL)
	{
    send_to_char("Tanımlı kısayolun yok.\n\r",ch);
	    return;
	}
  send_to_char("Kısayolların:\n\r",ch);

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    sprintf(buf,"    %s:  %s\n\r",rch->pcdata->alias[pos],
		    rch->pcdata->alias_sub[pos]);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!str_prefix_turkish("kısayolkaldır",arg) || !str_cmp("kısayol",arg))
    {
      send_to_char("Üzgünüm, ayrılmış(reserverd) sözcükler olmaz.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    if (!str_cmp(arg,rch->pcdata->alias[pos]))
	    {
        sprintf(buf,"%s = '%s'.\n\r",rch->pcdata->alias[pos],
    			rch->pcdata->alias_sub[pos]);
		send_to_char(buf,ch);
		return;
	    }
	}

  send_to_char("Bu kısayol tanımlanmamış.\n\r",ch);
	return;
    }

    if (!str_prefix_turkish(argument,"intihar") || !str_prefix_turkish(argument,"prefix"))
    {
      send_to_char("Olmaz!\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */
	{
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias_sub[pos] = str_dup(argument);
      sprintf(buf,"%s artık '%s' demek.\n\r",arg,argument);
	    send_to_char(buf,ch);
	    return;
	}
     }

     if (pos >= MAX_ALIAS)
     {
       send_to_char("Üzgünüm, maksimum kısayol sayısını aşamazsınız.\n\r",ch);
	return;
     }

     /* make a new alias */
     rch->pcdata->alias[pos]		= str_dup(arg);
     rch->pcdata->alias_sub[pos]	= str_dup(argument);
     sprintf(buf,"%s artık '%s' demek.\n\r",arg,argument);
     send_to_char(buf,ch);
}


void do_unalias(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    argument = one_argument(argument,arg);

    if (arg[0] == '\0') /* Changed check from arg == NULL to arg[0] == '\0' */
    {
      send_to_char("Hangi kısayolu kaldıracaksınız?\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
	    rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
	    rch->pcdata->alias[pos]		= NULL;
	    rch->pcdata->alias_sub[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->alias[pos]))
	{
    send_to_char("Kısayol kaldırıldı.\n\r",ch);
	    free_string(rch->pcdata->alias[pos]);
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
    send_to_char("O isimde bir kısayol yok.\n\r",ch);
}
