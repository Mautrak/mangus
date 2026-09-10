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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"
#include "recycle.h"
#include "prog_util.h"

void raw_kill (CHAR_DATA *victim);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_cb);
DECLARE_DO_FUN(do_murder);
DECLARE_DO_FUN(do_help);
int find_path( int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, int depth, int in_zone );
int lookup_religion_name (const char *name);
void heal_battle(CHAR_DATA *mob,CHAR_DATA *ch );
void	say_spell	( CHAR_DATA *ch, int sn );
void	one_hit		( CHAR_DATA *ch, CHAR_DATA *victim, int dt ,bool secondary);

DECLARE_MPROG_FUN_GREET( greet_prog_ruler_pre   );
DECLARE_MPROG_FUN_GREET( greet_prog_ruler 	);
DECLARE_MPROG_FUN_GREET( greet_prog_shalafi 	);
DECLARE_MPROG_FUN_GREET( greet_prog_battle 	);
DECLARE_MPROG_FUN_GREET( greet_prog_chaos 	);
DECLARE_MPROG_FUN_GREET( greet_prog_knight 	);
DECLARE_MPROG_FUN_GREET( greet_prog_invader 	);
DECLARE_MPROG_FUN_GREET( greet_prog_lions 	);
DECLARE_MPROG_FUN_GREET( greet_prog_hunter 	);
DECLARE_MPROG_FUN_SPEECH(speech_prog_hunter_cleric);

DECLARE_MPROG_FUN_BRIBE( bribe_prog_cityguard	);

DECLARE_MPROG_FUN_SPEECH( speech_prog_keeper 	);
DECLARE_MPROG_FUN_GREET( greet_prog_fireflash 	);
DECLARE_MPROG_FUN_GREET( greet_prog_solamnia 	);
DECLARE_MPROG_FUN_GIVE( give_prog_solamnia 	);
DECLARE_MPROG_FUN_GIVE( give_prog_fireflash 	);
DECLARE_MPROG_FUN_GIVE( give_prog_keeper 	);
DECLARE_MPROG_FUN_GIVE( give_prog_dressmaker 	);
DECLARE_MPROG_FUN_GREET( greet_prog_keeper 	);

DECLARE_MPROG_FUN_DEATH( death_prog_stalker 	);

DECLARE_MPROG_FUN_GREET( greet_prog_templeman 	);
DECLARE_MPROG_FUN_SPEECH( speech_prog_templeman );
DECLARE_MPROG_FUN_FIGHT( fight_prog_diana  	);
DECLARE_MPROG_FUN_FIGHT( fight_prog_ofcol_guard );
DECLARE_MPROG_FUN_SPEECH( speech_prog_wiseman	);

DECLARE_MPROG_FUN_GREET( greet_prog_armourer	);
DECLARE_MPROG_FUN_GREET( greet_prog_baker	);
DECLARE_MPROG_FUN_GREET( greet_prog_beggar	);
DECLARE_MPROG_FUN_GREET( greet_prog_drunk	);
DECLARE_MPROG_FUN_GREET( greet_prog_grocer	);

DECLARE_MPROG_FUN_BRIBE( bribe_prog_beggar 	);
DECLARE_MPROG_FUN_BRIBE( bribe_prog_drunk 	);

DECLARE_MPROG_FUN_FIGHT( fight_prog_beggar 	);

DECLARE_MPROG_FUN_DEATH( death_prog_beggar 	);
DECLARE_MPROG_FUN_DEATH( death_prog_vagabond	);

DECLARE_MPROG_FUN_SPEECH( speech_prog_crier	);

DECLARE_MPROG_FUN_AREA( area_prog_drunk		);
DECLARE_MPROG_FUN_AREA( area_prog_janitor	);
DECLARE_MPROG_FUN_AREA( area_prog_vagabond	);
DECLARE_MPROG_FUN_AREA( area_prog_baker		);
DECLARE_MPROG_FUN_AREA( area_prog_grocer	);
DECLARE_MPROG_FUN_FIGHT( fight_prog_golem  	);

/* speech_prog_templeman durumları */
#define GIVE_HELP_RELIGION	16
#define RELIG_CHOSEN		17

/* Ölümden sonra kabal karargâhına giremeyen "hayalet" süresi (saniye) */
#define GHOST_SECONDS		600

/* Keeper / FireFlash / Solamnia / terzi görev zinciri (limbo.are, ejderha_helezonu.are) */
#define OBJ_VNUM_KASSANDRA_STONE	89
#define OBJ_VNUM_KEEPER_DRESS		90
#define OBJ_VNUM_TURKISH_RUG		91
#define OBJ_VNUM_SILK_BOLT		2436
#define OBJ_VNUM_XXX_KEY		2438

/* Yeni Ofcol şehir muhafızları (yeni_ofcol.are) */
#define MOB_VNUM_OFCOL_GUARD		600
#define MOB_VNUM_OFCOL_CAPTAIN		603

/*
 * Ortak prog çekirdeği (obj_prog.c de kullanır).
 */
