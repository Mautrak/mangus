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
#include "recycle.h"

/*
 * Ortak yardımcılar: ROM'un yedi etki işlevi aynı iskeleti paylaşır
 * (oda -> nesneler, kurban -> envanter, nesne -> şans hesabı, mesaj,
 * kabın boşaltılması). Davranış ROM 2.4 ile aynıdır.
 */
typedef void EFFECT_FUN( void *vo, int level, int dam, int target );

/* Odadaki nesnelerin her birine etkiyi uygular. */
static void effect_room_objs( ROOM_INDEX_DATA *room, int level, int dam, EFFECT_FUN *fn )
{
    OBJ_DATA *obj, *obj_next;

    for ( obj = room->contents; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	fn( obj, level, dam, TARGET_OBJ );
    }
}

/* Kurbanın taşıdığı nesnelerin her birine etkiyi uygular. */
static void effect_inventory( CHAR_DATA *victim, int level, int dam, EFFECT_FUN *fn )
{
    OBJ_DATA *obj, *obj_next;

    for ( obj = victim->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	fn( obj, level, dam, TARGET_OBJ );
    }
}

/*
 * Nesne etkiden muaf mı? Yanmaz, silinmez ve sınırlı (limit) nesnelere
 * dokunulmaz; kalanlarda beşte bir rastgele bağışıklık vardır.
 */
static bool effect_immune( OBJ_DATA *obj )
{
    return IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
	|| IS_OBJ_STAT(obj, ITEM_NOPURGE)
	|| obj->pIndexData->limit != -1
	|| number_range( 0, 4 ) == 0;
}

/* Temel şans: seviye ve hasarla artar, 25/50 üstünde yavaşlar, nesne seviyesiyle düşer. */
static int effect_chance( OBJ_DATA *obj, int level, int dam )
{
    int chance = level / 4 + dam / 10;

    if ( chance > 25 )
	chance = (chance - 25) / 2 + 25;
    if ( chance > 50 )
	chance = (chance - 50) / 2 + 50;

    if ( IS_OBJ_STAT(obj, ITEM_BLESS) )
	chance -= 5;

    chance -= obj->level * 2;
    return chance;
}

/* Etki mesajını nesneyi taşıyana, yoksa odadakilere gösterir. */
static void effect_msg( OBJ_DATA *obj, const char *msg )
{
    if ( obj->carried_by != NULL )
	act( msg, obj->carried_by, obj, NULL, TO_ALL );
    else if ( obj->in_room != NULL && obj->in_room->people != NULL )
	act( msg, obj->in_room->people, obj, NULL, TO_ALL );
}

/* Zırhı aşındırır: AC etkisini bir puan kötüleştirir (yoksa yeni etki). */
static void etch_armor( OBJ_DATA *obj, int level, int duration )
{
    AFFECT_DATA *paf;
    int i;

    affect_enchant( obj );

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_AC )
	{
	    paf->type = -1;
	    paf->modifier += 1;
	    paf->level = UMAX( paf->level, level );
	    break;
	}
    }

    if ( paf == NULL )   /* yeni etki gerekli */
    {
	paf = new_affect();
	paf->type      = -1;
	paf->level     = level;
	paf->duration  = duration;
	paf->location  = APPLY_AC;
	paf->modifier  = 1;
	paf->bitvector = 0;
	paf->next      = obj->affected;
	obj->affected  = paf;
    }

    if ( obj->carried_by != NULL && obj->wear_loc != WEAR_NONE )
	for ( i = 0; i < 4; i++ )
	    obj->carried_by->armor[i] += 1;
}

/*
 * Yok edilen kabın içindekileri yere döker ve yarı güçle aynı etkiye
 * sokar; kap ne odada ne de birinin elindeyse içindekiler de yok olur.
 */
static void spill_contents( OBJ_DATA *obj, int level, int dam, EFFECT_FUN *fn )
{
    OBJ_DATA *t_obj, *n_obj;

    for ( t_obj = obj->contains; t_obj != NULL; t_obj = n_obj )
    {
	n_obj = t_obj->next_content;
	obj_from_obj( t_obj );
	if ( obj->in_room != NULL )
	    obj_to_room( t_obj, obj->in_room );
	else if ( obj->carried_by != NULL )
	    obj_to_room( t_obj, obj->carried_by->in_room );
	else
	{
	    extract_obj( t_obj );
	    continue;
	}
	fn( t_obj, level/2, dam/2, TARGET_OBJ );
    }
}

