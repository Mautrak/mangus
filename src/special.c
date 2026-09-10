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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "utf8.h"
#include "magic.h"

void	say_spell	( CHAR_DATA *ch, int sn );
void	one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool second );
bool    mob_cast_mage   ( CHAR_DATA *ch, CHAR_DATA *victim );
bool    mob_cast_cleric ( CHAR_DATA *ch, CHAR_DATA *victim );

/* command procedures needed */
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_open		);
DECLARE_DO_FUN(do_close		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_backstab	);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_tell		);
DECLARE_DO_FUN(do_cb		);
DECLARE_DO_FUN(do_track		);
DECLARE_DO_FUN(do_murder	);
DECLARE_DO_FUN(do_kill		);
DECLARE_DO_FUN(do_unlock	);
DECLARE_DO_FUN(do_lock		);
DECLARE_DO_FUN(do_drop		);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_north		);
DECLARE_DO_FUN(do_south		);
DECLARE_DO_FUN(do_east		);
DECLARE_DO_FUN(do_west		);
DECLARE_DO_FUN(do_sacrifice	);
DECLARE_DO_FUN(do_give		);
DECLARE_DO_FUN(do_rescue	);
DECLARE_DO_FUN(do_assassinate	);
DECLARE_DO_FUN(do_spellbane	);
DECLARE_DO_FUN(do_resistance	);
DECLARE_DO_FUN(do_bandage	);

/*
 * The following special functions are available for mobiles.
 */

DECLARE_SPEC_FUN(	spec_breath_any		);
DECLARE_SPEC_FUN(	spec_breath_acid	);
DECLARE_SPEC_FUN(	spec_breath_fire	);
DECLARE_SPEC_FUN(	spec_breath_frost	);
DECLARE_SPEC_FUN(	spec_breath_gas		);
DECLARE_SPEC_FUN(	spec_breath_lightning	);
DECLARE_SPEC_FUN(	spec_cast_adept		);
DECLARE_SPEC_FUN(	spec_cast_cleric	);
DECLARE_SPEC_FUN(	spec_cast_judge		);
DECLARE_SPEC_FUN(	spec_cast_mage		);
DECLARE_SPEC_FUN(	spec_cast_beholder	);
DECLARE_SPEC_FUN(	spec_cast_undead	);
DECLARE_SPEC_FUN(	spec_executioner	);
DECLARE_SPEC_FUN(	spec_fido		);
DECLARE_SPEC_FUN(	spec_guard		);
DECLARE_SPEC_FUN(	spec_janitor		);
DECLARE_SPEC_FUN(	spec_mayor		);
DECLARE_SPEC_FUN(	spec_poison		);
DECLARE_SPEC_FUN(	spec_thief		);
DECLARE_SPEC_FUN(	spec_nasty		);
DECLARE_SPEC_FUN(	spec_patrolman		);
DECLARE_SPEC_FUN(       spec_cast_cabal         );
DECLARE_SPEC_FUN(       spec_special_guard      );
DECLARE_SPEC_FUN(       spec_stalker            );
DECLARE_SPEC_FUN(       spec_questmaster        );
DECLARE_SPEC_FUN(       spec_duzenbaz        );
DECLARE_SPEC_FUN(       spec_assassinater       );
DECLARE_SPEC_FUN(       spec_repairman		);
DECLARE_SPEC_FUN(       spec_wishmaster		);
DECLARE_SPEC_FUN(       spec_kameni_dindar		);
DECLARE_SPEC_FUN(       spec_niryani_dindar		);
DECLARE_SPEC_FUN(       spec_nyahi_dindar		);
DECLARE_SPEC_FUN(       spec_sintaryan_dindar		);
DECLARE_SPEC_FUN(	spec_captain		);
DECLARE_SPEC_FUN(       spec_headlamia          );
/* cabal guardians */
DECLARE_SPEC_FUN(	spec_fight_enforcer	);
DECLARE_SPEC_FUN(	spec_fight_invader	);
DECLARE_SPEC_FUN(	spec_fight_ivan		);
DECLARE_SPEC_FUN(	spec_fight_seneschal	);
DECLARE_SPEC_FUN(	spec_fight_powerman	);
DECLARE_SPEC_FUN(	spec_fight_protector	);
DECLARE_SPEC_FUN(	spec_fight_hunter	);
DECLARE_SPEC_FUN(	spec_fight_lionguard	);


