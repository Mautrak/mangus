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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stddef.h>

#include "merc.h"
#include "db.h"
#include "lookup.h"
#include "tables.h"


/* values for db2.c */
struct		social_type	social_table		[MAX_SOCIALS];
int		social_count;

/*
 * snarf a socials file: ad satırı, ardından sırayla sekiz ileti satırı
 * ("$" = yok, "#" = kayıt burada biter).
 */
void load_socials( FILE *fp)
{
    static const size_t social_field[] =
    {
	offsetof(struct social_type, char_no_arg),
	offsetof(struct social_type, others_no_arg),
	offsetof(struct social_type, char_found),
	offsetof(struct social_type, others_found),
	offsetof(struct social_type, vict_found),
	offsetof(struct social_type, char_not_found),
	offsetof(struct social_type, char_auto),
	offsetof(struct social_type, others_auto),
    };

    for ( ; ; )
    {
    	struct social_type social;
    	char *temp;
	size_t i;

	memset( &social, 0, sizeof(social) );

    	temp = fread_word(fp);
    	if (!strcmp(temp,"#0"))
	    return;  /* done */

    	snprintf(social.name, sizeof(social.name), "%s", temp);
    	fread_to_eol(fp);

	for ( i = 0; i < sizeof(social_field) / sizeof(social_field[0]); i++ )
	{
	    temp = fread_string_eol(fp);
	    if (!strcmp(temp,"#"))
		break;
	    if (strcmp(temp,"$"))
		*(char **)((char *)&social + social_field[i]) = temp;
	}

	if ( social_count >= MAX_SOCIALS )
	{
	    bug( "Load_socials: MAX_SOCIALS (%d) aşıldı.", MAX_SOCIALS );
	    exit( 1 );
	}
	social_table[social_count] = social;
    	social_count++;
   }
   return;
}



void load_new_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;

    for ( ; ; )
    {
        int vnum;

        if ( ( vnum = read_mob_vnum( fp, "Load_new_mobiles" ) ) == 0 )
            break;

        pMobIndex                       = (MOB_INDEX_DATA *)alloc_perm( sizeof(*pMobIndex) );
        pMobIndex->vnum                 = vnum;
		newmobs++;
        pMobIndex->player_name          = fread_string( fp );
        pMobIndex->short_descr          = fread_string( fp );
        pMobIndex->long_descr           = fread_string( fp );
        pMobIndex->description          = fread_string( fp );
		pMobIndex->race		 			= race_lookup(fread_string( fp ));

        pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC | race_table[pMobIndex->race].act;
        pMobIndex->affected_by          = fread_flag( fp ) | race_table[pMobIndex->race].aff;
		pMobIndex->level                = fread_number( fp );

    if (pMobIndex->level <= 0)
      bug( "Load_new_mobiles: Level %d found.", pMobIndex->level );

		pMobIndex->long_descr        = first_case( pMobIndex->long_descr, TRUE );
        pMobIndex->description       = first_case( pMobIndex->description, TRUE );

		pMobIndex->practicer			= 0;
		pMobIndex->detection			= race_table[pMobIndex->race].det;

/* chronos smashed affection of ROM and created detection of ANATOLIA */
	if (IS_AFFECTED(pMobIndex,C))	/* detect evil */
		 SET_BIT(pMobIndex->detection,C);
	if (IS_AFFECTED(pMobIndex,D))	/* detect invis */
		 SET_BIT(pMobIndex->detection,D);
	if (IS_AFFECTED(pMobIndex,E))	/* detect magic */
		 SET_BIT(pMobIndex->detection,E);
	if (IS_AFFECTED(pMobIndex,F))	/* detect hidden */
		 SET_BIT(pMobIndex->detection,F);
	if (IS_AFFECTED(pMobIndex,G))	/* detect good */
		 SET_BIT(pMobIndex->detection,G);
	if (IS_AFFECTED(pMobIndex,Z))	/* dark vision */
		 SET_BIT(pMobIndex->detection,Z);
	if (IS_AFFECTED(pMobIndex,ee))	/* acute vision */
		 SET_BIT(pMobIndex->detection,ee);
	REMOVE_BIT(pMobIndex->affected_by,(C|D|E|F|G|Z|ee));

        pMobIndex->pShop                = NULL;
        pMobIndex->alignment            = 0;

        pMobIndex->hitroll              = 0;

	/* read hit dice */
        pMobIndex->hit[DICE_NUMBER]     = 1;
        /* 'd'          */
        pMobIndex->hit[DICE_TYPE]   	= 1;
        /* '+'          */
        pMobIndex->hit[DICE_BONUS]      = 1;

 	/* read mana dice */
	pMobIndex->mana[DICE_NUMBER]	= 1;
	pMobIndex->mana[DICE_TYPE]		= 1;
	pMobIndex->mana[DICE_BONUS]		= 1;

	/* read damage dice */
	pMobIndex->damage[DICE_NUMBER]	= 1;
	pMobIndex->damage[DICE_TYPE]	= 1;
	pMobIndex->damage[DICE_BONUS]	= 1;
	pMobIndex->dam_type				= 1;

	/* read armor class */
	pMobIndex->ac[AC_PIERCE]	= 1;
	pMobIndex->ac[AC_BASH]		= 1;
	pMobIndex->ac[AC_SLASH]		= 1;
	pMobIndex->ac[AC_EXOTIC]	= 1;

	/* read flags and add in data from the race table */
	pMobIndex->off_flags		= 0;
	pMobIndex->imm_flags		= 0;
	pMobIndex->res_flags		= 0;
	pMobIndex->vuln_flags		= 0;

	/* vital statistics */
	pMobIndex->start_pos		= 0;
	pMobIndex->default_pos		= 0;
	pMobIndex->sex				= 0;

	pMobIndex->form				= 0;
	pMobIndex->parts			= 0;
	/* size */
	pMobIndex->size				= 0;
	pMobIndex->material			= str_dup("none");
	pMobIndex->mprogs			= NULL;
	pMobIndex->progtypes		= 0;

        hash_insert_mob( pMobIndex );
        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }

    return;
}

