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

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"
#include "recycle.h"
#include "turkish_suffix_helper.h"

void raw_kill args( (CHAR_DATA *victim) );
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_cb);
DECLARE_DO_FUN(do_murder);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_murder);
int find_path( int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, int depth, int in_zone );
int lookup_religion_name (const char *name);
void heal_battle(CHAR_DATA *mob,CHAR_DATA *ch );
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ,bool secondary) );

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


#define GIVE_HELP_RELIGION	16
#define RELIG_CHOSEN		17


void mprog_set(MOB_INDEX_DATA *mobindex,const char *progtype, const char *name)
{
   if (!str_cmp(progtype, "bribe_prog"))
     {
 /*
  *    if (!str_cmp(name, "bribe_prog"))
  *          mobindex->mprogs->bribe_prog = bribe_prog_;
  *    else if (!str_cmp(name, "bribe_prog")) ...
  */

       if (!str_cmp(name, "bribe_prog_cityguard"))
	 mobindex->mprogs->bribe_prog = bribe_prog_cityguard;
       else if (!str_cmp(name, "bribe_prog_beggar"))
	 mobindex->mprogs->bribe_prog = bribe_prog_beggar;
       else if (!str_cmp(name, "bribe_prog_drunk"))
	 mobindex->mprogs->bribe_prog = bribe_prog_drunk;

       else
	 {
	   bug("Load_mprogs: 'M': Function not found for vnum %d",
	       mobindex->vnum);
	   exit(1);
	 }

       SET_BIT(mobindex->progtypes, MPROG_BRIBE);
       return;
     }

   if (!str_cmp(progtype, "entry_prog"))
     {
/*
       if (!str_cmp(name, "entry_prog_"))
	 mobindex->mprogs->entry_prog = entry_prog_;
       else
*/
	 {
	   bug("Load_mprogs: 'M': Function not found for vnum %d",
	       mobindex->vnum);
	   exit(1);
	 }
       SET_BIT(mobindex->progtypes, MPROG_ENTRY);
       return;
     }
   if (!str_cmp(progtype, "greet_prog"))
     {
       if (!str_cmp(name, "greet_prog_shalafi"))
	 mobindex->mprogs->greet_prog = greet_prog_shalafi;
       else if (!str_cmp(name, "greet_prog_knight"))
	 mobindex->mprogs->greet_prog = greet_prog_knight;
       else if (!str_cmp(name, "greet_prog_lions"))
	 mobindex->mprogs->greet_prog = greet_prog_lions;
       else if (!str_cmp(name, "greet_prog_hunter"))
	 mobindex->mprogs->greet_prog = greet_prog_hunter;
       else if (!str_cmp(name, "greet_prog_invader"))
	 mobindex->mprogs->greet_prog = greet_prog_invader;
       else if (!str_cmp(name, "greet_prog_ruler"))
	 mobindex->mprogs->greet_prog = greet_prog_ruler;
       else if (!str_cmp(name, "greet_prog_ruler_pre"))
	 mobindex->mprogs->greet_prog = greet_prog_ruler_pre;
       else if (!str_cmp(name, "greet_prog_battle"))
	 mobindex->mprogs->greet_prog = greet_prog_battle;
       else if (!str_cmp(name, "greet_prog_chaos"))
	 mobindex->mprogs->greet_prog = greet_prog_chaos;
       else if (!str_cmp(name, "greet_prog_fireflash"))
	 mobindex->mprogs->greet_prog = greet_prog_fireflash;
       else if (!str_cmp(name, "greet_prog_solamnia"))
	 mobindex->mprogs->greet_prog = greet_prog_solamnia;
       else if (!str_cmp(name, "greet_prog_keeper"))
	 mobindex->mprogs->greet_prog = greet_prog_keeper;
       else if (!str_cmp(name, "greet_prog_templeman"))
	 mobindex->mprogs->greet_prog = greet_prog_templeman;
       else if (!str_cmp(name, "greet_prog_armourer"))
	 mobindex->mprogs->greet_prog = greet_prog_armourer;
       else if (!str_cmp(name, "greet_prog_baker"))
	 mobindex->mprogs->greet_prog = greet_prog_baker;
       else if (!str_cmp(name, "greet_prog_beggar"))
	 mobindex->mprogs->greet_prog = greet_prog_beggar;
       else if (!str_cmp(name, "greet_prog_drunk"))
	 mobindex->mprogs->greet_prog = greet_prog_drunk;
       else if (!str_cmp(name, "greet_prog_grocer"))
	 mobindex->mprogs->greet_prog = greet_prog_grocer;
       else
	 {
	   bug("Load_mprogs: 'M': Function not found for vnum %d",
	       mobindex->vnum);
	   exit(1);
	 }

       SET_BIT(mobindex->progtypes, MPROG_GREET);
       return;
     }

   if (!str_cmp(progtype, "fight_prog"))
     {
       if (!str_cmp(name, "fight_prog_diana"))
	 mobindex->mprogs->fight_prog = fight_prog_diana;
       else if (!str_cmp(name, "fight_prog_ofcol_guard"))
	 mobindex->mprogs->fight_prog = fight_prog_ofcol_guard;
       else if (!str_cmp(name, "fight_prog_beggar"))
	 mobindex->mprogs->fight_prog = fight_prog_beggar;
       else if (!str_cmp(name, "fight_prog_golem"))
	 mobindex->mprogs->fight_prog = fight_prog_golem;
       else
	 {
	   bug("Load_mprogs: 'M': Function not found for vnum %d",
	       mobindex->vnum);
	   exit(1);
	 }
       SET_BIT(mobindex->progtypes, MPROG_FIGHT);
       return;
     }
   if (!str_cmp(progtype, "death_prog")) /* returning TRUE prevents death */
     {
       if (!str_cmp(name, "death_prog_stalker"))
	 mobindex->mprogs->death_prog = death_prog_stalker;
       else if (!str_cmp(name, "death_prog_beggar"))
	 mobindex->mprogs->death_prog = death_prog_beggar;
       else if (!str_cmp(name, "death_prog_vagabond"))
	 mobindex->mprogs->death_prog = death_prog_vagabond;
       else
	 {
	   bug("Load_mprogs: 'M': Function not found for vnum %d",
	       mobindex->vnum);
	   exit(1);
	 }
       SET_BIT(mobindex->progtypes, MPROG_DEATH);
       return;
     }
   if (!str_cmp(progtype, "area_prog"))
     {
       if (!str_cmp(name, "area_prog_drunk"))
	 mobindex->mprogs->area_prog = area_prog_drunk;
       else if (!str_cmp(name, "area_prog_janitor"))
	 mobindex->mprogs->area_prog = area_prog_janitor;
       else if (!str_cmp(name, "area_prog_vagabond"))
	 mobindex->mprogs->area_prog = area_prog_vagabond;
       else if (!str_cmp(name, "area_prog_baker"))
	 mobindex->mprogs->area_prog = area_prog_baker;
       else if (!str_cmp(name, "area_prog_grocer"))
	 mobindex->mprogs->area_prog = area_prog_grocer;
       else
	 {
	   bug("Load_mprogs: 'M': Function not found for vnum %d",
	       mobindex->vnum);
	   exit(1);
	 }
       SET_BIT(mobindex->progtypes, MPROG_AREA);
       return;
     }
   if (!str_cmp(progtype, "speech_prog"))
     {
       if (!str_cmp(name, "speech_prog_keeper"))
	 mobindex->mprogs->speech_prog = speech_prog_keeper;
       else if(!str_cmp(name, "speech_prog_templeman"))
	 mobindex->mprogs->speech_prog = speech_prog_templeman;
       else if(!str_cmp(name, "speech_prog_wiseman"))
	 mobindex->mprogs->speech_prog = speech_prog_wiseman;
       else if(!str_cmp(name, "speech_prog_crier"))
	 mobindex->mprogs->speech_prog = speech_prog_crier;
       else if(!str_cmp(name, "speech_prog_hunter_cleric"))
	 mobindex->mprogs->speech_prog = speech_prog_hunter_cleric;
       else
	 {
	   bug("Load_mprogs: 'M': Function not found for vnum %d",
	       mobindex->vnum);
	   exit(1);
	 }
       SET_BIT(mobindex->progtypes, MPROG_SPEECH);
       return;
     }
   if (!str_cmp(progtype, "give_prog"))
     {
       if (!str_cmp(name, "give_prog_fireflash"))
	 mobindex->mprogs->give_prog = give_prog_fireflash;
       else if (!str_cmp(name, "give_prog_solamnia"))
	 mobindex->mprogs->give_prog = give_prog_solamnia;
       else if (!str_cmp(name, "give_prog_keeper"))
 	 mobindex->mprogs->give_prog = give_prog_keeper;
       else if (!str_cmp(name, "give_prog_dressmaker"))
	 mobindex->mprogs->give_prog = give_prog_dressmaker;
       else
	 {
	   bug("Load_mprogs: 'M': Function not found for vnum %d",
	       mobindex->vnum);
	   exit(1);
	 }

       SET_BIT(mobindex->progtypes, MPROG_GIVE);
       return;
     }
   bug( "Load_mprogs: 'M': invalid program type for vnum %d",mobindex->vnum);
   exit(1);
 }