/* the function table */
const   struct  spec_type    spec_table[] =
{
    {	"spec_breath_any",		spec_breath_any		},
    {	"spec_breath_acid",		spec_breath_acid	},
    {	"spec_breath_fire",		spec_breath_fire	},
    {	"spec_breath_frost",		spec_breath_frost	},
    {	"spec_breath_gas",		spec_breath_gas		},
    {	"spec_breath_lightning",	spec_breath_lightning	},
    {	"spec_cast_adept",		spec_cast_adept		},
    {	"spec_cast_cleric",		spec_cast_cleric	},
    {	"spec_cast_judge",		spec_cast_judge		},
    {	"spec_cast_mage",		spec_cast_mage		},
    {	"spec_cast_beholder",		spec_cast_beholder	},
    {	"spec_cast_undead",		spec_cast_undead	},
    {	"spec_executioner",		spec_executioner	},
    {	"spec_fido",			spec_fido		},
    {	"spec_guard",			spec_guard		},
    {	"spec_janitor",			spec_janitor		},
    {	"spec_mayor",			spec_mayor		},
    {	"spec_poison",			spec_poison		},
    {	"spec_thief",			spec_thief		},
    {	"spec_nasty",			spec_nasty		},
    {	"spec_patrolman",		spec_patrolman		},
    {	"spec_cast_cabal",		spec_cast_cabal		},
    {	"spec_stalker",			spec_stalker		},
    {	"spec_special_guard",		spec_special_guard	},
    {   "spec_questmaster",             spec_questmaster        },
	{   "spec_duzenbaz",             spec_duzenbaz        },
    {   "spec_assassinater",            spec_assassinater	},
    {   "spec_repairman",		spec_repairman		},
    {   "spec_wishmaster",		spec_wishmaster		},
    {   "spec_kameni_dindar",		spec_kameni_dindar		},
    {   "spec_niryani_dindar",		spec_niryani_dindar		},
    {   "spec_nyahi_dindar",		spec_nyahi_dindar		},
    {   "spec_sintaryan_dindar",		spec_sintaryan_dindar		},
    {	"spec_captain",			spec_captain		},
    {   "spec_headlamia",		spec_headlamia		},
    {	"spec_fight_enforcer",		spec_fight_enforcer	},
    {	"spec_fight_invader",		spec_fight_invader	},
    {	"spec_fight_ivan",		spec_fight_ivan		},
    {	"spec_fight_seneschal",		spec_fight_seneschal	},
    {	"spec_fight_powerman",		spec_fight_powerman	},
    {	"spec_fight_protector",		spec_fight_protector	},
    {	"spec_fight_hunter",		spec_fight_hunter	},
    {	"spec_fight_lionguard",		spec_fight_lionguard	},
    {	NULL,				NULL			}
};

/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup( char *name )
{
   int i;

   for ( i = 0; spec_table[i].name != NULL; i++)
   {
        if (utf8_first_eq(name, spec_table[i].name)
        &&  !str_prefix( name,spec_table[i].name))
            return spec_table[i].function;
   }

    return 0;
}

char *spec_name( SPEC_FUN *function)
{
    int i;

    for (i = 0; spec_table[i].function != NULL; i++)
    {
	if (function == spec_table[i].function)
	    return (char*)spec_table[i].name;
    }

    return NULL;
}


/*
 * Ortak yardımcılar
 */

/* ch ile dövüşenlerden rastgele biri (her aday 1/(odds+1) şansla); dövüşmüyorsa NULL */
static CHAR_DATA *fighting_victim( CHAR_DATA *ch, int odds )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING )
	return NULL;

    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
	if ( victim->fighting == ch && number_range(0,odds) == 0 )
	    return victim;

    return NULL;
}

/* name adlı büyüyü victim üzerine yapar (say: sözcükler söylenir); büyü yoksa FALSE */
static bool cast_at( CHAR_DATA *ch, CHAR_DATA *victim, const char *name, bool say )
{
    int sn;

    if ( name == NULL || ( sn = skill_lookup( (char *) name ) ) < 0 )
	return FALSE;

    if ( say )
	say_spell( ch, sn );
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TARGET_CHAR );
    return TRUE;
}

/*
 * Kabal muhafızı dövüş büyüsü: 1..16 zarıyla spells[zar-1] seçilir (NULL = büyü yok);
 * zar low_hp_roll ise ve canı üçte birin altındaysa low_hp_spell yapılır.
 */
static bool spec_fight_cast( CHAR_DATA *ch, const char *const spells[16],
			     int low_hp_roll, const char *low_hp_spell )
{
    CHAR_DATA *victim;
    const char *spell;
    int roll;

    if ( ( victim = fighting_victim( ch, 1 ) ) == NULL )
	return FALSE;

    roll = dice(1,16);
    spell = spells[roll - 1];
    if ( roll == low_hp_roll )
	spell = ch->hit < ch->max_hit / 3 ? low_hp_spell : NULL;

    return cast_at( ch, victim, spell, TRUE );
}

