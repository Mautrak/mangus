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
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"
#include "turkish_suffix_helper.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);

/* used to converter of prac and train */
void do_gain(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people;
	  trainer != NULL;
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && 	(IS_SET(trainer->act,ACT_PRACTICE) ||
	    IS_SET(trainer->act,ACT_TRAIN) || IS_SET(trainer->act,ACT_GAIN)) )
	    break;

    if (trainer == NULL || !can_see(ch,trainer))
    {
      send_to_char("Burada yapamazsın.\n\r", ch);
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
      do_say(trainer,(char*)"10 pratiği 1 eğitime dönüştürebilirsin.");
    	do_say(trainer,(char*)"1 eğitimi 10 pratiğe dönüştürebilirsin.");
    	do_say(trainer,(char*)"Şunları kullan: 'kazan eğitim','kazan pratik'");
	return;
    }

    if (!str_prefix(arg,"pratik"))
    {
	if (ch->train < 1)
	{
    act("$N anlatıyor 'Hazır değilsin.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

  act("$N eğitimini pratiklere dönüştürmene yardım ediyor.",
		ch,NULL,trainer,TO_CHAR);
	ch->practice += 10;
	ch->train -=1 ;
	return;
    }

    if (!str_prefix(arg,"eğitim"))
    {
	if (ch->practice < 10)
	{
    act("$N anlatıyor 'Hazır değilsin.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

  act("$N pratiklerini eğitime dönüştürmene yardım ediyor.",
		ch,NULL,trainer,TO_CHAR);
	ch->practice -= 10;
	ch->train +=1 ;
	return;
    }

    act("$N anlatıyor 'Anlamadım...'",ch,NULL,trainer,TO_CHAR);

}


/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument)
{
    (void) argument; /* Unused parameter */
    char spell_list[LEVEL_HERO][MAX_STRING_LENGTH];
    char spell_columns[LEVEL_HERO];
    int sn,lev,mana;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    char output[4*MAX_STRING_LENGTH];

    if (IS_NPC(ch))
      return;

    /* initilize data */
    output[0] = '\0';
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
	spell_columns[lev] = 0;
	spell_list[lev][0] = '\0';
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      /* Check if the first name pointer is NULL */
      if (skill_table[sn].name[0] == NULL)
        break;

      if (skill_table[sn].skill_level[ch->iclass] < LEVEL_HERO &&
	  skill_table[sn].spell_fun != spell_null && RACE_OK(ch,sn) &&
(skill_table[sn].cabal == ch->cabal || skill_table[sn].cabal == CABAL_NONE)
	)
      {
	found = TRUE;
	lev = skill_table[sn].skill_level[ch->iclass];
	if (ch->level < lev)
	  sprintf(buf,"%-18s  n/a      ", skill_table[sn].name[1]);
	else
	{
	  mana = UMAX(skill_table[sn].min_mana,
		      100/(2 + ch->level - lev));
	  sprintf(buf,"%-18s  %3d mana  ",skill_table[sn].name[1],mana);
	}

	if (spell_list[lev][0] == '\0')
	  sprintf(spell_list[lev],"\n\rSeviye %2d: %s",lev,buf);
        else /* append */
	{
	  if ( ++spell_columns[lev] % 2 == 0)
            strcat(spell_list[lev],"\n\r          ");
	  strcat(spell_list[lev],buf);
        }
      }
    }

    /* return results */

    if (!found)
    {
      send_to_char("Büyü bilmiyorsun.\n\r",ch);
      return;
    }

    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (spell_list[lev][0] != '\0')
        strcat(output,spell_list[lev]);
    strcat(output,"\n\r");
    page_to_char(output,ch);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
    (void) argument; /* Unused parameter */
    char skill_list[LEVEL_HERO][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO];
    int sn,lev;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
      return;

    /* initilize data */
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
        skill_columns[lev] = 0;
        skill_list[lev][0] = '\0';
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      /* Check if the first name pointer is NULL */
      if (skill_table[sn].name[0] == NULL )
        break;


      if (skill_table[sn].skill_level[ch->iclass] < LEVEL_HERO &&
	  skill_table[sn].spell_fun == spell_null && RACE_OK(ch,sn) &&
(skill_table[sn].cabal == ch->cabal || skill_table[sn].cabal == CABAL_NONE)
	  )
      {
        found = TRUE;
        lev = skill_table[sn].skill_level[ch->iclass];
        if (ch->level < lev)
          sprintf(buf,"%-18s n/a      ", skill_table[sn].name[1]);
        else
          sprintf(buf,"%-18s %3d%%      ",skill_table[sn].name[1],
					 ch->pcdata->learned[sn]);

        if (skill_list[lev][0] == '\0')
          sprintf(skill_list[lev],"\n\rSeviye %2d: %s",lev,buf);
        else /* append */
        {
          if ( ++skill_columns[lev] % 2 == 0)
            strcat(skill_list[lev],"\n\r          ");
          strcat(skill_list[lev],buf);
        }
      }
    }

    /* return results */

    if (!found)
    {
      send_to_char("Büyü bilmiyorsun.\n\r",ch);
      return;
    }

    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (skill_list[lev][0] != '\0')
        send_to_char(skill_list[lev],ch);
    send_to_char("\n\r",ch);
}