const struct prog_entry *prog_lookup(char kind, int vnum, const struct prog_type *types,
                                     const struct prog_entry *table,
                                     const char *progtype, const char *name)
{
    const struct prog_type *t;
    const struct prog_entry *e;

    for (t = types; t->name != NULL; t++)
	if (!str_cmp(progtype, t->name))
	    break;
    if (t->name == NULL)
    {
	if (kind == 'M')
	    bug("Load_mprogs: 'M': invalid program type for vnum %d", vnum);
	else
	    bug("Load_oprogs: 'O': invalid program type for vnum %d", vnum);
	exit(1);
    }

    for (e = table; e->name != NULL; e++)
	if (e->bit == t->bit && !str_cmp(name, e->name))
	    return e;

    if (kind == 'M')
	bug("Load_mprogs: 'M': Function not found for vnum %d", vnum);
    else
	bug("Load_oprogs: 'O': Function not found for vnum %d", vnum);
    exit(1);
}

/* get_obj_index NULL dönerse çökmek yerine günlüğe yazıp NULL döner. */
OBJ_DATA *prog_create_object(int vnum, int level)
{
    OBJ_INDEX_DATA *pObjIndex = get_obj_index(vnum);

    if (pObjIndex == NULL)
    {
	bug("prog_create_object: vnum %d tanımlı değil.", vnum);
	return NULL;
    }
    return create_object(pObjIndex, level);
}

void prog_set_owner(OBJ_DATA *obj, const char *name)
{
    free_string(obj->owner);
    obj->owner = str_dup(name);
}

/* Sahiplik: obj->owner doluysa onunla; yoksa kısa tanımda ya da ek açıklamada
 * tam ad eşleşmesi ("Ali", "Alican"ın eşyasını tanımaz). */
bool prog_obj_owned_by(const OBJ_DATA *obj, const CHAR_DATA *ch)
{
    if (obj == NULL || ch == NULL)
	return FALSE;
    if (obj->owner != NULL && obj->owner[0] != '\0')
	return !str_cmp(obj->owner, ch->name);
    if (prog_name_in_text(obj->short_descr, ch->name))
	return TRUE;
    return obj->extra_descr != NULL
	&& prog_name_in_text(obj->extra_descr->description, ch->name);
}

/*
 * Yaratık prog tablosu: yeni prog eklemek için işlevi yazıp buraya bir satır ekle.
 */
static const struct prog_type mprog_types[] =
{
    { "bribe_prog",  MPROG_BRIBE  },
    { "entry_prog",  MPROG_ENTRY  },
    { "greet_prog",  MPROG_GREET  },
    { "give_prog",   MPROG_GIVE   },
    { "fight_prog",  MPROG_FIGHT  },
    { "death_prog",  MPROG_DEATH  },
    { "area_prog",   MPROG_AREA   },
    { "speech_prog", MPROG_SPEECH },
    { NULL, 0 }
};

#define MPROG(bit, fun)	{ #fun, bit, (void (*)(void)) fun }

static const struct prog_entry mprog_table[] =
{
    MPROG(MPROG_BRIBE,  bribe_prog_cityguard),
    MPROG(MPROG_BRIBE,  bribe_prog_beggar),
    MPROG(MPROG_BRIBE,  bribe_prog_drunk),

    MPROG(MPROG_GREET,  greet_prog_shalafi),
    MPROG(MPROG_GREET,  greet_prog_knight),
    MPROG(MPROG_GREET,  greet_prog_lions),
    MPROG(MPROG_GREET,  greet_prog_hunter),
    MPROG(MPROG_GREET,  greet_prog_invader),
    MPROG(MPROG_GREET,  greet_prog_ruler),
    MPROG(MPROG_GREET,  greet_prog_ruler_pre),
    MPROG(MPROG_GREET,  greet_prog_battle),
    MPROG(MPROG_GREET,  greet_prog_chaos),
    MPROG(MPROG_GREET,  greet_prog_fireflash),
    MPROG(MPROG_GREET,  greet_prog_solamnia),
    MPROG(MPROG_GREET,  greet_prog_keeper),
    MPROG(MPROG_GREET,  greet_prog_templeman),
    MPROG(MPROG_GREET,  greet_prog_armourer),
    MPROG(MPROG_GREET,  greet_prog_baker),
    MPROG(MPROG_GREET,  greet_prog_beggar),
    MPROG(MPROG_GREET,  greet_prog_drunk),
    MPROG(MPROG_GREET,  greet_prog_grocer),

    MPROG(MPROG_FIGHT,  fight_prog_diana),
    MPROG(MPROG_FIGHT,  fight_prog_ofcol_guard),
    MPROG(MPROG_FIGHT,  fight_prog_beggar),
    MPROG(MPROG_FIGHT,  fight_prog_golem),

    MPROG(MPROG_DEATH,  death_prog_stalker),	/* TRUE dönerse ölümü engeller */
    MPROG(MPROG_DEATH,  death_prog_beggar),
    MPROG(MPROG_DEATH,  death_prog_vagabond),

    MPROG(MPROG_AREA,   area_prog_drunk),
    MPROG(MPROG_AREA,   area_prog_janitor),
    MPROG(MPROG_AREA,   area_prog_vagabond),
    MPROG(MPROG_AREA,   area_prog_baker),
    MPROG(MPROG_AREA,   area_prog_grocer),

    MPROG(MPROG_SPEECH, speech_prog_keeper),
    MPROG(MPROG_SPEECH, speech_prog_templeman),
    MPROG(MPROG_SPEECH, speech_prog_wiseman),
    MPROG(MPROG_SPEECH, speech_prog_crier),
    MPROG(MPROG_SPEECH, speech_prog_hunter_cleric),

    MPROG(MPROG_GIVE,   give_prog_fireflash),
    MPROG(MPROG_GIVE,   give_prog_solamnia),
    MPROG(MPROG_GIVE,   give_prog_keeper),
    MPROG(MPROG_GIVE,   give_prog_dressmaker),

    { NULL, 0, NULL }
};