/* Kurbana kısa süreli körlük (ateş nefesi / kum fırtınası). */
static void blind_victim( CHAR_DATA *victim, int sn, int level )
{
    AFFECT_DATA af = {0};

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_range( 0, level/10 );
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
}

void acid_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
    {
	effect_room_objs( (ROOM_INDEX_DATA *) vo, level, dam, acid_effect );
	return;
    }

    if (target == TARGET_CHAR)  /* do the effect on a victim */
    {
	effect_inventory( (CHAR_DATA *) vo, level, dam, acid_effect );
	return;
    }

    if (target == TARGET_OBJ) /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	const char *msg;

	if ( effect_immune( obj ) )
	    return;

	chance = effect_chance( obj, level, dam );

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_CONTAINER:
	    case ITEM_CORPSE_PC:
	    case ITEM_CORPSE_NPC:
		msg = "$p duman çıkararak eriyor.";
		break;
	    case ITEM_ARMOR:
		msg = "$p delik delik oldu.";
		break;
	    case ITEM_CLOTHING:
		msg = "$p çürüyerek parçalara ayrıldı.";
	 	break;
	    case ITEM_STAFF:
	    case ITEM_WAND:
		chance -= 10;
		msg = "$p çürüyerek kırıldı.";
		break;
	    case ITEM_SCROLL:
		chance += 10;
		msg = "$p işe yaramaz hale geldi.";
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	effect_msg( obj, msg );

	if (obj->item_type == ITEM_ARMOR)  /* etch it */
	{
	    etch_armor( obj, level, -1 );
	    return;
	}

	/* get rid of the object */
	spill_contents( obj, level, dam, acid_effect );
	extract_obj(obj);
	return;
    }
}


void cold_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
    {
	effect_room_objs( (ROOM_INDEX_DATA *) vo, level, dam, cold_effect );
        return;
    }

    if (target == TARGET_CHAR) /* whack a character */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	/* chill touch effect */
	if (!saves_spell(level/4 + dam / 20, victim, DAM_COLD))
	{
	    AFFECT_DATA af = {0};

	    act("$s morarıyor ve titriyor.",victim,NULL,NULL,TO_ROOM);
	    act("Soğuk kemiklerinin içine işliyor.",victim,NULL,NULL,TO_CHAR);
            af.where     = TO_AFFECTS;
            af.type      = gsn_chill_touch;
            af.level     = level;
            af.duration  = 6;
            af.location  = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = 0;
            affect_join( victim, &af );
	}

	/* hunger! (warmth sucked out */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_HUNGER,dam/20);

	/* let's toast some gear */
	effect_inventory( victim, level, dam, cold_effect );
	return;
   }

   if (target == TARGET_OBJ) /* toast an object */
   {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	const char *msg;

	if ( effect_immune( obj ) )
	    return;

	chance = effect_chance( obj, level, dam );

	switch(obj->item_type)
	{
	    default:
		return;
	    case ITEM_POTION:
		msg = "$p donarak kırılıyor!";
		chance += 25;
		break;
	    case ITEM_DRINK_CON:
		msg = "$p donarak kırılıyor!";
		chance += 5;
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	effect_msg( obj, msg );
	extract_obj(obj);
	return;
    }
}



