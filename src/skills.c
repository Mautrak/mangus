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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "utf8.h"
#include "magic.h"
#include "recycle.h"
#include "lookup.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);

/* Liste sütunlarında yetenek adı için görsel genişlik. */
#define SKILL_COL	18

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
      do_say(trainer,"10 pratiği 1 eğitime dönüştürebilirsin.");
    	do_say(trainer,"1 eğitimi 10 pratiğe dönüştürebilirsin.");
    	do_say(trainer,"Şunları kullan: 'kazan eğitim','kazan pratik'");
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


/*
 * Seviyeye göre sıralı, iki sütunlu yetenek/büyü listesi.
 * do_spells, do_skills ve do_slist aynı iskeleti paylaşır; yalnız süzgeç,
 * satır biçimi ve sayfalama değişir.
 */
enum skill_list_kind { LIST_SPELLS, LIST_SKILLS, LIST_CLASS };

static bool skill_list_keep(CHAR_DATA *ch, int iclass, int sn, enum skill_list_kind kind)
{
    if (skill_table[sn].skill_level[iclass] >= LEVEL_HERO)
	return FALSE;

    if (kind == LIST_CLASS)
	return skill_table[sn].cabal == CABAL_NONE && skill_table[sn].race == RACE_NONE;

    if ((skill_table[sn].spell_fun != spell_null) != (kind == LIST_SPELLS))
	return FALSE;

    return RACE_OK(ch,sn) && CABAL_OK(ch,sn);
}

static void skill_list_entry(CHAR_DATA *ch, int sn, int lev, enum skill_list_kind kind,
			     char *buf, size_t cap)
{
    const char *name = skill_table[sn].name[1];
    int width = utf8_width(name, SKILL_COL);

    switch (kind)
    {
    case LIST_SPELLS:
	if (ch->level < lev)
	    snprintf(buf, cap, "%-*s  n/a      ", width, name);
	else
	    snprintf(buf, cap, "%-*s  %3d mana  ", width, name,
		     UMAX(skill_table[sn].min_mana, 100/(2 + ch->level - lev)));
	break;
    case LIST_SKILLS:
	if (ch->level < lev)
	    snprintf(buf, cap, "%-*s n/a      ", width, name);
	else
	    snprintf(buf, cap, "%-*s %3d%%      ", width, name, ch->pcdata->learned[sn]);
	break;
    default:
	snprintf(buf, cap, "%-*s          ", width, name);
	break;
    }
}

static void skill_list(CHAR_DATA *ch, int iclass, enum skill_list_kind kind,
		       const char *none_msg, bool page)
{
    BUFFER *out;
    char buf[MAX_STRING_LENGTH];
    int sn, lev, columns;
    bool found = FALSE;

    if (IS_NPC(ch))
	return;

    out = new_buf();

    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
	columns = 0;
	for (sn = 0; sn < MAX_SKILL; sn++)
	{
	    if (skill_table[sn].name[0] == NULL)
		break;
	    if (skill_table[sn].skill_level[iclass] != lev
	    ||  !skill_list_keep(ch, iclass, sn, kind))
		continue;

	    if (columns == 0)
		snprintf(buf, sizeof(buf), "\n\rSeviye %2d: ", lev);
	    else if (columns % 2 == 0)
		snprintf(buf, sizeof(buf), "\n\r          ");
	    else
		buf[0] = '\0';
	    add_buf(out, buf);

	    skill_list_entry(ch, sn, lev, kind, buf, sizeof(buf));
	    add_buf(out, buf);
	    columns++;
	    found = TRUE;
	}
    }

    if (!found)
	send_to_char(none_msg, ch);
    else
    {
	add_buf(out, "\n\r");
	if (page)
	    page_to_char(buf_string(out), ch);
	else
	    send_to_char(buf_string(out), ch);
    }
    free_buf(out);
}

/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument)
{
    skill_list(ch, ch->iclass, LIST_SPELLS, "Büyü bilmiyorsun.\n\r", TRUE);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
    skill_list(ch, ch->iclass, LIST_SKILLS, "Yetenek bilmiyorsun.\n\r", FALSE);
}