void mprog_set(MOB_INDEX_DATA *mobindex, const char *progtype, const char *name)
{
    const struct prog_entry *e =
	prog_lookup('M', mobindex->vnum, mprog_types, mprog_table, progtype, name);
    MPROG_DATA *p = mobindex->mprogs;

    switch (e->bit)
    {
    case MPROG_BRIBE:  p->bribe_prog  = (MPROG_FUN_BRIBE *)  e->fun; break;
    case MPROG_ENTRY:  p->entry_prog  = (MPROG_FUN_ENTRY *)  e->fun; break;
    case MPROG_GREET:  p->greet_prog  = (MPROG_FUN_GREET *)  e->fun; break;
    case MPROG_GIVE:   p->give_prog   = (MPROG_FUN_GIVE *)   e->fun; break;
    case MPROG_FIGHT:  p->fight_prog  = (MPROG_FUN_FIGHT *)  e->fun; break;
    case MPROG_DEATH:  p->death_prog  = (MPROG_FUN_DEATH *)  e->fun; break;
    case MPROG_AREA:   p->area_prog   = (MPROG_FUN_AREA *)   e->fun; break;
    case MPROG_SPEECH: p->speech_prog = (MPROG_FUN_SPEECH *) e->fun; break;
    }
    SET_BIT(mobindex->progtypes, e->bit);
}

/*
 * Yerel yardımcılar
 */

/* Kabal karargâhı muhafızı: üyeyi selamlar, hayaleti öldürür, davetsiz misafiri
 * kabala duyurur. welcome NULL ise üyenin önünde eğilir (Yargıçlar). */
static void cabal_guard_greet(CHAR_DATA *mob, CHAR_DATA *ch, int cabal, char *welcome)
{
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
	return;

    mob->cabal = cabal;
    SET_BIT(mob->off_flags, OFF_AREA_ATTACK);

    if (ch->cabal == cabal)
    {
	if (welcome != NULL)
	    do_say(mob, welcome);
	else
	{
	    snprintf(buf, sizeof(buf), "eğil %s", ch->name);
	    interpret(mob, buf, FALSE);
	}
	return;
    }

    if (ch->last_death_time != -1 && current_time - ch->last_death_time < GHOST_SECONDS)
    {
	do_say(mob, "Hayaletler buraya giremez.");
	do_slay(mob, ch->name);
	return;
    }

    if (IS_IMMORTAL(ch))
	return;

    do_cb(mob, "Dikkat!!! Davetsiz Misafir!!!");
    do_say(mob, "Kabalımı rahatsız etmemeliydin!");
}

/* Esnafın hitabı: aynı memleketten olana adıyla, başkasına "yolcu" der. */
static const char *greet_name(CHAR_DATA *mob, CHAR_DATA *ch)
{
    if (mob->in_room == NULL
    ||  str_cmp(mob->in_room->area->name, hometown_table[ch->hometown].name))
	return "yolcu";
    return ch->name;
}

/* Esnaf greet'lerinin ortak ön koşulu */
static bool shop_greet_ok(CHAR_DATA *mob, CHAR_DATA *ch)
{
    return can_see(mob, ch) && !IS_NPC(ch) && !IS_IMMORTAL(ch);
}

/* Avcı Kılıcı: ismi alan dosyasındaki şablona güvenle yerleştirilir. */
static OBJ_DATA *make_eyed_sword(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *eyed;
    EXTRA_DESCR_DATA *ed;
    int i = IS_GOOD(ch) ? 0 : IS_EVIL(ch) ? 2 : 1;

    if ((eyed = prog_create_object(OBJ_VNUM_EYED_SWORD, 0)) == NULL)
	return NULL;

    prog_set_owner(eyed, ch->name);
    free_string(eyed->from);
    eyed->from = str_dup(ch->name);
    eyed->altar = hometown_table[ch->hometown].altar[i];
    eyed->pit = hometown_table[ch->hometown].pit[i];

    prog_subst(buf, sizeof(buf), eyed->short_descr, ch->name, NULL);
    free_string(eyed->short_descr);
    eyed->short_descr = str_dup(buf);

    if ((ed = eyed->pIndexData->extra_descr) != NULL)
    {
	prog_subst(buf, sizeof(buf), ed->description, ch->name, NULL);
	eyed->extra_descr = new_extra_descr();
	eyed->extra_descr->keyword = str_dup(ed->keyword);
	eyed->extra_descr->description = str_dup(buf);
	eyed->extra_descr->next = NULL;
    }

    eyed->value[2] = (ch->level / 10) + 3;
    eyed->level = ch->level;
    eyed->cost = 0;
    return eyed;
}