void bribe_prog_cityguard(CHAR_DATA *mob, CHAR_DATA *ch, int amount)
{
  if (amount < 100)
    {
      do_say(mob, (char*)"Cimri!!!");
      do_murder(mob, ch->name);
      return;
    }
  else if (amount >= 5000)
    {
      interpret(mob, (char*)"gülümse", FALSE);
      do_sleep(mob, (char*)"");
      return;
    }
  else
    {
      do_say(mob,(char*)"Bana rüşvet mi öneriyorsun? Pekala, ama bu sana pahalıya patlar!");
      return;
    }
}

void greet_prog_shalafi(CHAR_DATA *mob,CHAR_DATA *ch)
{
  if ( IS_NPC(ch) )
    return;

  mob->cabal = CABAL_SHALAFI;
  SET_BIT(mob->off_flags,OFF_AREA_ATTACK);

  if (ch->cabal == CABAL_SHALAFI) {
    do_say(mob,(char*)"Selamlar arif olan.");
    return;
  }
  if (ch->last_death_time != -1 && current_time - ch->last_death_time < 600)
    {
      do_say(mob,(char*)"Hayaletler buraya giremez.");
      do_slay(mob, ch->name);
      return;
    }

  if (IS_IMMORTAL(ch))	return;

  do_cb(mob, (char*)"Dikkat!!! Davetsiz Misafir!!!");
  do_say(mob,(char*)"Kabalımı rahatsız etmemeliydin!");
}

