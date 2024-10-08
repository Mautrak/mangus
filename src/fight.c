/***************************************************************************
 *                                                                         *
 * Uzak Diyarlar a��k kaynak T�rk�e Mud projesidir.                        *
 * Oyun geli�tirmesi Jai ve Maru taraf�ndan y�netilmektedir.               *
 * Unutulmamas� gerekenler: Nir, Kame, Randalin, Nyah, Sint                          *
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
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "merc.h"

int unlink(const char *pathname);

#define MAX_DAMAGE_MESSAGE 34

/* command procedures needed */
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sacrifice     );
DECLARE_DO_FUN(do_quit_count    );
DECLARE_DO_FUN(do_sleep		);
DECLARE_DO_FUN(do_tail		);
DECLARE_DO_FUN(do_crush		);
DECLARE_DO_FUN(do_dismount	);

DECLARE_SPELL_FUN(      spell_null              );

extern void do_visible( CHAR_DATA *ch, char *argument );

/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_block	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_blink	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_hand	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_cross	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, bool immune ,int dam_type) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	death_cry_org	args( ( CHAR_DATA *ch, int part) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim,
			    int total_levels,int members ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ,bool secondary) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim ) );
void	raw_kill_org	args( ( CHAR_DATA *victim, int part ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ,int disarm_second) );
void	check_weapon_destroy	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool second) );
void	damage_to_object	args( (CHAR_DATA *ch, OBJ_DATA *wield, OBJ_DATA *worn, int damage) );
int	critical_strike	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam ) );
int	ground_strike	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam ) );
void	check_shield_destroyed	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool second) );
void	check_weapon_destroyed	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool second) );


bool	mob_cast_mage	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	mob_cast_cleric	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    say_spell       args( ( CHAR_DATA *ch, int sn ) );


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next	= ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;


	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	if (!IS_NPC(victim))
	  ch->last_fought = victim;

/*	more efficient DOPPLEGANGER
	if (!IS_NPC(victim))
	  ch->last_fought =
	(is_affected(victim,gsn_doppelganger) && victim->doppel ) ?
			victim->doppel : victim;
*/

	ch->last_fight_time = current_time;


	for (obj = ch->carrying;obj != NULL; obj = obj->next_content)
	  {
	    if (IS_SET(obj->progtypes,OPROG_FIGHT))
	    {
	      if (ch->fighting == NULL) break; /* previously death victims! */
	      (obj->pIndexData->oprogs->fight_prog) (obj,ch);
	    }
	  }

	if ((victim = ch->fighting) == NULL) continue; /* death victim */

	if (IS_SET(ch->progtypes,MPROG_FIGHT) && (ch->wait <= 0))
	  (ch->pIndexData->mprogs->fight_prog) (ch,victim);


	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;

	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch)
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_emote(rch,(char*)"���l�k atarak sald�r�yor!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(rch) || IS_AFFECTED(rch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM))
		&&   is_same_group(ch,rch) )
		    multi_hit (rch,victim,TYPE_UNDEFINED);

		continue;
	    }

	    if (!IS_NPC(ch) && RIDDEN(rch) == ch )
	    {
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* now check the NPC cases */

 	    if (IS_NPC(ch))

	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && RACE(rch) == RACE(ch)
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))

		||   (rch->pIndexData == ch->pIndexData
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_range(0,1) == 0)
			continue;

		    target = NULL;
		    number = 0;

		    for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_emote(rch,(char*)"���l�k atarak sald�r�yor!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;

    /* decrement the wait */
    if (ch->desc == NULL)
	ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    /* ridden's adjustment */
    if (RIDDEN(victim) && !IS_NPC(victim->mount))
	{
	 if (victim->mount->fighting == NULL
	     || victim->mount->fighting == ch)
	 victim = victim->mount;
	 else do_dismount(victim->mount,(char*)"");
	}

    /* no attacks on ghosts or attacks by ghosts */
    if ((!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST)) ||
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_GHOST)))
	return;

    if (IS_AFFECTED(ch,AFF_WEAK_STUN) )
	{
		act_color("$CSen $S sald�r�s�na kar��l�k veremeyecek kadar sersemledin.$c",
	 		ch,NULL,victim,TO_CHAR,POS_FIGHTING,CLR_WHITE );
	  act_color("$C$n sald�r�na kar��l�k veremeyecek kadar sersemledi.$c",
	 		ch,NULL,victim,TO_VICT,POS_FIGHTING,CLR_WHITE );
	  act_color("$C$n sersemlemi� g�r�n�yor.$c",
	 		ch,NULL,victim,TO_NOTVICT,POS_FIGHTING,CLR_WHITE );
	 REMOVE_BIT(ch->affected_by,AFF_WEAK_STUN);
	 return;
	}

    if (IS_AFFECTED(ch,AFF_STUN) )
	{
		act_color("$CSen $S sald�r�s�na kar��l�k veremeyecek kadar sersemledin.$c",
	 		ch,NULL,victim,TO_CHAR,POS_FIGHTING,CLR_WHITE );
	  act_color("$C$n sald�r�na kar��l�k veremeyecek kadar sersemledi.$c",
	 		ch,NULL,victim,TO_VICT,POS_FIGHTING,CLR_WHITE );
	  act_color("$C$n sersemlemi� g�r�n�yor.$c",
	 		ch,NULL,victim,TO_NOTVICT,POS_FIGHTING,CLR_WHITE );
	 affect_strip(ch,gsn_power_stun);
         SET_BIT(ch->affected_by,AFF_WEAK_STUN);
	 return;
	}

    if (IS_NPC(ch))
    {
	mob_hit(ch,victim,dt);
	return;
    }

    one_hit( ch, victim, dt ,FALSE);

    if (ch->fighting != victim)
	return;

   if (CLEVEL_OK(ch,gsn_area_attack)
		&& number_percent() < get_skill(ch,gsn_area_attack) )
    {
	int count=0,max_count;
	CHAR_DATA *vch,*vch_next;

	check_improve( ch,gsn_area_attack, TRUE, 6);

	if (ch->level < 70)	max_count = 1;
	else if (ch->level < 80)	max_count = 2;
	else if (ch->level < 90)	max_count = 3;
	else max_count = 4;

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next_in_room;
	    if ((vch != victim && vch->fighting == ch))
		{
		 one_hit(ch,vch,dt,FALSE);
		 count++;
		}
	    if ( count == max_count ) break;
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE))
	one_hit(ch,victim,dt, FALSE);

    if ( ch->fighting != victim || dt == gsn_backstab || dt == gsn_cleave
	|| dt == gsn_ambush || dt == gsn_dual_backstab || dt == gsn_circle
	|| dt == gsn_assassinate || dt == gsn_vampiric_bite)
	return;

    chance = get_skill(ch,gsn_second_attack)/2;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt,FALSE );
	check_improve(ch,gsn_second_attack,TRUE,5);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/3;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt ,FALSE);
	check_improve(ch,gsn_third_attack,TRUE,6);
	if ( ch->fighting != victim )
	    return;
    }


    chance = get_skill(ch,gsn_fourth_attack)/5;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt ,FALSE);
	check_improve(ch,gsn_fourth_attack,TRUE,7);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch,gsn_fifth_attack)/6;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt ,FALSE);
	check_improve(ch,gsn_fifth_attack,TRUE,8);
	if ( ch->fighting != victim )
	    return;
    }

    chance = 20 + (int)(get_skill(ch,gsn_second_weapon) * 0.8);
    if ( number_percent( ) < chance )
    {
     if (get_wield_char(ch,TRUE))
	{
	 one_hit(ch , victim , dt, TRUE );
	 check_improve(ch, gsn_second_weapon,TRUE,2);
	 if (ch->fighting != victim )
		return;
	}
    }

    chance = get_skill(ch,gsn_secondary_attack) / 4;
    if ( number_percent( ) < chance )
    {
     if (get_wield_char(ch,TRUE))
	{
	 one_hit(ch , victim , dt, TRUE );
	 check_improve(ch, gsn_secondary_attack,TRUE,2);
	 if (ch->fighting != victim )
		return;
	}
    }

    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    CHAR_DATA *vch, *vch_next;

    /* no attacks on ghosts */
    if (!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST))
	return;

    /* no attack by ridden mobiles except spec_casts */
    if (RIDDEN(ch) )
	{
	 if (ch->fighting != victim) set_fighting(ch,victim);
	 return;
	}

    one_hit(ch,victim,dt,FALSE);

    if (ch->fighting != victim)
	return;


    /* Area attack -- BALLS nasty! */

    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next_in_room;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt,FALSE);
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_SET(ch->off_flags,OFF_FAST))
	one_hit(ch,victim,dt,FALSE);

    if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle ||
	dt == gsn_dual_backstab || dt == gsn_cleave || dt == gsn_ambush
		|| dt == gsn_vampiric_bite)
	return;

    chance = get_skill(ch,gsn_second_attack)/2;
    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/4;
    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,gsn_fourth_attack)/6;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt ,FALSE);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch,gsn_second_weapon) / 2;
    if ( number_percent( ) < chance )
    {
     if (get_wield_char(ch,TRUE))
	{
	 one_hit(ch , victim , dt, TRUE );
	 if (ch->fighting != victim )
		return;
	}
    }

    if (IS_SET(ch->act,ACT_MAGE))
    {
	if ( number_percent() < 60 &&  ch->spec_fun == NULL )
	{
	   mob_cast_mage(ch,victim);
	   return;
	}
    }


    if (IS_SET(ch->act,ACT_CLERIC))
    {
	if ( number_percent() < 60 &&  ch->spec_fun == NULL )
	{
	   mob_cast_cleric(ch,victim);
	   return;
	}
    }

    /* PC waits */

    if (ch->wait > 0)
	return;

    /* now for the skills */

    number = number_range(0,7);

    switch(number)
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_bash(ch,(char*)"");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_berserk(ch,(char*)"");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM)
	|| (get_weapon_sn(ch,FALSE) != gsn_hand_to_hand
	&& (IS_SET(ch->act,ACT_WARRIOR)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_disarm(ch,(char*)"");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_kick(ch,(char*)"");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_dirt(ch,(char*)"");
	break;

    case (5) :
	if (IS_SET(ch->off_flags,OFF_TAIL))
	  do_tail(ch,(char*)"");
	break;

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_trip(ch,(char*)"");
	break;
    case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))
	    do_crush(ch,(char*)"");
	break;
    }
}

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt ,bool secondary)
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam, i;
    int diceroll;
    int sn,skill;
    int dam_type;
    bool counter;
    bool result;
    OBJ_DATA *corpse;
    int sercount;
	char eventbuf[MAX_STRING_LENGTH];

    sn = -1;
    counter = FALSE;

    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /* ghosts can't fight */
    if ((!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST)) ||
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_GHOST)))
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
	if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
		return;

	familya_check_improve(ch, victim);

    /*
     * Figure out the type of damage message.
     */

    wield = get_wield_char( ch, secondary );

    /*
     * if there is no weapon held by pro-hand, and there is a weapon
     * in the other hand, than don't fight with punch.
     */
    if (!secondary && dt == TYPE_UNDEFINED
	&& wield == NULL && get_wield_char(ch, TRUE))
    {
	if (ch->fighting != victim)
	{
	    secondary = TRUE;
	    wield = get_wield_char(ch, secondary);
	}
	else return;
    }

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch,secondary);
    skill = 20 + get_weapon_skill(ch,sn);
    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = class_table[ch->iclass].thac0_00;
	thac0_32 = class_table[ch->iclass].thac0_32;
    }

    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    if (thac0 < 0)
        thac0 = thac0/2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

    if (dt == gsn_dual_backstab)
        thac0 -= 10 * (100 - get_skill(ch,gsn_dual_backstab));

    if (dt == gsn_cleave)
        thac0 -= 10 * (100 - get_skill(ch,gsn_cleave));

    if (dt == gsn_ambush)
        thac0 -= 10 * (100 - get_skill(ch,gsn_ambush));

    if (dt == gsn_vampiric_bite)
	thac0 -= 10 * (100 - get_skill(ch,gsn_vampiric_bite));

    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    };

    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;

    if ( get_skill(victim,gsn_armor_use) > 70)
	{
	 check_improve(victim,gsn_armor_use,TRUE,8);
	 victim_ac -= (victim->level) / 2;
	}

    if ( !can_see( ch, victim ) )
	{
	 if ( ch->level > skill_table[gsn_blind_fighting].skill_level[ch->iclass]
		&& number_percent() < get_skill(ch,gsn_blind_fighting) )
		{
		 check_improve(ch,gsn_blind_fighting,TRUE,16);
		}
	 else victim_ac -= 4;
	}

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;

    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_range(0,31) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type, TRUE );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */

/*
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format)
	{
	    dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	    if ( wield != NULL )
	    	dam += dam / 2;
	}
	else
	    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
*/
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	if ( wield != NULL )
	{
	    if (wield->pIndexData->new_format)
		dam = dice(wield->value[1],wield->value[2]) * skill/100;
	    else
	    	dam = number_range( wield->value[1] * skill/100,
				wield->value[2] * skill/100);

	    if (get_shield_char(ch) == NULL)  /* no shield = more */
		dam = dam * 21/20;

            /* sharpness! */
            if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
            {
                int percent;

                if ((percent = number_percent()) <= (skill / 8))
                    dam = 2 * dam + (dam * 2 * percent / 100);
            }
	    /* holy weapon */
            if (IS_WEAPON_STAT(wield,WEAPON_HOLY) &&
		  IS_GOOD(ch) && IS_EVIL(victim) && number_percent() < 30)
            {
							act_color("$C$p kutsal ���kla parl�yor.$c",
								ch,wield,NULL,TO_CHAR,POS_DEAD,CLR_YELLOW);
							act_color("$C$p kutsal ���kla parl�yor.$c",
								ch,wield,NULL,TO_ROOM,POS_DEAD,CLR_YELLOW);
                dam += dam * 120 / 100;
            }
	}
	else
	{
	  if (CLEVEL_OK(ch, gsn_hand_to_hand))
	  {
	      if (number_percent() < get_skill(ch, gsn_hand_to_hand) )
		dam = number_range(4+ch->level/10, 2*ch->level/3) * skill/100;
	      else
	      {
		dam = number_range(5, ch->level / 2) * skill/100;
		check_improve(ch, gsn_hand_to_hand, FALSE, 5);
	      }
	  }
	  else dam = number_range(5, ch->level / 2) * skill/100;

	  if ( get_skill(ch,gsn_master_hand) > 0 )
    	  {
	     int d;

       	    if ((d=number_percent()) <= get_skill(ch,gsn_master_hand))
       	    {
		check_improve(ch,gsn_master_hand,TRUE,6);
            	dam *= 2;
		if ( d < 10 )
		{
		   SET_BIT(victim->affected_by,AFF_WEAK_STUN);
			 act_color("$CSen $E sersemletici bir g��le vuruyorsun!$c",
			ch,NULL,victim,TO_CHAR,POS_DEAD,CLR_RED);
		   act_color("$C$n sana sersemletici bir g��le vuruyor!$c",
			ch,NULL,victim,TO_VICT,POS_DEAD,CLR_RED);
		   act_color("$C$n $E sersemletici bir g��le vuruyor!$c",
			ch,NULL,victim,TO_NOTVICT,POS_DEAD,CLR_RED);
            	   check_improve(ch,gsn_master_hand,TRUE,6);
		}
       	    }

	  }

	}
    }

    /*
     * Bonuses.
     */
    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
	    int div;
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
	    div = (ch->iclass == CLASS_WARRIOR) ? 100 :
			(ch->iclass == CLASS_CLERIC) ? 130 : 114;
            dam += dam * diceroll/div;
        }
    }

    if ( get_skill(ch,gsn_master_sword) > 0 && sn== gsn_sword)
    {
        if (number_percent() <= get_skill(ch,gsn_master_sword))
        {
	    OBJ_DATA *katana;

            check_improve(ch,gsn_master_sword,TRUE,6);
            dam += dam * 110 /100;

	    if ( (katana = get_wield_char(ch,FALSE)) != NULL )
	    {
	      AFFECT_DATA *paf;

	      if ( IS_WEAPON_STAT(katana,WEAPON_KATANA)
		  && strstr(katana->extra_descr->description,ch->name) != NULL )
	      {
		katana->cost++;
		if (katana->cost > 180)
		{
		   paf =  affect_find(katana->affected,gsn_katana);
		   if (paf != NULL)
		   {
			int old_mod = paf->modifier;
			paf->modifier = UMIN((paf->modifier+1),(ch->level / 3));
			ch->hitroll += paf->modifier - old_mod;
			if (paf->next != NULL)
			{
				 paf->next->modifier = paf->modifier;
				 ch->damroll += paf->modifier - old_mod;
			}
			act("$s katanas� mavi renkte parl�yor.\n\r",ch,NULL,NULL,TO_ROOM);
			send_to_char("Katanan mavi renkte parl�yor.\n\r",ch);
		   }
		   katana->cost = 0;
		}
	      }
	    }
	    else if ( (katana=get_wield_char(ch,TRUE)) != NULL)
	    {
	      AFFECT_DATA *paf;

	      if ( IS_WEAPON_STAT(katana,WEAPON_KATANA)
		  && strstr(katana->extra_descr->description,ch->name) != NULL )
	      {
		katana->cost++;
		if (katana->cost > 180)
		{
		   paf =  affect_find(katana->affected,gsn_katana);
		   if (paf != NULL)
		   {
			int old_mod = paf->modifier;
			paf->modifier = UMIN((paf->modifier+1),(ch->level / 3));
			ch->hitroll += paf->modifier - old_mod;
			if (paf->next != NULL)
			{
				 paf->next->modifier = paf->modifier;
				 ch->damroll += paf->modifier - old_mod;
			}
			act("$s katanas� mavi renkte parl�yor.\n\r",ch,NULL,NULL,TO_ROOM);
			send_to_char("Katanan mavi renkte parl�yor.\n\r",ch);
		   }
		   katana->cost = 0;
		}
	      }
	    }
        }
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    sercount = number_percent();

    if (dt==gsn_backstab || dt==gsn_vampiric_bite)
	sercount += 40;  /* 80% chance decrease of counter */

    if ( victim->last_fight_time != -1 &&
        (current_time - victim->last_fight_time) < FIGHT_DELAY_TIME)
	sercount += 25;	/* 50% chance decrease of counter */

    sercount *= 2;

    if (victim->fighting == NULL && !IS_NPC(victim) &&
	 !is_safe_nomessage(victim, ch) && !is_safe_nomessage(ch,victim) &&
	(victim->position == POS_SITTING || victim->position == POS_STANDING)
	&& dt != gsn_assassinate &&
	(sercount <= get_skill(victim,gsn_counter) ))
      {
	counter = TRUE;
	check_improve(victim,gsn_counter,TRUE,1);
	act("$N sald�r�n� sana �eviriyor!",ch,NULL,victim,TO_CHAR);
	act("Sen $s sald�r�s�n� ona �eviriyorsun!",ch,NULL,victim,TO_VICT);
	act("$N $s sald�r�s�n� ona �eviriyor!",ch,NULL,victim,TO_NOTVICT);
	ch->fighting = victim;
      }
    else if (!victim->fighting) check_improve(victim,gsn_counter,FALSE,1);

    if ( dt == gsn_backstab && wield != NULL)
      dam = (1 + ch->level/8) * dam + ch->level;

    else if ( dt == gsn_dual_backstab && wield != NULL)
      dam = (1 + ch->level/12) * dam + ch->level;

    else if (dt == gsn_circle)
      dam = (ch->level/35 + 1) * dam + ch->level;

    else if ( dt == gsn_vampiric_bite && IS_VAMPIRE(ch))
      dam = (ch->level/14 + 1) * dam + ch->level;

    else if ( dt == gsn_cleave && wield != NULL)
      {
	if (number_percent() < URANGE(4, 5+(ch->level-victim->level),10) && !counter)
	  {
			act_color("Yarma hareketin $M $C�K�YE B�L�YOR!$c",
		      ch,NULL,victim,TO_CHAR,POS_RESTING,CLR_RED);
	    act_color("$s yarma hareketi seni $C�K�YE B�L�YOR!$c",
		      ch,NULL,victim,TO_VICT,POS_RESTING,CLR_RED);
	    act_color("$s yarma hareketi $M $C�K�YE B�L�YOR!$c",
		      ch,NULL,victim,TO_NOTVICT,POS_RESTING,CLR_RED);
	    send_to_char("� L D � R � L D � N!\n\r",victim);
	    act("$n �LD�!",victim,NULL,NULL,TO_ROOM);

		/* event */
		if (!IS_NPC(victim))
		{
		sprintf(eventbuf,"%s, %s taraf�ndan ikiye b�l�nerek �ld�r�ld�.",victim->name,ch->name);
		write_event_log(eventbuf);
		}

	    WAIT_STATE( ch, 2 );
	    raw_kill(victim);
	    if ( !IS_NPC(ch) && IS_NPC(victim) )
	      {
		corpse = get_obj_list( ch, (char*)"ceset", ch->in_room->contents );

		if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		    corpse && corpse->contains) /* exists and not empty */
		  do_get( ch, (char*)"t�m� ceset" );

		if (IS_SET(ch->act,PLR_AUTOAKCE) &&
		    corpse && corpse->contains ) /* exists and not empty */
		 {
		  do_get(ch, (char*)"ak�e ceset");
		}

		if ( IS_SET(ch->act, PLR_AUTOSAC) )
		{
		  if (IS_SET(ch->act,PLR_AUTOLOOT) && corpse
		      && corpse->contains)
		    return;  /* leave if corpse has treasure */
		  else
		    do_sacrifice( ch, (char*)"ceset" );
		    }
	      }
	    return;
	  }
	else dam = (dam * 2 + ch->level);
      }

    if (dt == gsn_assassinate)
      {
	if (number_percent() <= URANGE(10, 20+(ch->level-victim->level)*2, 50) && !counter)
	  {
			act_color("Sen $E $C+++SU�KAST+++$c d�zenledin!",ch,NULL,victim,TO_CHAR,
		      POS_RESTING,CLR_RED);
	    act("$N �LD�!",ch,NULL,victim,TO_CHAR);
	    act_color("$n $E $C+++SU�KAST+++$c d�zenledi!",ch,NULL,victim,
		      TO_NOTVICT,POS_RESTING,CLR_RED);
	    act("$N �LD�!",ch,NULL,victim,TO_NOTVICT);
	    act_color("$n sana $C+++SU�KAST+++$c d�zenledi!",ch,NULL,victim,
		      TO_VICT,POS_DEAD,CLR_RED);
	    send_to_char("� L D � R � L D � N!\n\r",victim);

		/* event */
		if (!IS_NPC(victim))
		{
		sprintf(eventbuf,"%s, %s taraf�ndan suikastle �ld�r�ld�.",victim->name,ch->name);
		write_event_log(eventbuf);
		}

	    check_improve(ch,gsn_assassinate,TRUE,1);
	    raw_kill(victim);
	    if ( !IS_NPC(ch) && IS_NPC(victim) )
	      {
		corpse = get_obj_list( ch, (char*)"ceset", ch->in_room->contents );

		if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		    corpse && corpse->contains) /* exists and not empty */
		  do_get( ch, (char*)"t�m� ceset" );

		if (IS_SET(ch->act,PLR_AUTOAKCE) &&
		    corpse && corpse->contains ) /* exists and not empty */
		  {
				do_get(ch, (char*)"ak�e ceset");
			}

		if ( IS_SET(ch->act, PLR_AUTOSAC) )
		{
		  if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse
		      && corpse->contains)
		    return;  /* leave if corpse has treasure */
		  else
		    do_sacrifice( ch, (char*)"ceset" );
		    }
	      }
	    return;

	  }
	else
	  {
	    check_improve(ch,gsn_assassinate,FALSE,1);
	    dam *= 2;
	  }
      }


    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

		/*
		 * karizma etkisi
		 */

		//�nce vuran�n karizmas�
		if(!IS_NPC(ch))
			dam = (dam * cha_app[get_curr_stat(ch,STAT_CHA)].extra_zarar);
		//victim'�n karizmas�
		if(!IS_NPC(victim))
			dam = (dam / cha_app[get_curr_stat(victim,STAT_CHA)].extra_zarar);

			/*
			 * karizma etkisi bitti
			 */

    if (dt == gsn_ambush)
      dam *= 3;

			/*
			 * �rk bilgisi etkisi (familya)
			 */
			if(!IS_NPC(ch))
			{
				if ( ch->pcdata->familya[victim->race] > 0 &&
						 number_range(1,200) < ch->pcdata->familya[victim->race] &&
						 dam > 0 )
				{
					i = number_range(1,100);
					if (i<=5)
					{
						dam = dam * 5;
					}
					else if (i>5 && i<=10)
					{
						dam = dam * 3;
					}
					else if (i>15 && i<=30)
					{
						dam = dam * 2;
					}
					else
					{
						dam = int(dam * 3 / 2);
					}
					printf_to_char(ch,"{gBirden %s �rk�na ili�kin bilgini kullanabilece�in bir an yakal�yorsun.{x\n\r",race_table[victim->race].name[1]);
				}
			}
			/*
			 * �rk bilgisi etkisi bitti
			 */

			/*
			 * Kabal muhafizlarinin vurus gucu artirilsin.
			 */
			if(IS_NPC(ch))
			{
				if( ch->spec_fun == spec_lookup( (char*)"spec_fight_enforcer" ) ||
			  		ch->spec_fun == spec_lookup( (char*)"spec_fight_invader" ) ||
						ch->spec_fun == spec_lookup( (char*)"spec_fight_ivan" ) ||
						ch->spec_fun == spec_lookup( (char*)"spec_fight_seneschal" ) ||
						ch->spec_fun == spec_lookup( (char*)"spec_fight_powerman" ) ||
						ch->spec_fun == spec_lookup( (char*)"spec_fight_protector" ) ||
						ch->spec_fun == spec_lookup( (char*)"spec_fight_hunter" ) ||
						ch->spec_fun == spec_lookup( (char*)"spec_fight_lionguard" ) )
						{
							dam *= 4;
						}

			}
			/*
			 * Kabal muhafizlari duzenlemesi bitti
			 */

    if (!IS_NPC(ch) && get_skill(ch,gsn_deathblow) > 1 &&
	ch->level >= skill_table[gsn_deathblow].skill_level[ch->iclass] )
      {
	if (number_percent() < 0.125 * get_skill(ch,gsn_deathblow))
	  {
			act("�ld�r�c� bir r�zgar getiriyorsun!",ch,NULL,NULL,TO_CHAR);
	    act("$n �ld�r�c� bir r�zgar getiriyor!",ch,NULL,NULL,TO_ROOM);
	    if (cabal_ok(ch,gsn_deathblow)) {
	      dam *= (int)(((float)ch->level) / 20);
	      check_improve(ch,gsn_deathblow,TRUE,1);
	    }
	  }
	else check_improve(ch,gsn_deathblow,FALSE,3);
      }

    if ( dam <= 0 )
	dam = 1;

    if (counter)
      {
	result = damage(ch,ch,2*dam,dt,dam_type,TRUE);
	multi_hit(victim,ch,TYPE_UNDEFINED);
      }
    else result = damage( ch, victim, dam, dt, dam_type, TRUE );

    /* vampiric bite gives hp to ch from victim */
	if (dt == gsn_vampiric_bite)
	{
	 int hit_ga = UMIN( (dam / 2 ), victim->max_hit );

	 ch->hit += hit_ga;
    	 ch->hit  = UMIN( ch->hit , ch->max_hit);
	 update_pos( ch );
	 send_to_char("Kurban�n�n kan�n� emdik�e sa�l���na kavu�uyorsun.\n\r",ch);
	}

    /* but do we have a funky weapon? */
    if (result && wield != NULL)
    {
        int dam;

        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
        {
            int level;
            AFFECT_DATA *poison, af;

            if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
                level = wield->level;
            else
                level = poison->level;
            if (!saves_spell(level / 2,victim,DAM_POISON))
            {
							send_to_char("Zehrin damarlar�nda dola�t���n� hissediyorsun.\n\r",victim);
							act("$n $p �zerindeki zehirle zehirlendi.",
                    victim,wield,NULL,TO_ROOM);

                af.where     = TO_AFFECTS;
                af.type      = gsn_poison;
                af.level     = level * 3/4;
                af.duration  = level / 2;
                af.location  = APPLY_STR;
                af.modifier  = -1;
                af.bitvector = AFF_POISON;
                affect_join( victim, &af );
            }

            /* weaken the poison if it's temporary */
            if (poison != NULL)
            {
                poison->level = UMAX(0,poison->level - 2);
                poison->duration = UMAX(0,poison->duration - 1);
                if (poison->level == 0 || poison->duration == 0)
								act("$p �zerindeki zehrin etkisi ge�iyor.",ch,wield,NULL,TO_CHAR);
            }
        }
        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
        {
            dam = number_range(1, wield->level / 5 + 1);
						act("$p $z ya�am �ekiyor.",victim,wield,NULL,TO_ROOM);
            act("Ya�am�n�n  $p taraf�ndan emildi�ini hissediyorsun.",
                victim,wield,NULL,TO_CHAR);
            damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
            ch->hit += dam/2;
        }
        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
        {
            dam = number_range(1,wield->level / 4 + 1);
						act("$n $p taraf�ndan kavruluyor.",victim,wield,NULL,TO_ROOM);
            act("$p etini kavuruyor.",victim,wield,NULL,TO_CHAR);
            fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_FIRE,FALSE);
        }
        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
        {
            dam = number_range(1,wield->level / 6 + 2);
						act("$p $m donduruyor.",victim,wield,NULL,TO_ROOM);
            act("$p objesinin so�uk dokunu�u seni buzla �evreliyor.",
                victim,wield,NULL,TO_CHAR);
            cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_COLD,FALSE);
        }
        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
        {
            dam = number_range(1,wield->level/5 + 2);
						act("$n $p objesinin y�ld�r�m�yla vuruluyor.",victim,wield,NULL,TO_ROOM);
            act("$p seni �ok etti.",victim,wield,NULL,TO_CHAR);
            shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
        }
    }

    tail_chain( );
    return;
}



