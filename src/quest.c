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
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT		           *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Mandrake}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *
 ***************************************************************************/

/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.01. Please do not remove this notice from this file. *
****************************************************************************/

/****************************************************************************
 *  Revised and adopted to Anatolia by chronos.                             *
 ****************************************************************************/

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
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "utf8.h"
#include "prog_util.h"

DECLARE_SPELL_FUN(	spell_identify	);

void do_tell_quest( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument);
extern	MOB_INDEX_DATA	*mob_index_hash	[MAX_KEY_HASH];

/* Local functions */

void generate_quest	( CHAR_DATA *ch, CHAR_DATA *questman );
void quest_update	( void );

/* Oyun dengesi sabitleri */
#define QUEST_TIME_MIN		15	/* görev süresi (dakika) */
#define QUEST_TIME_MAX		30
#define QUEST_COOLDOWN		5	/* iptal / görev bulunamadı: yeni görev için bekleme (dk) */
#define QUEST_COOLDOWN_DONE	1	/* biten görevden sonra bekleme (dk) */
#define QUEST_LEVEL_RANGE	5	/* görev yaratığı seviye farkı */
#define QUEST_NO_DISCORD_DIV	3	/* Discord ID yoksa ödül bölünür */
#define QUEST_BANK_MAX		39985000L	/* banka 40 milyon akçeden fazlasını almaz */
#define QUEST_BANK_GIFT		15000
#define QUEST_PRACTICE_MAX	10	/* "pratik" ödülü en çok bu kadar alınır */
#define QUEST_MOB_CABAL_MIN	500	/* kabal yaratıkları görev hedefi olmaz */
#define QUEST_MOB_CABAL_MAX	580
#define ENIYI_COST		200	/* do_eniyi ücreti; pazarlıkla 80 ya da 20 */

/* CHANCE function. I use this everywhere in my code, very handy :> */

static bool chance(int num)
{
    return number_range(1,100) <= num;
}

/*
 * Görevciden alınabilen eşyalar. liste / özellik / satınal / sigorta / iade
 * hepsi bu tablodan çalışır; iade puanı fiyatın %90'ıdır.
 */
struct quest_item
{
    char *	keys;		/* is_name anahtarı */
    const char *label;		/* listede görünen ad */
    int		flag;		/* ch->quest biti */
    int		vnum;
    int		price;
    bool	weapon;		/* parlak / mat: silah türü ister */
    bool	matte;		/* renk öneki "mat-" (yoksa "parlak-") */
};

static const struct quest_item quest_items[] =
{
    { "çanta",	  "sırtçantası",    QUEST_BACKPACK, QUEST_ITEM4,	 5000, FALSE, FALSE },
    { "miğfer",	  "miğfer",	    QUEST_MIGFER,   QUEST_ITEM_MIGFER,	 1000, FALSE, FALSE },
    { "kolluk",	  "kolluk",	    QUEST_KOLLUK,   QUEST_ITEM_KOLLUK,	 1000, FALSE, FALSE },
    { "bacaklık", "bacaklık",	    QUEST_BACAKLIK, QUEST_ITEM_BACAKLIK, 1000, FALSE, FALSE },
    { "kalkan",	  "kalkan",	    QUEST_KALKAN,   QUEST_ITEM_KALKAN,	 1000, FALSE, FALSE },
    { "kemer",	  "kemer",	    QUEST_GIRTH,    QUEST_ITEM1,	 1000, FALSE, FALSE },
    { "parlak",	  "parlak silah",   QUEST_SILAH1,   QUEST_ITEM_SILAH1,	 1000, TRUE,  FALSE },
    { "mat",	  "mat silah",	    QUEST_SILAH2,   QUEST_ITEM_SILAH2,	 1000, TRUE,  TRUE  },
    { "işlemeli", "işlemeli yüzük", QUEST_YUZUK1,   QUEST_ITEM_YUZUK1,	  750, FALSE, FALSE },
    { "desenli",  "desenli yüzük",  QUEST_YUZUK2,   QUEST_ITEM_YUZUK2,	  750, FALSE, FALSE },
    { "oymalı",	  "oymalı yüzük",   QUEST_YUZUK3,   QUEST_ITEM_YUZUK3,	  750, FALSE, FALSE },
    { "kakmalı",  "kakmalı yüzük",  QUEST_YUZUK4,   QUEST_ITEM_YUZUK4,	  750, FALSE, FALSE },
    { "testi",	  "testi",	    QUEST_DECANTER, QUEST_ITEM5,	  500, FALSE, FALSE },
    { NULL, NULL, 0, 0, 0, FALSE, FALSE }
};

/* parlak/mat silahın alt türleri: value[0] silah sınıfı, value[3] attack_table indeksi */
static const struct
{
    char *	keys;
    int		weapon_type;
    int		dam_type;
} quest_weapon_kinds[] =
{
    { "kılıç",	 WEAPON_SWORD,	 3  },	/* slash  */
    { "hançer",	 WEAPON_DAGGER, 11  },	/* pierce */
    { "kırbaç",	 WEAPON_WHIP,	 4  },	/* whip   */
    { "balta",	 WEAPON_AXE,	 8  },	/* crush  */
    { "egzotik", WEAPON_EXOTIC, 18  },	/* wrath  */
    { NULL, 0, 0 }
};

/* Eşya olmayan ödüller (yetenek, sözleşme, puan). iclass: -1 herkes; listede
 * yalnızca o sınıfa gösterilir. buy() ön koşulları denetler, ücreti quest_pay ile alır. */
struct quest_service
{
    char *	keys;
    const char *list_key;	/* listede parantez içinde görünen ad */
    const char *label;
    int		price;
    int		iclass;
    void	(*buy)(CHAR_DATA *ch, CHAR_DATA *qm, int price);
};