void greet_prog_invader(CHAR_DATA *mob,CHAR_DATA *ch)
{
  if ( IS_NPC(ch) )
    return;

  mob->cabal = CABAL_INVADER;
  SET_BIT(mob->off_flags,OFF_AREA_ATTACK);

  if (ch->cabal == CABAL_INVADER) {
    do_say(mob, (char*)"Selamlar karanlığın ta kendisi!");
    return;
  }
  if (ch->last_death_time != -1 && current_time - ch->last_death_time < 600)
    {
      do_say(mob, (char*)"Hayaletler buraya giremez.");
      do_slay(mob, ch->name);
      return;
    }

  if (IS_IMMORTAL(ch))	return;

  do_cb(mob, (char*)"Dikkat!!! Davetsiz Misafir!!!");
  if (!IS_NPC(ch))  do_say(mob,(char*)"Kabalımı rahatsız etmemeliydin!");
}

void greet_prog_ruler_pre(CHAR_DATA *mob,CHAR_DATA *ch)
{
  char buf[100];

  if ( IS_NPC(ch) )
    return;

  if (ch->cabal == CABAL_RULER) {
    sprintf(buf,"eğil %s",ch->name);
    interpret(mob,buf, FALSE);
    return;
  }

  do_say( mob, (char*)"Daha fazla ilerlemesen iyi olur." );
  do_say( mob, (char*)"Bu bölge özel kullanıma aittir." );
  return;
}

void greet_prog_ruler(CHAR_DATA *mob,CHAR_DATA *ch)
{
  char buf[100];

  if ( IS_NPC(ch) )
    return;

  mob->cabal = CABAL_RULER;
  SET_BIT(mob->off_flags,OFF_AREA_ATTACK);

  if (ch->cabal == CABAL_RULER) {
    sprintf(buf,"eğil %s",ch->name);
    interpret(mob,buf, FALSE);
    return;
  }
  if (ch->last_death_time != -1 && current_time - ch->last_death_time < 600)
    {
      do_say(mob,(char*)"Hayaletler buraya giremez.");
      do_slay(mob, ch->name);
      return;
    }

  if (IS_IMMORTAL(ch))	return;

  do_cb(mob,(char*) "Dikkat!!! Davetsiz Misafir!!!");
  do_say(mob,(char*) "Kabalımı rahatsız etmemeliydin!");
}

void greet_prog_chaos(CHAR_DATA *mob,CHAR_DATA *ch)
{
  if ( IS_NPC(ch) )
    return;

  mob->cabal = CABAL_CHAOS;
  SET_BIT(mob->off_flags,OFF_AREA_ATTACK);

  if (ch->cabal == CABAL_CHAOS) {
    do_say(mob, (char*)"Selamlar kaosun efendisi!");
    return;
  }
  if (ch->last_death_time != -1 && current_time - ch->last_death_time < 600)
    {
      do_say(mob,(char*)"Hayaletler buraya giremez.");
      do_slay(mob, ch->name);
      return;
    }

  if (IS_IMMORTAL(ch))	return;

  do_cb(mob, (char*)"Dikkat!!! Davetsiz Misafir!!!");
  do_say(mob,(char*) "Kabalımı rahatsız etmemeliydin!");
}

void greet_prog_battle(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if ( IS_NPC(ch))
    return;

  mob->cabal = CABAL_BATTLE;
  SET_BIT(mob->off_flags,OFF_AREA_ATTACK);

  if (ch->cabal == CABAL_BATTLE) {
    do_say(mob, (char*)"Selamlar yüce savaşçı!");
    return;
  }
  if (ch->last_death_time != -1 && current_time - ch->last_death_time < 600)
    {
      do_say(mob,(char*)"Hayaletler buraya giremez.");
      do_slay(mob, ch->name);
      return;
    }

  if (IS_IMMORTAL(ch))	return;

  do_cb(mob, (char*)"Dikkat!!! Davetsiz Misafir!!!");
  do_say(mob, (char*)"Kabalımı rahatsız etmemeliydin!");
}


void give_prog_keeper(CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj)
{
  OBJ_DATA *rug;
  char buf[100];

  if (obj->pIndexData->vnum == 90)
    {
      do_say(mob,(char*) "İşte beklediğim giysi!");
      act("$n giysiyi masanın altına sokuşturuyor.",mob,NULL,NULL,TO_ROOM);
      obj_from_char(obj);
      extract_obj(obj);
      if ((rug = get_obj_carry(ch, (char*)"rug")) != NULL)
	{
    sprintf(buf, "Sanırım %s ziyaret etmek isteyeceksin.", TR_ACC("FireFlash"));
    do_say(mob, buf);
	  do_say(mob, (char*)"Dikkatli ol, canı biraz sıkkın.");
	  do_unlock (mob, (char*)"door");
	  do_open (mob, (char*)"door");
	}
      else
	{
    do_say(mob,(char*)"Sanırım FireFlash ile işin yok.");
    do_say(mob,(char*)"Sanırım şimdi buradan ayrılacak ve eksiklerini tamamlayacaksın.");
	}
    }
    else
    {
     sprintf(buf,"%s %s",obj->name,ch->name);
     do_give(mob,buf);
     do_say(mob,(char*)"Neden buna ihtiyacım olsun?");
    }
 return;
}


void speech_prog_keeper(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
  OBJ_DATA *obj;

  if (!str_cmp(speech, "keeper") && !IS_NPC(ch) )
    {
      obj = create_object(get_obj_index(90), 0);
      free_string(obj->name);
      obj->name = str_dup("keeper dress");
      act("$n ipekten beyaz bir giysi hazırlıyor.", mob, NULL,
	NULL, TO_ROOM);
      act("Keeper için beyaz bir giysi hazırlıyorsun.", mob, NULL, NULL, TO_CHAR);
      do_say(mob,(char*)"İşte Keeper için bir giysi.");
      obj_to_char(obj, ch);
    }
}