/* Şehir muhafızlarını mob'un odasına çağırır; odaya varan (ya da zaten orada olan)
 * muhafız arrive() ile hazırlanıp ch'ye saldırır. */
static void guard_rally(CHAR_DATA *mob, CHAR_DATA *ch, bool with_captain, bool skip_last_fought,
			void (*arrive)(CHAR_DATA *guard, CHAR_DATA *ch),
			char *yell_far, char *say_far)
{
    CHAR_DATA *ach, *ach_next;
    int vnum, door;

    if (mob->in_room == NULL)
	return;

    for (ach = char_list; ach != NULL; ach = ach_next)
    {
	ach_next = ach->next;
	if (!IS_NPC(ach) || ach->in_room == NULL || ach->in_room->area != mob->in_room->area)
	    continue;
	vnum = ach->pIndexData->vnum;
	if (vnum != MOB_VNUM_OFCOL_GUARD && !(with_captain && vnum == MOB_VNUM_OFCOL_CAPTAIN))
	    continue;
	if (ach->fighting || (skip_last_fought && ach->last_fought))
	    continue;

	if (ach->in_room == mob->in_room)
	{
	    arrive(ach, ch);
	    do_murder(ach, ch->name);
	    continue;
	}

	door = find_path(ach->in_room->vnum, mob->in_room->vnum, ach, -40, 1);
	if (door == -1)
	{
	    bug("Couldn't find a path with -40", 0);
	    continue;
	}
	if (number_percent() < 25)
	    do_yell(ach, yell_far);
	else
	    do_say(ach, say_far);
	move_char(ach, door, FALSE);
    }
}

/*
 * Prog işlevleri
 */

void bribe_prog_cityguard(CHAR_DATA *mob, CHAR_DATA *ch, int amount)
{
  if (amount < 100)
    {
      do_say(mob, "Cimri!!!");
      do_murder(mob, ch->name);
    }
  else if (amount >= 5000)
    {
      interpret(mob, "gülümse", FALSE);
      do_sleep(mob, "");
    }
  else
      do_say(mob,"Bana rüşvet mi öneriyorsun? Pekala, ama bu sana pahalıya patlar!");
}

void greet_prog_shalafi(CHAR_DATA *mob,CHAR_DATA *ch)
{
  cabal_guard_greet(mob, ch, CABAL_SHALAFI, "Selamlar arif olan.");
}

void greet_prog_invader(CHAR_DATA *mob,CHAR_DATA *ch)
{
  cabal_guard_greet(mob, ch, CABAL_INVADER, "Selamlar karanlığın ta kendisi!");
}

void greet_prog_ruler_pre(CHAR_DATA *mob,CHAR_DATA *ch)
{
  char buf[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) )
    return;

  if (ch->cabal == CABAL_RULER) {
    snprintf(buf, sizeof(buf),"eğil %s",ch->name);
    interpret(mob,buf, FALSE);
    return;
  }

  do_say( mob, "Daha fazla ilerlemesen iyi olur." );
  do_say( mob, "Bu bölge özel kullanıma aittir." );
}

void greet_prog_ruler(CHAR_DATA *mob,CHAR_DATA *ch)
{
  cabal_guard_greet(mob, ch, CABAL_RULER, NULL);
}

void greet_prog_chaos(CHAR_DATA *mob,CHAR_DATA *ch)
{
  cabal_guard_greet(mob, ch, CABAL_CHAOS, "Selamlar kaosun efendisi!");
}

void greet_prog_battle(CHAR_DATA *mob, CHAR_DATA *ch)
{
  cabal_guard_greet(mob, ch, CABAL_BATTLE, "Selamlar yüce savaşçı!");
}

void greet_prog_knight(CHAR_DATA *mob, CHAR_DATA *ch)
{
  cabal_guard_greet(mob, ch, CABAL_KNIGHT, "Merhaba onurlu olan!");
}

void greet_prog_lions(CHAR_DATA *mob, CHAR_DATA *ch)
{
  cabal_guard_greet(mob, ch, CABAL_LIONS, "Hoşgeldiniz aslan efendiler.");
}

void greet_prog_hunter(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_INPUT_LENGTH];
  OBJ_DATA *eyed;

  cabal_guard_greet(mob, ch, CABAL_HUNTER, "Merhaba sevgili avcı.");

  /* üyeye ilk gelişinde Avcı Kılıcı verilir */
  if (IS_NPC(ch) || ch->cabal != CABAL_HUNTER || IS_SET(ch->quest, QUEST_EYE))
    return;

  if ((eyed = make_eyed_sword(ch)) == NULL)
    return;
  SET_BIT(ch->quest, QUEST_EYE);
  obj_to_char(eyed, mob);
  interpret(mob, "emote bir Avcı Kılıcı yaratıyor.", FALSE);
  do_say(mob, "Sana avcının kılıcını veriyorum.");
  snprintf(buf, sizeof(buf), "give eyed %s", ch->name);
  interpret(mob, buf, FALSE);
  do_say(mob, "Şunu unutma ki, eğer onu kaybedersen kabal şifacısından yeni bir tane isteyebilirsin.");
  do_say(mob, "Ona sadece 'felaket' de.");
}