static const char *const quest_help_lines[] =
{
    "GÖREV KOMUTLARI: puan bilgi zaman iste bitti iptal liste özellik satınal sigorta iade.\n\r",
    "Daha fazla bilgi için: yardım görev.\n\r",
    NULL
};

/*
 * Küçük yardımcılar
 */

static void quest_tellf(CHAR_DATA *ch, CHAR_DATA *qm, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

static void quest_tellf(CHAR_DATA *ch, CHAR_DATA *qm, const char *fmt, ...)
{
    char buf[MAX_INPUT_LENGTH];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    do_tell_quest(ch, qm, buf);
}

/* Yeterli görev puanı varsa düşer; yoksa görevci söyler ve FALSE döner. */
static bool quest_pay(CHAR_DATA *ch, CHAR_DATA *qm, int cost)
{
    if (ch->pcdata->questpoints < cost)
    {
	quest_tellf(ch, qm, "Üzgünüm %s, bunun için yeterli görev puanın yok.", ch->name);
	return FALSE;
    }
    ch->pcdata->questpoints -= cost;
    return TRUE;
}

/* Görev durumunu sıfırlar; nextquest yeni görev için bekleme süresidir (dk). */
static void quest_clear(CHAR_DATA *ch, int nextquest)
{
    REMOVE_BIT(ch->act, PLR_QUESTOR);
    ch->pcdata->questgiver = 0;
    ch->pcdata->countdown = 0;
    ch->pcdata->questmob = 0;
    ch->pcdata->questroom = 0;
    ch->pcdata->nextquest = nextquest;
}

/* Odadaki görevci (spec_questmaster) */
static CHAR_DATA *find_questmaster(ROOM_INDEX_DATA *room)
{
    SPEC_FUN *spec = spec_lookup("spec_questmaster");
    CHAR_DATA *qm;

    if (room == NULL)
	return NULL;
    for (qm = room->people; qm != NULL; qm = qm->next_in_room)
	if (IS_NPC(qm) && qm->spec_fun == spec)
	    return qm;
    return NULL;
}

static const struct quest_item *quest_item_lookup(char *arg)
{
    const struct quest_item *qi;

    if (arg[0] == '\0')
	return NULL;
    for (qi = quest_items; qi->keys != NULL; qi++)
	if (is_name(arg, qi->keys))
	    return qi;
    return NULL;
}

/* Silah alt türü; bilinmeyen ad egzotik sayılır (eski davranış). */
static void quest_weapon_kind(char *arg, int *weapon_type, int *dam_type)
{
    int i;

    for (i = 0; quest_weapon_kinds[i].keys != NULL; i++)
	if (is_name(arg, quest_weapon_kinds[i].keys))
	    break;
    if (quest_weapon_kinds[i].keys == NULL)
	i--;	/* egzotik */
    *weapon_type = quest_weapon_kinds[i].weapon_type;
    *dam_type = quest_weapon_kinds[i].dam_type;
}

/* Silah alt türü ister; eksikse seçenekleri anlatıp FALSE döner. */
static bool quest_need_weapon_kind(CHAR_DATA *ch, const char *cmd, const struct quest_item *qi, char *arg3)
{
    if (arg3[0] != '\0')
	return TRUE;
    printf_to_char(ch,"Hangi tür silah istiyorsun?\n\r");
    printf_to_char(ch,"Seçenekler: kılıç, hançer, kırbaç, balta ve egzotik\n\r");
    printf_to_char(ch,"Örn: görev %s %s kılıç\n\r", cmd, qi->keys);
    return FALSE;
}

/* Görev eşyasını sahibinin adı ve rengiyle yaratır (alan şablonu biçim dizgisi değildir). */
static OBJ_DATA *quest_make_item(CHAR_DATA *ch, const struct quest_item *qi, char *arg3)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    const char *color;

    if ((obj = prog_create_object(qi->vnum, ch->level)) == NULL)
	return NULL;

    if (qi->matte)
	color = IS_GOOD(ch) ? "mat-mavi" : IS_NEUTRAL(ch) ? "mat-yeşil" : "mat-kızıl";
    else
	color = IS_GOOD(ch) ? "parlak-mavi" : IS_NEUTRAL(ch) ? "parlak-yeşil" : "parlak-kızıl";

    prog_subst(buf, sizeof(buf), obj->short_descr, color, ch->name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);
    prog_set_owner(obj, ch->name);

    if (qi->weapon)
    {
	int weapon_type, dam_type;

	quest_weapon_kind(arg3, &weapon_type, &dam_type);
	obj->value[0] = weapon_type;
	obj->value[3] = dam_type;
    }
    return obj;
}

/* Oyuncunun bu görev eşyasından elindekini (dünyanın neresinde olursa olsun) yok eder. */
static void quest_extract_object(CHAR_DATA *ch, int obj_vnum)
{
    OBJ_DATA *obj, *obj_next;

    for (obj = object_list; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next;
	if (obj->pIndexData->vnum == obj_vnum && prog_obj_owned_by(obj, ch))
	{
	    extract_obj(obj);
	    break;
	}
    }
}

/* liste satırı: "{Cad{x......({yanahtar{x )......{R5000 gp{x" */
static void quest_list_line(CHAR_DATA *ch, const char *label, const char *keys, int price)
{
    char dots[32];
    int n, w;

    w = UMAX(0, 20 - (int) utf8_strlen(label));
    for (n = 0; n < w && n < (int) sizeof(dots) - 1; n++)
	dots[n] = '.';
    dots[n] = '\0';
    printf_to_char(ch, "{C%s{x%s({y%-*s{x)", label, dots, utf8_width(keys, 8), keys);

    w = 6 + (price < 1000) + (price < 100) + (price < 10);
    for (n = 0; n < w; n++)
	dots[n] = '.';
    dots[n] = '\0';
    printf_to_char(ch, "%s{R%d gp{x\n\r", dots, price);
}