void greet_prog_fireflash(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[100];
  OBJ_DATA *obj;

  if (!can_see(mob,ch) || IS_NPC(ch) || IS_IMMORTAL(ch))
    return;

  if ((obj = get_obj_carry(ch,(char*)"rug")) == NULL)
    {
      do_say(mob,(char*)"Bu değersiz halıyı yakınlarımda görmek istemiyorum.");
    do_say(mob, (char*)"Neden onu Tear'dan aptal rahibeye vermiyorsun?");
      do_unlock(mob, (char*)"box");
      do_open(mob, (char*)"box");
      do_get(mob, (char*)"papers box");
      do_say(mob,(char*) "Bu kağıtların sana yardımı olabilir.");
      act("$n sana dudak büküyor.", mob, NULL, ch, TO_VICT);
      act("$E dudak büküyorsun.", mob, NULL, ch, TO_CHAR);
      act("$n $E dudak büküyor.", mob, NULL, ch, TO_NOTVICT);
      sprintf(buf, "papers %s", ch->name);
      do_give(mob, buf);
      do_close(mob, (char*)"box");
      do_lock(mob, (char*)"box");
    }
}

void give_prog_fireflash(CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj)
{
  char buf[100];

  if (!can_see(mob,ch))
  do_say(mob,(char*)"Orada biri mi var?");
else if (IS_NPC(ch))
 do_say(mob,(char*) "Birşeyler getiren bir hayvan... Ne kadar garip!");

  else if (obj->pIndexData->vnum != 91)
    {
      do_say(mob,(char*)"Ne kadar ilginç!... Nedir bu?");
      interpret(mob, (char*)"kıkırda", FALSE);
      sprintf(buf,"%s %s",obj->name,ch->name);
      do_give(mob,buf);
    }
  else
    {
      do_say(mob, (char*)"Ne kadar şahane bir halı! Acaba nereye koysam...");
      act("$n odayı dolanarak kendi kendine mırıldanıyor.", mob,
	NULL, NULL, TO_ROOM);
      act("$n ellerini cebine sokuyor.",mob,NULL,NULL,TO_ROOM);
      do_load(mob, (char*)"obj 2438");
      do_say(mob,(char*)"Nedir bu? Bir anahtar? Hmm, geri alabilirsin.");
      sprintf(buf, "xxx %s",ch->name);
      do_give(mob, buf);
      act("$n dalgınlıkla halıyı sandalyenin altına itiyor.",mob,NULL,NULL,TO_ROOM);
      obj_from_char(obj);
      extract_obj(obj);
    }
   return;
}

void greet_prog_solamnia(CHAR_DATA *mob, CHAR_DATA *ch)
{

  OBJ_DATA *obj;
  char arg[100];

  if (!can_see(mob,ch) || IS_NPC(ch) || IS_IMMORTAL(ch))
    return;

  sprintf(arg,"xxx");

  if ((obj = get_obj_carry(ch, arg)) != NULL)
    {
      do_say(mob,(char*) "Sanırım benim için birşey getirdin.");
      interpret(mob,(char*) "gülümse", FALSE);
    }
}

void give_prog_solamnia(CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj)

{
  char buf[100];
  OBJ_DATA *kassandra;

  if (obj->pIndexData->vnum == 2438 )
    {
      do_say(mob,(char*) "İşte ödülün!");
      kassandra = create_object(get_obj_index(89), 0);
      kassandra->timer = 500;
      obj_to_char(kassandra, mob);
      sprintf(buf,"kasandra %s",ch->name);
      do_give(mob, buf);
      do_say(mob,(char*)"Bu taşın özel güçleri vardır, dikkatli kullan.");
      obj_from_char(obj);
      extract_obj(obj);
    }
}

bool death_prog_stalker(CHAR_DATA *mob)
{
  char buf[100];

  mob->cabal = CABAL_RULER;
  sprintf(buf, "%s öldürmeyi beceremedim, son nefesimi vermeliyimg.",
   TR_ACC(mob->last_fought->name));
  do_cb(mob, buf);
  return FALSE;
}

void greet_prog_knight(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if ( IS_NPC(ch))
    return;

  mob->cabal = CABAL_KNIGHT;
  SET_BIT(mob->off_flags,OFF_AREA_ATTACK);

  if (ch->cabal == CABAL_KNIGHT) {
    do_say(mob,(char*)"Merhaba onurlu olan!");
    return;
  }
  if (ch->last_death_time != -1 && current_time - ch->last_death_time < 600)
    {
      do_say(mob, (char*)"Hayaletler buraya giremez.");
      do_slay(mob, ch->name);
      return;
    }

  if (IS_IMMORTAL(ch))	return;

  do_cb(mob, (char*)"Dikkat!!! Davetsiz Misafir!!!");
  do_say(mob, (char*)"Kabalımı rahatsız etmemeliydin!");
}

void give_prog_dressmaker(CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj)
{

  if (IS_NPC(ch))
    return;

  if (!can_see(mob, ch))
    {
      do_say(mob,(char*)"Bu nereden geldi?");
      return;
    }

  if (obj->pIndexData->vnum != 2436)
    {
      do_say(mob,(char*)"Bu işime yaramaz. Bana ipek lazım.");
      do_drop(mob, obj->name);
      return;
    }

  else
    {
      do_say(mob,(char*)"Bu elbiseyi kime yapıyorum?");
      obj_from_char(obj);
      extract_obj(obj);
    }
}