void fire_effect(void *vo, int level, int dam, int target)
{

    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
	effect_room_objs( (ROOM_INDEX_DATA *) vo, level, dam, fire_effect );
	return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	/* chance of blindness */
	if (!IS_AFFECTED(victim,AFF_BLIND)
	&&  !saves_spell(level / 4 + dam / 20, victim,DAM_FIRE))
	{
            act("$n dumandan kör oldu!",victim,NULL,NULL,TO_ROOM);
            act("Dumandan gözlerin taşarıyor...göremiyorsun!",
		victim,NULL,NULL,TO_CHAR);
	    blind_victim( victim, gsn_fire_breath, level );
	}

	/* getting thirsty */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_THIRST,dam/20);

	/* let's toast some gear! */
	effect_inventory( victim, level, dam, fire_effect );
	return;
    }

    if (target == TARGET_OBJ)  /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	const char *msg;

	if ( effect_immune( obj ) )
            return;

	chance = effect_chance( obj, level, dam );

        if  ( check_material( obj, "ice" ) )  {
          chance += 30;
          msg = "$p eriyerek buharlaşıyor!";
        }
        else
        switch ( obj->item_type )
        {
        default:
	    return;
        case ITEM_CONTAINER:
	    msg = "$p tutuşarak yanıyor!";
            break;
        case ITEM_POTION:
            chance += 25;
	    msg = "$p fokurdayarak kaynıyor!";
            break;
        case ITEM_SCROLL:
            chance += 50;
	    msg = "$p çatırdayarak yanıyor!";
            break;
        case ITEM_STAFF:
            chance += 10;
            msg = "$p duman çıkararak kömürleşiyor!";
            break;
        case ITEM_WAND:
            msg = "$p kıvılcımlar çıkararak çatırdıyor!";
            break;
        case ITEM_FOOD:
            msg = "$p kavrularak kararıyor!";
            break;
        case ITEM_PILL:
            msg = "$p eriyerek damlamaya başlıyor!";
            break;
        }

        chance = URANGE(5,chance,95);

        if (number_percent() > chance)
            return;

	effect_msg( obj, msg );
	spill_contents( obj, level, dam, fire_effect );
        extract_obj( obj );
	return;
    }
}

void poison_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
	effect_room_objs( (ROOM_INDEX_DATA *) vo, level, dam, poison_effect );
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        CHAR_DATA *victim = (CHAR_DATA *) vo;

	/* chance of poisoning */
        if (!saves_spell(level / 4 + dam / 20,victim,DAM_POISON))
        {
	    AFFECT_DATA af = {0};

	    send_to_char("Zehrin damarlarında dolaştığını hissediyorsun.\n\r",victim);
	    act("$n çok hasta görünüyor.",victim,NULL,NULL,TO_ROOM);

            af.where     = TO_AFFECTS;
            af.type      = gsn_poison;
            af.level     = level;
            af.duration  = level / 2;
            af.location  = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = AFF_POISON;
            affect_join( victim, &af );
        }

	/* equipment */
	effect_inventory( victim, level, dam, poison_effect );
	return;
    }

    if (target == TARGET_OBJ)  /* do some poisoning */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;

	/* zehir: kutsanmış nesne muaf, silinmez olması önemsiz */
	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
  	||  IS_OBJ_STAT(obj,ITEM_BLESS)
	||  obj->pIndexData->limit != -1
	||  number_range(0,4) == 0)
	    return;

	chance = effect_chance( obj, level, dam );

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_FOOD:
		break;
	    case ITEM_DRINK_CON:
		if (obj->value[0] == obj->value[1])
		    return;
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	obj->value[3] = 1;
	return;
    }
}


void shock_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {
	effect_room_objs( (ROOM_INDEX_DATA *) vo, level, dam, shock_effect );
	return;
    }

    if (target == TARGET_CHAR)
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	/* daze and confused? */
	if (!saves_spell(level/4 + dam/20,victim,DAM_LIGHTNING))
	{
	    send_to_char("Kasların yanıt vermiyor.\n\r",victim);
	    DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
	}

	/* toast some gear */
	effect_inventory( victim, level, dam, shock_effect );
	return;
    }

    if (target == TARGET_OBJ)
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	const char *msg;

	if ( effect_immune( obj ) )
	    return;

	chance = effect_chance( obj, level, dam );

	switch(obj->item_type)
	{
	    default:
		return;
	   case ITEM_WAND:
	   case ITEM_STAFF:
		chance += 10;
		msg = "$p aşırı yüklenerek patlıyor!";
		break;
	   case ITEM_JEWELRY:
		chance -= 10;
		msg = "$p çatlayarak değersizleşiyor.";
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	effect_msg( obj, msg );
	extract_obj(obj);
	return;
    }
}