void give_prog_keeper(CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj)
{
  char buf[MAX_INPUT_LENGTH];

  if (obj->pIndexData->vnum == OBJ_VNUM_KEEPER_DRESS)
    {
      do_say(mob,"İşte beklediğim giysi!");
      act("$n giysiyi masanın altına sokuşturuyor.",mob,NULL,NULL,TO_ROOM);
      obj_from_char(obj);
      extract_obj(obj);
      if (get_obj_carry(ch, "rug") != NULL)
	{
	  do_say(mob, "Sanırım FireFlash'i ziyaret etmek isteyeceksin.");
	  do_say(mob, "Dikkatli ol, canı biraz sıkkın.");
	  do_unlock (mob, "door");
	  do_open (mob, "door");
	}
      else
	{
	  do_say(mob,"Sanırım FireFlash ile işin yok.");
	  do_say(mob,"Sanırım şimdi buradan ayrılacak ve eksiklerini tamamlayacaksın.");
	}
    }
  else
    {
      snprintf(buf, sizeof(buf),"%s %s",obj->name,ch->name);
      do_give(mob,buf);
      do_say(mob,"Neden buna ihtiyacım olsun?");
    }
}

void speech_prog_keeper(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
  OBJ_DATA *obj;

  if (str_cmp(speech, "keeper") || IS_NPC(ch))
    return;
  if ((obj = prog_create_object(OBJ_VNUM_KEEPER_DRESS, 0)) == NULL)
    return;
  free_string(obj->name);
  obj->name = str_dup("keeper dress");
  act("$n ipekten beyaz bir giysi hazırlıyor.", mob, NULL, NULL, TO_ROOM);
  act("Keeper için beyaz bir giysi hazırlıyorsun.", mob, NULL, NULL, TO_CHAR);
  do_say(mob,"İşte Keeper için bir giysi.");
  obj_to_char(obj, ch);
}

void greet_prog_fireflash(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_INPUT_LENGTH];

  if (!shop_greet_ok(mob, ch))
    return;

  if (get_obj_carry(ch, "rug") == NULL)
    {
      do_say(mob,"Bu değersiz halıyı yakınlarımda görmek istemiyorum.");
      do_say(mob, "Neden onu Tear'dan aptal rahibeye vermiyorsun?");
      do_unlock(mob, "box");
      do_open(mob, "box");
      do_get(mob, "papers box");
      do_say(mob,"Bu kağıtların sana yardımı olabilir.");
      act("$n sana dudak büküyor.", mob, NULL, ch, TO_VICT);
      act("$E dudak büküyorsun.", mob, NULL, ch, TO_CHAR);
      act("$n $E dudak büküyor.", mob, NULL, ch, TO_NOTVICT);
      snprintf(buf, sizeof(buf), "papers %s", ch->name);
      do_give(mob, buf);
      do_close(mob, "box");
      do_lock(mob, "box");
    }
}

void give_prog_fireflash(CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj)
{
  char buf[MAX_INPUT_LENGTH];

  if (!can_see(mob,ch))
    do_say(mob,"Orada biri mi var?");
  else if (IS_NPC(ch))
    do_say(mob,"Birşeyler getiren bir hayvan... Ne kadar garip!");
  else if (obj->pIndexData->vnum != OBJ_VNUM_TURKISH_RUG)
    {
      do_say(mob,"Ne kadar ilginç!... Nedir bu?");
      interpret(mob, "kıkırda", FALSE);
      snprintf(buf, sizeof(buf),"%s %s",obj->name,ch->name);
      do_give(mob,buf);
    }
  else
    {
      do_say(mob, "Ne kadar şahane bir halı! Acaba nereye koysam...");
      act("$n odayı dolanarak kendi kendine mırıldanıyor.", mob, NULL, NULL, TO_ROOM);
      act("$n ellerini cebine sokuyor.",mob,NULL,NULL,TO_ROOM);
      snprintf(buf, sizeof(buf), "obj %d", OBJ_VNUM_XXX_KEY);
      do_load(mob, buf);
      do_say(mob,"Nedir bu? Bir anahtar? Hmm, geri alabilirsin.");
      snprintf(buf, sizeof(buf), "xxx %s",ch->name);
      do_give(mob, buf);
      act("$n dalgınlıkla halıyı sandalyenin altına itiyor.",mob,NULL,NULL,TO_ROOM);
      obj_from_char(obj);
      extract_obj(obj);
    }
}

void greet_prog_solamnia(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if (!shop_greet_ok(mob, ch))
    return;

  if (get_obj_carry(ch, "xxx") != NULL)
    {
      do_say(mob,"Sanırım benim için birşey getirdin.");
      interpret(mob,"gülümse", FALSE);
    }
}

void give_prog_solamnia(CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj)
{
  char buf[MAX_INPUT_LENGTH];
  OBJ_DATA *kassandra;

  if (obj->pIndexData->vnum != OBJ_VNUM_XXX_KEY)
    return;
  if ((kassandra = prog_create_object(OBJ_VNUM_KASSANDRA_STONE, 0)) == NULL)
    return;
  do_say(mob,"İşte ödülün!");
  kassandra->timer = 500;
  obj_to_char(kassandra, mob);
  snprintf(buf, sizeof(buf),"kasandra %s",ch->name);
  do_give(mob, buf);
  do_say(mob,"Bu taşın özel güçleri vardır, dikkatli kullan.");
  obj_from_char(obj);
  extract_obj(obj);
}