/* Yüzde birlik şansla tek cümle söyler (görevci, düzenbaz, tamirci) */
static bool say_rarely( CHAR_DATA *ch, const char *msg )
{
    if ( !IS_AWAKE(ch) )
	return FALSE;
    if ( number_range(0,100) != 0 )
	return FALSE;
    do_say( ch, (char *) msg );
    return TRUE;
}

/* Dindarlar: tanrılarının el yazmalarını arar */
static bool dindar_says( CHAR_DATA *ch, const char *god, const char *place )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_AWAKE(ch) || number_range(0,100) != 0 )
	return FALSE;

    do_say( ch, "Merhaba tanrının merhametine muhtaç kişi." );
    snprintf( buf, sizeof(buf), "%s el yazmalarını arıyorum.", god );
    do_say( ch, buf );
    snprintf( buf, sizeof(buf), "%s o el yazmalarından bahsedildiğini duydum.", place );
    do_say( ch, buf );
    do_say( ch, "Ama bulmak nasip olmadı." );
    return TRUE;
}

/* Muhafız: aranan oyuncuya bağırıp saldırır */
static bool attack_wanted( CHAR_DATA *ch, CHAR_DATA *victim, const char *cry )
{
    char buf[MAX_STRING_LENGTH];

    snprintf( buf, sizeof(buf), "%s bir SUÇLU!  MASUMU KORU!  %s", victim->name, cry );
    do_yell( ch, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}

/* Rahip yardımcıları: görülebilen bir hedefe sihirli sözcüklerle koruyucu büyü */
struct buff_spell
{
    const char *words;
    const char *spell;
};

static bool cast_buff( CHAR_DATA *ch, CHAR_DATA *victim, const struct buff_spell *table, int ntable )
{
    char buf[MAX_STRING_LENGTH];
    int i = number_range(0,15);

    if ( i >= ntable )
	return FALSE;

    snprintf( buf, sizeof(buf), "$n sihirli sözcükler söylüyor '%s'.", table[i].words );
    act( buf, ch, NULL, NULL, TO_ROOM );
    cast_at( ch, victim, table[i].spell, FALSE );
    return TRUE;
}

bool spec_patrolman(CHAR_DATA *ch)
{
    CHAR_DATA *vch,*victim = NULL;
    const char *message;
    int count = 0;

    if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == NULL
    ||  IS_AFFECTED(ch,AFF_CHARM) || ch->fighting != NULL)
        return FALSE;

    /* look for a fight in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch == ch)
	    continue;

	if (vch->fighting != NULL)  /* break it up! */
	{
	    if (number_range(0,count) == 0)
	        victim = (vch->level > vch->fighting->level)
		    ? vch : vch->fighting;
	    count++;
	}
    }

    if (victim == NULL || (IS_NPC(victim) && victim->spec_fun == ch->spec_fun))
	return FALSE;
    switch (number_range(0,6))
    {
	default:	message = NULL;		break;
  case 0:	message = "$n 'Hey hey heey, ayrılın!' diye bağırıyor";
		break;
	case 1: message =
		"$n 'Sorumlu olan toplum, tek başıma ne yapabilirim ki?' dedi";
		break;
	case 2: message =
		"$n 'Bu serseriler hepimizi mahvedecek.' diye mırıldanıyor";
		break;
	case 3: message = "$n 'Durun! Durun!' diye bağırıyor ve saldırıyor.";
		break;
	case 4: message = "$n sopasını eline alarak işe girişiyor.";
		break;
	case 5: message =
		"$n iç çekerek kavgayı ayırmaya girişiyor.";
		break;
	case 6: message = "$n 'Sakin olun, sizi fanatikler!' diyor.";
		break;
    }

    if (message != NULL)
	act(message,ch,NULL,NULL,TO_ALL);

    multi_hit(ch,victim,TYPE_UNDEFINED);

    return TRUE;
}

/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

   for ( victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
	v_next = victim->next_in_room;
	if ( ((RIDDEN(ch) && RIDDEN(ch)->fighting == victim)
		|| victim->fighting == ch)
	     && number_range(0,7) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
	return FALSE;

    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    switch ( number_range(0,7) )
    {
    case 0: return spec_breath_fire		( ch );
    case 1:
    case 2: return spec_breath_lightning	( ch );
    case 3: return spec_breath_gas		( ch );
    case 4: return spec_breath_acid		( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost		( ch );
    }

    return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, NULL,TARGET_CHAR);
    return TRUE;
}



bool spec_breath_lightning( CHAR_DATA *ch )
{
    return dragon( ch, "lightning breath" );
}



bool spec_cast_adept( CHAR_DATA *ch )
{
    static const struct buff_spell spells[] =
    {
	{ "abrazak",			"armor"		},
	{ "fido",			"bless"		},
	{ "judicandus noselacri",	"cure blindness" },
	{ "judicandus dies",		"cure light"	},
	{ "judicandus sausabru",	"cure poison"	},
	{ "candusima",			"refresh"	},
	{ "judicandus eugzagz",		"cure disease"	},
    };
    CHAR_DATA *victim;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
    {
	if ( victim != ch && can_see( ch, victim ) && number_range(0,1) == 0
	     && !IS_NPC(victim) && victim->level < 11)
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    return cast_buff( ch, victim, spells, sizeof spells / sizeof spells[0] );
}



bool spec_cast_cleric( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ( victim = fighting_victim( ch, 3 ) ) == NULL )
	return FALSE;

    mob_cast_cleric(ch, victim);
    return TRUE;
}

bool spec_cast_judge( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ( victim = fighting_victim( ch, 3 ) ) == NULL )
        return FALSE;

    return cast_at( ch, victim, "high explosive", FALSE );
}



bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ( victim = fighting_victim( ch, 3 ) ) == NULL )
	return FALSE;

    mob_cast_mage(ch, victim);
    return TRUE;
}



bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    const char *spell;

    if ( ( victim = fighting_victim( ch, 3 ) ) == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_range(0,15) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	case  7: min_level = 21; spell = "teleport";       break;
	case  8: min_level = 20; spell = "plague";	   break;
	default: min_level = 18; spell = "harm";           break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    return cast_at( ch, victim, spell, TRUE );
}


bool spec_executioner( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
    {
	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED)
	&&   can_see(ch,victim))
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    REMOVE_BIT(ch->comm,COMM_NOSHOUT);
    return attack_wanted( ch, victim, "KAAAANN!!!" );
}



bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

      act( "$n cesedi vahşice parçalayıp yiyor.", ch, NULL, NULL, TO_ROOM );
	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, ch->in_room );
	}
	extract_obj( corpse );
	return TRUE;
    }

    return FALSE;
}


bool spec_janitor( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) || !can_loot(ch,trash))
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10 )
	{
    act( "$n çöpleri topluyor.", ch, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    if (IS_SET(trash->progtypes,OPROG_GET))
		(trash->pIndexData->oprogs->get_prog) (trash,ch);
	    return TRUE;
	}
    }

    return FALSE;
}



