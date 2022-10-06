/***************************************************************************
 *                                                                         *
 * Uzak Diyarlar açýk kaynak Türkçe Mud projesidir.                        *
 * Oyun geliþtirmesi Jai ve Maru tarafýndan yönetilmektedir.               *
 * Unutulmamasý gerekenler: Nir, Kame, Randalin, Nyah, Sint                *
 *                                                                         *
 * Github  : https://github.com/yelbuke/UzakDiyarlar                       *
 * Web     : http://www.uzakdiyarlar.net                                   *
 * Discord : https://discord.gg/kXyZzv                                     *
 *                                                                         *
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
#include <math.h>
#include "merc.h"
#include "interp.h"
#include "tables.h"
#include "db.h"

int obj_random_paf_modifier(int location, int level)
{
	int random_number;
	
	if (location == APPLY_STR || location == APPLY_DEX || location == APPLY_INT || 
		location == APPLY_WIS || location == APPLY_CON || location == APPLY_CHA )
	{
		random_number = number_range(1,100);
		
		if(random_number<=92)
			return 1;
		else if(random_number<=98)
			return number_range(1,2);
		else
			return number_range(1,3);
	}
	if (location == APPLY_MANA || location == APPLY_HIT || location == APPLY_MOVE)
	{
		random_number = number_range(1,100);
		
		if(random_number<=30)
			return number_range(10,UMAX(11,int(level)));
		else if(random_number<=65)
			return number_range(10,UMAX(11,int(level*2)));
		else if(random_number<=75)
			return number_range(10,UMAX(11,int(level*3)));
		else if(random_number<=83)
			return number_range(10,UMAX(11,int(level*4)));
		else if(random_number<=90)
			return number_range(10,UMAX(11,int(level*5)));
		else
			return number_range(10,UMAX(11,int(level*6)));
	}
	if (location == APPLY_HITROLL || location == APPLY_DAMROLL)
	{
		random_number = number_range(1,100);
		
		if(random_number<=30)
			return number_range(1,UMAX(2,int(level/11)));
		else if(random_number<=45)
			return number_range(1,UMAX(2,int(level/9)));
		else if(random_number<=60)
			return number_range(1,UMAX(2,int(level/7)));
		else if(random_number<=75)
			return number_range(1,UMAX(2,int(level/5)));
		else if(random_number<=90)
			return number_range(1,UMAX(2,int(level/3)));
		else
			return number_range(1,UMAX(2,int(level/2)));
	}
	return 1;
}

int obj_random_paf_find_available_location(OBJ_DATA *obj)
{
	AFFECT_DATA *paf;
	int random_number = number_range(1,500);
	int random_location = 0;
	
	if(random_number==1)
	{
		switch(number_range(1,6))
		{
			case 1:		random_location = APPLY_STR; break;
			case 2:		random_location = APPLY_DEX; break;
			case 3:		random_location = APPLY_INT; break;
			case 4:		random_location = APPLY_WIS; break;
			case 5:		random_location = APPLY_CON; break;
			case 6:		random_location = APPLY_CHA; break;
			default:	random_location = APPLY_STR; break;
		}
	}
	else if(random_number<200)
	{
		switch(number_range(7,9))
		{
			case 7:		random_location = APPLY_MANA; break;
			case 8:		random_location = APPLY_HIT; break;
			case 9:		random_location = APPLY_MOVE; break;
			default:	random_location = APPLY_HIT; break;
		}
	}
	else
	{
		switch(number_range(10,11))
		{
			case 10:	random_location = APPLY_HITROLL; break;
			case 11:	random_location = APPLY_DAMROLL; break;
			default:	random_location = APPLY_HITROLL; break;
		}
	}
	
	
	for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
		if(paf->location == random_location)
			return 0;
	}
	
	return random_location;
}

int obj_random_paf_find_available_resistance(OBJ_DATA *obj)
{
	AFFECT_DATA *paf;
	int random_number = number_range(1,23); //str,int,wis,dex,con,cha
	int random_res = 0;
	
	switch(random_number)
	{
		case 1:		random_res = RES_SUMMON; break;
		case 2:		random_res = RES_CHARM; break;
		case 3:		random_res = RES_MAGIC; break;
		case 4:		random_res = RES_WEAPON; break;
		case 5:		random_res = RES_BASH; break;
		case 6:		random_res = RES_PIERCE; break;
		case 7:		random_res = RES_SLASH; break;
		case 8:		random_res = RES_FIRE; break;
		case 9:		random_res = RES_COLD; break;
		case 10:	random_res = RES_LIGHTNING; break;
		case 11:	random_res = RES_ACID; break;
		case 12:	random_res = RES_POISON; break;
		case 13:	random_res = RES_NEGATIVE; break;
		case 14:	random_res = RES_HOLY; break;
		case 15:	random_res = RES_ENERGY; break;
		case 16:	random_res = RES_MENTAL; break;
		case 17:	random_res = RES_DISEASE; break;
		case 18:	random_res = RES_DROWNING; break;
		case 19:	random_res = RES_LIGHT; break;
		case 20:	random_res = RES_SOUND; break;
		case 21:	random_res = RES_WOOD; break;
		case 22:	random_res = RES_SILVER; break;
		case 23:	random_res = RES_IRON; break;
		default:	random_res = RES_IRON; break;
	}
	
	
	for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
		if(paf->where == TO_RESIST && IS_SET(paf->bitvector,random_res) )
			return 0;
	}
	
	return random_res;
}

int obj_random_paf_find_available_vulnerability(OBJ_DATA *obj)
{
	AFFECT_DATA *paf;
	int random_number = number_range(1,23); //str,int,wis,dex,con,cha
	int random_vuln = 0;
	
	switch(random_number)
	{
		case 1:		random_vuln = VULN_SUMMON; break;
		case 2:		random_vuln = VULN_CHARM; break;
		case 3:		random_vuln = VULN_MAGIC; break;
		case 4:		random_vuln = VULN_WEAPON; break;
		case 5:		random_vuln = VULN_BASH; break;
		case 6:		random_vuln = VULN_PIERCE; break;
		case 7:		random_vuln = VULN_SLASH; break;
		case 8:		random_vuln = VULN_FIRE; break;
		case 9:		random_vuln = VULN_COLD; break;
		case 10:	random_vuln = VULN_LIGHTNING; break;
		case 11:	random_vuln = VULN_ACID; break;
		case 12:	random_vuln = VULN_POISON; break;
		case 13:	random_vuln = VULN_NEGATIVE; break;
		case 14:	random_vuln = VULN_HOLY; break;
		case 15:	random_vuln = VULN_ENERGY; break;
		case 16:	random_vuln = VULN_MENTAL; break;
		case 17:	random_vuln = VULN_DISEASE; break;
		case 18:	random_vuln = VULN_DROWNING; break;
		case 19:	random_vuln = VULN_LIGHT; break;
		case 20:	random_vuln = VULN_SOUND; break;
		case 21:	random_vuln = VULN_WOOD; break;
		case 22:	random_vuln = VULN_SILVER; break;
		case 23:	random_vuln = VULN_IRON; break;
		default:	random_vuln = VULN_IRON; break;
	}
	
	
	for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
		if(paf->where == TO_VULN && IS_SET(paf->bitvector,random_vuln) )
			return 0;
	}
	
	return random_vuln;
}

int obj_random_paf_find_available_immunity(OBJ_DATA *obj)
{
	AFFECT_DATA *paf;
	int random_number = number_range(1,23); //str,int,wis,dex,con,cha
	int random_imm = 0;
	
	switch(random_number)
	{
		case 1:		random_imm = IMM_SUMMON; break;
		case 2:		random_imm = IMM_CHARM; break;
		case 3:		random_imm = IMM_MAGIC; break;
		case 4:		random_imm = IMM_WEAPON; break;
		case 5:		random_imm = IMM_BASH; break;
		case 6:		random_imm = IMM_PIERCE; break;
		case 7:		random_imm = IMM_SLASH; break;
		case 8:		random_imm = IMM_FIRE; break;
		case 9:		random_imm = IMM_COLD; break;
		case 10:	random_imm = IMM_LIGHTNING; break;
		case 11:	random_imm = IMM_ACID; break;
		case 12:	random_imm = IMM_POISON; break;
		case 13:	random_imm = IMM_NEGATIVE; break;
		case 14:	random_imm = IMM_HOLY; break;
		case 15:	random_imm = IMM_ENERGY; break;
		case 16:	random_imm = IMM_MENTAL; break;
		case 17:	random_imm = IMM_DISEASE; break;
		case 18:	random_imm = IMM_DROWNING; break;
		case 19:	random_imm = IMM_LIGHT; break;
		case 20:	random_imm = IMM_SOUND; break;
		case 21:	random_imm = IMM_WOOD; break;
		case 22:	random_imm = IMM_SILVER; break;
		case 23:	random_imm = IMM_IRON; break;
		default:	random_imm = IMM_IRON; break;
	}
	
	
	for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
		if(paf->where == TO_IMMUNE && IS_SET(paf->bitvector,random_imm) )
			return 0;
	}
	
	return random_imm;
}

void obj_random_paf(OBJ_DATA *obj)
{
	int location=0;

	
	if( !IS_SET( obj->wear_flags, ITEM_TAKE) )
	{
		return;
	}

	if(	obj->item_type == ITEM_TREASURE || obj->item_type == ITEM_FURNITURE		|| obj->item_type == ITEM_TRASH		|| obj->item_type == ITEM_KEY 		||
		obj->item_type == ITEM_MONEY	|| obj->item_type == ITEM_CORPSE_NPC	|| obj->item_type == ITEM_CORPSE_PC	|| obj->item_type == ITEM_FOUNTAIN	||
		obj->item_type == ITEM_PROTECT	|| obj->item_type == ITEM_MAP			|| obj->item_type == ITEM_ROOM_KEY	|| obj->item_type == ITEM_GEM		||
		obj->item_type == ITEM_JEWELRY	|| obj->item_type == ITEM_JUKEBOX		|| obj->item_type == ITEM_TATTOO	|| obj->item_type == ITEM_MAYMUNCUK ||
		obj->item_type == ITEM_FOOD		|| obj->item_type == ITEM_POTION 		|| obj->item_type == ITEM_DRINK_CON )
	{
		return;
	}
	
	if(obj->level < 10)
	{
		if(number_range(1,11)!=1)
		{
			return;
		}
	}
	else if(obj->level < 20)
	{
		if(number_range(1,9)!=1)
		{
			return;
		}
	}
	else if(obj->level < 30)
	{
		if(number_range(1,8)!=1)
		{
			return;
		}
	}
	else if(obj->level < 40)
	{
		if(number_range(1,7)!=1)
		{
			return;
		}
	}
	else if(obj->level < 50)
	{
		if(number_range(1,5)!=1)
		{
			return;
		}
	}
	else if(obj->level < 60)
	{
		if(number_range(1,4)!=1)
		{
			return;
		}
	}
	else if(obj->level < 70)
	{
		if(number_range(1,3)!=1)
		{
			return;
		}
	}
	else if(obj->level < 80)
	{
		if(number_range(1,2)!=1)
		{
			return;
		}
	}
	else
	{
		if(number_range(1,2)!=1)
		{
			return;
		}
	}
	
	while(number_range(1,10)<8)
	{
		obj->enchanted	= TRUE;
		if(number_percent()<95)
		{
			location = obj_random_paf_find_available_location(obj);
			if(location == 0)
				continue;
			AFFECT_DATA *paf;
			paf						= (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
			paf->where				= TO_OBJECT;
			paf->type               = -1;
			paf->level              = obj->level;
			paf->duration           = -1;
			paf->location           = location;
			paf->modifier           = obj_random_paf_modifier(location,obj->level);
			paf->bitvector          = 0;
			paf->next               = obj->affected;
			obj->affected			= paf;
			top_affect++;
		}
		else if(number_percent()<97)
		{
			AFFECT_DATA *paf;
			paf						= (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
			paf->where				= TO_VULN;
			paf->type               = -1;
            paf->level              = obj->level;
			paf->duration           = -1;
			paf->location           = 0;
			paf->modifier           = 0;
			paf->bitvector          = obj_random_paf_find_available_vulnerability(obj);
			paf->next               = obj->affected;
			obj->affected			= paf;
			top_affect++;
		}
		else if(number_percent()<99)
		{
			AFFECT_DATA *paf;
			paf						= (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
			paf->where				= TO_RESIST;
			paf->type               = -1;
            paf->level              = obj->level;
			paf->duration           = -1;
			paf->location           = 0;
			paf->modifier           = 0;
			paf->bitvector          = obj_random_paf_find_available_resistance(obj);
			paf->next               = obj->affected;
			obj->affected			= paf;
			top_affect++;
		}
		else
		{
			AFFECT_DATA *paf;
			paf						= (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
			paf->where				= TO_IMMUNE;
			paf->type               = -1;
            paf->level              = obj->level;
			paf->duration           = -1;
			paf->location           = 0;
			paf->modifier           = 0;
			paf->bitvector          = obj_random_paf_find_available_immunity(obj);
			paf->next               = obj->affected;
			obj->affected			= paf;
			top_affect++;
		}
	}
}

int obj_random_condition()
{
	int percent = 0;
	
	percent = number_percent();
	
	if(percent<=50)
		return 100;
	else if(percent>50 && percent<=70)
		return 90;
	else if(percent>70 && percent<=85)
		return 75;
	else if(percent>85 && percent<=90)
		return 50;
	else if(percent>90 && percent<=95)
		return 35;
	else
		return 20;
	return 20;
}

int obj_random_cost(int level)
{
	int min_cost = number_range(1,50);
	int max_cost = level * level * number_range(1,5);
	return UMAX( min_cost, max_cost );
}

int obj_random_weight(int level)
{
	return (UMAX(1,level/10) * number_range(1,10));
}

int obj_random_extra_flag()
{
	int newflag = 0;
	
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_GLOW);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_HUM);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_DARK);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_LOCK);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_EVIL);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_INVIS);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_MAGIC);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_NODROP);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_BLESS);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_ANTI_GOOD);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_ANTI_EVIL);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_ANTI_NEUTRAL);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_NOREMOVE);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_NOPURGE);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_NOSAC);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_NOLOCATE);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_MELT_DROP);
	if(number_percent()<2)
		SET_BIT(newflag,ITEM_BURN_PROOF);

    return newflag;
}

int obj_random_weapon_flag()
{
	int newflag = 0;
	
	if(number_percent()<2)
		SET_BIT(newflag,WEAPON_FLAMING);
	if(number_percent()<2)
		SET_BIT(newflag,WEAPON_FROST);
	if(number_percent()<2)
		SET_BIT(newflag,WEAPON_VAMPIRIC);
	if(number_percent()<2)
		SET_BIT(newflag,WEAPON_SHARP);
	if(number_percent()<2)
		SET_BIT(newflag,WEAPON_VORPAL);
	if(number_percent()<2)
		SET_BIT(newflag,WEAPON_TWO_HANDS);
	if(number_percent()<2)
		SET_BIT(newflag,WEAPON_SHOCKING);
	if(number_percent()<2)
		SET_BIT(newflag,WEAPON_POISON);
	if(number_percent()<2)
		SET_BIT(newflag,WEAPON_HOLY);

    return newflag;
}

char *obj_random_wand_potion_spell()
{
	return (char*)wand_spell_table[number_range(1,76)-1].name;
}