/*
 * Inflict damage from a hit.
 */
bool damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type, bool show )
{
	OBJ_DATA *corpse;
	bool immune;
	int lost_exp;
	char eventbuf[MAX_STRING_LENGTH];

	if  ( victim->position == POS_DEAD)
	{
		return FALSE;
	}

	/*
	* Stop up any residual loopholes.
	*/
	if ( dam > 1000 && !IS_IMMORTAL(ch))
	{
		char buf[MAX_STRING_LENGTH];
		sprintf(buf,"%s:Damage more than 1000 points :%d",ch->name,dam);
		bug( buf,0);
		if (IS_NPC(ch) && !IS_NPC(ch))
		{
			dam = 1000;
		}
	}


	if ( victim != ch )
	{
		/*
		* Certain attacks are forbidden.
		* Most other attacks are returned.
		*/
		if ( is_safe( ch, victim ) )
		{
			return FALSE;
		}

		if ( victim->position > POS_STUNNED )
		{
			if ( victim->fighting == NULL )
			{
				set_fighting( victim, ch );
			}
			if (victim->timer <= 4)
			{
				victim->position = POS_FIGHTING;
			}
		}

		if ( victim->position > POS_STUNNED )
		{
			if ( ch->fighting == NULL )
			set_fighting( ch, victim );

			/*
			* If victim is charmed, ch might attack victim's master.
			*/
			if ( IS_NPC(ch)
			&&   IS_NPC(victim)
			&&   IS_AFFECTED(victim, AFF_CHARM)
			&&   victim->master != NULL
			&&   victim->master->in_room == ch->in_room
			&&   number_range(0,7) == 0 )
			{
				stop_fighting( ch, FALSE );
				multi_hit( ch, victim->master, TYPE_UNDEFINED );
				return FALSE;
			}
		}

		/*
		* More charm and group stuff.
		*/
		if ( victim->master == ch )
		{
			stop_follower( victim );
		}

		if ( MOUNTED(victim) == ch || RIDDEN(victim) == ch )
		{
			victim->riding = ch->riding = FALSE;
		}
	}

	/*
	* No one in combat can sneak, hide, or be invis or camoed.
	*/
	if ( IS_SET(ch->affected_by, AFF_HIDE) || IS_SET(ch->affected_by, AFF_INVISIBLE)
			|| IS_SET(ch->affected_by, AFF_SNEAK) || IS_SET(ch->affected_by, AFF_FADE)
			|| IS_SET(ch->affected_by, AFF_CAMOUFLAGE) || IS_SET(ch->affected_by, AFF_IMP_INVIS)
			|| CAN_DETECT(ch, ADET_EARTHFADE) )
	{
		do_visible(ch, (char*)"");
	}

	/*
	* Damage modifiers.
	*/
	if ( IS_AFFECTED(victim, AFF_SANCTUARY) &&
	!( (dt == gsn_cleave) && (number_percent() < 50) ) )
	{
		dam /= 2;
	}
	else if ( CAN_DETECT(victim,ADET_PROTECTOR) )
	{
		dam = (3 * dam)/5;
	}

	if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
	{
		dam -= dam / 4;
	}

	if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )
	{
		dam -= dam / 4;
	}

	if ( CAN_DETECT(victim, ADET_AURA_CHAOS) && victim->cabal == CABAL_CHAOS
	&& (!IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED)) )
	{
		dam -= dam / 4;
	}

	if (is_affected(victim, gsn_protection_heat) && (dam_type == DAM_FIRE) )
	{
		dam -= dam / 4;
	}

	if (is_affected(victim, gsn_protection_cold) && (dam_type == DAM_COLD) )
	{
		dam -= dam / 4;
	}

	immune = FALSE;

	if (dt > 0 && dt < MAX_SKILL )
	{
		if (CAN_DETECT(victim, ADET_ABSORB)
		&& skill_table[dt].target == TAR_CHAR_OFFENSIVE
		&& skill_table[dt].spell_fun != spell_null
		&& ch != victim
		&& (number_percent() < 2*get_skill(victim, gsn_absorb)/3)
		/* update.c damages */
		&& dt != gsn_poison
		&& dt != gsn_plague
		&& dt != gsn_witch_curse
		/* update.c damages */
		&& dt != gsn_mental_knife
		&& dt != gsn_lightning_breath )
		{
			act("B�y�n $S enerji alan�n� ge�emiyor!",ch,NULL,victim,TO_CHAR);
			act("Sen $s b�y�s�n� so�uruyorsun!",ch,NULL,victim,TO_VICT);
			act("$N $s b�y�s�n� so�uruyor!",ch,NULL,victim,TO_NOTVICT);
			check_improve(victim,gsn_absorb,TRUE,1);
			victim->mana += skill_table[dt].min_mana;
			return FALSE;
		}

		if (CAN_DETECT(victim, ADET_SPELLBANE)
		&& skill_table[dt].target != TAR_IGNORE
		&& skill_table[dt].spell_fun != spell_null
		&& (number_percent() < 2*get_skill(victim, gsn_spellbane)/3)
		/* update.c damages */
		&& dt != gsn_poison
		&& dt != gsn_plague
		&& dt != gsn_witch_curse
		/* spellbane passing spell damages */
		&& dt != gsn_mental_knife
		&& dt != gsn_lightning_breath )
		{
			act("$N senin b�y�n� sapt�r�yor!",ch,NULL,victim,TO_CHAR);
			act("Sen $s b�y�s�n� sapt�r�yorsun!",ch,NULL,victim,TO_VICT);
			act("$N $s b�y�s�n� sapt�r�yor!",ch,NULL,victim,TO_NOTVICT);
			check_improve(victim,gsn_spellbane,TRUE,1);
			damage(victim,ch,3 * victim->level,gsn_spellbane,DAM_NEGATIVE, TRUE);
			return FALSE;
		}
	}

	/*
	* Check for parry, and dodge.
	*/
	if ( dt >= TYPE_HIT && ch != victim)
	{
		/*
		* Some funny stuf.
		*/
		if (is_affected(victim,gsn_mirror))
		{
			act("$n k�r�larak k���k cam par�alar�na ayr�l�yor.",
			victim,NULL,NULL,TO_ROOM);
			extract_char(victim,TRUE);
			return FALSE;
		}

		if ( check_parry( ch, victim ) )
			return FALSE;
		if ( check_cross( ch, victim ) )
			return FALSE;
		if ( check_block( ch, victim) )
			return FALSE;
		if ( check_dodge( ch, victim ) )
			return FALSE;
		if ( check_hand( ch, victim ) )
			return FALSE;
		if ( check_blink( ch, victim ) )
			return FALSE;
	}

	switch(check_immune(victim,dam_type))
	{
		case(IS_IMMUNE):
			immune = TRUE;
			dam = 0;
			break;
		case(IS_RESISTANT):
			dam -= dam/3;
			break;
		case(IS_VULNERABLE):
			dam += dam/2;
			break;
	}

	if ( dt != TYPE_HUNGER &&  dt >= TYPE_HIT && ch != victim )
	{
		dam = critical_strike(ch, victim, dam);
		dam = ground_strike(ch, victim, dam);
	}

	if ( show )
	{
		dam_message( ch, victim, dam, dt, immune ,dam_type);
	}

	if (dam == 0)
	{
		return FALSE;
	}

	/* temporarily second wield doesn't inflict damage */
	if (dt != TYPE_HUNGER && ( dt >= TYPE_HIT && ch != victim) )
	{
		check_weapon_destroy( ch, victim , FALSE);
	}

	/*
	* Hurt the victim.
	* Inform the victim of his new state.
	* make sure that negative overflow doesn't happen!
	*/
	if (dam < 0 || dam > (victim->hit + 16))
	{
		victim->hit = -16;
	}
	else
	{
		victim->hit -= dam;
	}

	if ( !IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1 )
	{
		victim->hit = 1;
	}

	update_pos( victim );

	switch( victim->position )
	{
		case POS_MORTAL:
			if ( dam_type == DAM_HUNGER || dam_type == DAM_THIRST)
			{
				break;
			}
			act("$n k�t� �ekilde yaralanm��, iyile�tirilmezse �lecek.",victim, NULL, NULL, TO_ROOM );
			send_to_char("K�t� �ekilde yaraland�n, iyile�tirilmezsen �leceksin.\n\r",victim );
			break;

		case POS_INCAP:
			if ( dam_type == DAM_HUNGER || dam_type == DAM_THIRST)
			{
				break;
			}
			act("$n aciz durumda ve iyile�tirilmezse �lecek.",victim, NULL, NULL, TO_ROOM );
			send_to_char("Aciz durumdas�n ve iyile�tirilmezsen �leceksin.\n\r",victim );
			break;

		case POS_STUNNED:
			if ( dam_type == DAM_HUNGER || dam_type == DAM_THIRST)
			{
				break;
			}
			act( "$n sersemlemi� fakat kendine gelecektir.",victim, NULL, NULL, TO_ROOM );
			send_to_char("Sersemledin fakat kendine geleceksin.\n\r",victim );
			break;

		case POS_DEAD:
			act( "$n �LD�!!", victim, 0, 0, TO_ROOM );
			send_to_char("� L D � R � L D � N!!\n\r\n\r", victim );
			/* event */
			if (!IS_NPC(victim))
			{
			sprintf(eventbuf,"%s �ld�.",victim->name);
			write_event_log(eventbuf);
			}
			break;

		default:
			if ( dam_type == DAM_HUNGER || dam_type == DAM_THIRST)
			{
				break;
			}
			if ( dam > victim->max_hit / 4 )
			{
				send_to_char( "Bu ger�ekten ACITTI!\n\r", victim );
			}
			if ( victim->hit < victim->max_hit / 4 )
			{
				send_to_char( "KANAMAN var!\n\r", victim );
			}
			break;
	}

	/*
	* Sleep spells and extremely wounded folks.
	*/
	if ( !IS_AWAKE(victim) )
	{
		stop_fighting( victim, FALSE );
	}

	/*
	* Payoff for killing things.
	*/
	if ( victim->position == POS_DEAD )
	{

		group_gain( ch, victim );

		if ( !IS_NPC(victim) )
		{

			/*
			* Dying penalty:
			* 2/3 way back.
			*/
			if (victim == ch || (IS_NPC(ch) && (ch->master == NULL && ch->leader == NULL)) || IS_SET(victim->act,PLR_WANTED))
			{
				if ( victim->exp > exp_per_level(victim,victim->pcdata->points) * victim->level )
				{
					lost_exp = (2 * ( exp_per_level(victim,victim->pcdata->points) * victim->level - victim->exp) / 3 ) + 50;
					gain_exp( victim , lost_exp);
				}
			}

			/*
			*  Die too much and deleted ... :(
			*/
			if ( !IS_NPC(victim) && ( victim == ch || (IS_NPC(ch) && (ch->master == NULL && ch->leader == NULL))
					|| IS_SET(victim->act,PLR_WANTED) ) )
			{
				if(victim->level >= KIDEMLI_OYUNCU_SEVIYESI && !IS_IMMORTAL(victim))
				{
					victim->pcdata->death++;

					if ( (victim->pcdata->death % 3) == 2)
					{
						victim->perm_stat[STAT_CHA] = UMAX(1,victim->perm_stat[STAT_CHA]-1);
						victim->perm_stat[STAT_CON] = UMAX(3,victim->perm_stat[STAT_CON]-1);
						printf_to_char( victim, "{RBu �l�mle b�nye ve karizman�n azald���n� hissediyorsun.{x\n\r" );
					}

					if( victim->perm_stat[STAT_CON] == 3 )
					{
						char strsave[160];
						printf_to_char( victim, "{RBir hayalete d�n��erek d�nya ger�ekli�ini terkediyorsun.{x\n\r" );
						act("$n �ld� ve bir daha d�nemeyecek.\n\r",victim,NULL,NULL,TO_ROOM);
						victim->last_fight_time = -1;
						victim->hit = 1;
						victim->position = POS_STANDING;
						sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );
						wiznet("$N b�nyesi yetersiz geldi�i i�in silindi.",ch,NULL,0,0,0);
						do_quit_count(victim,(char*)"");
						unlink(strsave);
						return TRUE;
					}

					if ( victim->iclass == CLASS_SAMURAI && victim->pcdata->death > 15 )
					{
						char strsave[160];
						printf_to_char( victim, "{RBir hayalete d�n��erek d�nya ger�ekli�ini terkediyorsun.{x\n\r" );
						act("$n �ld� ve bir daha d�nemeyecek.\n\r",victim,NULL,NULL,TO_ROOM);
						victim->last_fight_time = -1;
						victim->hit = 1;
						victim->position = POS_STANDING;
						sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );
						wiznet("$N 15 �l�m� ge�ti�i i�in silindi.",ch,NULL,0,0,0);
						do_quit_count(victim,(char*)"");
						unlink(strsave);
						return TRUE;
					}
				}
			}
		}

		raw_kill( victim );

		/* don't remember killed victims anymore */
		if (IS_NPC(ch))
		{
			if ( ch->pIndexData->vnum == MOB_VNUM_STALKER )
			{
				ch->status = 10;
			}
			remove_mind(ch,victim->name);
			if ( IS_SET(ch->act, ACT_HUNTER) && ch->hunting == victim )
			{
				ch->hunting = NULL;
				REMOVE_BIT(ch->act, ACT_HUNTER);
			}
		}

		/* RT new auto commands */

		if ( !IS_NPC(ch) && IS_NPC(victim) )
		{
			corpse = get_obj_list( ch, (char*)"ceset", ch->in_room->contents );

			if ( IS_SET(ch->act, PLR_AUTOLOOT) && corpse && corpse->contains) /* exists and not empty */
			{
				do_get( ch, (char*)"t�m� ceset" );
			}

			if (IS_SET(ch->act,PLR_AUTOAKCE) && corpse && corpse->contains ) /* exists and not empty */
			{
				do_get(ch, (char*)"ak�e ceset");
			}

			if ( ch->iclass == CLASS_VAMPIRE && ch->level > 10 && corpse)
			{
				act_color( "$C$n $S cesedinden kan emiyor!!$c",ch, NULL,victim,TO_ROOM,POS_SLEEPING,CLR_RED_BOLD);
				send_ch_color("$CCesetten kan emiyorsun!!$c\n\r\n\r",ch,POS_SLEEPING,CLR_RED_BOLD );
				gain_condition(ch,COND_BLOODLUST,4);
				
				/*
				 * Vampirlerin emdikleri kandan YP iyile�tirmeleri i�in
				 */
				if(victim->level >= ch->level)
				{
					ch->hit += ch->level * 2;
					ch->hit = UMIN(ch->hit,ch->max_hit);
					send_to_char("Emdi�in kanla �ifa buluyorsun!!!\n\r",ch);
				}
				else if(ch->level - victim->level < 5)
				{
					ch->hit += ch->level;
					ch->hit = UMIN(ch->hit,ch->max_hit);
					send_to_char("Emdi�in kanla �ifa buluyorsun!!\n\r",ch);
				}
				else if(ch->level - victim->level < 10)
				{
					ch->hit += int(ch->level / 3);
					ch->hit = UMIN(ch->hit,ch->max_hit);
					send_to_char("Emdi�in kanla �ifa buluyorsun!\n\r",ch);
				}
				else
				{
					send_to_char("Emdi�in de�ersiz kandan �ifa alman imkans�z!\n\r",ch);
				}
			}

			if ( IS_SET(ch->act, PLR_AUTOSAC) )
			{
				if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
				{
					return TRUE;  /* leave if corpse has treasure */
				}
				else
				{
					do_sacrifice( ch, (char*)"ceset" );
				}
			}
		}

		return TRUE;
	}

	if ( victim == ch )
	{
		return TRUE;
	}

	/*
	* Take care of link dead people.
	*/
	if ( !IS_NPC(victim) && victim->desc == NULL )
	{
		if ( number_range( 0, victim->wait ) == 0 )
		{
		if (victim->level < 11)
		{
			do_recall( victim, (char*)"" );
		}
		else
		{
			do_flee( victim, (char*)"" );
		}
		return TRUE;
		}
	}

	/*
	* Wimp out?
	*/
	if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
	{
		if ( ( ( IS_SET(victim->act, ACT_WIMPY) && number_range(0,3) == 0
				&& victim->hit < victim->max_hit / 5)
				||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
				&&     victim->master->in_room != victim->in_room ) )
				|| ( CAN_DETECT(victim,ADET_FEAR) && !IS_SET(victim->act,ACT_NOTRACK) ))
		{
			do_flee( victim, (char*)"" );
			victim->last_fought = NULL;
		}
	}

	if ( !IS_NPC(victim) && victim->hit > 0
	  && ( victim->hit <= victim->wimpy || CAN_DETECT(victim, ADET_FEAR) )
	  && victim->wait < PULSE_VIOLENCE / 2 )
	{
		do_flee( victim, (char*)"" );
	}

	tail_chain( );
	return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
  if (is_safe_nomessage(ch,victim))
    {
			act("Tanr�lar $M koruyor.",ch,NULL,victim,TO_CHAR);
      act("Tanr�lar $M $z koruyor.",ch,NULL,victim,TO_ROOM);
      return TRUE;
    }
  else return FALSE;
}