bool spec_mayor( CHAR_DATA *ch )
{
    static const char open_path[] =
	"W3a3003b000c000d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
	"W3a3003b000c000d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;
    OBJ_DATA *key;

    if ( !move )
    {
	if ( time_info.hour ==  6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0', FALSE );
	break;

    case 'W':
	ch->position = POS_STANDING;
  act("$n uyanıyor ve gürültülü bir inilti koparıyor.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
  act("$n yatıp uyuyor.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'a':
    do_say( ch,"Merhaba tatlım!" );
	break;

    case 'b':
    do_say(ch,"Şu rezilliğe bak! Çöpler konusunda birşey yapmalıyım!");
	break;

    case 'c':
    do_say(ch,"Gençlerde saygı denen şey kalmadı.");
	break;

    case 'd':
    do_say(ch,"İyi günler, vatandaşlar!");
	break;

    case 'e':
    do_say(ch,"Selenge şehir kapılarının açıldığını bildiririm.");
	break;

    case 'E':
    do_say(ch,"Selenge şehir kapılarının kapandığını bildiririm.");
	break;

    case 'O':
	do_unlock( ch, "kapı" );
	do_open( ch, "kapı" );
  interpret( ch, "duygu şehir kapısının kilidini açıyor.",FALSE);
	for( key=ch->in_room->contents; key!=NULL; key=key->next_content )
	  if ( key->pIndexData->vnum == 3379 )
	    break;
	if ( key != NULL )
	  SET_BIT( key->wear_flags, ITEM_TAKE );
	do_get( ch, "anahtar" );
	break;

    case 'C':
	do_close( ch, "kapı" );
	do_lock( ch, "kapı" );
	do_drop( ch, "anahtar" );
  interpret( ch, "duygu şehir kapısını zincirliyor.",FALSE);
	for( key=ch->in_room->contents; key!=NULL; key=key->next_content )
	  if ( key->pIndexData->vnum == 3379 )
	    break;
	if ( key != NULL )
	  REMOVE_BIT( key->wear_flags, ITEM_TAKE );
	break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}



bool spec_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > 2 * ch->level )
	return FALSE;

  act( "$M ısırıyorsun!",  ch, NULL, victim, TO_CHAR    );
  act( "$n $M ısırıyor!",  ch, NULL, victim, TO_NOTVICT );
  act( "$n seni ısırıyor!", ch, NULL, victim, TO_VICT    );
    spell_poison( gsn_poison, ch->level, ch, victim,TARGET_CHAR);
    return TRUE;
}



bool spec_thief( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    long silver;

    if ( ch->position != POS_STANDING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC(victim)
	||   victim->level >= LEVEL_IMMORTAL
	||   number_range(0,31) != 0
	||   !can_see(ch,victim))
	    continue;

	if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
	{
    act( "$s ellerinin para keseni kurcaladığını farkediyorsun!",
  ch, NULL, victim, TO_VICT );
    act( "$N $s ellerinin para kesesini kurcaladığını farkediyor!",
		ch, NULL, victim, TO_NOTVICT );
	    return TRUE;
	}
	else
	{
	    silver = victim->silver * UMIN(number_range(1,20),ch->level/2)/100;
	    silver = UMIN(silver,ch->level*ch->level * 25);
	    ch->silver	+= silver;
	    victim->silver -= silver;
	    return TRUE;
	}
    }

    return FALSE;
}


bool spec_cast_cabal( CHAR_DATA *ch )
{
    static const struct buff_spell spells[] =
    {
	{ "abracal",			"armor"		},
	{ "balc",			"bless"		},
	{ "judicandus noselacba",	"cure blindness" },
	{ "judicandus bacla",		"cure light"	},
	{ "judicandus sausabcla",	"cure poison"	},
	{ "candabala",			"refresh"	},
    };
    CHAR_DATA *victim;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
    {
	if ( victim != ch && can_see( ch, victim ) && number_range(0,1) == 0)
	  break;
    }

    if ( victim == NULL )
	return FALSE;

    return cast_buff( ch, victim, spells, sizeof spells / sizeof spells[0] );
}



bool spec_guard( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *ech;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
      return FALSE;

    ech      = NULL;

    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
      {
        if (!can_see(ch,victim)) continue;

	if (IS_SET(ch->in_room->area->area_flag,AREA_HOMETOWN)
		&& number_percent() < 2 )
	 {
	  do_say( ch, "Seni tanıyor muyum?");
	  if (str_cmp(ch->in_room->area->name,
		hometown_table[victim->hometown].name) )
	   {
	    do_say( ch,"Seni hatırlamıyorum. Uzaklaş!");
	   }
	  else
	   {
	    do_say(ch, "Tamam, şimdi hatırladım.");
	    interpret( ch, "gülümse",FALSE);
	   }
	 }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED) )
	  break;

	if ( victim->fighting != NULL
	     && victim->fighting != ch
	     && victim->ethos != 1
	     && !IS_GOOD(victim)
	     && !IS_EVIL(victim->fighting) )
	  { ech = victim; victim=NULL; break; }
      }

    if ( victim != NULL )
	return attack_wanted( ch, victim, "SALDIR!!!" );

    if ( ech != NULL )
      {
        act( "$n bağırıyor 'MASUMU KORU!!  SALDIR!!!'",
			ch, NULL, NULL, TO_ROOM );
	multi_hit( ch, ech, TYPE_UNDEFINED );
	return TRUE;
      }

    return FALSE;
  }




bool spec_special_guard( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *ech;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
	return FALSE;

    ech      = NULL;

    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
    {
        if (!can_see(ch,victim)) continue;

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED) )
	  break;

	if ( victim->fighting != NULL
	     && victim->fighting != ch
	     && victim->fighting->cabal == CABAL_RULER )
	  { ech = victim; victim=NULL; break; }
    }

    if ( victim != NULL )
	return attack_wanted( ch, victim, "SALDIR!!!" );

    if ( ech != NULL )
    {
      act( "$n bağırıyor 'MASUMU KORU!  SALDIR!!!'",
			ch, NULL, NULL, TO_ROOM );
	multi_hit( ch, ech, TYPE_UNDEFINED );
	return TRUE;
    }

    return FALSE;
}

bool spec_stalker(CHAR_DATA *ch)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *wch;
  int i;

  victim = ch->last_fought;

  if (ch->fighting != NULL)
    return FALSE;

  if (ch->status == 10)
    {
      ch->cabal = CABAL_RULER;
      do_cb(ch, "Kurbanımı öldürdüğüme göre geldiğim yere dönebilirim.");
      extract_char(ch, TRUE);
      return TRUE;
    }

  if (victim == NULL)
    {
      ch->cabal = CABAL_RULER;
      do_cb(ch,"Kurbanım oyundan ayrıldı. Bu benim gitme vaktimin geldiğinin de göstergesi.");
      extract_char(ch, TRUE);
      return TRUE;
    }

  i = align_index(victim);

  for (wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room)
    {
     if (victim == wch)
      {
        snprintf(buf, sizeof(buf),"Sen bir suçlusun %s! Ve öleceksin!!!", victim->name);
       do_yell(ch,buf);
       multi_hit(ch,wch,TYPE_UNDEFINED);
       return TRUE;
      }
  }
  do_track(ch, victim->name);

  if (ch->status == 5)
    {
      if (ch->in_room !=
get_room_index(hometown_table[victim->hometown].recall[i]))
	{
      	  char_from_room(ch);
      	  char_to_room(ch,
		   get_room_index(hometown_table[victim->hometown].recall[i]));
      	  do_track(ch, victim->name);
      	  return TRUE;
    	}
       else
    	{
      	  ch->cabal = CABAL_RULER;
          snprintf(buf, sizeof(buf), "Üzgünüm ama %s izini kaybettirdi. Gitmeliyim.",
	      victim->name);
      	  do_cb(ch, buf);
      	  extract_char(ch, TRUE);
      	  return TRUE;
    	}
    }
  return FALSE;
}