void greet_prog_keeper(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if (IS_NPC(ch))
    return;

  if (!can_see(mob, ch))
    return;

    do_say(mob,(char*)"Burada ne işin var? O şey sipariş ettiğim giysi mi?");
}

void speech_prog_templeman(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
	char buf[160];
	int chosen = 0;

	if ( !str_cmp( speech, "din" )|| !str_cmp(speech,"dinler") )
	{
		mob->status = GIVE_HELP_RELIGION;
	}
	else if (( chosen = lookup_religion_name( speech)) != 0 )
	{
		mob->status = RELIG_CHOSEN;
	}
	else
	{
		return;
	}

	if ( mob->status == RELIG_CHOSEN )
	{
		if (( ch->religion > 0) && (ch->religion < MAX_RELIGION) )
		{
			sprintf(buf,"Zaten %s yolundasın.", religion_table[ch->religion].name);
			do_say(mob,buf);
			return;
		}

		ch->religion = chosen;
		sprintf(buf,"Bundan böyle sonsuza kadar %s yolundasın.",religion_table[ch->religion].name);
		do_say(mob,buf);
		return;
	}
	do_say(mob,(char*)"Hmmm... Eveeett.. Din.. Bununla gerçekten ilgileniyor musun?");
	do_say(mob,(char*)"Bildiğin gibi bu diyarda dört din vardır.");
	do_say(mob,(char*)"Kame efendinin dini Kamenilik.");
	do_say(mob,(char*)"Nir efendinin dini Niryanilik. Nyahilik ve Sintiyanlık.");
	do_say(mob,(char*)"Nyah efendinin dini Nyahilik.");
	do_say(mob,(char*)"Sint efendinin dini Sintiyanlık.");
	do_say(mob,(char*)"Bana seçmek istediğin dinin efendisini söylemelisin.");
	do_say(mob,(char*)"Unutma ki dinini bir kez seçersin.");
	do_say(mob,(char*)"Din seçimi yeniyaşamdan sonra dahi değişmez.");
	do_say(mob,(char*)"Dinini değiştirmek istersen bunun için kimi görevleri bitirmen gerekir.");
	return;
}

void greet_prog_templeman(CHAR_DATA *mob, CHAR_DATA *ch)
{

  char arg[100];

  if (!can_see(mob,ch) || IS_NPC(ch) || IS_IMMORTAL(ch))
    return;

    sprintf(arg,"gülümse %s",ch->name);
  interpret(mob, arg, FALSE);
}


int lookup_religion_name (const char *name)
{
	int value;

	for ( value = 1; value < MAX_RELIGION ; value++)
	{
		if (!strcmp(name,religion_table[value].leader))
		{
			return value;
		}
	}

	return 0;
}

void greet_prog_lions(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if ( IS_NPC(ch))
    return;

  mob->cabal = CABAL_LIONS;
  SET_BIT(mob->off_flags,OFF_AREA_ATTACK);

  if (ch->cabal == CABAL_LIONS )
  {
    do_say(mob, (char*)"Hoşgeldiniz aslan efendiler.");
    return;
  }
  if (ch->last_death_time != -1 && current_time - ch->last_death_time < 600)
    {
      do_say(mob,(char*)"Hayaletler buraya giremez.");
      do_slay(mob, ch->name);
      return;
    }

  if (IS_IMMORTAL(ch))	return;

  do_cb(mob, (char*)"Dikkat!!! Davetsiz Misafir!!!");
  do_say(mob, (char*)"Kabalımı rahatsız etmemeliydin!");
}

void greet_prog_hunter_old(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if ( IS_NPC(ch))
    return;

  mob->cabal = CABAL_HUNTER;
  SET_BIT(mob->off_flags,OFF_AREA_ATTACK);

  if (ch->cabal == CABAL_HUNTER )
  {
    do_say(mob, (char*)"Merhaba sevgili avcı.");
    return;
  }
  if (ch->last_death_time != -1 && current_time - ch->last_death_time < 600)
    {
      do_say(mob,(char*)"Hayaletler buraya giremez.");
      do_slay(mob, ch->name);
      return;
    }

  if (IS_IMMORTAL(ch))	return;

  do_cb(mob, (char*)"Dikkat!!! Davetsiz Misafir!!!");
  do_say(mob, (char*)"Kabalımı rahatsız etmemeliydin!");
}