bool death_prog_stalker(CHAR_DATA *mob)
{
  char buf[MAX_INPUT_LENGTH];

  mob->cabal = CABAL_RULER;
  if (mob->last_fought == NULL)
    return FALSE;
  snprintf(buf, sizeof(buf), "%s adlı düşmanı öldürmeyi beceremedim, son nefesimi vermeliyim.",
	   mob->last_fought->name);
  do_cb(mob, buf);
  return FALSE;
}

void give_prog_dressmaker(CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj)
{
  if (IS_NPC(ch))
    return;

  if (!can_see(mob, ch))
    {
      do_say(mob,"Bu nereden geldi?");
      return;
    }

  if (obj->pIndexData->vnum != OBJ_VNUM_SILK_BOLT)
    {
      do_say(mob,"Bu işime yaramaz. Bana ipek lazım.");
      do_drop(mob, obj->name);
      return;
    }

  do_say(mob,"Bu elbiseyi kime yapıyorum?");
  obj_from_char(obj);
  extract_obj(obj);
}

void greet_prog_keeper(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if (IS_NPC(ch) || !can_see(mob, ch))
    return;

  do_say(mob,"Burada ne işin var? O şey sipariş ettiğim giysi mi?");
}

void speech_prog_templeman(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
	char buf[MAX_INPUT_LENGTH];
	int chosen = 0;
	int status;

	if ( !str_cmp( speech, "din" )|| !str_cmp(speech,"dinler") )
		status = GIVE_HELP_RELIGION;
	else if (( chosen = lookup_religion_name( speech)) != 0 )
		status = RELIG_CHOSEN;
	else
		return;

	if ( status == RELIG_CHOSEN )
	{
		if (( ch->religion > 0) && (ch->religion < MAX_RELIGION) )
		{
			snprintf(buf, sizeof(buf),"Zaten %s yolundasın.", religion_table[ch->religion].name);
			do_say(mob,buf);
			return;
		}

		ch->religion = chosen;
		snprintf(buf, sizeof(buf),"Bundan böyle sonsuza kadar %s yolundasın.",religion_table[ch->religion].name);
		do_say(mob,buf);
		return;
	}
	do_say(mob,"Hmmm... Eveeett.. Din.. Bununla gerçekten ilgileniyor musun?");
	do_say(mob,"Bildiğin gibi bu diyarda dört din vardır.");
	do_say(mob,"Kame efendinin dini Kamenilik.");
	do_say(mob,"Nir efendinin dini Niryanilik. Nyahilik ve Sintiyanlık.");
	do_say(mob,"Nyah efendinin dini Nyahilik.");
	do_say(mob,"Sint efendinin dini Sintiyanlık.");
	do_say(mob,"Bana seçmek istediğin dinin efendisini söylemelisin.");
	do_say(mob,"Unutma ki dinini bir kez seçersin.");
	do_say(mob,"Din seçimi yeniyaşamdan sonra dahi değişmez.");
	do_say(mob,"Dinini değiştirmek istersen bunun için kimi görevleri bitirmen gerekir.");
}

void greet_prog_templeman(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char arg[MAX_INPUT_LENGTH];

  if (!shop_greet_ok(mob, ch))
    return;

  snprintf(arg, sizeof(arg),"gülümse %s",ch->name);
  interpret(mob, arg, FALSE);
}

int lookup_religion_name (const char *name)
{
	int value;

	for ( value = 1; value < MAX_RELIGION ; value++)
	{
		if (religion_table[value].leader != NULL
		&&  !str_cmp(name, religion_table[value].leader))
			return value;
	}

	return 0;
}

/* Diana'nın yardımına gelen muhafız tanrılarca güçlendirilir. */
static void diana_guard_arrives(CHAR_DATA *ach, CHAR_DATA *ch)
{
    int i;

    act_color("$C$n tanrıları yardımına çağırıyor.$c",
	      ach,NULL,NULL,TO_ROOM,POS_SLEEPING,CLR_BLUE);
    act_color("$CTanrılar $m Diana'ya yardım etmesi için geliştiriyor.$c",
	      ach,NULL,NULL,TO_ROOM,POS_SLEEPING,CLR_GREEN);
    ach->max_hit = 6000;
    ach->hit = 6000;
    ach->level = 60;
    ach->timer = 0;
    ach->damage[DICE_NUMBER] = number_range(3,5);
    ach->damage[DICE_TYPE] = number_range(12,22);
    ach->damage[DICE_BONUS] = number_range(6, 8);
    for (i = 0; i < MAX_STATS; i++)
	ach->perm_stat[i] = 23;
    do_say(ach,"İşte geldim Diana.");
}

void fight_prog_diana( CHAR_DATA *mob, CHAR_DATA *ch )
{
   if ( !mob->in_room || number_percent() < 25 ) return;
   if (mob->in_room->area != mob->zone) return;

   do_yell(mob,"Yardım edin nöbetçiler.");
   guard_rally(mob, ch, TRUE, TRUE, diana_guard_arrives,
	       " Dayan Diana! Geliyorum!", "Gidip Dianaya yardım etmeliyim.");
}