bool spec_nasty( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    long silver;

    if (!IS_AWAKE(ch)) {
       return FALSE;
    }

    if (ch->position != POS_FIGHTING) {
       for ( victim = ch->in_room->people; victim != NULL; victim = v_next)
       {
          v_next = victim->next_in_room;
          if (!IS_NPC(victim)
             && (victim->level > ch->level)
             && (victim->level < ch->level + 10))
          {
	     do_backstab(ch,victim->name);
             if (ch->position != POS_FIGHTING)
                 do_murder(ch,victim->name);
             /* should steal some coins right away? :) */
             return TRUE;
          }
       }
       return FALSE;    /*  No one to attack */
    }

    /* okay, we must be fighting.... steal some coins and flee */
    if ( (victim = ch->fighting) == NULL)
        return FALSE;   /* let's be paranoid.... */

    switch ( number_range(0,3) )
    {
      case 0:  act("$n para keseni yırtıp atarak paranın saçılmasına neden oluyor!",
                   ch, NULL, victim, TO_VICT);
               act("$S para kesesini yırtıyor ve akçelerini topluyorsun.",
                   ch, NULL, victim, TO_CHAR);
               act("$S para kesesi yırtılıyor!",
                     ch, NULL, victim, TO_NOTVICT);
                 silver = victim->silver / 2;  /* steal half of his silver */
                 victim->silver -= silver;
                 ch->silver     += silver;
                 return TRUE;

        case 1:  do_flee( ch, "");
                 return TRUE;

        default: return FALSE;
    }
}

bool spec_questmaster( CHAR_DATA *ch )
{
    return say_rarely( ch, "Bir görev istemez misin?" );
}

bool spec_duzenbaz( CHAR_DATA *ch )
{
    return say_rarely( ch, "Hoşgeldin, var mıdır bir sıkıntı? Yardımcı olalım." );
}

bool spec_assassinater( CHAR_DATA *ch )
{
    /* 1..40 zarında yalnızca 5-10 konuşur ve saldırır */
    static const char *const sayings[] =
    {
	"Ölüm tek sondur...",
	"Ölme zamanı....",
	"Ölme zamanı....",
	"Kaderin seni bekliyor....",
	"Ölümsüzlere bir kurban.... ",
	"Hiç şeytanla raks ettin mi....",
    };
    CHAR_DATA *victim;
    int rnd_say;

    if ( ch->fighting != NULL )
	return FALSE;

    /* hırsız ve ninja dışındaki ilk kişi (yaratıklar aşağıda elenir) */
    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
    {
        if ((victim->iclass != CLASS_THIEF) && (victim->iclass != CLASS_NINJA))
	    break;
    }

    if ( victim == NULL || victim == ch || IS_IMMORTAL(victim) )
        return FALSE;
    if ( victim->level > ch->level + 7 || IS_NPC(victim))
        return FALSE;
    if (victim->hit < victim->max_hit)
	return FALSE;

    rnd_say = number_range (1, 40);
    if ( rnd_say < 5 || rnd_say > 10 )
	return FALSE;

    do_say( ch, (char *) sayings[rnd_say - 5] );
    multi_hit( ch, victim ,gsn_assassinate );
    return TRUE;
}


bool spec_repairman( CHAR_DATA *ch )
{
    return say_rarely( ch, "Şimdi diğer ekipmanları tamir etme zamanı." );
}