int base_exp(CHAR_DATA *ch, int points)
{
  (void) points; /* Unused parameter */
  int expl;

  if (IS_NPC(ch))    return 1500;
  expl = 1000 + race_table[ORG_RACE(ch)].points + class_table[ch->iclass].points;

  return expl;
}

int exp_to_level(CHAR_DATA *ch, int points)
{
 int base;

  base = base_exp(ch,points);
  return ( base - exp_this_level(ch,ch->level,points) );
}

int exp_this_level(CHAR_DATA *ch, int level, int points)
{
  (void) level; /* Unused parameter */
  (void) points; /* Unused parameter */
  int base;

  base = base_exp(ch,points);
  return (ch->exp - (ch->level * base ) );
}


int exp_per_level(CHAR_DATA *ch, int points)
{
    (void) points; /* Unused parameter */
    int expl;

    if (IS_NPC(ch))
	return 1000;

    expl = 1000 + race_table[ORG_RACE(ch)].points + class_table[ch->iclass].points;

    return expl;
}


/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance;
    char buf[100];

    if (IS_NPC(ch))
	return;

    if (ch->level < skill_table[sn].skill_level[ch->iclass]
    ||  skill_table[sn].rating[ch->iclass] == 0
    ||  ch->pcdata->learned[sn] == 0
    ||  ch->pcdata->learned[sn] == 100)
	return;  /* skill is not known */

    /* check to see if the character has a chance to learn */
    chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
    chance /= (		multiplier
		*	skill_table[sn].rating[ch->iclass]
		*	4);
    chance += ch->level;

    if (number_range(1,1000) > chance)
	return;

    /* now that the character has a CHANCE to learn, see if they really have */

    if (success)
    {
	chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
	if (number_percent() < chance)
	{
    sprintf(buf,"$C%s gelişiyor!$c",
		    skill_table[sn].name[1]);
	    act_color(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD, CLR_GREEN);
	    ch->pcdata->learned[sn]++;
	    gain_exp(ch,2 * skill_table[sn].rating[ch->iclass]);
	}
    }

    else
    {
	chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
	if (number_percent() < chance)
	{
	    sprintf(buf,
        "$CHatalarından ders alıyorsun ve %s gelişiyor.$c",
		skill_table[sn].name[1]);
	    act_color(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD,CLR_GREEN);
	    ch->pcdata->learned[sn] += number_range(1,3);
	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
	    gain_exp(ch,2 * skill_table[sn].rating[ch->iclass]);
	}
    }
}


/* use for adding all skills available for that ch  */
void group_add( CHAR_DATA *ch )
{
    int sn;

    if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

     for (sn = 0;  sn < MAX_SKILL; sn++)
          if (skill_table[sn].cabal == 0 && RACE_OK(ch,sn)
		&& ch->pcdata->learned[sn] < 1
              && skill_table[sn].skill_level[ch->iclass] < LEVEL_IMMORTAL)
            ch->pcdata->learned[sn] = 1;
        return;

}


