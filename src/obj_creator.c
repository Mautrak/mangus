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
		
		if(random_number<=50)
			return number_range(10,UMAX(11,int(level)));
		else if(random_number<=85)
			return number_range(10,UMAX(11,int(level*2)));
		else if(random_number<=95)
			return number_range(10,UMAX(11,int(level*3)));
		else if(random_number<=98)
			return number_range(10,UMAX(11,int(level*4)));
		else if(random_number<=99)
			return number_range(10,UMAX(11,int(level*5)));
		else
			return number_range(10,UMAX(11,int(level*6)));
	}
	if (location == APPLY_HITROLL || location == APPLY_DAMROLL)
	{
		random_number = number_range(1,100);
		
		if(random_number<=50)
			return number_range(1,UMAX(2,int(level/11)));
		else if(random_number<=70)
			return number_range(1,UMAX(2,int(level/9)));
		else if(random_number<=80)
			return number_range(1,UMAX(2,int(level/7)));
		else if(random_number<=90)
			return number_range(1,UMAX(2,int(level/5)));
		else if(random_number<=95)
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
	else if(random_number<100)
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

void obj_random_paf(OBJ_DATA *obj)
{
	int location=0;
	int bitvector=0;
	
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
	
	if(number_range(1,30)!=1)
	{
		return;
	}
	
	if(obj->level < 10)
	{
		if(number_range(1,30)!=1)
		{
			return;
		}
	}
	else if(obj->level < 20)
	{
		if(number_range(1,25)!=1)
		{
			return;
		}
	}
	else if(obj->level < 30)
	{
		if(number_range(1,20)!=1)
		{
			return;
		}
	}
	else if(obj->level < 40)
	{
		if(number_range(1,17)!=1)
		{
			return;
		}
	}
	else if(obj->level < 50)
	{
		if(number_range(1,15)!=1)
		{
			return;
		}
	}
	else if(obj->level < 60)
	{
		if(number_range(1,12)!=1)
		{
			return;
		}
	}
	else if(obj->level < 70)
	{
		if(number_range(1,9)!=1)
		{
			return;
		}
	}
	else if(obj->level < 80)
	{
		if(number_range(1,6)!=1)
		{
			return;
		}
	}
	else
	{
		if(number_range(1,3)!=1)
		{
			return;
		}
	}
	
	while(number_range(1,10)<6)
	{
		if(number_percent()<90)
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
		else
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