bool is_safe_nomessage(CHAR_DATA *ch, CHAR_DATA *victim )
{
  if (victim->fighting == ch || ch==victim)
    return FALSE;

  /* Ghosts are safe */
  if ((!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST)) ||
      (!IS_NPC(ch) && IS_SET(ch->act, PLR_GHOST)))
    return TRUE;

  /* link dead players whose adrenalin is not gushing are safe */
  if (!IS_NPC(victim) && ((victim->last_fight_time == -1) ||
	((current_time - victim->last_fight_time) > FIGHT_DELAY_TIME)) &&
	victim->desc == NULL)
    return TRUE;

     if  ((!IS_NPC(ch) &&  !IS_NPC(victim) && victim->level < 5 ) ||
         ( !IS_NPC(ch) &&  !IS_NPC(victim) && ch->level < 5 ))
  return TRUE;

  /* newly death staff */
  if (!IS_IMMORTAL(ch) && !IS_NPC(victim) &&
      ((ch->last_death_time != -1 && current_time - ch->last_death_time < 600 )
      || (victim->last_death_time != -1 &&
            current_time - victim->last_death_time < 600)) )
    return TRUE;

  /* level adjustement */
  if ( ch != victim && !IS_IMMORTAL(ch) && !IS_NPC(ch) && !IS_NPC(victim) &&
  	( ch->level >= (victim->level + UMAX(3,(int)(ch->level/18)) )
	  ||	 ch->level <= (victim->level - UMAX(3,(int)(ch->level/18))) )&&
  	( victim->level >= (ch->level + UMAX(3,(int)(victim->level/18)) )
		|| victim->level <= (ch->level - UMAX(3,(int)(victim->level/18))) ))
    return TRUE;
	
	// oyuncu katline katilmak istemeyen oyunculari haric tutalim
	if (IS_PC(ch) && IS_PC(victim) && (ch->pcdata->oyuncu_katli == 0 || victim->pcdata->oyuncu_katli == 0))
	{
		return TRUE;
	}

  return FALSE;
}


bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{

    if (ch == victim && !area)
	return TRUE;

    if (IS_IMMORTAL(victim) &&  area)
	return TRUE;

    if (is_same_group(ch,victim) && area)
      return TRUE;

    if (ch == victim && area && ch->in_room->sector_type == SECT_INSIDE)
      return TRUE;

    if ((RIDDEN(ch) == victim || MOUNTED(ch) == victim) && area)
      return TRUE;

    return is_safe(ch,victim);
}


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( get_wield_char( victim, FALSE ) == NULL )
	return FALSE;

    if ( IS_NPC(victim) )
    {
	chance	= UMIN( 40, victim->level );
    }
    else
    {
	chance	= get_skill(victim,gsn_parry) / 2;
	if ( victim->iclass == CLASS_WARRIOR || victim->iclass == CLASS_SAMURAI)
	chance = (int)(chance * 1.2);
    }


	if ( number_percent( ) >= chance + victim->level - ch->level )
		return FALSE;

	act("Sen $s sald�r�s�n� kar��l�yorsun.",  ch, NULL, victim, TO_VICT    );
	act( "$N sald�r�n� kar��l�yor.", ch, NULL, victim, TO_CHAR    );
    check_weapon_destroyed( ch, victim, FALSE);
    if ( number_percent() >  get_skill(victim,gsn_parry) )
    {
     /* size  and weight */
     chance += ch->carry_weight / 25;
     chance -= victim->carry_weight / 20;

     if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 25;
     else
	chance += (ch->size - victim->size) * 10;


     /* stats */
     chance += get_curr_stat(ch,STAT_STR);
     chance -= get_curr_stat(victim,STAT_DEX) * 4/3;

     if (IS_AFFECTED(ch,AFF_FLYING))
	chance -= 10;

     /* speed */
     if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
     if (IS_SET(victim->off_flags,OFF_FAST))
	chance -= 20;

     /* level */
     chance += (ch->level - victim->level) * 2;

     /* now the attack */
     if (number_percent() < ( chance / 20  ))
     {
			 act("Pozisyonunu korumaya devam edemiyorsun!",
		 		ch,NULL,victim,TO_VICT);
		 	act("Yere d��t�n!",ch,NULL,victim,TO_VICT);
		 	act("$N sald�r�na direnemiyor ve d���yor!",
		 		ch,NULL,victim,TO_CHAR);
		 	act("$s sersemletici g�c� $N d���r�yor.",
		 		ch,NULL,victim,TO_NOTVICT);

	WAIT_STATE(victim,skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;
     }
    }
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}

/*
 * check blink
 */
bool check_blink( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_BLINK_ON(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
        return FALSE;
    else
        chance  = victim->pcdata->learned[gsn_blink] / 2;

    if ( ( number_percent( ) >= chance + victim->level - ch->level )
	|| ( number_percent( ) < 50 )
	|| ( victim->mana < 10 ) )
        return FALSE;

    victim->mana -= UMAX(victim->level / 10,1);

		act( "Zayi olarak $s sald�r�s�n� atlat�yorsun.", ch, NULL, victim, TO_VICT    );
    act( "$N zayi olarak sald�r�n� atlat�yor.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_blink,TRUE,6);
    return TRUE;
}


/*
 * Check for shield block.
 */

bool check_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( get_shield_char( victim ) == NULL )
      return FALSE;

    if (IS_NPC(victim))
    {
      chance = 10;
    }
    else
    {
     if (get_skill(victim,gsn_shield_block) <= 1)
      	return FALSE;
     chance = get_skill(victim,gsn_shield_block) / 2;
     chance -= (victim->iclass == CLASS_WARRIOR) ? 0 : 10;
    }

	if ( number_percent( ) >= chance + victim->level - ch->level )
		return FALSE;

	act( " Sen $s sald�r�s�n� kalkan�nla kar��l�yorsun.",  ch, NULL, victim, TO_VICT    );
	act( "$N sald�r�n� kalkan�yla kar��l�yor.", ch,NULL,victim,TO_CHAR);
    check_shield_destroyed( ch, victim, FALSE);
    check_improve(victim,gsn_shield_block,TRUE,6);
    return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( MOUNTED(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
         chance  = UMIN( 30, victim->level );
    else {
          chance  = get_skill(victim,gsn_dodge) / 2;
    	  /* chance for high dex. */
          chance += 2 * (get_curr_stat(victim,STAT_DEX) - 20);
	 if ( victim->iclass == CLASS_WARRIOR || victim->iclass == CLASS_SAMURAI)
	    chance = (int)((float)chance * 1.2);
	if ( victim->iclass == CLASS_THIEF || victim->iclass ==CLASS_NINJA)
	    chance = (int)((float)chance * 1.1);
	 }

    if ( number_percent( ) >= chance + (victim->level - ch->level) / 2 )
        return FALSE;

				act( "Sen $s sald�r�s�ndan s�yr�l�yorsun.", ch, NULL, victim, TO_VICT    );
		    act( "$N sald�r�ndan s�yr�l�yor.", ch, NULL, victim, TO_CHAR    );
   if ( number_percent() < (get_skill(victim,gsn_dodge) / 20 )
	&& !(IS_AFFECTED(ch,AFF_FLYING) || ch->position < POS_FIGHTING))
   {
     /* size */
     if (victim->size < ch->size)
        chance += (victim->size - ch->size) * 10;  /* bigger = harder to trip */

     /* dex */
     chance += get_curr_stat(victim,STAT_DEX);
     chance -= get_curr_stat(ch,STAT_DEX) * 3 / 2;

     if (IS_AFFECTED(victim,AFF_FLYING) )
	chance -= 10;

     /* speed */
     if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance += 10;
     if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance -= 20;

     /* level */
     chance += (victim->level - ch->level) * 2;


     /* now the attack */
     if (number_percent() < (chance / 20) )
     {
			 act("$n pozisyonunu kaybederek d���yor!",ch,NULL,victim,TO_VICT);
		 	act("Sen $S hareketiyle pozisyonunu kaybediyor ve d���yorsun!",ch,NULL,victim,TO_CHAR);
		 	act("$n $S sald�r�s�ndan s�yr�l�nca $N yere d���yor.",ch,NULL,victim,TO_NOTVICT);

        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	ch->position = POS_RESTING;
     }
    }
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}



/*
 * Check for cross.
 */
bool check_cross( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( get_wield_char( victim, FALSE ) == NULL ||
	      get_wield_char( victim, TRUE ) == NULL )
    	return FALSE;

    if ( IS_NPC(victim) )
    {
	chance	= UMIN( 35, victim->level );
    }
    else
    {
	chance	= get_skill(victim,gsn_cross_block) / 3;
	if ( victim->iclass == CLASS_WARRIOR || victim->iclass == CLASS_SAMURAI)
	chance = (int)((float)chance * 1.2);
    }


	if ( number_percent( ) >= chance + victim->level - ch->level )
		return FALSE;

	act( "$s sald�r�s�n� �apraz blokla kar��l�yorsun.",  ch, NULL, victim, TO_VICT    );
	act("$N sald�r�n� �apraz blokla kar��l�yor.", ch, NULL, victim, TO_CHAR    );
    check_weapon_destroyed( ch, victim, FALSE);
    if ( number_percent() >  get_skill(victim,gsn_cross_block) )
    {
     /* size  and weight */
     chance += ch->carry_weight / 25;
     chance -= victim->carry_weight / 10;

     if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 25;
     else
	chance += (ch->size - victim->size) * 10;


     /* stats */
     chance += get_curr_stat(ch,STAT_STR);
     chance -= get_curr_stat(victim,STAT_DEX) * 5/3;

     if (IS_AFFECTED(ch,AFF_FLYING))
	chance -= 20;

     /* speed */
     if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
     if (IS_SET(victim->off_flags,OFF_FAST))
	chance -= 20;

     /* level */
     chance += (ch->level - victim->level) * 2;

     /* now the attack */
     if (number_percent() < ( chance / 20  ))
     {
			 act("Pozisyonunu korumaya devam edemiyorsun!",
		 		ch,NULL,victim,TO_VICT);
		 	act("You fall down!",ch,NULL,victim,TO_VICT);
		 	act("$N sald�r�na direnemeerek d���yor!",
		 		ch,NULL,victim,TO_CHAR);
		 	act("$n sersemletici g�c� $N d���r�yor.",
		 		ch,NULL,victim,TO_NOTVICT);

	WAIT_STATE(victim,skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;
     }
    }
    check_improve(victim,gsn_cross_block,TRUE,6);
    return TRUE;
}

/*
 * Check for hand.
 */
bool check_hand( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( get_wield_char( victim, FALSE ) != NULL )
	return FALSE;

    if ( IS_NPC(victim) )
    {
	chance	= UMIN( 35, victim->level );
    }
    else
    {
	chance	= get_skill(victim,gsn_hand_block) / 3;
	if ( victim->iclass == CLASS_NINJA )
		chance = (int)((float)chance * 1.5);
    }


	if ( number_percent( ) >= chance + victim->level - ch->level )
		return FALSE;

	act( "$s sald�r�s�n� ellerinle kar��l�yorsun.",  ch, NULL, victim, TO_VICT    );
	act( "$S elleri sald�r�n� durduruyor.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_hand_block,TRUE,6);
    return TRUE;
}


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting	= NULL;
	    fch->position	= IS_NPC(fch) ? ch->default_pos : POS_STANDING;
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    int i;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->silver > 0)
	  {
	    if (IS_SET(ch->form,FORM_INSTANT_DECAY))
	      obj_to_room( create_money( ch->silver), ch->in_room);
	    else
	      obj_to_obj( create_money( ch->silver ), corpse );
	    ch->silver = 0;
	}
	corpse->from = str_dup(ch->short_descr);
	corpse->cost = 0;
    }
    else
      {
	if (IS_GOOD(ch))
	  i = 0;
	if (IS_EVIL(ch))
	  i = 2;
	else
	  i = 1;

	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	corpse->owner = str_dup(ch->name);
	corpse->from = str_dup(ch->name);
	corpse->altar = hometown_table[ch->hometown].altar[i];
	corpse->pit = hometown_table[ch->hometown].pit[i];

	if ( ch->silver > 0)
	{
	    obj_to_obj( create_money( ch->silver ), corpse );
	    ch->silver = 0;
	}
	corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))  {
	    obj->timer = number_range(5,10);
	    if ( obj->item_type == ITEM_POTION )
	       obj->timer += obj->level * 20;
        }
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
	REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY )  ||
	    (obj->pIndexData->limit != -1 &&
		(obj->pIndexData->count > obj->pIndexData->limit) ) )
	  {
	    extract_obj( obj );
	    continue;
	  }
	else if (IS_SET(ch->form,FORM_INSTANT_DECAY))
	  obj_to_room(obj, ch->in_room);

	else
	  obj_to_obj( obj, corpse );
    }

    obj_to_room( corpse, ch->in_room );
    return;
}