void greet_prog_hunter(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if ( IS_NPC(ch))
    return;

  mob->cabal = CABAL_HUNTER;
  SET_BIT(mob->off_flags,OFF_AREA_ATTACK);

  if (ch->cabal == CABAL_HUNTER )
  {
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *eyed;
    int i;

    do_say(mob,(char*)"Merhaba sevgili avcı.");
        if (IS_SET(ch->quest,QUEST_EYE)) return;

        SET_BIT(ch->quest,QUEST_EYE);

	if (IS_GOOD(ch))
		i=0;
	else if (IS_EVIL(ch))
		i=2;
	else i = 1;

	eyed	= create_object(get_obj_index(OBJ_VNUM_EYED_SWORD), 0);
	eyed->owner = str_dup(ch->name);
	eyed->from = str_dup(ch->name);
	eyed->altar = hometown_table[ch->hometown].altar[i];
	eyed->pit = hometown_table[ch->hometown].pit[i];
	eyed->level = ch->level;

	sprintf( buf, eyed->short_descr, ch->name );
	free_string( eyed->short_descr );
	eyed->short_descr = str_dup( buf );

        sprintf( buf, eyed->pIndexData->extra_descr->description, ch->name );
        eyed->extra_descr = new_extra_descr();
        eyed->extra_descr->keyword =
		str_dup( eyed->pIndexData->extra_descr->keyword );
        eyed->extra_descr->description = str_dup( buf );
        eyed->extra_descr->next = NULL;

  	eyed->value[2] = (ch->level / 10) + 3;
  	eyed->level = ch->level;
	eyed->cost = 0;
	obj_to_char( eyed, mob);
  interpret( mob, (char*)"emote bir Avcı Kılıcı yaratıyor.", FALSE);
	do_say( mob ,(char*)"Sana avcının kılıcını veriyorum.");
	sprintf( buf , "give eyed %s" , ch->name);
	interpret( mob , buf , FALSE);
	do_say( mob , (char*)"Şunu unutma ki, eğer onu kaybedersen kabal şifacısından yeni bir tane isteyebilirsin.");
	do_say( mob ,(char*)"Ona sadece 'felaket' de.");
    return;
  }
  if (ch->last_death_time != -1 && current_time - ch->last_death_time < 600)
    {
      do_say(mob, (char*)"Hayaletler buraya giremez.");
      do_slay(mob, ch->name);
      return;
    }

  if (IS_IMMORTAL(ch))	return;

  do_cb(mob, (char*)"Dikkat!!! Davetsiz Misafir!!!");
  do_say(mob,(char*)"Kabalımı rahatsız etmemeliydin!");
}


void fight_prog_diana( CHAR_DATA *mob, CHAR_DATA *ch )
{
   CHAR_DATA *ach, *ach_next;
   int door;

   if ( !mob->in_room || number_percent() < 25 ) return;
   if (mob->in_room->area != mob->zone) return;

   do_yell(mob,(char*)"Yardım edin nöbetçiler.");
   for( ach = char_list; ach != NULL; ach = ach_next )
   {
     ach_next = ach->next;
     if ( !ach->in_room
	 || ach->in_room->area != ch->in_room->area
	 || !IS_NPC(ach) )
		continue;
     if ( ach->pIndexData->vnum == 600
	|| ach->pIndexData->vnum == 603 )
	{
	 if (ach->fighting || ach->last_fought) continue;
	 if (mob->in_room == ach->in_room)
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
	   for(i=0;i<MAX_STATS;i++)
		ach->perm_stat[i] = 23;
    do_say(ach,(char*)"İşte geldim Diana.");
	   do_murder(ach,ch->name);
	   continue;
	  }
         door = find_path( ach->in_room->vnum, mob->in_room->vnum,
			ach,-40,1);
	 if (door == -1) bug("Couldn't find a path with -40",0);
	 else {
		if (number_percent() < 25)
    do_yell(ach,(char*)" Dayan Diana! Geliyorum!");
   else
    do_say(ach,(char*)"Gidip Dianaya yardım etmeliyim.");
		move_char(ach,door,FALSE);
	      }
	}
   }
}

void fight_prog_ofcol_guard( CHAR_DATA *mob, CHAR_DATA *ch )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *ach, *ach_next;
   int door;

   if (number_percent() < 25) return;
   sprintf(buf,"Yardım edin nöbetçiler! %s benimle dövüşüyor!",ch->name);
   do_yell(mob,buf);
   for( ach = char_list; ach != NULL; ach = ach_next )
   {
     ach_next = ach->next;
     if ( ach->in_room->area != ch->in_room->area || !IS_NPC(ach) ) continue;
     if ( ach->pIndexData->vnum == 600 )
	{
	 if (ach->fighting) continue;
	 if (mob->in_room == ach->in_room)
	  {
      sprintf(buf,"Şimdi %s, muhafızlara saldırmanın cezasını çekeceksin.",ch->name);
	   do_say(ach,buf);
	   do_murder(ach,ch->name);
	   continue;
	  }
         door = find_path( ach->in_room->vnum, mob->in_room->vnum,
			ach,-40,1);
	 if (door == -1) bug("Couldn't find a path with -40",0);
	 else {
		if (number_percent() < 25)
    do_yell(ach,(char*)" Dayan nöbetçi! Geliyorum!");
   else
    do_say(ach,(char*)"Gidip muhafıza yardım etmeliyim.");
		move_char(ach,door,FALSE);
	      }
	}
   }
}

void speech_prog_wiseman(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
 char arg[512];

 one_argument(speech,arg);
 if (arg[0] == '\0')	return;
 if (!str_cmp(speech,"bilge iyileştir şifa"))
	heal_battle(mob,ch);
 else return;
}

void greet_prog_armourer(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_STRING_LENGTH];

  if (!can_see(mob,ch) || IS_NPC(ch) || IS_IMMORTAL(ch))
    return;
  interpret(mob,(char*)"gülümse", FALSE);
  sprintf(buf,"Zırhçıya hoşgeldin %s,",
   str_cmp(mob->in_room->area->name,hometown_table[ch->hometown].name) ?
   "yolcu" : ch->name );
  do_say(mob,buf);
  do_say(mob,(char*)"Sana nasıl yardımcı olabilirim?");
  {
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "Dükkanımda gördüğün %s tümü çok kalitelidir.", TR_PLU("zırh") );
    do_say(mob, buf);
  }
  interpret(mob,(char*)"emote gururla geriniyor.", FALSE);
}