void sand_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
    {
	effect_room_objs( (ROOM_INDEX_DATA *) vo, level, dam, sand_effect );
	return;
    }

    if (target == TARGET_CHAR)  /* do the effect on a victim */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!IS_AFFECTED(victim,AFF_BLIND)
	&&  !saves_spell(level / 4 + dam / 20, victim,DAM_COLD))
	{
            act("$n uçuşan kumlarla körleşti!",victim,NULL,NULL,TO_ROOM);
            act("Kum gözlerini yaşartıyor...göremiyorsun!",
		victim,NULL,NULL,TO_CHAR);
	    blind_victim( victim, gsn_sand_storm, level );
	}

	/* let's toast some gear */
	effect_inventory( victim, level, dam, sand_effect );
	return;
    }

    if (target == TARGET_OBJ) /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	const char *msg;

	if ( effect_immune( obj ) )
	    return;

	chance = effect_chance( obj, level, dam );

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_CONTAINER:
	    case ITEM_CORPSE_PC:
	    case ITEM_CORPSE_NPC:
		chance += 50;
		msg = "$p kumla dolarak buharlaşıyor.";
		break;
	    case ITEM_ARMOR:
		chance -=10;
		msg = "$p kumla yontuluyor.";
		break;
	    case ITEM_CLOTHING:
		msg = "$p kumla aşınıyor.";
	 	break;
	    case ITEM_WAND:
		chance = 50;
		msg = "$p çarpan kum taneleri ile şeklini kaybediyor.";
		break;
	    case ITEM_SCROLL:
		chance += 20;
		msg = "$p kumla kaplanıyor.";
		break;
	    case ITEM_POTION:
		chance +=10;
		msg = "$p çarpan kum taneleri ile parçalara ayrılıyor.";
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	effect_msg( obj, msg );

	if (obj->item_type == ITEM_ARMOR)  /* etch it */
	{
	    etch_armor( obj, level, level );
	    return;
	}

	/* get rid of the object */
	spill_contents( obj, level, dam, sand_effect );
	extract_obj(obj);
	return;
    }
}

void scream_effect(void *vo, int level, int dam, int target)
{

    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
	effect_room_objs( (ROOM_INDEX_DATA *) vo, level, dam, scream_effect );
	return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if  (!saves_spell(level / 4 + dam / 20, victim,DAM_SOUND))
	{
            AFFECT_DATA af = {0};

            act("$n duyamıyor!",victim,NULL,NULL,TO_ROOM);
            act("Duyamıyorsun!",victim,NULL,NULL,TO_CHAR);

            af.where        = TO_AFFECTS;
            af.type         = gsn_scream;
            af.level        = level;
            af.duration     = 0;
            af.location     = APPLY_NONE;
            af.modifier     = 0;
            af.bitvector    = AFF_SCREAM;
            affect_to_char(victim,&af);
	}

	/* daze and confused? */
	if (!saves_spell(level/4 + dam/20,victim,DAM_SOUND))
	{
	    send_to_char("Kulakların uğulduyor, kasların yanıt vermiyor.\n\r",victim);
	    DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
	}

	/* getting thirsty */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_THIRST,dam/20);

	/* let's toast some gear! */
	effect_inventory( victim, level, dam, scream_effect );
	return;
    }

    if (target == TARGET_OBJ)  /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	const char *msg;

	if ( effect_immune( obj ) )
            return;

	chance = effect_chance( obj, level, dam );

        if  ( check_material( obj, "ice" ) )
	{
          chance += 30;
          msg = "$p parçalanarak buharlaşıyor!";
        }
        else
	if ( check_material ( obj, "glass" ) )
	{
	    chance += 30;
	    msg = "$p küçük parçalara ayrılıyor.";
	}
	else
        switch ( obj->item_type )
        {
        default:
	    return;
        case ITEM_POTION:
            chance += 25;
            msg = "$p şişesi kırılarak içindekini yere döküyor!";
            break;
        case ITEM_SCROLL:
            chance += 50;
            msg = "$p küçük parçalara ayrılıyor!";
            break;
        case ITEM_DRINK_CON:
	    msg = "$p kırılıyor ve içindeki bozuluyor!";
	    chance += 5;
	    break;
        case ITEM_PILL:
	    msg = "$p parçalara ayrılıyor!";
            break;
        }

        chance = URANGE(5,chance,95);

        if (number_percent() > chance)
            return;

	effect_msg( obj, msg );
	spill_contents( obj, level, dam, scream_effect );
        extract_obj( obj );
	return;
    }
}