void death_cry( CHAR_DATA *ch )
{
	death_cry_org( ch, -1 );
}

/*
* Improved Death_cry contributed by Diavolo.
*/
void death_cry_org( CHAR_DATA *ch, int part )
{
	ROOM_INDEX_DATA *was_in_room;
	const char *msg;
	int door;
	int vnum;

	vnum = 0;
	msg = "$s ac� ���l���n� duyuyorsun.";

	if ( part == -1 )
		part = number_range(0,15);

	switch ( part )
	{
		case  0:
			msg  = "$n yere d���yor... $n �LD�.";
			break;
		case  1:
			if (ch->material == 0)
			{
				msg  = "$s kan� z�rh�na s��r�yor.";
				break;
			}
		case  2:
			if (IS_SET(ch->parts,PART_GUTS))
			{
				msg = "$s ba��rsaklar� yere sa��l�yor.";
				vnum = OBJ_VNUM_GUTS;
			}
		break;
		case  3:
			if (IS_SET(ch->parts,PART_HEAD))
			{
				msg  = "$s kopan kafas� yerde yuvarlan�yor.";
				vnum = OBJ_VNUM_SEVERED_HEAD;
			}
		break;
		case  4:
			if (IS_SET(ch->parts,PART_HEART))
			{
				msg  = "$s kalbi g��s�nden d��ar� f�rl�yor.";
				vnum = OBJ_VNUM_TORN_HEART;
			}
		break;
		case  5:
			if (IS_SET(ch->parts,PART_ARMS))
			{
				msg  = "$s kolu �l� g�vdeden ayr�l�yor.";
				vnum = OBJ_VNUM_SLICED_ARM;
			}
			break;
		case  6:
			if (IS_SET(ch->parts,PART_LEGS))
			{
				msg  = "$s baca�� �l� g�vdeden ayr�l�yor.";
				vnum = OBJ_VNUM_SLICED_LEG;
			}
			break;
		case 7:
			if (IS_SET(ch->parts,PART_BRAINS))
			{
				msg = "$s kafas� k�r�l�yor ve beyin par�alar� �st�ne s��r�yor.";
				vnum = OBJ_VNUM_BRAINS;
			}
	}

	act( msg, ch, NULL, NULL, TO_ROOM );

	if ( vnum != 0 )
	{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	obj->from = str_dup(name);

	if (obj->item_type == ITEM_FOOD)
	{
	if (IS_SET(ch->form,FORM_POISON))
	obj->value[3] = 1;
	else if (!IS_SET(ch->form,FORM_EDIBLE))
	obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
	}

	if ( IS_NPC(ch) )
	{
	msg = "Bir�eyin ac� ���l���n� duyuyorsun.";
	}
	else
	{
	msg = "Birinin ac� ���l���n� duyuyorsun.";
	}

	was_in_room = ch->in_room;
	for ( door = 0; door <= 5; door++ )
	{
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	ch->in_room = pexit->u1.to_room;
	act( msg, ch, NULL, NULL, TO_ROOM );
	}
	}
	ch->in_room = was_in_room;

	return;
}


void raw_kill( CHAR_DATA *victim )
{
  raw_kill_org( victim, -1 );
  return;
}