/* do_eniyi için: eşyanın bulunduğu bölge (taşıyan odasızsa "Bir yerlerde") */
static const char *obj_area_name(OBJ_DATA *obj)
{
    CHAR_DATA *who = obj->carried_by;

    if (who != NULL && who->in_room != NULL && who->in_room->area != NULL)
	return who->in_room->area->name;
    return "Bir yerlerde";
}

struct top3
{
    OBJ_DATA *	obj[3];
    int		val[3];
};

static void top3_insert(struct top3 *t, OBJ_DATA *obj, int val)
{
    int i;

    for (i = 0; i < 3; i++)
	if (val >= t->val[i])
	    break;
    if (i >= 3)
	return;
    memmove(&t->obj[i + 1], &t->obj[i], (2 - i) * sizeof(t->obj[0]));
    memmove(&t->val[i + 1], &t->val[i], (2 - i) * sizeof(t->val[0]));
    t->obj[i] = obj;
    t->val[i] = val;
}

/*
 * Eşya olmayan ödüller
 */

static void buy_ateist(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    if (ch->religion == 0)
    {
	do_tell_quest(ch, qm, "Zaten herhangi bir tanrıya inanmıyorsun.");
	return;
    }
    if (!quest_pay(ch, qm, price))
	return;
    ch->pcdata->din_puani = 0;
    ch->religion = 0;
    act("$n artık herhangi bir tanrıya inanmıyor.", ch, NULL, qm, TO_ROOM );
    act("$N dinden çıkmana yardım ediyor.",   ch, NULL, qm, TO_CHAR );
}

static void buy_kasa(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    if (ch->pcdata->kisisel_kasa == 1)
    {
	do_tell_quest(ch, qm, "Zaten kişisel bir kasan var. İnanmıyorsan Otho'ya sorabilirsin.");
	return;
    }
    if (!quest_pay(ch, qm, price))
	return;
    ch->pcdata->kisisel_kasa = 1;
    act("$n kişisel bir kasa için gereken sözleşmeyi imzalıyor.", ch, NULL, qm, TO_ROOM );
    act("$N kişisel kasa sözleşmesini imzalarken sana yardımcı oluyor.",   ch, NULL, qm, TO_CHAR );
}

static void buy_katlet(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    if (ch->pcdata->oyuncu_katli == 1)
    {
	do_tell_quest(ch, qm, "Zaten oyuncu katli sözleşmesini kabul etmişsin.");
	return;
    }
    if (!quest_pay(ch, qm, price))
	return;
    ch->pcdata->oyuncu_katli = 1;
    act("$n oyuncu katli sözleşmesini kabul ediyor.", ch, NULL, qm, TO_ROOM );
    act("$N oyuncu katli sözleşmesini imzalarken sana yardımcı oluyor.",   ch, NULL, qm, TO_CHAR );
}

static void buy_pratik(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    if (ch->pcdata->questpractice >= QUEST_PRACTICE_MAX)
    {
	quest_tellf(ch, qm, "Üzgünüm %s, bu ödülü daha önce %d kez almıştın!", ch->name, QUEST_PRACTICE_MAX);
	return;
    }
    if (!quest_pay(ch, qm, price))
	return;
    ch->practice += 100;
    act("$N $e 100 pratik seansı veriyor.", ch, NULL, qm, TO_ROOM );
    act("$N sana 100 pratik seansı veriyor.",   ch, NULL, qm, TO_CHAR );
    ch->pcdata->questpractice += 1;
}

static void buy_vampir(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    int sn;

    if (ch->iclass != CLASS_VAMPIRE)
    {
	quest_tellf(ch, qm, "Sen bu yeteneği kazanamazsın %s.", ch->name);
	return;
    }
    if ((sn = skill_lookup("vampire")) < 0)
    {
	bug("buy_vampir: 'vampire' yeteneği yok.", 0);
	return;
    }
    if (!quest_pay(ch, qm, price))
	return;
    ch->pcdata->learned[sn] = 100;
    act( "$N hortlaklığın sırrını $e veriyor.", ch, NULL, qm, TO_ROOM );
    act( "$N sana hortlaklığın sırrını veriyor.",   ch, NULL, qm, TO_CHAR );
    act_color( "$CGökyüzünde şimşekler çakıyor.$c",   ch, NULL, qm, TO_ALL,POS_SLEEPING,CLR_BLUE );
}

static void buy_bunye(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    if (ch->perm_stat[STAT_CON] >= get_max_train(ch, STAT_CON))
    {
	quest_tellf(ch, qm, "Üzgünüm %s, bünye niteliğin yeterince güçlü.", ch->name);
	return;
    }
    if (!quest_pay(ch, qm, price))
	return;
    ch->perm_stat[STAT_CON] += 1;
}

static void buy_akce(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    if (ch->pcdata->bank_s > QUEST_BANK_MAX)
    {
	send_to_char("Banka 40 milyon akçeden fazlasını kabul etmez.\n\r",ch);
	return;
    }
    if (!quest_pay(ch, qm, price))
	return;
    ch->pcdata->bank_s += QUEST_BANK_GIFT;
    printf_to_char(ch, "Banka hesabına %d akçe yatırıldı. Güle güle harca.\n\r", QUEST_BANK_GIFT);
}

