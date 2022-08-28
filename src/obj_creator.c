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

int obj_random_paf_modifier()
{
	int random_number = number_range(1,100);
	
	if(random_number<=50)
		return 1;
	else if(random_number<=80)
		return 2;
	else if(random_number<=90)
		return 3;
	else if(random_number<=95)
		return 4;
	else if(random_number<=98)
		return 5;
	else
		return 6;
	return 1;
}

int obj_random_paf_find_available_location(OBJ_DATA *obj)
{
	int random_location = number_range(1,6); //str,int,wis,dex,con,cha
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
		paf->modifier           = obj_random_paf_modifier();
		paf->bitvector          = 0;
		paf->next               = obj->affected;
		obj->affected			= paf;
		top_affect++;
	}
}