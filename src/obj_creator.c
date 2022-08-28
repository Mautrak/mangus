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
		
		if(random_number<=50)
			return number_range(1,2);
		else if(random_number<=80)
			return number_range(1,3);
		else if(random_number<=90)
			return number_range(1,4);
		else if(random_number<=95)
			return number_range(1,5);
		else if(random_number<=98)
			return number_range(1,6);
		else
			return 6;
	}
	if (location == APPLY_MANA || location == APPLY_HIT || location == APPLY_MOVE)
	{
		random_number = number_range(1,100);
		
		if(random_number<=50)
			return number_range(10,UMAX(11,int(level/3)));
		else if(random_number<=80)
			return number_range(10,UMAX(10,int(level/2)));
		else if(random_number<=90)
			return number_range(10,UMAX(10,int(level)));
		else if(random_number<=95)
			return number_range(10,UMAX(10,int(level*(3/2))));
		else if(random_number<=98)
			return number_range(10,UMAX(10,int(level*2)));
		else
			return number_range(10,UMAX(10,int(level*3)));
	}
	if (location == APPLY_HITROLL || location == APPLY_DAMROLL)
	{
		random_number = number_range(1,100);
		
		if(random_number<=50)
			return number_range(1,UMAX(2,int(level/20)));
		else if(random_number<=80)
			return number_range(1,UMAX(2,int(level/15)));
		else if(random_number<=90)
			return number_range(1,UMAX(2,int(level/12)));
		else if(random_number<=95)
			return number_range(1,UMAX(2,int(level/10)));
		else if(random_number<=98)
			return number_range(1,UMAX(2,int(level/8)));
		else
			return number_range(1,UMAX(2,int(level/6)));
	}
	return 1;
}

int obj_random_paf_find_available_location(OBJ_DATA *obj)
{
	AFFECT_DATA *paf;
	int random_number = number_range(1,11); //str,int,wis,dex,con,cha
	int random_location = 0;
	
	switch(random_number)
	{
		case 1:		random_location = APPLY_STR; break;
		case 2:		random_location = APPLY_DEX; break;
		case 3:		random_location = APPLY_INT; break;
		case 4:		random_location = APPLY_WIS; break;
		case 5:		random_location = APPLY_CON; break;
		case 6:		random_location = APPLY_CHA; break;
		case 7:		random_location = APPLY_MANA; break;
		case 8:		random_location = APPLY_HIT; break;
		case 9:		random_location = APPLY_MOVE; break;
		case 10:	random_location = APPLY_HITROLL; break;
		case 11:	random_location = APPLY_DAMROLL; break;
		default:	return 0;
	}
	
	
	for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
		if(paf->location == random_location)
			return 0;
	}
	
	return random_location;
}

void obj_random_paf(OBJ_DATA *obj)
{
	int location=0;
	while(number_percent()<50)
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
}