static void buy_samuray_olum(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    if (ch->iclass != CLASS_SAMURAI)
    {
	quest_tellf(ch, qm, "Üzgünüm %s, fakat sen bir samuray değilsin.", ch->name);
	return;
    }
    if (ch->pcdata->death < 1)
    {
	quest_tellf(ch, qm, "Üzgünüm %s, henüz ölmemişsin.", ch->name);
	return;
    }
    if (!quest_pay(ch, qm, price))
	return;
    ch->pcdata->death -= 1;
}

/* katana / keskin: samurayın yanındaki katanaya kalıcı silah bayrağı işler */
static void buy_katana_flag(CHAR_DATA *ch, CHAR_DATA *qm, int price, int flag)
{
    AFFECT_DATA af;
    OBJ_DATA *katana;

    if (ch->iclass != CLASS_SAMURAI)
    {
	quest_tellf(ch, qm, "Üzgünüm %s, fakat sen bir samuray değilsin.", ch->name);
	return;
    }
    if ( (katana = get_obj_list(ch,"katana",ch->carrying)) == NULL)
    {
	quest_tellf(ch, qm, "Üzgünüm %s, fakat katanan yanında değil.", ch->name);
	return;
    }
    if (flag == WEAPON_KATANA && IS_WEAPON_STAT(katana,WEAPON_KATANA))
    {
	quest_tellf(ch, qm, "Üzgünüm %s, fakat katanan ilk görevi geçmiş zaten.", ch->name);
	return;
    }
    if (flag != WEAPON_KATANA && !IS_WEAPON_STAT(katana,WEAPON_KATANA))
    {
	quest_tellf(ch, qm, "Üzgünüm %s, fakat katanan ilk görevi henüz geçmemiş.", ch->name);
	return;
    }
    if (!quest_pay(ch, qm, price))
	return;

    af.where	= TO_WEAPON;
    af.type	= gsn_reserved;
    af.level	= 100;
    af.duration	= -1;
    af.modifier	= 0;
    af.bitvector = flag;
    af.location	= APPLY_NONE;
    affect_to_obj(katana,&af);
    do_tell_quest(ch, qm, flag == WEAPON_KATANA
		  ? "Katanandaki gücün giderek artacağını hissediyorsun."
		  : "Şu andan sonra katanan en keskin kılıçlardan daha keskin olacak.");
}

static void buy_katana(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    buy_katana_flag(ch, qm, price, WEAPON_KATANA);
}

static void buy_keskin(CHAR_DATA *ch, CHAR_DATA *qm, int price)
{
    buy_katana_flag(ch, qm, price, WEAPON_SHARP);
}

static const struct quest_service quest_services[] =
{
    { "pratik",		"pratik", "100 pratik",		  1000, -1,	       buy_pratik	},
    { "bünye",		"bünye",  "1 bünye puanı",	   250, -1,	       buy_bunye	},
    { "akçe",		"akçe",	  "15.000 akçe",	    10, -1,	       buy_akce		},
    { "ateist",		"ateist", "dinden çıkma",	  1000, -1,	       buy_ateist	},
    { "katlet",		"katlet", "oyuncu katline giriş",  100, -1,	       buy_katlet	},
    { "kasa",		"kasa",	  "kişisel kasa",	   500, -1,	       buy_kasa		},
    { "katana",		"katana", "katana",		   100, CLASS_SAMURAI, buy_katana	},
    { "keskin ikinci",	"keskin", "ikinci katana",	   100, CLASS_SAMURAI, buy_keskin	},
    { "samuray ölüm",	"ölüm",	  "ölüm azaltma",	    50, CLASS_SAMURAI, buy_samuray_olum },
    { "vampir",		"vampir", "vampir yeteneği",	    50, CLASS_VAMPIRE, buy_vampir	},
    { NULL, NULL, NULL, 0, 0, NULL }
};

/*
 * Alt komutlar
 */

static void quest_cmd_bilgi(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    MOB_INDEX_DATA *questinfo;
    ROOM_INDEX_DATA *pRoomIndex;

    if (!IS_SET(ch->act, PLR_QUESTOR))
    {
	send_to_char("Henüz bir görevin yok.\n\r",ch);
	return;
    }
    if (ch->pcdata->questmob == -1 )
    {
	printf_to_char(ch,"{cGörevin neredeyse tamamlandı!\n\rZamanın bitmeden önce görevciye git!{x\n\r");
	return;
    }
    if (ch->pcdata->questmob <= 0
    ||  (questinfo = get_mob_index(ch->pcdata->questmob)) == NULL)
    {
	send_to_char("Henüz bir görevin yok.\n\r",ch);
	return;
    }
    if ((pRoomIndex = get_room_index(ch->pcdata->questroom)) == NULL)
	printf_to_char(ch,"Görevin, {c%s{w adlı korkunç yaratığı öldürmek! (Bulunduğu oda artık yok.)\n\r",
		       questinfo->short_descr);
    else
	printf_to_char(ch,"Görevin, {c%s{w bölgesinde {c%s{w isimli odadaki {c%s{w adlı korkunç yaratığı öldürmek!\n\r",
		       pRoomIndex->area->name,pRoomIndex->name,questinfo->short_descr);
}

static void quest_cmd_puan(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    printf_to_char(ch, "%d görev puanın var.\n\r",ch->pcdata->questpoints);
}

static void quest_cmd_zaman(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    if (!IS_SET(ch->act, PLR_QUESTOR))
    {
	send_to_char("Henüz bir görevin yok.\n\r",ch);
	if (ch->pcdata->nextquest > 1)
	    printf_to_char(ch, "Yeni bir görev isteyebilmen için %d dakika kaldı.\n\r",ch->pcdata->nextquest);
	else if (ch->pcdata->nextquest == 1)
	    send_to_char("Yeni bir görev isteyebilmen için bir dakikadan az zaman kaldı.\n\r", ch);
    }
    else if (ch->pcdata->countdown > 0)
	printf_to_char(ch, "Görevi bitirmek için kalan zaman: %d dakika.\n\r",ch->pcdata->countdown);
}