void do_slist(CHAR_DATA *ch, char *argument)
{
    char skill_list[LEVEL_HERO][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO];
    int sn,lev,iclass;
    bool found = FALSE;
    char output[4*MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
      return;

    output[0] = '\0';
    argument = one_argument(argument, arg);
    if (arg[0] == '\0')
	{
    send_to_char("Yazım: syetenek <sınıf>.\n\r",ch);
	 return;
	}
    iclass = class_lookup(arg);
    if (iclass == -1)
	{
    send_to_char("Geçerli bir sınıf değil.\n\r",ch);
	 return;
	}
    /* initilize data */
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
        skill_columns[lev] = 0;
        skill_list[lev][0] = '\0';
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      /* Check if the first name pointer is NULL */
      if (skill_table[sn].name[0] == NULL )
        break;


      if (skill_table[sn].skill_level[iclass] < LEVEL_HERO &&
	  skill_table[sn].cabal == CABAL_NONE &&
	  skill_table[sn].race == RACE_NONE )
      {
        found = TRUE;
        lev = skill_table[sn].skill_level[iclass];
        sprintf(buf,"%-18s          ",skill_table[sn].name[1]);
        if (skill_list[lev][0] == '\0')
          sprintf(skill_list[lev],"\n\rrSeviye %2d: %s",lev,buf);
        else /* append */
        {
          if ( ++skill_columns[lev] % 2 == 0)
            strcat(skill_list[lev],"\n\r          ");
          strcat(skill_list[lev],buf);
        }
      }
    }

    /* return results */

    if (!found)
    {
      send_to_char("O sınıfta yetenek yok.\n\r",ch);
      return;
    }

    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (skill_list[lev][0] != '\0')
        strcat(output,skill_list[lev]);
    strcat(output,"\n\r");
    page_to_char(output,ch);
}


/* returns group number */
int group_lookup (const char *name)
{
   int gr;

   for ( gr = 0; prac_table[gr].sh_name != NULL; gr++)
   {
	if (LOWER(name[0]) == LOWER(prac_table[gr].sh_name[0])
	&&  !str_prefix( name,prac_table[gr].sh_name))
	    return gr;
   }

   return -1;
}

void do_glist( CHAR_DATA *ch , char *argument)
{
 char arg[MAX_INPUT_LENGTH];
 char buf[MAX_STRING_LENGTH];
 int group,count;

 one_argument(argument,arg);

 if (arg[0] == '\0')
  {
    send_to_char("Yazım : grupliste <grup>\n\r",ch);
   return;
  }

 if ((group = group_lookup(arg) ) == -1)
  {
    send_to_char("Geçerli bir grup değil.\n\r",ch);
   return;
  }

  sprintf(buf,"%s grubunu listeliyor :\n\r",prac_table[group].sh_name);
 send_to_char(buf,ch);
 buf[0] = '\0';
 for(count = 0 ; count < MAX_SKILL; count++)
  {
   if ( (group == GROUP_NONE && !CLEVEL_OK(ch,count) &&
	skill_table[count].group == GROUP_NONE ) ||
	(group != skill_table[count].group) || !CABAL_OK(ch,count) )
     continue;
   if ( buf[0] != '\0')
    {
     sprintf(buf , "%-18s%-18s\n\r", buf,skill_table[count].name[1]);
     send_to_char(buf,ch);
     buf[0] = '\0';
    }
   else sprintf(buf, "%-18s",skill_table[count].name[1]);
  }

}

void do_slook( CHAR_DATA *ch, char *argument)
{
     int sn;
     char arg[MAX_INPUT_LENGTH];
     char buf[MAX_STRING_LENGTH];

     one_argument(argument,arg);
     if (arg[0] == '\0')
	{
	 send_to_char("Yazım : slook <skill or spell name>.\n\r",ch);
	 return;
	}

     if ( (sn = skill_lookup(arg)) == -1 )
	{
	 send_to_char("Böyle bir büyü veya yetenek yok.\n\r",ch);
	 return;
	}

     sprintf(buf,"Yetenek: %s, Grup: %s.\n\r",
	skill_table[sn].name[1],prac_table[skill_table[sn].group].sh_name);
     send_to_char(buf,ch);

     return;
}