void do_slist(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    int iclass;

    if (IS_NPC(ch))
      return;

    one_argument(argument, arg);
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

    skill_list(ch, iclass, LIST_CLASS, "O sınıfta yetenek yok.\n\r", TRUE);
}


/*
 * Tecrübe yardımcıları. ROM'un yaratılış puanı sistemi kaldırıldığından
 * 'points' parametresi kullanılmaz; imza eski çağıranlar için korunur.
 */
#define NPC_BASE_EXP		1500
#define NPC_EXP_PER_LEVEL	1000

static int pc_exp_per_level(CHAR_DATA *ch)
{
    return 1000 + race_table[ORG_RACE(ch)].points + class_table[ch->iclass].points;
}

int base_exp(CHAR_DATA *ch, int points)
{
    (void)points;
    return IS_NPC(ch) ? NPC_BASE_EXP : pc_exp_per_level(ch);
}

int exp_per_level(CHAR_DATA *ch, int points)
{
    (void)points;
    return IS_NPC(ch) ? NPC_EXP_PER_LEVEL : pc_exp_per_level(ch);
}

int exp_this_level(CHAR_DATA *ch, int level, int points)
{
    return ch->exp - level * base_exp(ch, points);
}

int exp_to_level(CHAR_DATA *ch, int points)
{
    return base_exp(ch, points) - exp_this_level(ch, ch->level, points);
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
    snprintf(buf, sizeof(buf),"$C%s gelişiyor!$c",
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
	    snprintf(buf, sizeof(buf),
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


/* returns group number (kısa ada göre; db.c prac_lookup uzun adla bit döndürür) */
int group_lookup (const char *name)
{
    return name_table_lookup(name, &prac_table[0].sh_name, sizeof prac_table[0]);
}

void do_glist( CHAR_DATA *ch , char *argument)
{
 char arg[MAX_INPUT_LENGTH];
 char buf[MAX_STRING_LENGTH];
  char line[MAX_STRING_LENGTH];
 int group,sn;

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

  snprintf(buf, sizeof(buf),"%s grubunu listeliyor :\n\r",prac_table[group].sh_name);
 send_to_char(buf,ch);
 buf[0] = '\0';
 for(sn = 0 ; sn < MAX_SKILL; sn++)
  {
   if ( (group == GROUP_NONE && !CLEVEL_OK(ch,sn) &&
	skill_table[sn].group == GROUP_NONE ) ||
	(group != skill_table[sn].group) || !CABAL_OK(ch,sn) )
     continue;
   if ( buf[0] != '\0')
    {
     snprintf(line, sizeof(line), "%-*s%-*s\n\r", utf8_width(buf, SKILL_COL), buf,
	      utf8_width(skill_table[sn].name[1], SKILL_COL), skill_table[sn].name[1]);
     send_to_char(line,ch);
     buf[0] = '\0';
    }
   else snprintf(buf, sizeof(buf), "%s",skill_table[sn].name[1]);
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
	 send_to_char("Yazım : slook <yetenek ya da büyü adı>.\n\r",ch);
	 return;
	}

     if ( (sn = skill_lookup(arg)) == -1 )
	{
	 send_to_char("Böyle bir büyü veya yetenek yok.\n\r",ch);
	 return;
	}

     snprintf(buf, sizeof(buf),"Yetenek: %s, Grup: %s.\n\r",
	skill_table[sn].name[1],prac_table[skill_table[sn].group].sh_name);
     send_to_char(buf,ch);

     return;
}

/*
 * ch->status "öğretmen modu" değeri. Aynı alanı act_move.c (5: stalker),
 * fight.c (10) ve mob_prog.c (16/17: din) de kullanır; çakışmasın.
 */
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

	if ( sn == gsn_vampire || sn == gsn_evolve_bear )
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

	if ( IS_NPC(mob) || mob->level != LEVEL_HERO )
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
    snprintf(buf, sizeof(buf), "%s konusunu zaten öğrendin.\n\r",
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
 if (IS_NPC(ch) || ch->level != LEVEL_HERO)
  {
    send_to_char("Bir kahraman olmalısın.\n\r",ch);
	return;
  }
 ch->status = PC_PRACTICER;
 send_to_char("Artık %100 olan yeteneklerini gençlere öğretebilirsin.\n\r",ch);
 return;
}