static void quest_cmd_liste(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    const struct quest_item *qi;
    const struct quest_service *svc;

    act("$n $Z görev ekipmanlarının listesini istiyor.", ch, NULL, qm, TO_ROOM);
    act("$Z görev ekipmanlarının listesini istiyorsun.",ch, NULL, qm, TO_CHAR);

    for (qi = quest_items; qi->keys != NULL; qi++)
	quest_list_line(ch, qi->label, qi->keys, qi->price);
    for (svc = quest_services; svc->keys != NULL; svc++)
	if (svc->iclass == -1 || ch->iclass == svc->iclass)
	    quest_list_line(ch, svc->label, svc->list_key, svc->price);

    printf_to_char(ch, "Bir eşya satın almak için {Rgörev satınal <eşya_adı>{x yaz.\n\r");
}

static void quest_cmd_ozellik(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    const struct quest_item *qi;
    OBJ_DATA *obj;

    if ((qi = quest_item_lookup(arg2)) == NULL)
    {
	printf_to_char(ch,"Hangi görev ekipmanının özelliklerini öğrenmek istiyorsun?\n\rKullanım: {Rgörev özellik <ekipman>{x\n\r");
	return;
    }
    if ((obj = prog_create_object(qi->vnum, ch->level)) == NULL)
	return;
    spell_identify( 0, 0, ch, obj ,0);
    extract_obj(obj);
}

static void quest_cmd_satinal(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    const struct quest_item *qi;
    const struct quest_service *svc;
    OBJ_DATA *obj;

    if (arg2[0] == '\0')
    {
	send_to_char("Bir ödülü satın almak için 'görev satınal <görev_eşyası>' yazın.\n\r",ch);
	return;
    }

    for (svc = quest_services; svc->keys != NULL; svc++)
	if (is_name(arg2, svc->keys))
	{
	    (*svc->buy)(ch, qm, svc->price);
	    return;
	}

    if ((qi = quest_item_lookup(arg2)) == NULL)
    {
	quest_tellf(ch, qm, "Ondan bende yok, %s.", ch->name);
	return;
    }
    if (IS_SET(ch->quest, qi->flag))
    {
	do_tell_quest(ch, qm, "Bu eşyayı zaten almışsın. Kaybettiysen sigortadan faydalan.");
	return;
    }
    if (qi->weapon && !quest_need_weapon_kind(ch, "satınal", qi, arg3))
	return;
    if (!quest_pay(ch, qm, qi->price))
	return;
    if ((obj = quest_make_item(ch, qi, arg3)) == NULL)
    {
	ch->pcdata->questpoints += qi->price;
	return;
    }
    SET_BIT(ch->quest, qi->flag);
    act("$N $e $p veriyor.", ch, obj, qm, TO_ROOM );
    act("$N sana $p veriyor.",   ch, obj, qm, TO_CHAR );
    obj_to_char(obj, ch);
}

static void quest_cmd_sigorta(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    const struct quest_item *qi;
    OBJ_DATA *obj;

    if (arg2[0] == '\0')
    {
	send_to_char("Görev objesinin sigortasından faydalanmak için 'görev sigorta <obje>' yazın.\n\r",ch);
	return;
    }
    if ((qi = quest_item_lookup(arg2)) == NULL)
	return;
    if (qi->weapon && !quest_need_weapon_kind(ch, "sigorta", qi, arg3))
	return;
    if (!IS_SET(ch->quest, qi->flag))
    {
	quest_tellf(ch, qm, "Üzgünüm %s, fakat bahsettiğin ödülü henüz almamışsın.", ch->name);
	return;
    }

    quest_extract_object(ch, qi->vnum);
    if ((obj = quest_make_item(ch, qi, arg3)) == NULL)
	return;
    act( "$N $p objesini $e veriyor.", ch, obj, qm, TO_ROOM );
    act( "$N $p objesini sana veriyor.",   ch, obj, qm, TO_CHAR );
    obj_to_char(obj, ch);
}

static void quest_cmd_iade(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    const struct quest_item *qi;

    if ((qi = quest_item_lookup(arg2)) == NULL)
    {
	printf_to_char(ch,"Hangi görev ekipmanını iade etmek istiyorsun?\n\rKullanım: {Rgörev iade <ekipman>{x\n\r");
	return;
    }
    if (!IS_SET(ch->quest, qi->flag))
    {
	quest_tellf(ch, qm, "Üzgünüm %s, fakat bahsettiğin ödülü henüz almamışsın.", ch->name);
	return;
    }

    quest_extract_object(ch, qi->vnum);
    REMOVE_BIT(ch->quest, qi->flag);
    ch->pcdata->questpoints += qi->price * 9 / 10;
    printf_to_char(ch,"İade işlemi tamamlandı. Puanların hesabına geçti. Yine bekleriz.\n\r");
}