void raw_kill_org( CHAR_DATA *victim, int part )
{

  CHAR_DATA *tmp_ch;
  OBJ_DATA *obj,*obj_next;
  int i;
  OBJ_DATA *tattoo;
	AFFECT_DATA af;

  stop_fighting( victim, TRUE );

  for (obj = victim->carrying;obj != NULL;obj = obj_next)
    {
      obj_next = obj->next_content;
      if (IS_SET(obj->progtypes,OPROG_DEATH) && (obj->wear_loc != WEAR_NONE))
	if ((obj->pIndexData->oprogs->death_prog) (obj,victim))
	  {
	    victim->position = POS_STANDING;
	    return;
	  }
    }
  victim->last_fight_time = -1;
  if ( IS_SET(victim->progtypes,MPROG_DEATH))
    if ((victim->pIndexData->mprogs->death_prog) (victim))
      {
	victim->position = POS_STANDING;
	return;
      }

  victim->last_death_time = current_time;

  tattoo = get_eq_char(victim, WEAR_TATTOO);
  if (tattoo != NULL)
    obj_from_char(tattoo);

  death_cry_org( victim, part );
  make_corpse( victim );


  if ( IS_NPC(victim) )
    {
      victim->pIndexData->killed++;
      kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
      extract_char( victim, TRUE );
      return;
    }

		send_to_char("20 dakikal���na yenilmez bir hayalete d�n��t�n.\n\r",victim);
	    send_to_char("Bu s�rada y�r�mek, i�mek, yemek gibi temel �eyleri yapabileceksin.\n\r", victim);

    extract_char( victim, FALSE );

  while ( victim->affected )
    affect_remove( victim, victim->affected );
  victim->affected_by	= 0;
  victim->detection	= 0;
  for (i = 0; i < 4; i++)
    victim->armor[i]= 100;
  victim->position	= POS_RESTING;
  victim->hit		= victim->max_hit / 10;
  victim->mana	= victim->max_mana / 10;
  victim->move	= victim->max_move;

  /* RT added to prevent infinite deaths */
  REMOVE_BIT(victim->act, PLR_WANTED);
  REMOVE_BIT(victim->act, PLR_BOUGHT_PET);
	SET_BIT(victim->act, PLR_GHOST);
	victim->pcdata->ghost_mode_counter = 20;

  victim->pcdata->condition[COND_THIRST] = 40;
  victim->pcdata->condition[COND_HUNGER] = 40;
  victim->pcdata->condition[COND_FULL] = 40;
  victim->pcdata->condition[COND_BLOODLUST] = 40;
  victim->pcdata->condition[COND_DESIRE] = 40;

	af.where     = TO_AFFECTS;
	af.type      = gsn_fly;
	af.level	 = 91;
	af.duration  = 25;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_FLYING;
	affect_to_char( victim, &af );

	af.where     = TO_AFFECTS;
	af.type      = gsn_pass_door;
	af.level     = 91;
	af.duration  = 25;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_PASS_DOOR;
	affect_to_char( victim, &af );

	af.where     = TO_AFFECTS;
	af.type      = gsn_imp_invis;
	af.level     = 91;
	af.duration  = 25 ;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_IMP_INVIS;
	affect_to_char( victim, &af );

  if (tattoo != NULL)
    {
      obj_to_char(tattoo, victim);
      equip_char(victim, tattoo, WEAR_TATTOO);
    }
 save_char_obj( victim );

  /*
   * Calm down the tracking mobiles
   */
  for (tmp_ch = char_list; tmp_ch != NULL; tmp_ch = tmp_ch->next)
    if (tmp_ch->last_fought == victim)
      tmp_ch->last_fought = NULL;

  return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *gch;
	CHAR_DATA *lch;
	int xp;
	int members;
	int group_levels;

	if ( victim == ch || (IS_NPC(victim) && victim->pIndexData->vnum < 100 ) )
		return;

	/* quest */
	if (IS_GOLEM(ch) && ch->master != NULL && ch->master->iclass == CLASS_NECROMANCER)
		gch = ch->master;
	else
		gch = ch;

	if (!IS_NPC(gch) && IS_QUESTOR(gch) && IS_NPC(victim))
	{
		if (gch->pcdata->questmob == victim->pIndexData->vnum)
		{
			printf_to_char(gch,"{cG�revin neredeyse tamamland�!\n\rZaman�n bitmeden �nce g�revciye git!{x\n\r");
			gch->pcdata->questmob = -1;
		}
	}
	/* end quest */

	if (!IS_NPC(victim))
		return;

	if (IS_NPC(victim) && (victim->master != NULL || victim->leader != NULL))
		return;

	members = 1;
	group_levels = 0;
	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
		if ( is_same_group( gch, ch ) )
		{
			if (!IS_NPC(gch) && gch != ch)
				members++;
			group_levels += gch->level;
		}
	}

	lch = (ch->leader != NULL) ? ch->leader : ch;

	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		if ( !is_same_group( gch, ch ) || IS_NPC(gch))
			continue;


		if ( gch->level - lch->level > 8 )
		{
			send_to_char("Seviyen bu gruba girebilmek i�in fazla.\n\r", gch );
			continue;
		}

		if ( gch->level - lch->level < -8 )
		{
			send_to_char( "Seviyen bu gruba girebilmek i�in az.\n\r", gch );
			continue;
		}

		if ( IS_SET(ch->act,PLR_NO_DESCRIPTION) )
		{
			printf_to_char( ch,"{rEn az 350 karakterlik tan�m�n olmadan TP kazanamazs�n!{x\n\r" );
		}
		else
		{
			xp = xp_compute( gch, victim, group_levels,members );

			if( ikikat_tp > 0 )
			{
					printf_to_char( ch , "{C�ki kat TP kazanma etkinli�i nedeniyle kazand���n TP art�yor.{x\n\r" );
					xp *= 2;
			}

			sprintf( buf, "{g%d tecr�be puan� kazand�n.{x\n\r", xp );
			send_to_char( buf, gch );
			gain_exp( gch, xp );
		}


		for ( obj = ch->carrying; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;
			if ( obj->wear_loc == WEAR_NONE )
				continue;

			if ((( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
			||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
			||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
			&& !IS_IMMORTAL(ch))
			{
				act( "Sen $p taraf�ndan �arp�ld�n.", ch, obj, NULL, TO_CHAR );
				act( "$n $p taraf�ndan �arp�ld�.",   ch, obj, NULL, TO_ROOM );
				obj_from_char( obj );
				obj_to_room( obj, ch->in_room );
			}
		}
	}
	return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute(CHAR_DATA *gch, CHAR_DATA *victim, int total_levels,int members)
{
	int xp;
	int base_exp;
	int level_range;
	int neg_cha=0, pos_cha=0;

	level_range = victim->level - gch->level;

	switch (level_range)
	{
		default : 	base_exp =   0;		break;
		case -9 :	base_exp =   1;		break;
		case -8 :	base_exp =   2;		break;
		case -7 :	base_exp =   5;		break;
		case -6 : 	base_exp =   9;		break;
		case -5 :	base_exp =  11;		break;
		case -4 :	base_exp =  22;		break;
		case -3 :	base_exp =  33;		break;
		case -2 :	base_exp =  43;		break;
		case -1 :	base_exp =  60;		break;
		case  0 :	base_exp =  74;		break;
		case  1 :	base_exp =  84;		break;
		case  2 :	base_exp =  99;		break;
		case  3 :	base_exp = 121;		break;
		case  4 :	base_exp = 143;		break;
	}

	if (level_range > 4)
	{
		base_exp = 140 + 20 * (level_range - 4);
	}


	/* calculate exp multiplier */
	if (IS_SET(victim->act,ACT_NOALIGN))
	{
		xp = base_exp;
	}

	/* alignment */
	else if ((IS_EVIL(gch) && IS_GOOD(victim)) || (IS_EVIL(victim) && IS_GOOD(gch)))
	{
		xp = (int)((float)base_exp * 1.6);
	}

	else if ( IS_GOOD(gch) && IS_GOOD(victim) )
	{
		xp = 0;
	}

	else if ( !IS_NEUTRAL(gch) && IS_NEUTRAL(victim) )
	{
		xp = (int)((float)base_exp * 1.1);
	}

	else if ( IS_NEUTRAL(gch) && !IS_NEUTRAL(victim) )
	{
		xp = (int)((float)base_exp * 1.3);
	}

	else
	{
		xp = base_exp;
	}

    /* more exp at the low levels */
    if (gch->level < 6)
	{
		xp = 15 * xp / (gch->level + 4);
	}

    /* randomize the rewards */
    xp = number_range ((int)((float)xp * 0.8), (int)((float)xp * 1.2));

    /* adjust for grouping */
    xp = (int)((float)xp * (float)gch->level/(float)total_levels);

    if (members == 2)
	{
		xp *= 2;
	}
    if (members == 3)
	{
		xp *= 3;
	}

    if (gch->level < KIDEMLI_OYUNCU_SEVIYESI)
	{
		xp = UMIN((250 + dice(1,40)),xp);
	}
    else if (gch->level < 40)
	{
		xp = UMIN((225 + dice(1,40)),xp);
	}
    else if (gch->level < 60)
	{
		xp = UMIN((200 + dice(1,40)),xp);
	}
    else
	{
		xp = UMIN((180 + dice(1,20)),xp);
	}

    xp += (int)((float)(xp * ( gch->max_hit - gch->hit )) / (float)(gch->max_hit * 5 ));

	if (IS_GOOD(gch))
	{
		if (IS_GOOD(victim)) { gch->pcdata->anti_killed++; neg_cha = 1; }
		else if (IS_NEUTRAL(victim)) {gch->pcdata->has_killed++; pos_cha =1;}
		else if (IS_EVIL(victim)) {gch->pcdata->has_killed++; pos_cha = 1;}
	}

	if (IS_NEUTRAL(gch))
	{
		if (IS_GOOD(victim)) {gch->pcdata->has_killed++; pos_cha = 1;}
		else if (IS_NEUTRAL(victim)) {gch->pcdata->anti_killed++; neg_cha = 1;}
		else if (IS_EVIL(victim)) {gch->pcdata->has_killed++; pos_cha =1;}
	}

	if (IS_EVIL(gch))
	{
		if (IS_GOOD(victim)) {gch->pcdata->has_killed++; pos_cha = 1;}
		else if (IS_NEUTRAL(victim)) {gch->pcdata->has_killed++; pos_cha = 1;}
		else if (IS_EVIL(victim)) {gch->pcdata->anti_killed++; neg_cha = 1;}
	}

	if ( neg_cha )
	{
		if ( (gch->pcdata->anti_killed % 100) == 99 )
		{
			printf_to_char(gch,"�imdiye kadar %d adet seninle ayn� y�nelime sahip canl� �ld�rd�n.",gch->pcdata->anti_killed);
			if (gch->perm_stat[STAT_CHA] > 3)
			{
				printf_to_char(gch,"Bu y�zden karizman bir azald�.\n\r");
				gch->perm_stat[STAT_CHA] -= 1;
			}
		}
	}
	else if ( pos_cha )
	{
		if ( (gch->pcdata->has_killed % 200) == 199 )
		{
			printf_to_char(gch,"�imdiye kadar %d adet seninle ayn� y�nelime sahip olmayan canl� �ld�rd�n.",gch->pcdata->has_killed);
			if ( gch->perm_stat[STAT_CHA] < get_max_train( gch, STAT_CHA ) )
			{
				printf_to_char(gch,"Bu y�zden karizman bir artt�.\n\r");
				gch->perm_stat[STAT_CHA] += 1;
			}
		}
	}
	
	// Oyuncu katline dahil olan oyuncular %20 fazla TP kazansin.
	if (gch->pcdata->oyuncu_katli == 1)
	{
		xp += (int)((float)xp / (float)5);
	}

	if(IS_SET(gch->pcdata->dilek,DILEK_FLAG_TECRUBE))
	{
		printf_to_char( gch , "{CTecr�be dile�in sayesinde kazand���n TP art�yor.{x\n\r" );
		xp *= 2;
	}
	
	if(gch->pcdata->discord_id[0] == '\0')
	{
		printf_to_char( gch , "{CDiscord ID girmedi�in i�in kazand���n TP azal�yor.{x\n\r" );
		xp = int((float)xp / (float)3);
	}

	return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune ,int dam_type)
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

		if ( dam ==   0 ) { vs = "�skalad�n";	vp = "�skalad�";		}
     else if ( dam <=   4 ) { vs = "�izdin";	vp = "�izdi";	}//�rseledin,ufalad�n
     else if ( dam <=   8 ) { vs = "s�y�rd�n";	vp = "s�y�rd�";		}//katlettin
     else if ( dam <=  12 ) { vs = "t�rmalad�n";	vp = "t�rmalad�";		}
     else if ( dam <=  16 ) { vs = "�rseledin";	vp = "�rseledi";		}
     else if ( dam <=  20 ) { vs = "h�rpalad�n";	vp = "h�rpalad�";		}//sildin,
     else if ( dam <=  24 ) { vs = "y�pratt�n";       vp = "y�pratt�";		}
     else if ( dam <=  28 ) { vs = "yaralad�n";	vp = "yaralad�";	}
     else if ( dam <=  32 ) { vs = "kestin";	vp = "kesti";	}//zedelemek
     else if ( dam <=  36 ) { vs = "bi�tin";	vp = "bi�ti";		}//un ufak etmek
     else if ( dam <=  42 ) { vs = "PAR�ALADIN";	vp = "PAR�ALADI";	}//becermek
     else if ( dam <=  52 ) { vs = "DO�RADIN";	vp = "DO�RADI";	}//tepelemek
     else if ( dam <=  65 ) { vs = "-YIKIMA U�RATTIN-";	vp = "-YIKIMA U�RATTI-";	}//�pt�n
     else if ( dam <=  80 ) { vs = "*KAZIDIN*";	vp = "*KAZIDI*";	}
     else if ( dam <=  100 ) { vs = "**K�T�R�M ETT�N**";	vp = "**K�T�R�M ETT�**";		}
     else if ( dam <=  130 ) { vs = "*** HARAP ETT�N ***";//eline vermek
 			     vp = "*** HARAP ETT� ***";			}
     else if ( dam <= 175 ) { vs = "*** MAHVETT�N ***";
 			     vp = "*** MAHVETT� ***";			}
     else if ( dam <= 250)  { vs = "=== S�LD�N ===";
 			     vp = "=== S�LD� ===";		}
     else if ( dam <= 325)  { vs = "==== BOZGUNA U�RATTIN ====";
 			     vp = "==== BOZGUNA U�RATTI ====";	}
     else if ( dam <= 400)  { vs = "<*> <*> KANA BO�DUN <*> <*>";
 			     vp = "<*> <*> KANA BO�DU <*> <*>";	}
     else if ( dam <= 500)  { vs = "<*>!<*> TELEF ETT�N <*>!<*>";
 			     vp = "<*>!<*> TELEF ETT� <*>!<*>";	}
     else if ( dam <= 650)  { vs = "<*><*><*> REZ�L ETT�N <*><*><*>";
 			     vp = "<*><*><*> REZ�L ETT� <*><*><*>";	}
     else if ( dam <= 800)  { vs = "(<*>)!(<*>) KATLETT�N (<*>)!(<*>)";
 			     vp = "(<*>)!(<*>) KATLETT� (<*>)!(<*>)";}
     else if ( dam <= 1000)  { vs = "(*)!(*)!(*) UN UFAK ETT�N (*)!(*)!(*)";
 			     vp = "(*)!(*)!(*) UN UFAK ETT� (*)!(*)!(*)";	}
     else if ( dam <= 1250)  { vs = "(*)!<*>!(*) ATOMLARINA AYIRDIN (*)!<*>!(*)";
 			     vp = "(*)!<*>!(*) ATOMLARINA AYIRDI (*)!<*>!(*)";	}
     else if ( dam <= 1500)  { vs = "<*>!(*)!<*>> �pT�N <<*)!(*)!<*>";
 			     vp = "<*>!(*)!<*>> �pT� <<*)!(*)!<*>";	}
     else                   { vs = "\007=<*) (*>= ! �pT�N ! =<*) (*>=\007";
 			     vp = "\007=<*) (*>= ! �pT� ! =<*) (*>=\007";}

    if (victim->level < 20 )	punct   = (dam <= 24) ? '.' : '!';
    else if (victim->level < 50)  punct = (dam <= 50 ) ? '.' : '!';
    else punct = (dam <= 75) ? '.' : '!';

    if ( (dt == TYPE_HIT)  || (dt == TYPE_HUNGER) )
    {
	if (ch  == victim)
	{
	  if ( dam_type == DAM_HUNGER )
	  {
			sprintf( buf1, "A�l��� $m $C%s$c%c [%d]",vp,punct,dam);
		    sprintf( buf2, "A�l���n seni $C%s$c%c [%d]",vs,punct,dam);
	  }

	  else if ( dam_type == DAM_THIRST )
	  {
			sprintf( buf1, "Susuzlu�u $m $C%s$c%c [%d]",vp,punct,dam);
		    sprintf( buf2, "Susuzlu�un seni $C%s$c%c [%d]",vs,punct,dam);
	  }
	  else if ( dam_type == DAM_LIGHT_V )
	  {
			sprintf( buf1, "Odadaki ���k $m $C%s$c%c [%d]",vp,punct,dam);
		    sprintf( buf2, "Odadaki ���k seni $C%s$c%c [%d]",vs,punct,dam);
	  }
	  else if ( dam_type == DAM_TRAP_ROOM )
	  {
			sprintf( buf1, "Odadaki tuzak $m $C%s$c%c [%d]",vp,punct,dam);
		    sprintf( buf2, "Odadaki tuzak seni $C%s$c%c [%d]",vs,punct,dam);
	  }
	  else {
			sprintf( buf1, "$n kendini $C%s$c%c [%d]",vp,punct,dam);
		    sprintf( buf2, "Sen kendini $C%s$c%c [%d]",vs,punct,dam);
		}
	}
	else
	{
		sprintf( buf1, "$n $M $C%s$c%c [%d]",  vp, punct,dam );
	    sprintf( buf2, "Sen $M $C%s$c%c [%d]", vs, punct,dam );
	    sprintf( buf3, "$n seni $C%s$c%c [%d]", vp, punct,dam );
	}
    }

    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
	    attack	= attack_table[dt - TYPE_HIT].noun;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

	if (immune)
	{
	    if (ch == victim)
	    {
				sprintf(buf1,"$n kendi %s sald�r�s�ndan etkilenmedi.",attack);
				sprintf(buf2,"�ans eseri bu tip bir sald�r� seni etkilemiyor.");
	    }
	    else
	    {
				sprintf(buf1,"$N $s %s sald�r�s�ndan etkilenmedi!",attack);
		    	sprintf(buf2,"$N senin %s sald�r�ndan etkilenmedi!",attack);
		    	sprintf(buf3,"$s %s sald�r�s� sana kar�� g��s�z kald�.",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
				sprintf( buf1, "$s %s sald�r�s�yla kendini $C%s$c%c [%d]",attack,vp,punct,dam);
				sprintf( buf2, "%s sald�r�nla kendini $C%s$c%c [%d]",attack,vp,punct,dam);
	    }
	    else
	    {
				sprintf( buf1, "$s %s sald�r�s� $M $C%s$c%c [%d]",  attack, vp, punct,dam );
		    	sprintf( buf2, "%s sald�r�n $M $C%s$c%c [%d]",  attack, vp, punct,dam );
		    	sprintf( buf3, "$s %s sald�r�s� seni $C%s$c%c [%d]", attack, vp, punct,dam );
	    }
	}
    }

    if (ch == victim)
    {
	act_color(buf1,ch,NULL,NULL,TO_ROOM,POS_RESTING,CLR_BROWN);
	act_color(buf2,ch,NULL,NULL,TO_CHAR,POS_RESTING,CLR_RED);
    }
    else
    {
    	act_color( buf1, ch, NULL, victim, TO_NOTVICT,POS_RESTING,CLR_BROWN );
    	act_color( buf2, ch, NULL, victim, TO_CHAR,POS_RESTING,CLR_GREEN );
    	act_color( buf3, ch, NULL, victim, TO_VICT,POS_RESTING,CLR_RED );
    }

    return;
}