#define PC_PRACTICER	123

void do_learn( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;
    CHAR_DATA *mob;
    int adept;

    if ( IS_NPC(ch) )
	return;

	if ( !IS_AWAKE(ch) )
	{
    send_to_char("Rüyalarında mı?\n\r", ch);
	    return;
	}

	if ( argument[0] == '\0')
	{
    send_to_char( "Yazım: öğren <yetenek|büyü> <öğretici>.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
    send_to_char("Pratik seansın yok.\n\r", ch );
	    return;
	}

	argument = one_argument(argument,arg);

	if ( ( sn = find_spell( ch,arg ) ) < 0
	|| ( !IS_NPC(ch)
	&&   (ch->level < skill_table[sn].skill_level[ch->iclass]
 	|| !RACE_OK(ch,sn) ||
(skill_table[sn].cabal != ch->cabal && skill_table[sn].cabal != CABAL_NONE) )))
	{
    send_to_char( "Bunu pratik edemezsin.\n\r", ch );
	    return;
	}

	if ( sn == gsn_vampire )
	{
    send_to_char( "Bu konuda ancak görevci yardım edebilir.\n\r",ch);
	 return;
	}

	if ( sn == gsn_evolve_bear )
	{
    send_to_char( "Bu konuda ancak görevci yardım edebilir.\n\r",ch);
	 return;
	}

	argument = one_argument(argument,arg);

	if ( (mob = get_char_room(ch,arg) ) == NULL)
	{
    send_to_char( "Kahramanın burada değil.\n\r", ch );
	    return;
	}

	if ( IS_NPC(mob) || mob->level != HERO )
	{
    send_to_char( "Bir kahraman bulmalısın, sıradan birini değil.\n\r",ch);
	  return;
	}

	if ( mob->status != PC_PRACTICER )
	{
    send_to_char( "Kahramanın öğretmeye istekli görünmüyor.\n\r",ch);
	  return;
	}

	if (get_skill(mob,sn) < 100)
	{
    send_to_char("Kahramanın bu konuyu öğretebilecek kadar bilmiyor.\n\r",ch);
	  return;
	}

	adept = class_table[ch->iclass].skill_adept;

	if ( ch->pcdata->learned[sn] >= adept )
	{
    sprintf( buf, "%s konusunu zaten öğrendin.\n\r",
		skill_table[sn].name[1] );
	    send_to_char( buf, ch );
	}
	else
	{
	    if (!ch->pcdata->learned[sn]) ch->pcdata->learned[sn] = 1;
	    ch->practice--;
	    ch->pcdata->learned[sn] +=
		int_app[get_curr_stat(ch,STAT_INT)].learn /
	        UMAX(skill_table[sn].rating[ch->iclass],1);
	    mob->status = 0;
      act( "$T öğretiyorsun.",
		    mob, NULL, skill_table[sn].name[1], TO_CHAR );
	    act( "$n $T öğretiyor.",
		    mob, NULL, skill_table[sn].name[1], TO_ROOM );
	    if ( ch->pcdata->learned[sn] < adept )
	    {
        act( "$T öğreniyorsun.",
    		    ch, NULL, skill_table[sn].name[1], TO_CHAR );
    		act( "$n $T öğreniyor.",
    		    ch, NULL, skill_table[sn].name[1], TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
    act( "$T konusunu öğrendin.",
		    ch, NULL, skill_table[sn].name[1], TO_CHAR );
		act( "$n $T konusunu öğrendi.",
		    ch, NULL, skill_table[sn].name[1], TO_ROOM );
	    }
	}
    return;
}


void do_teach( CHAR_DATA *ch, char *argument)
{
 (void) argument; /* Unused parameter */
 if (IS_NPC(ch) || ch->level != LEVEL_HERO)
  {
    send_to_char("Bir kahraman olmalısın.\n\r",ch);
	return;
  }
 ch->status = PC_PRACTICER;
 {
   char buf[MAX_STRING_LENGTH];
   sprintf(buf, "Artık %%100 olan %s gençlere öğretebilirsin.\n\r", TR_ACC_PLU("yetenek"));
   send_to_char(buf, ch);
 }
 return;
}