/*
 * Snarf an obj section. new style (OBJECTS sabit, NEW_OBJECTS rastgele eşya:
 * A/F etkileri okunup atılır, eşya yaratılırken rastgele üretilir).
 */
static void load_obj_section( FILE *fp, bool random_object )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( ; ; )
    {
        int vnum;
        char letter;

        if ( ( vnum = read_obj_vnum( fp, "Load_objects" ) ) == 0 )
            break;

        pObjIndex                       = (OBJ_INDEX_DATA *)alloc_perm( sizeof(*pObjIndex) );
        pObjIndex->vnum                 = vnum;
		pObjIndex->random_object		= random_object;
        pObjIndex->new_format           = TRUE;
	pObjIndex->reset_num		= 0;
	newobjs++;
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );
        pObjIndex->material		= fread_string( fp );

        pObjIndex->item_type            = item_lookup(fread_word( fp ));
        pObjIndex->extra_flags          = fread_flag( fp );
        pObjIndex->wear_flags           = fread_flag( fp );
	switch(pObjIndex->item_type)
	{
	case ITEM_WEAPON:
	    pObjIndex->value[0]		= weapon_type(fread_word(fp));
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= attack_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_flag(fp);
	    break;
	case ITEM_CONTAINER:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_flag(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= fread_number(fp);
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
        case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
            pObjIndex->value[0]         = fread_number(fp);
            pObjIndex->value[1]         = fread_number(fp);
            pObjIndex->value[2]         = liq_lookup(fread_word(fp));
            pObjIndex->value[3]         = fread_number(fp);
            pObjIndex->value[4]         = fread_number(fp);
            break;
	case ITEM_WAND:
	case ITEM_STAFF:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
 	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[2]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= skill_lookup(fread_word(fp));
	    break;
	default:
            pObjIndex->value[0]             = fread_flag( fp );
            pObjIndex->value[1]             = fread_flag( fp );
            pObjIndex->value[2]             = fread_flag( fp );
            pObjIndex->value[3]             = fread_flag( fp );
	    pObjIndex->value[4]		    = fread_flag( fp );
	    break;
	}
	pObjIndex->level		= fread_number( fp );
        pObjIndex->weight               = fread_number( fp );
        pObjIndex->cost                 = fread_number( fp );
        pObjIndex->progtypes            = 0;
        pObjIndex->oprogs               = NULL;
        pObjIndex->limit                = -1;

        /* condition */
        letter 				= fread_letter( fp );
	switch (letter)
 	{
	    case ('P') :		pObjIndex->condition = 100; break;
	    case ('G') :		pObjIndex->condition =  90; break;
	    case ('A') :		pObjIndex->condition =  75; break;
	    case ('W') :		pObjIndex->condition =  50; break;
	    case ('D') :		pObjIndex->condition =  25; break;
	    case ('B') :		pObjIndex->condition =  10; break;
	    case ('R') :		pObjIndex->condition =   0; break;
	    default:			pObjIndex->condition = 100; break;
	}

	load_obj_affects( fp, pObjIndex, pObjIndex->level, TRUE, !random_object );

        hash_insert_obj( pObjIndex );
    }

    return;
}

void load_objects( FILE *fp )
{
    load_obj_section( fp, FALSE );
}

void load_new_objects( FILE *fp )
{
    load_obj_section( fp, TRUE );
}

/*
 * Snarf a mprog section
 */
static void omprog_directive( FILE *fp, char letter )
{
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    char progtype[MAX_INPUT_LENGTH];
    char progname[MAX_INPUT_LENGTH];
    int vnum = fread_number( fp );

    snprintf( progtype, sizeof(progtype), "%s", fread_word(fp) );
    snprintf( progname, sizeof(progname), "%s", fread_word(fp) );

    if ( letter == 'O' )
    {
	pObjIndex = get_obj_index( vnum );
	if (pObjIndex->oprogs == NULL)
	  pObjIndex->oprogs = (OPROG_DATA*)alloc_perm(sizeof(OPROG_DATA));
	oprog_set( pObjIndex, progtype, progname);
    }
    else
    {
	pMobIndex = get_mob_index( vnum );
	if (pMobIndex->mprogs == NULL)
	  pMobIndex->mprogs = (MPROG_DATA*)alloc_perm(sizeof(MPROG_DATA));
	mprog_set( pMobIndex,progtype,progname);
    }
}

void load_omprogs( FILE *fp )
{
    load_directives( fp, "Load_omprogs", "OM", omprog_directive );
}