static void ofcol_guard_arrives(CHAR_DATA *ach, CHAR_DATA *ch)
{
    char buf[MAX_INPUT_LENGTH];

    snprintf(buf, sizeof(buf),"Şimdi %s, muhafızlara saldırmanın cezasını çekeceksin.",ch->name);
    do_say(ach,buf);
}

void fight_prog_ofcol_guard( CHAR_DATA *mob, CHAR_DATA *ch )
{
   char buf[MAX_INPUT_LENGTH];

   if (number_percent() < 25) return;
   snprintf(buf, sizeof(buf),"Yardım edin nöbetçiler! %s benimle dövüşüyor!",ch->name);
   do_yell(mob,buf);
   guard_rally(mob, ch, FALSE, FALSE, ofcol_guard_arrives,
	       " Dayan nöbetçi! Geliyorum!", "Gidip muhafıza yardım etmeliyim.");
}

void speech_prog_wiseman(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
 if (!str_cmp(speech,"bilge iyileştir şifa"))
	heal_battle(mob,ch);
}

void greet_prog_armourer(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_INPUT_LENGTH];

  if (!shop_greet_ok(mob, ch))
    return;
  interpret(mob,"gülümse", FALSE);
  snprintf(buf, sizeof(buf),"Zırhçıya hoşgeldin %s,", greet_name(mob, ch));
  do_say(mob,buf);
  do_say(mob,"Sana nasıl yardımcı olabilirim?");
  do_say(mob,"Dükkanımda gördüğün zırhların tümü çok kalitelidir.");
  interpret(mob,"emote gururla geriniyor.", FALSE);
}

void greet_prog_baker(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_INPUT_LENGTH];

  if (!shop_greet_ok(mob, ch))
    return;
  interpret(mob,"gülümse", FALSE);
  snprintf(buf, sizeof(buf),"Fırına hoşgeldin %s.", greet_name(mob, ch));
  do_say(mob,buf);
}

void greet_prog_beggar(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_INPUT_LENGTH];

  if (!shop_greet_ok(mob, ch))
    return;
  snprintf(buf, sizeof(buf),"Allah rızası için %s,", greet_name(mob, ch));
  do_say(mob,buf);
  do_say(mob,"Birkaç akçe ver...");
}

void greet_prog_drunk(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if (!shop_greet_ok(mob, ch))
    return;
  if (number_percent() < 5)
   {
     do_yell(mob,"Canavar! Bir canavar buldum! Saldırın!");
     do_murder(mob,ch->name);
   }
}

void greet_prog_grocer(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_INPUT_LENGTH];

  if (!shop_greet_ok(mob, ch))
    return;
  snprintf(buf, sizeof(buf),"Dükkanıma hoşgeldin %s.", greet_name(mob, ch));
  do_say(mob,buf);
}

void bribe_prog_beggar(CHAR_DATA *mob, CHAR_DATA *ch, int amount)
{
  char buf[MAX_INPUT_LENGTH];

  if (amount < 10)
    {
     snprintf(buf, sizeof(buf),"teşekkür %s", greet_name(mob, ch));
     interpret(mob,buf, FALSE);
    }
  else if (amount < 100)
      do_say(mob,"Ohaaa! Çok teşekkür ederim.");
  else if (amount < 500)
    {
      do_say(mob,"Tanrıma şükürler olsun! Teşekkürler! Teşekkürler!");
      snprintf(buf, sizeof(buf),"öp %s",ch->name);
      interpret(mob,buf, FALSE);
    }
  else
    {
     snprintf(buf, sizeof(buf),"dans %s",ch->name);
     interpret(mob,buf, FALSE);
     snprintf(buf, sizeof(buf),"öp %s",ch->name);
     interpret(mob,buf, FALSE);
    }
}

void bribe_prog_drunk(CHAR_DATA *mob, CHAR_DATA *ch, int amount)
{
  do_say(mob,"Ahh! Daha fazla ruh! Güzel Ruhlar!");
  interpret(mob,"şarkı", FALSE);
}

void fight_prog_beggar(CHAR_DATA *mob, CHAR_DATA *ch)
{
  /* yp %45-%55 bandındayken */
  if (mob->hit > (mob->max_hit * 0.45) && mob->hit < (mob->max_hit * 0.55))
    do_say(mob,"İşte ölüyorum...");
}

bool death_prog_beggar(CHAR_DATA *mob)
{
  if (number_percent() < 50 )
    do_say(mob,"Her nereyse gittiğim yer, biliyorum ki buradan iyidir...");
  else
    do_say(mob,"Günahlarımı affet tanrım...");
  return FALSE;
}

bool death_prog_vagabond(CHAR_DATA *mob)
{
  interpret(mob,"emote kafasını arkaya atıp deli gibi kahkaha atıyor!", FALSE);
  return FALSE;
}

void speech_prog_crier(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
 char arg[MAX_INPUT_LENGTH];

 one_argument(speech,arg);
 if (is_name(arg,"ne"))
	do_say(mob,"Sevgilim beni terketti.");
}