bool spec_captain( CHAR_DATA *ch )
{

    static const char open_path[] =
"Wn0onc0oe1f2212211s2tw3xw3xd3322a22b22yO00d00a0011e1fe1fn0o3300300w3xs2ts2tS.";

    static const char close_path[] =
"Wn0on0oe1f2212211s2twc3xw3x3322d22a22EC0a00d0b0011e1fe1fn0o3300300w3xs2ts2tS.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
	if ( time_info.hour ==  6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting != NULL )
	return spec_cast_cleric( ch );

    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0' ,FALSE);
	break;

    case 'W':
	ch->position = POS_STANDING;
  act_color( "$C$n uyanıyor ve esniyor.$c", ch, NULL, NULL, TO_ROOM,
POS_RESTING,CLR_WHITE );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
  act_color( "$C$n yatıp uyumaya başlıyor.$c", ch, NULL, NULL, TO_ROOM,
POS_RESTING,CLR_WHITE );
	break;

    case 'a':
    act_color( "$C$n 'Selamlar! İyi avlar dilerim!' dedi$c", ch, NULL,
        NULL,TO_ROOM,POS_RESTING, CLR_YELLOW );
	break;

    case 'b':
    act_color("$C$n 'Caddeleri temiz tutalım ki Solace temiz olsun,' dedi.$c",ch,NULL, NULL, TO_ROOM, POS_RESTING,CLR_YELLOW );
	break;

    case 'c':
    act_color( "$C$n 'Bu kapılara bir çare bulmalıyım,' dedi.$c",ch,
    NULL, NULL, TO_ROOM, POS_RESTING,CLR_YELLOW );
    act_color("$C$n 'Buradan hiç ayrılmayacağım,' dedi.$c", ch, NULL, NULL,
        TO_ROOM,POS_RESTING,CLR_YELLOW );
	break;

    case 'd':
    act_color("$C$n says 'Selamlar Solace vatadaşları!' dedi.$c", ch, NULL, NULL,
        TO_ROOM, POS_RESTING,CLR_YELLOW );
	break;

    case 'y':
    act_color( "$C$n 'Solace şehrinin açıldığını bildiririm!' dedi.$c", ch,
NULL, NULL, TO_ROOM, POS_RESTING,CLR_YELLOW );
	break;

    case 'E':
    act_color( "$C$n 'Solace şehrinin kapandığını deklare ederim!' dedi.$c", ch,
NULL, NULL, TO_ROOM, POS_RESTING,CLR_YELLOW );
	break;

    case 'O':
	do_unlock( ch, "kapı" );
	do_open( ch, "kapı" );
	break;

    case 'C':
	do_close( ch, "kapı" );
	do_lock( ch, "kapı" );
	break;

    case 'n':
	do_open( ch, "kuzey" );
	break;

    case 'o':
        do_close( ch, "güney" );
        break;

    case 's':
	do_open( ch, "güney" );
	break;

    case 't':
        do_close( ch, "kuzey" );
        break;

    case 'e':
	do_open( ch, "doğu" );
	break;

    case 'f':
        do_close( ch, "batı" );
        break;

    case 'w':
	do_open( ch, "batı" );
	break;

    case 'x':
        do_close( ch, "doğu" );
        break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}

bool spec_headlamia( CHAR_DATA *ch )
{
    static const char path[] = "T111111100003332222232211.";
    static int pos=0;
    static bool move;
    static int count=0;
    CHAR_DATA *vch, *vch_next;

    if ( !move )
    {
      if (count++ == 10000) move=1;
    }

    if (  ch->position < POS_SLEEPING || ch->fighting )
	return FALSE;

    for(vch=ch->in_room->people;vch;vch=vch->next_in_room)
	{
	 if (IS_NPC(vch) && vch->pIndexData->vnum == 3143)
	  {
	   do_kill(ch,vch->name);
	   break;
	  }
	}

    if (!move) return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0', FALSE );
	pos++;
	break;

    case 'T':
	pos++;
	for(vch=char_list;vch;vch=vch_next)
	 {
	   vch_next = vch->next;
	   if (!IS_NPC(vch)) continue;
	   if (vch->pIndexData->vnum == 5201)
	    {
	     if (!vch->fighting && !vch->last_fought)
		{
		 char_from_room( vch );
		 char_to_room( vch , ch->in_room );
		 vch->master = ch;
		 vch->leader = ch;
		}
	    }
	 }
	break;

    case '.' :
	move = FALSE;
	count = 0;
	pos = 0;
	break;
    }

    return FALSE;
}


bool spec_cast_beholder( CHAR_DATA *ch )
{
    static const char *const spells[16] =
    {
	"fear", "slow", "cause serious", "cause critical", "harm", "harm",
	"dispel magic", "dispel magic",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };
    CHAR_DATA *victim;

    if ( ( victim = fighting_victim( ch, 1 ) ) == NULL )
        return FALSE;

    return cast_at( ch, victim, spells[dice(1,16) - 1], FALSE );
}



/* Kabal muhafızlarının büyü tabloları: dice(1,16) -> spells[zar-1], NULL = büyü yok */
static const char *const enforcer_spells[16] =
{
    "dispel magic", "acid arrow", "acid arrow", "caustic font", "caustic font",
    "acid blast", "acid blast", "acid blast", "acid blast", "acid blast",
    NULL, NULL, NULL, NULL, NULL, NULL
};

static const char *const invader_spells[16] =
{
    "blindness", "dispel magic", "dispel magic", "weaken", "weaken",
    "energy drain", "energy drain", "plague", "plague", "acid arrow", "acid arrow",
    "acid blast", "acid blast", "acid blast",
    NULL /* 15: shadow cloak (canı azsa) */, NULL
};