void greet_prog_baker(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_STRING_LENGTH];

  if (!can_see(mob,ch) || IS_NPC(ch) || IS_IMMORTAL(ch))
    return;
  interpret(mob,(char*)"gülümse", FALSE);
  sprintf(buf,"Fırına hoşgeldin %s.",
   str_cmp(mob->in_room->area->name,hometown_table[ch->hometown].name) ?
   "yolcu" : ch->name );
  do_say(mob,buf);
}

void greet_prog_beggar(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_STRING_LENGTH];

  if (!can_see(mob,ch) || IS_NPC(ch) || IS_IMMORTAL(ch))
    return;
    sprintf(buf,"Allah rızası için %s,",
   str_cmp(mob->in_room->area->name,hometown_table[ch->hometown].name) ?
   "yolcu" : ch->name );
  do_say(mob,buf);
  do_say(mob,(char*)"Birkaç akçe ver...");
}

void greet_prog_drunk(CHAR_DATA *mob, CHAR_DATA *ch)
{
  if (!can_see(mob,ch) || IS_NPC(ch) || IS_IMMORTAL(ch))
    return;
  if (number_percent() < 5)
   {
     do_yell(mob,(char*)"Canavar! Bir canavar buldum! Saldırın!");
    do_murder(mob,ch->name);
   }
}

void greet_prog_grocer(CHAR_DATA *mob, CHAR_DATA *ch)
{
  char buf[MAX_STRING_LENGTH];

  if (!can_see(mob,ch) || IS_NPC(ch) || IS_IMMORTAL(ch))
    return;
    sprintf(buf,"Dükkanıma hoşgeldin %s.",
   str_cmp(mob->in_room->area->name,hometown_table[ch->hometown].name) ?
   "yolcu" : ch->name );
  do_say(mob,buf);
}


void bribe_prog_beggar(CHAR_DATA *mob, CHAR_DATA *ch, int amount)
{
  char buf[MAX_STRING_LENGTH];

  if (amount < 10)
    {
     sprintf(buf,"teşekkür %s",
      str_cmp(mob->in_room->area->name,hometown_table[ch->hometown].name) ?
      "yolcu" : ch->name );
     interpret(mob,buf, FALSE);
     return;
    }
  else if (amount < 100)
    {
      do_say(mob,(char*)"Ohaaa! Çok teşekkür ederim.");
     return;
    }
  else if (amount < 500)
    {
      do_say(mob,(char*)"Tanrıma şükürler olsun! Teşekkürler! Teşekkürler!");
      sprintf(buf,"öp %s",ch->name);
     interpret(mob,buf, FALSE);
     return;
    }
  else
    {
     sprintf(buf,"dans %s",ch->name);
     interpret(mob,buf, FALSE);
     sprintf(buf,"öp %s",ch->name);
     interpret(mob,buf, FALSE);
     return;
    }
}


void bribe_prog_drunk(CHAR_DATA *mob, CHAR_DATA *ch, int amount)
{
  (void)ch;
  (void)amount;  
  do_say(mob,(char*) "Ahh! Daha fazla ruh! Güzel Ruhlar!");
  interpret(mob,(char*)"şarkı", FALSE);
   return;
}



void fight_prog_beggar(CHAR_DATA *mob, CHAR_DATA *ch)
{
  (void)ch;
  if (mob->hit < (mob->max_hit * 0.45) && mob->hit > (mob->max_hit * 0.55))
  do_say(mob,(char*)"İşte ölüyorum...");
  return;
}


bool death_prog_beggar(CHAR_DATA *mob)
{
  if (number_percent() < 50 )
  do_say(mob,(char*)"Her nereyse gittiğim yer, biliyorum ki buradan iyidir...");
  else do_say(mob,(char*)"Günahlarımı affet tanrım...");
  return FALSE;
}

bool death_prog_vagabond(CHAR_DATA *mob)
{
  interpret(mob,(char*)"emote kafasını arkaya atıp deli gibi kahkaha atıyor!", FALSE);
  return FALSE;
}


void speech_prog_crier(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
  (void)ch;
 char arg[512];

 speech = one_argument(speech,arg);
 if (is_name(arg,(char*)"ne"))
	do_say(mob,(char*)"Sevgilim beni terketti.");
 return;
}


void area_prog_drunk(CHAR_DATA *mob)
{
  if (number_percent() < 5)
  interpret(mob, (char*)"dans", FALSE);
else if (number_percent() < 10)
  interpret(mob,(char*) "şarkı", FALSE);
  return;
}

void area_prog_janitor(CHAR_DATA *mob)
{
  if (number_percent() < 20)
   {
    interpret(mob,(char*)"grumble", FALSE);
    do_say(mob,(char*)"Çöpler");
    if (number_percent() < 20 )
     {
       do_say(mob,(char*)"Hergün yaptığım tek iş başkalarının pisliklerini temizlemek.");
      if (number_percent() < 20 )
      do_say(mob,(char*)"Yeterince kazanamıyorum.");
      else if (number_percent() < 20)
	   {
       do_say(mob,(char*)"Gün başlıyor, gün bitiyor. Hiç aralıksız çöp topluyorum.");
	    if ( number_percent() < 10 )
      do_yell(mob,(char*)"Tatil istiyorum!");
	   }
     }
   }
  return;
}

void area_prog_vagabond(CHAR_DATA *mob)
{
  if (number_percent() < 10)
  do_say(mob,(char*)"Kan! Zulüm!");
  return;
}