static void quest_cmd_iste(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    act("$n $Z görev istiyor.", ch, NULL, qm, TO_ROOM);
    act("$Z görev istiyorsun.",ch, NULL, qm, TO_CHAR);

    if (IS_SET(ch->act, PLR_QUESTOR))
    {
	do_tell_quest(ch, qm, "Zaten görevdesin. İnanmıyorsan \"görev bilgi\" yaz!");
	return;
    }
    if (ch->pcdata->nextquest > 0)
    {
	quest_tellf(ch, qm, "Çok cesursun %s, fakat izin ver başkaları da nasiplensin.", ch->name);
	do_tell_quest(ch, qm, "Daha sonra tekrar uğra.");
	return;
    }

    quest_tellf(ch, qm, "Teşekkür ederim, cesur %s!", ch->name);

    generate_quest(ch, qm);

    if (ch->pcdata->questmob > 0 )
    {
	ch->pcdata->countdown = number_range(QUEST_TIME_MIN, QUEST_TIME_MAX);
	SET_BIT(ch->act, PLR_QUESTOR);
	quest_tellf(ch, qm, "Bu görevi tamamlamak için %d dakikan var.", ch->pcdata->countdown);
	do_tell_quest(ch, qm, "Tanrılar seninle olsun!");
    }
}

static void quest_cmd_iptal(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    if (!IS_SET(ch->act, PLR_QUESTOR))
    {
	do_tell_quest(ch, qm, "Görevde değilsin.");
	return;
    }
    quest_clear(ch, QUEST_COOLDOWN);
    do_tell_quest(ch, qm, "Görevini iptal ettim.");
}

static void quest_cmd_bitti(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3)
{
    act("$n $E görevi bitirdiğini haber veriyor.", ch, NULL, qm, TO_ROOM);
    act("$E görevi bitirdiğini haber veriyorsun.",ch, NULL, qm, TO_CHAR);
    if (ch->pcdata->questgiver != qm->pIndexData->vnum)
    {
	do_tell_quest(ch, qm, "Seni bir göreve gönderdiğimi hatırlamıyorum!");
	return;
    }

    if (IS_SET(ch->act, PLR_QUESTOR))
    {
	if (ch->pcdata->questmob == -1 && ch->pcdata->countdown > 0)
	{
	    int reward, pointreward, pracreward;

	    reward = 700 + number_range(100,600);
	    pointreward = number_range(21,35);

	    if(IS_SET(ch->pcdata->dilek,DILEK_FLAG_GOREV))
	    {
		printf_to_char( ch , "{CGörev dileğin sayesinde kazandığın GP artıyor.{x\n\r" );
		pointreward *= 2;
	    }
	    if( ikikat_gp > 0 )
	    {
		printf_to_char( ch , "{Cİki kat GP kazanma etkinliği nedeniyle kazandığın GP artıyor.{x\n\r" );
		pointreward *= 2;
	    }
	    if(ch->pcdata->discord_id[0] == '\0')
	    {
		printf_to_char( ch , "{CDiscord ID girmediğin için kazandığın GP ve akçe azalıyor.{x\n\r" );
		pointreward /= QUEST_NO_DISCORD_DIV;
		reward /= QUEST_NO_DISCORD_DIV;
	    }

	    do_tell_quest(ch, qm, "Tebrikler!");
	    quest_tellf(ch, qm, "Karşılığında sana %d GP ve %d akçe veriyorum.", pointreward, reward);
	    if (chance(2))
	    {
		pracreward = number_range(1,6);
		printf_to_char(ch, "%d pratik seansı kazandın!\n\r", pracreward);
		ch->practice += pracreward;
	    }
	    if (number_range(1,8)==1)
	    {
		pracreward = number_range(1,7);
		printf_to_char(ch,"%d RK puanı kazandın.\n\r",pracreward);
		ch->pcdata->rk_puani += pracreward;
	    }

	    quest_clear(ch, QUEST_COOLDOWN_DONE);
	    ch->silver += reward;
	    ch->pcdata->questpoints += pointreward;
	    return;
	}
	else if ( ch->pcdata->questmob > 0 && ch->pcdata->countdown > 0 )
	{
	    do_tell_quest(ch, qm, "Henüz görevi bitirmedin. Fakat hala zamanın var!");
	    return;
	}
    }
    if (ch->pcdata->nextquest > 0)
	do_tell_quest(ch, qm, "Maalesef görevi zamanında tamamlayamadın!");
    else
	quest_tellf(ch, qm, "Önce bir görev istemelisin, %s.", ch->name);
}

/* Alt komut tablosu: tam ad önce, sonra tablo sırasıyla ön ek (yıkıcı olanlar sonda). */
static const struct
{
    const char *name;
    bool	needs_questman;
    void	(*fun)(CHAR_DATA *ch, CHAR_DATA *qm, char *arg2, char *arg3);
} quest_commands[] =
{
    { "bilgi",	 FALSE, quest_cmd_bilgi	  },
    { "puan",	 FALSE, quest_cmd_puan	  },
    { "zaman",	 FALSE, quest_cmd_zaman	  },
    { "liste",	 TRUE,  quest_cmd_liste	  },
    { "özellik", TRUE,  quest_cmd_ozellik },
    { "satınal", TRUE,  quest_cmd_satinal },
    { "sigorta", TRUE,  quest_cmd_sigorta },
    { "iste",	 TRUE,  quest_cmd_iste	  },
    { "bitti",	 TRUE,  quest_cmd_bitti	  },
    { "iade",	 TRUE,  quest_cmd_iade	  },
    { "iptal",	 TRUE,  quest_cmd_iptal	  },
    { NULL, FALSE, NULL }
};

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman = NULL;
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    int i, found = -1;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (IS_NPC(ch) )  return;

    if (arg1[0] != '\0')
    {
	for (i = 0; quest_commands[i].name != NULL; i++)
	    if (!str_cmp(arg1, quest_commands[i].name))
	    {
		found = i;
		break;
	    }
	if (found < 0)
	    for (i = 0; quest_commands[i].name != NULL; i++)
		if (!str_prefix(arg1, quest_commands[i].name))
		{
		    found = i;
		    break;
		}
    }

    if (found < 0)
    {
	for (i = 0; quest_help_lines[i] != NULL; i++)
	    send_to_char(quest_help_lines[i], ch);
	return;
    }

    if (quest_commands[found].needs_questman)
    {
	/* Checks for a character in the room with spec_questmaster set. */
	if ((questman = find_questmaster(ch->in_room)) == NULL)
	{
	    send_to_char("Odada görevci göremiyorum.\n\r", ch);
	    return;
	}
	if ( questman->fighting != NULL)
	{
	    send_to_char("Dövüş bitene kadar bekle.\n\r",ch);
	    return;
	}
	ch->pcdata->questgiver = questman->pIndexData->vnum;
    }

    (*quest_commands[found].fun)(ch, questman, arg2, arg3);
}