void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wield;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
			send_to_char( "Kimi �ld�receksin?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
			send_to_char("Burada de�il.\n\r", ch);
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( ch->position == POS_FIGHTING)
    {
      if (victim == ch->fighting)
			send_to_char( "Yapabilece�inin en iyisi bu!\n\r", ch );
		      else if (victim->fighting != ch)
			send_to_char("Ayn� anda tek d�v��, l�tfen.\n\r",ch);
      else
	{
		act( "$E y�neldin.",ch,NULL,victim,TO_CHAR);
	  ch->fighting = victim;
	}
	return;
    }

    if ( !IS_NPC(victim) )
    {
			send_to_char("Oyuncular� KATLETmelisin.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
			send_to_char( "Kendine vuruyorsun.  Aaah!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }


    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
			act("$N senin pek sevgili efendin.", ch, NULL, victim, TO_CHAR );
	return;
    }


    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

    if (!ch_skill_nok_nomessage(ch,gsn_mortal_strike)
	&& (wield = get_wield_char(ch,FALSE)) != NULL
	&& wield->level > (victim->level - 5)
	&& cabal_ok(ch,gsn_mortal_strike) )
     {
      int chance = 1 + get_skill(ch,gsn_mortal_strike) / 30;
      chance += (ch->level - victim->level) / 2;
      if (number_percent() < chance)
	{
		act_color("$CAni sald�r�n $M �ld�rd�!$c",
			ch,NULL,victim,TO_CHAR,POS_RESTING,CLR_RED);
		act_color("$C$s ani sald�r�s� $M �ld�rd�!$c",
			ch,NULL,victim,TO_NOTVICT,POS_RESTING,CLR_RED);
		act_color("$C$s ani sald�r�s� seni �ld�rd�!$c",
		ch,NULL,victim,TO_VICT,POS_DEAD,CLR_RED);
	 damage(ch,victim,(victim->hit + 1),gsn_mortal_strike,DAM_NONE, TRUE);
	 check_improve( ch,gsn_mortal_strike, TRUE, 1);
	 return;
	}
      else check_improve( ch,gsn_mortal_strike, FALSE, 3);
     }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
	send_to_char( "KATLETmek istiyorsan tam olarak yaz.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wield;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
			send_to_char( "Kimi katledeceksin?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
	return;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
			send_to_char("Burada de�il.\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
			send_to_char( "�ntihar g�naht�r.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
			act( "$N senin pek sevgili efendin.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
			send_to_char( "Yapabilece�inin en iyisi bu!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if (!can_see(victim, ch))
      do_yell(victim, (char*)"�mdat! Biri bana sald�r�yor!");
    else
      {
	if (IS_NPC(ch))
	sprintf(buf, "�mdat! %s bana sald�r�yor!",ch->short_descr);
	else
	  sprintf( buf, "�mdat!  %s bana sald�r�yor!",
		  (is_affected(ch,gsn_doppelganger) && !IS_IMMORTAL(victim)) ?
		  ch->doppel->name : ch->name );
	do_yell( victim, buf );
      }

    if (!ch_skill_nok_nomessage(ch,gsn_mortal_strike)
	&& (wield = get_wield_char(ch,FALSE)) != NULL
	&& wield->level > (victim->level - 5)
	&& cabal_ok(ch,gsn_mortal_strike) )
     {
      int chance = 1 + get_skill(ch,gsn_mortal_strike) / 30;
      chance += (ch->level - victim->level) / 2;
      if (number_percent() < chance)
	{
		act_color("$CAni sald�r�n $M �ld�rd�!$c",
			ch,NULL,victim,TO_CHAR,POS_RESTING,CLR_RED);
		act_color("$C$s ani sald�r�s� $M �ld�rd�!$c",
			ch,NULL,victim,TO_NOTVICT,POS_RESTING,CLR_RED);
		act_color("$C$s ani sald�r�s� seni �ld�rd�!$c",
		ch,NULL,victim,TO_VICT,POS_DEAD,CLR_RED);
	 damage(ch,victim,(victim->hit + 1),gsn_mortal_strike,DAM_NONE, TRUE);
	 check_improve( ch,gsn_mortal_strike, TRUE, 1);
	 return;
	}
      else check_improve( ch,gsn_mortal_strike, FALSE, 3);
     }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}


void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if (RIDDEN(ch))
    {
			send_to_char( "Binicine sormal�s�n!\n\r", ch );
	return;
    }

    if (MOUNTED(ch))
	do_dismount(ch,(char*)"");

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
						send_to_char("Kimseyle d�v��m�yorsun.\n\r", ch );
	return;
    }

    if ((ch->iclass == 9) && (ch->level >=10) )
	{
		send_to_char( "Onurun ka�mana izin vermiyor, kendini dishonor etmeyi dene.\n\r", ch );
	 return;
	}

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	|| ( IS_SET(pexit->exit_info, EX_CLOSED)
      && (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS))
      &&   !IS_TRUSTED(ch,ANGEL) )
	|| ( IS_SET(pexit->exit_info , EX_NOFLEE) )
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n ka�t�!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
		send_to_char( "D�v��ten ka�t�n!  10 TP kaybettin.\n\r", ch );
	    if ((ch->iclass == 9) && (ch->level >=10) )
			gain_exp( ch,( -1 * ch->level ) );
	    else  gain_exp( ch, -10 );
	}
	else
	  ch->last_fought = NULL;

	stop_fighting( ch, TRUE );
	return;
    }

		send_to_char("PAN�K! Ka�amad�n!\n\r", ch );
    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
	send_to_char("SLAY etmek istiyorsan komutu tam yazmal�s�n.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
	char eventbuf[MAX_STRING_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
			send_to_char( "Kimi?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
			send_to_char("Burada de�il.\n\r", ch);
	return;
    }

    if ( ch == victim )
    {
			send_to_char("�ntihar g�naht�r.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch)
	 && !(IS_NPC(ch) && ch->cabal != CABAL_NONE && !IS_IMMORTAL(victim)) )
    {
			send_to_char("Ba�aramad�n.\n\r",ch );
	return;
    }

		act("$M yokettin!",  ch, NULL, victim, TO_CHAR    );
    act("$n seni yoketti!", ch, NULL, victim, TO_VICT    );
    act("$n $M yoketti!",  ch, NULL, victim, TO_NOTVICT );

	/* event */
	if (!IS_NPC(victim))
	{
	sprintf(eventbuf,"%s, tanr�lar taraf�ndan �l�mle cezaland�r�ld�!",victim->name);
	write_event_log(eventbuf);
	}

    raw_kill( victim );
    return;
}

/*
 * Check for obj dodge.
 */
bool check_obj_dodge( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int bonus )
{
    int chance;

    if ( !IS_AWAKE(victim) || MOUNTED(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
         chance  = UMIN( 30, victim->level );
    else {
          chance  = get_skill(victim,gsn_dodge) / 2;
    	  /* chance for high dex. */
          chance += 2 * (get_curr_stat(victim,STAT_DEX) - 20);
	 if ( victim->iclass == CLASS_WARRIOR || victim->iclass == CLASS_SAMURAI)
	    chance = (int)((float)chance*1.2);
	if ( victim->iclass == CLASS_THIEF || victim->iclass ==CLASS_NINJA)
	    chance = (int)((float)chance*1.1);
	 }

    chance -= (bonus - 90);
    chance /= 2;
    if ( number_percent( ) >= chance &&
	 (IS_NPC(victim) || victim->cabal != CABAL_BATTLE ))
        return FALSE;

    if (!IS_NPC(victim) && victim->cabal == CABAL_BATTLE
	  && IS_SET(victim->act,PLR_CANINDUCT))
    {
			act( "Sana at�lan $p objesini yakal�yorsun.",ch,obj,victim,TO_VICT);
      act("$N kendisine at�lan $p objesini yakal�yor.",ch,obj,victim,TO_CHAR);
      act("$n kendisine at�lan $p objesini yakal�yor.",victim,obj,ch,TO_NOTVICT);
     obj_to_char(obj,victim);
     return TRUE;
    }

		act("Sana at�lan $p objesinden s�yr�l�yorsun.",ch,obj,victim,TO_VICT);
    act("$N kendisine at�lan $p objesinden s�yr�l�yor.",ch,obj,victim,TO_CHAR);
    act("$n kendisine at�lan $p objesinden s�yr�l�yor.",victim,obj,ch,TO_NOTVICT);
    obj_to_room(obj,victim->in_room);
    check_improve(victim,gsn_dodge,TRUE,6);

    return TRUE;
}



void do_dishonor( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];
    int attempt,level = 0;

    if (RIDDEN(ch))
    {
			send_to_char( "Binicine sormal�s�n!\n\r", ch );
	return;
    }

    if ((ch->iclass != 9) || (ch->level <10) )
	{
		send_to_char("Soytar�l�k yapma...\n\r", ch );
	 return;
	}

    if ( ch->fighting == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
						send_to_char("Kimseyle d�v��m�yorsun.\n\r", ch );
	return;
    }

    for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	  if ( is_same_group( gch, ch->fighting )
		|| gch->fighting == ch )
		level += gch->level;
	}

    if ( (ch->fighting->level - ch->level) < 5
         &&  ch->level > (level / 3))
	{
		send_to_char("Bunu yapmaya de�mez.\n\r", ch );
	 return;
	}

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	|| ( IS_SET(pexit->exit_info, EX_CLOSED)
      && (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS))
      &&   !IS_TRUSTED(ch,ANGEL) )
	|| ( IS_SET(pexit->exit_info , EX_NOFLEE) )
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n ka�arak rezil oldu!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
		send_to_char( "D�v��ten ka�arak kendini rezil ettin.\n\r",ch);
	  sprintf(buf,"%d TP kaybettin.\n\r",ch->level);
	  send_to_char( buf, ch );
	  gain_exp( ch, -(ch->level) );
	}
	else
	  ch->last_fought = NULL;

	stop_fighting( ch, TRUE );
    	if (MOUNTED(ch))
		do_dismount(ch,(char*)"");

	return;
    }

		send_to_char( "PAN�K! Ka�amad�n!\n\r", ch );
    return;
}



bool mob_cast_cleric( CHAR_DATA *ch, CHAR_DATA *victim )
{
    const char *spell;
    int sn;

    for ( ;; )
    {
        int min_level;

        switch ( number_range(0,15) )
        {
        case  0: min_level =  0; spell = "blindness";      break;
        case  1: min_level =  3; spell = "cause serious";  break;
        case  2: min_level =  7; spell = "earthquake";     break;
        case  3: min_level =  9; spell = "cause critical"; break;
        case  4: min_level = 10; spell = "dispel evil";    break;
        case  5: min_level = 12; spell = "curse";          break;
        case  6: min_level = 14; spell = "cause critical"; break;
        case  7: min_level = 18; spell = "flamestrike";    break;
        case  8:
        case  9:
        case 10: min_level = 20; spell = "harm";           break;
        case 11: min_level = 25; spell = "plague";         break;
	case 12:
        case 13: min_level = 45; spell = "severity force"; break;
        default: min_level = 26; spell = "dispel magic";   break;
        }

        if ( ch->level >= min_level )
            break;
    }


    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return FALSE;
    say_spell(ch,sn);
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return TRUE;
}

bool mob_cast_mage( CHAR_DATA *ch, CHAR_DATA *victim )
{
    const char *spell;
    int sn;

    for ( ;; )
    {
        int min_level;

        switch ( number_range(0,15) )
        {
        case  0: min_level =  0; spell = "blindness";      break;
        case  1: min_level =  3; spell = "chill touch";    break;
        case  2: min_level =  7; spell = "weaken";         break;
        case  3: min_level =  9; spell = "teleport";       break;
        case  4: min_level = 14; spell = "colour spray";   break;
        case  5: min_level = 19; spell = "caustic font";   break;
        case  6: min_level = 25; spell = "energy drain";   break;
        case  7:
        case  8:
        case  9: min_level = 35; spell = "caustic font";       break;
        case 10: min_level = 40; spell = "plague";         break;
	case 11:
	case 12:
        case 13: min_level = 40; spell = "acid arrow";     break;
        default: min_level = 55; spell = "acid blast";     break;
        }

        if ( ch->level >= min_level )
            break;
    }


    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return FALSE;

    say_spell(ch,sn);
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return TRUE;
}