void area_prog_baker(CHAR_DATA *mob)
{
  if (number_percent() < 5)
  do_say(mob,(char*)"Kurabiyelerimin tadına bakmak ister misin?");
  return;
}

void area_prog_grocer(CHAR_DATA *mob)
{
  if (number_percent() < 5)
  do_say(mob, (char*)"Şu fenerin işçiliğine bakar mısın?");
  return;
}


void speech_prog_hunter_cleric(CHAR_DATA *mob, CHAR_DATA *ch, char *speech)
{
 char buf[MAX_STRING_LENGTH];
 OBJ_DATA *obj,*in_obj;
 int matched,i;

    if (str_cmp(speech,"felaket"))	return;

    if (ch->cabal != CABAL_HUNTER)
    {
      do_say(mob,(char*)"Ikınırsan belki...");
     return;
    }

    if (!IS_SET(ch->quest,QUEST_EYE))
    {
      do_say(mob,(char*)"Ne kastediyorsun?");
     return;
    }

    matched = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if (obj->pIndexData->vnum != OBJ_VNUM_EYED_SWORD ||
		strstr(obj->short_descr,ch->name) == NULL)
	continue;

	matched = 1;
	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL )
	{
	    if (in_obj->carried_by == ch)
	    {
        do_say(mob,(char*)"Benle dalga mı geçiyorsun? İşte kılıcın elinde...");
	     do_smite(mob,ch->name);
	     return;
	    }

      sprintf( buf, "Kılıcını %s taşıyor!",
		PERS(in_obj->carried_by, ch) );
	    do_say(mob, buf);
	    if ( in_obj->carried_by->in_room )
	    {
        sprintf(buf, "%s %s bölgesinde, %s civarlarında dolaşıyor!",
		PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->area->name,
		in_obj->carried_by->in_room->name );
	     do_say(mob, buf);
	     return;
	    }
	    else
	    {
	     extract_obj( obj );
       do_say( mob, (char*)"Sana yenisini vereceğim.");
	    }
	}
	else
	{
	    if (in_obj->in_room != NULL)
	    {
        sprintf( buf, "Kılıcın %s bölgesinde, %s civarlarında!",
			in_obj->in_room->area->name, in_obj->in_room->name );
		do_say(mob,buf);
		return;
	    }
	    else
	    {
	     extract_obj( obj );
       do_say( mob,(char*)"Sana yenisini vereceğim.");
	    }
	}
     break;
    }

    if (!matched) do_say(mob,(char*)"Kılıcın kayıp!");

    if (IS_GOOD(ch)) 	i=0;
    else if (IS_EVIL(ch)) i=2;
    else i = 1;

    obj = create_object(get_obj_index(OBJ_VNUM_EYED_SWORD), 0);
    obj->owner = str_dup(ch->name);
    obj->from = str_dup(ch->name);
    obj->altar = hometown_table[ch->hometown].altar[i];
    obj->pit = hometown_table[ch->hometown].pit[i];
    obj->level = ch->level;

    sprintf( buf, obj->short_descr, ch->name );
    free_string( obj->short_descr );
    obj->short_descr = str_dup( buf );

    sprintf( buf, obj->pIndexData->extra_descr->description, ch->name );
    obj->extra_descr = new_extra_descr();
    obj->extra_descr->keyword =
		str_dup( obj->pIndexData->extra_descr->keyword );
    obj->extra_descr->description = str_dup( buf );
    obj->extra_descr->next = NULL;

    obj->value[2] = (ch->level / 10) + 3;
    obj->level = ch->level;
    obj->cost = 0;
    interpret( mob, (char*)"emote bir Avcı Kılıcı yaratıyor.", FALSE);
    do_say( mob ,(char*) "Sana başka bir Avcı Kılıcı veriyorum.");
    act( "$N $e $p veriyor.", ch, obj, mob, TO_ROOM );
    act( "$N sana $p veriyor.",   ch, obj, mob, TO_CHAR );
    obj_to_char(obj, ch);
    do_say( mob , (char*)"Tekrar kaybetme!");
}



void fight_prog_golem( CHAR_DATA *mob, CHAR_DATA *ch)
{
    (void)ch;
    CHAR_DATA *master;
    CHAR_DATA *m_next;
    char buf[MAX_INPUT_LENGTH];
    const char *spell;
    int sn;

    for ( master = mob->in_room->people; master != NULL; master = m_next )
    {
	m_next = master->next_in_room;
	if ( !IS_NPC(master) && mob->master == master &&
	     master->iclass == CLASS_NECROMANCER)
	    break;
    }

    if ( master == NULL )
	return;

    if ( !master->fighting )
	return;

    if ( master->fighting->fighting == master)
     {
      sprintf(buf,"%s",master->name);
      do_rescue(mob,buf);
     }

    switch ( number_range(0,15) )
     {
	case  0: spell = "curse";          break;
	case  1: spell = "weaken";         break;
	case  2: spell = "chill touch";    break;
	case  3: spell = "blindness";      break;
	case  4: spell = "poison";         break;
	case  5: spell = "energy drain";   break;
	case  6: spell = "harm";           break;
	case  7: spell = "teleport";       break;
	case  8: spell = "plague";	   break;
	default: spell = NULL;           break;
     }

    if ( spell == NULL || ( sn=skill_lookup(spell) ) < 0 )
	return;

    m_next = (mob->fighting) ? mob->fighting : master->fighting;
    if (m_next)
     {
      say_spell(mob,sn);
      (*skill_table[sn].spell_fun) (sn,mob->level,mob,m_next,TARGET_CHAR);
     }
    return;
}