/* Görev hedefi olamayacak özel yaratıklar (çağrılanlar, kabal, görev zinciri) */
static const int noquest_vnums[] =
{
    MOB_VNUM_SHADOW, MOB_VNUM_SPECIAL_GUARD, MOB_VNUM_BEAR, MOB_VNUM_DEMON,
    MOB_VNUM_NIGHTWALKER, MOB_VNUM_STALKER, MOB_VNUM_SQUIRE, MOB_VNUM_MIRROR_IMAGE,
    MOB_VNUM_UNDEAD, MOB_VNUM_LION, MOB_VNUM_WOLF, MOB_VNUM_LESSER_GOLEM,
    MOB_VNUM_STONE_GOLEM, MOB_VNUM_IRON_GOLEM, MOB_VNUM_ADAMANTITE_GOLEM, MOB_VNUM_HUNTER,
    MOB_VNUM_SUM_SHADOW, MOB_VNUM_DOG, MOB_VNUM_ELM_EARTH, MOB_VNUM_ELM_AIR,
    MOB_VNUM_ELM_FIRE, MOB_VNUM_ELM_WATER, MOB_VNUM_ELM_LIGHT, MOB_VNUM_WEAPON,
    MOB_VNUM_ARMOR
};

static bool quest_mob_ok(CHAR_DATA *ch, CHAR_DATA *victim)
{
    int vnum, level_diff;
    size_t i;

    if (!IS_NPC(victim))
	return FALSE;
    level_diff = victim->level - ch->level;
    if (level_diff > QUEST_LEVEL_RANGE || level_diff < -QUEST_LEVEL_RANGE)
	return FALSE;
    if ( IS_SET(victim->act,ACT_TRAIN) || IS_SET(victim->act,ACT_PRACTICE)
      || IS_SET(victim->act,ACT_IS_HEALER) || IS_SET(victim->act,ACT_NOTRACK) )
	return FALSE;

    vnum = victim->pIndexData->vnum;
    for (i = 0; i < sizeof(noquest_vnums) / sizeof(noquest_vnums[0]); i++)
	if (vnum == noquest_vnums[i])
	    return FALSE;
    /* kabal moblarini eleyelim */
    if (vnum >= QUEST_MOB_CABAL_MIN && vnum <= QUEST_MOB_CABAL_MAX)
	return FALSE;

    if (victim->in_room == NULL || !room_has_exit(victim->in_room))
	return FALSE;
    if ( IS_GOOD(victim) && IS_GOOD(ch) )
	return FALSE;
    return TRUE;
}

/* Uygun yaratıklardan rastgele biri (rezervuar örnekleme: bellek ayırmaz). */
CHAR_DATA * find_a_quest_mob( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *chosen = NULL;
    int count = 0;

    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if (!quest_mob_ok(ch, victim))
	    continue;
	if (number_range(1, ++count) == 1)
	    chosen = victim;
    }
    return chosen;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;

    victim = find_a_quest_mob(ch);

    if (victim == NULL)
    {
      do_tell_quest(ch,questman,"Üzgünüm ama şu an sana verebileceğim bir görev yok.");
      do_tell_quest(ch,questman,"Daha sonra tekrar dene.");
      ch->pcdata->nextquest = QUEST_COOLDOWN;
      return;
    }

    if (IS_GOOD(ch))
    {
      quest_tellf(ch,questman, "Diyarın azılı asilerinden %s, zindandan kaçtı!",victim->short_descr);
      quest_tellf(ch,questman, "Kaçışından bu yana tahminimizce %d sivili katletti!", number_range(2,20));
      do_tell_quest(ch,questman,"Bunun cezası ölümdür!");
    }
    else
    {
      quest_tellf(ch,questman, "Şahsi düşmanım %s, kraliyet tacına karşı tehdit oluşturuyor.",victim->short_descr);
      do_tell_quest(ch,questman, "Bu tehdit yokedilmeli!");
    }

    if (victim->in_room->name != NULL)
    {
      quest_tellf(ch,questman, "%s şu sıralar %s bölgesindedir!",victim->short_descr,victim->in_room->area->name);
      quest_tellf(ch,questman, "Yeri %s civarında.",victim->in_room->name);
    }

    ch->pcdata->questmob = victim->pIndexData->vnum;
    ch->pcdata->questroom = victim->in_room->vnum;
}

/* Called from update_handler() by pulse_area */

void quest_update(void)
{
    CHAR_DATA *ch, *ch_next;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (IS_NPC(ch)) continue;

	if (ch->pcdata->nextquest > 0)
	{
	    ch->pcdata->nextquest--;

	    if (ch->pcdata->nextquest == 0)
		send_to_char("Yeniden görev isteyebilirsin.\n\r",ch);
	}
        else if (IS_SET(ch->act,PLR_QUESTOR))
        {
	    if (--ch->pcdata->countdown <= 0)
	    {
		send_to_char("Görev süren doldu!\n\rYeni bir görev isteyebilirsin.\n\r", ch);
		quest_clear(ch, 0);
	    }
	    else if (ch->pcdata->countdown < 6)
		send_to_char("Acele et, görev süren dolmak üzere!\n\r",ch);
        }
    }
}