void area_prog_drunk(CHAR_DATA *mob)
{
  if (number_percent() < 5)
    interpret(mob, "dans", FALSE);
  else if (number_percent() < 10)
    interpret(mob,"şarkı", FALSE);
}

void area_prog_janitor(CHAR_DATA *mob)
{
  if (number_percent() < 20)
   {
    interpret(mob,"grumble", FALSE);
    do_say(mob,"Çöpler");
    if (number_percent() < 20 )
     {
       do_say(mob,"Hergün yaptığım tek iş başkalarının pisliklerini temizlemek.");
      if (number_percent() < 20 )
	do_say(mob,"Yeterince kazanamıyorum.");
      else if (number_percent() < 20)
	   {
	    do_say(mob,"Gün başlıyor, gün bitiyor. Hiç aralıksız çöp topluyorum.");
	    if ( number_percent() < 10 )
	      do_yell(mob,"Tatil istiyorum!");
	   }
     }
   }
}

void area_prog_vagabond(CHAR_DATA *mob)
{
  if (number_percent() < 10)
    do_say(mob,"Kan! Zulüm!");
}

void area_prog_baker(CHAR_DATA *mob)
{
  if (number_percent() < 5)
    do_say(mob,"Kurabiyelerimin tadına bakmak ister misin?");
}

void area_prog_grocer(CHAR_DATA *mob)
{
  if (number_percent() < 5)
    do_say(mob, "Şu fenerin işçiliğine bakar mısın?");
}

void speech_prog_hunter_cleric(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
 char buf[MAX_STRING_LENGTH];
 OBJ_DATA *obj,*in_obj;
 bool matched = FALSE;

    if (str_cmp(speech,"felaket"))	return;

    if (ch->cabal != CABAL_HUNTER)
    {
      do_say(mob,"Ikınırsan belki...");
      return;
    }

    if (!IS_SET(ch->quest,QUEST_EYE))
    {
      do_say(mob,"Ne kastediyorsun?");
      return;
    }

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if (obj->pIndexData->vnum != OBJ_VNUM_EYED_SWORD || !prog_obj_owned_by(obj, ch))
	    continue;

	matched = TRUE;
	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL )
	{
	    if (in_obj->carried_by == ch)
	    {
		do_say(mob,"Benle dalga mı geçiyorsun? İşte kılıcın elinde...");
		do_smite(mob,ch->name);
		return;
	    }

	    snprintf(buf, sizeof(buf), "Kılıcını %s taşıyor!",
		     PERS(in_obj->carried_by, ch) );
	    do_say(mob, buf);
	    if ( in_obj->carried_by->in_room )
	    {
		snprintf(buf, sizeof(buf), "%s %s bölgesinde, %s civarlarında dolaşıyor!",
			 PERS(in_obj->carried_by, ch),
			 in_obj->carried_by->in_room->area->name,
			 in_obj->carried_by->in_room->name );
		do_say(mob, buf);
		return;
	    }
	}
	else if (in_obj->in_room != NULL)
	{
	    snprintf(buf, sizeof(buf), "Kılıcın %s bölgesinde, %s civarlarında!",
		     in_obj->in_room->area->name, in_obj->in_room->name );
	    do_say(mob,buf);
	    return;
	}

	extract_obj( obj );
	do_say( mob, "Sana yenisini vereceğim.");
	break;
    }

    if (!matched) do_say(mob,"Kılıcın kayıp!");

    if ((obj = make_eyed_sword(ch)) == NULL)
	return;
    interpret( mob, "emote bir Avcı Kılıcı yaratıyor.", FALSE);
    do_say( mob ,"Sana başka bir Avcı Kılıcı veriyorum.");
    act( "$N $e $p veriyor.", ch, obj, mob, TO_ROOM );
    act( "$N sana $p veriyor.",   ch, obj, mob, TO_CHAR );
    obj_to_char(obj, ch);
    do_say( mob , "Tekrar kaybetme!");
}

void fight_prog_golem( CHAR_DATA *mob, CHAR_DATA *ch)
{
    static const char *const golem_spells[] = {
	"curse", "weaken", "chill touch", "blindness", "poison",
	"energy drain", "harm", "teleport", "plague"
    };
    CHAR_DATA *master;
    CHAR_DATA *target;
    int roll, sn;

    if (mob->in_room == NULL)
	return;

    for ( master = mob->in_room->people; master != NULL; master = master->next_in_room )
    {
	if ( !IS_NPC(master) && mob->master == master &&
	     master->iclass == CLASS_NECROMANCER)
	    break;
    }

    if ( master == NULL || !master->fighting )
	return;

    if ( master->fighting->fighting == master)
	do_rescue(mob, master->name);

    roll = number_range(0,15);
    if ( roll >= (int) (sizeof(golem_spells) / sizeof(golem_spells[0]))
    ||   ( sn = skill_lookup(golem_spells[roll]) ) < 0 )
	return;

    target = (mob->fighting) ? mob->fighting : master->fighting;
    if (target)
     {
      say_spell(mob,sn);
      (*skill_table[sn].spell_fun) (sn,mob->level,mob,target,TARGET_CHAR);
     }
}