static const char *const ivan_spells[16] =
{
    "dispel magic", "acid arrow", "acid arrow", "caustic font", "caustic font",
    "acid blast", "acid blast", "acid blast", "disgrace",
    NULL /* 10: garble (canı azsa) */, NULL, NULL, NULL, NULL, NULL, NULL
};

static const char *const seneschal_spells[16] =
{
    "dispel magic", "weaken", "blindness", "acid arrow", "caustic font",
    "energy drain", "acid blast", "acid blast", "acid blast", "plague",
    "acid blast", "lightning breath", "lightning breath", "mental knife",
    "mental knife", NULL
};

static const char *const hunter_spells[16] =
{
    "dispel magic", "acid arrow", "acid arrow", "caustic font", "caustic font",
    "acid blast", "acid blast", "acid blast", "acid blast",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static const char *const lionguard_spells[16] =
{
    "dispel magic", "acid blast", "acid blast", "caustic font", "caustic font",
    "acid arrow", "acid arrow", "acid arrow",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

bool spec_fight_enforcer( CHAR_DATA *ch )
{
    return spec_fight_cast( ch, enforcer_spells, 0, NULL );
}

bool spec_fight_invader( CHAR_DATA *ch )
{
    return spec_fight_cast( ch, invader_spells, 15, "shadow cloak" );
}

bool spec_fight_ivan( CHAR_DATA *ch )
{
    return spec_fight_cast( ch, ivan_spells, 10, "garble" );
}

bool spec_fight_seneschal( CHAR_DATA *ch )
{
    return spec_fight_cast( ch, seneschal_spells, 0, NULL );
}

bool spec_fight_powerman( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING )
        return FALSE;

    ch->cabal = CABAL_BATTLE;

    if ( !is_affected(ch, gsn_spellbane))
        do_spellbane(ch, "");

    if ( ( victim = fighting_victim( ch, 1 ) ) == NULL )
        return FALSE;

    if ( number_percent() < 33)
    {
        act("Ölümcül üçlü saldırıyı yapıyorsun!",ch,NULL,NULL,TO_CHAR);
        act("$n ölümcül üçlü saldırıyı yapıyor!",ch,NULL,NULL,TO_ROOM);
        one_hit( ch, victim, TYPE_UNDEFINED, FALSE );
        one_hit( ch, victim, TYPE_UNDEFINED, FALSE );
        one_hit( ch, victim, TYPE_UNDEFINED, FALSE );
    }

    if ( !is_affected(ch, gsn_resistance))
        do_resistance(ch, "");

    if ( ch->hit < (ch->max_hit /3) && !IS_AFFECTED(ch, AFF_REGENERATION))
        do_bandage(ch, "");

    return TRUE;
}

bool spec_fight_protector( CHAR_DATA *ch )
{
    return spec_fight_cast( ch, enforcer_spells, 0, NULL );
}

bool spec_fight_lionguard( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ( victim = fighting_victim( ch, 1 ) ) == NULL )
        return FALSE;

    if ( number_percent() < 33)
    {
        int damage_claw;

        damage_claw = dice(ch->level, 24) + ch->damroll;
        damage(ch, victim, damage_claw, gsn_claw, DAM_BASH, TRUE);
        return TRUE;
    }

    return cast_at( ch, victim, lionguard_spells[dice(1,16) - 1], TRUE );
}

bool spec_fight_hunter( CHAR_DATA *ch )
{
    return spec_fight_cast( ch, hunter_spells, 0, NULL );
}

bool spec_wishmaster( CHAR_DATA *ch )
{
    int i;

    if ( !IS_AWAKE(ch) )
        return FALSE;

    i=number_range(0,150);

    switch(i)
    {
        case 0:
            do_say(ch, "Bir dilek tutmak istemez misin? Öyleyse dilek listeme bakmalısın.");
            return TRUE;
        case 1:
            do_say(ch, "Uygun bir ücrete harika bir dilek dilemek istemez misin? Listeme bakmalısın.");
            return TRUE;
    }
    return FALSE;
}

bool spec_kameni_dindar( CHAR_DATA *ch )
{
    return dindar_says( ch, "Kame'nin", "Yeraltı'nda" );
}

bool spec_niryani_dindar( CHAR_DATA *ch )
{
    return dindar_says( ch, "Nir'in", "Eski Thalos'ta" );
}

bool spec_nyahi_dindar( CHAR_DATA *ch )
{
    return dindar_says( ch, "Nyah'ın", "Bölümüş Ruhlar'da" );
}

bool spec_sintaryan_dindar( CHAR_DATA *ch )
{
    return dindar_says( ch, "Sint'in", "Haon Dor'da" );
}