void do_tell_quest( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument )
{
    act_color("$N: $C$t$c",ch,argument,victim,TO_CHAR,POS_DEAD, CLR_MAGENTA_BOLD );
}

bool gorev_ekipmani_mi( OBJ_DATA *obj )
{
    const struct quest_item *qi;

    for (qi = quest_items; qi->keys != NULL; qi++)
	if (obj->pIndexData->vnum == qi->vnum)
	    return TRUE;
    return FALSE;
}

/* do_eniyi: vücut bölgesi adları */
static const struct
{
    const char *name;
    int		flag;
} eniyi_slots[] =
{
    { "parmak",	  ITEM_WEAR_FINGER },
    { "boyun",	  ITEM_WEAR_NECK   },
    { "gövde",	  ITEM_WEAR_BODY   },
    { "kafa",	  ITEM_WEAR_HEAD   },
    { "bacaklar", ITEM_WEAR_LEGS   },
    { "ayaklar",  ITEM_WEAR_FEET   },
    { "eller",	  ITEM_WEAR_HANDS  },
    { "kollar",	  ITEM_WEAR_ARMS   },
    { "vücut",	  ITEM_WEAR_ABOUT  },
    { "bel",	  ITEM_WEAR_WAIST  },
    { "bilek",	  ITEM_WEAR_WRIST  },
    { "süzülen",  ITEM_WEAR_FLOAT  },
    { NULL, 0 }
};

static void eniyi_tell_list(CHAR_DATA *ch, CHAR_DATA *qm, const struct top3 *t, const char *intro)
{
    int i;

    for (i = 0; i < 3; i++)
    {
	if (t->obj[i] == NULL)
	    continue;
	if (intro != NULL && i == 0)
	    do_tell_quest(ch, qm, intro);
	quest_tellf(ch, qm, "[%s] %s", obj_area_name(t->obj[i]), t->obj[i]->short_descr);
    }
}

void do_eniyi(CHAR_DATA *ch,char *argument)
{
    CHAR_DATA *questman;
    char arg1 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    AFFECT_DATA *paf;
    struct top3 hitdam, armor;
    int i, bolge, cost = ENIYI_COST;

    if ((questman = find_questmaster(ch->in_room)) == NULL)
    {
        send_to_char("Odada bu işleri yapan bir görevci göremiyorum.\n\r", ch);
        return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char("Hangi bölgene giyeceğin ekipmanlar hakkında bilgi istiyorsun?\n\r",ch);
	return;
    }
    argument = one_argument(argument, arg1);

    for (i = 0; eniyi_slots[i].name != NULL; i++)
	if (!str_cmp(arg1, eniyi_slots[i].name))
	    break;
    if (eniyi_slots[i].name == NULL)
    {
	send_to_char("Vücudunda böyle bir bölge göremiyorum!\n\r",ch);
	return;
    }
    bolge = eniyi_slots[i].flag;

    if (number_percent() <= get_skill(ch, gsn_haggle))
	cost = number_percent() > 90 ? 20 : 80;

    if (ch->silver < cost)
    {
	send_to_char("Yeterli akçen yok, bilgi veremem.\n\r",ch);
	return;
    }
    ch->silver -= cost;
    printf_to_char(ch,"Aldığın hizmet için %d akçe ödüyorsun.\n\r", cost);

    memset(&hitdam, 0, sizeof(hitdam));
    memset(&armor, 0, sizeof(armor));

    act("$n $Z ekipman bilgisi istiyor.", ch, NULL, questman, TO_ROOM);

    for( obj=object_list; obj!=NULL; obj = obj->next )
    {
	int zararzari = 0;

	/* limit ve görev eşyalarını söylemesin */
	if (obj->pIndexData->limit != -1 || gorev_ekipmani_mi(obj))
	    continue;
	if (!CAN_WEAR(obj,ITEM_TAKE) || !CAN_WEAR(obj, bolge) || obj->level > ch->level)
	    continue;

	if (!obj->enchanted)
	{
	    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
		if ( paf->modifier != 0
		&&   (paf->location == APPLY_DAMROLL || paf->location == APPLY_HITROLL) )
		    zararzari += paf->modifier;
	    top3_insert(&hitdam, obj, zararzari);
	}

	if (obj->item_type == ITEM_ARMOR)
	    top3_insert(&armor, obj,
			obj->value[0] + obj->value[1] + obj->value[2] + obj->value[3]);
    }

    if (hitdam.obj[0] == NULL && armor.obj[0] == NULL)
    {
	do_tell_quest(ch,questman,"Şu an birşey hatırlayamıyorum. Sanırım yaşlanıyorum.");
	do_tell_quest(ch,questman,"Daha sonra tekrar uğra lütfen.");
	ch->silver += cost;
	printf_to_char(ch,"%d akçeni geri alıyorsun.\n\r", cost);
	return;
    }
    do_tell_quest(ch,questman,"Bir düşüneyim... Evet sanırım birşeyler hatırladım.");
    do_tell_quest(ch,questman,"Bazı ekipmanlar hatırlıyorum, senin giyebileceğin seviyede ekipmanlar.");

    eniyi_tell_list(ch, questman, &hitdam, "Vuruşlarının gücünü ve isabetini artıracak ekipmanlar. Mesela...");
    eniyi_tell_list(ch, questman, &armor, "Bir de seni koruyacak ekipmanlar var aklıma gelen. Mesela...");
}
