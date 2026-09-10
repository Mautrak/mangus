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

/**************************************************************************r
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
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "utf8.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_return	);
DECLARE_DO_FUN(do_wake		);
DECLARE_DO_FUN(do_raffects	);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_track		);

/*
 * Local functions.
 */
void	affect_modify	( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd );
ROOM_INDEX_DATA *	find_location	( CHAR_DATA *ch, char *arg );

/* reset_char sırasında ağır silah düşürme denetimi kapalı tutulur */
static bool reset_in_progress;

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
	return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
	if (fch->on == obj)
	    count++;

    return count;
}

/*
 * Ad tablolarında ortak arama: tablonun her satırı `stride` bayt, ad(lar)
 * satırın `name_off` ofsetinde ardışık `nnames` işaretçidir. Son adın NULL
 * olduğu satır tablonun sonudur; `max` >= 0 ise en çok o kadar satır gezilir.
 * Eşleşen satırın indeksi, yoksa -1 döner.
 */
static int name_table_lookup(const char *name, const void *table, size_t stride,
			     size_t name_off, int nnames, int max)
{
    int i, k;

    for (i = 0; max < 0 || i < max; i++)
    {
	const char *const *names =
	    (const char *const *) ((const char *) table + i * stride + name_off);

	if (names[nnames - 1] == NULL)
	    return -1;
	for (k = 0; k < nnames; k++)
	    if (utf8_first_eq(name, names[k]) && !str_prefix(name, names[k]))
		return i;
    }
    return -1;
}

#define TABLE_LOOKUP(name, tbl, field, nnames, max) \
    name_table_lookup((name), (tbl), sizeof (tbl)[0], \
		      offsetof(__typeof__((tbl)[0]), field), (nnames), (max))

/* returns race number */
int race_lookup (const char *name)
{
    int race = TABLE_LOOKUP(name, race_table, name, 2, -1);

    if (race < 0)
    {
	bugf("Race_lookup: race not found %s.", name);
	return 0;
    }
    return race;
}

int liq_lookup (const char *name)
{
    int liq = TABLE_LOOKUP(name, liq_table, liq_name, 1, -1);

    return liq < 0 ? LIQ_WATER : liq;
}

int weapon_lookup (const char *name)
{
    return TABLE_LOOKUP(name, weapon_table, name, 1, -1);
}

/* Yönelim dizini: hometown_table altar/recall/pit dizileri için 0 iyi, 1 tarafsız, 2 kötü */
int align_index( CHAR_DATA *ch )
{
    return IS_GOOD(ch) ? 0 : IS_EVIL(ch) ? 2 : 1;
}

/* vnum bir kabal eşyasıysa kabalın indeksi (1..MAX_CABAL-1), değilse 0 */
int cabal_obj_index( int vnum )
{
    int i;

    for ( i = 1; i < MAX_CABAL; i++ )
	if ( cabal_table[i].obj_vnum == vnum )
	    return i;
    return 0;
}

/* vnum bir kabalın sunak odasıysa kabalın indeksi, değilse 0 */
int cabal_room_index( int vnum )
{
    int i;

    for ( i = 1; i < MAX_CABAL; i++ )
	if ( cabal_table[i].room_vnum == vnum )
	    return i;
    return 0;
}

/*
 * Belirtilen büyü/yetenek için ch'nin cabal durumu OK midir?
 */
bool cabal_ok(CHAR_DATA *ch, sh_int sn)
{
  int i;

  // ch bir NPC
  if (IS_NPC(ch))
    return TRUE;

  // belirtilen yetenek/büyü kabal bağımlı değil.
  if (skill_table[sn].cabal == CABAL_NONE)
    return TRUE;

  // kabal eşyasının tablodaki pointer'ı henüz boş.
  // kabal eşyası extract edilmiş olabilir.
  if (cabal_table[ch->cabal].obj_ptr == NULL)
    return TRUE;

  // Kabal eşyası bir odaya bırakılmamış.
  // Alan kişi envanterinde taşıyor olabilir.
  if (cabal_table[ch->cabal].obj_ptr->in_room == NULL )
    return TRUE;

  // Kabal eşyası olması gereken odada duruyor.
  if (cabal_table[ch->cabal].obj_ptr->in_room->vnum == cabal_table[ch->cabal].room_vnum)
    return TRUE;

  for (i=1;i < MAX_CABAL; i++)
  {
    // Kabal eşyası başka bir kabalın hedef odasına koyulmuş.
    // Artık eşyası alınan kabalın güçleri iptal edilebilir.
    if (cabal_table[ch->cabal].obj_ptr->in_room->vnum == cabal_table[i].room_vnum)
    {
      send_to_char( "Kabal gücünün varolmadığını hissediyorsun.\n\r",ch);
      return FALSE;
    }
  }

  return TRUE;
}

int weapon_type (const char *name)
{
    int type = TABLE_LOOKUP(name, weapon_table, name, 1, -1);

    return type < 0 ? WEAPON_EXOTIC : weapon_table[type].type;
}


int item_lookup(const char *name)
{
    int type = TABLE_LOOKUP(name, item_table, name, 1, -1);

    return type < 0 ? -1 : item_table[type].type;
}

const char *item_name(int item_type)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
	if (item_type == item_table[type].type)
	    return item_table[type].name;
    return "none";
}

const char *weapon_name( int weapon_type)
{
    int type;

    for (type = 0; weapon_table[type].name != NULL; type++)
        if (weapon_type == weapon_table[type].type)
            return weapon_table[type].name;
    return "exotic";
}

void familya_check_improve(CHAR_DATA* ch,CHAR_DATA* victim)
{
  int chance;

  if (IS_NPC(ch))
  {
    return;
  }
  if(ch->pcdata->familya[victim->race] >= 100)
  {
    return;
  }
  chance = 2 * int_app[get_curr_stat(ch,STAT_INT)].learn;

  if(number_range(1,10000) > chance)
  {
    return;
  }
  ch->pcdata->familya[victim->race]++;
  printf_to_char(ch,"{g%s ırkına ilişkin irfanın artıyor!{x\n\r",race_table[victim->race].name[1]);
  return;
}

/*
 * Check the material
 */
bool check_material( OBJ_DATA *obj, char *material )
{
    if ( strstr( obj->material, material ) != NULL )
	return TRUE;
    else
        return FALSE;

}

/* obj->material listedeki adlardan birini içeriyor mu? (check_material gibi alt dizgi) */
static bool material_in(OBJ_DATA *obj, const char *const *list)
{
    for (; *list != NULL; list++)
	if (check_material(obj, (char *) *list))
	    return TRUE;
    return FALSE;
}

static const char *const metal_materials[] =
{
    "silver", "gold", "iron", "mithril", "adamantite", "steel", "lead",
    "bronze", "copper", "brass", "platinum", "titanium", "aluminum", NULL
};

static const char *const floating_materials[] =
{
    "wood", "ebony", "ice", "energy", "hardwood", "softwood", "flesh",
    "silk", "wool", "cloth", "fur", "water", "oak", NULL
};

static const char *const sinking_materials[] =
{
    "steel", "iron", "brass", "silver", "gold", "ivory", "copper", "diamond",
    "pearl", "gem", "platinum", "ruby", "bronze", "titanium", "mithril",
    "obsidian", "lead", NULL
};

bool is_metal( OBJ_DATA *obj )
{
    return material_in(obj, metal_materials);
}

bool may_float( OBJ_DATA *obj )
{
    return material_in(obj, floating_materials) || obj->item_type == ITEM_BOAT;
}

bool cant_float( OBJ_DATA *obj )
{
    return material_in(obj, sinking_materials);
}

int floating_time( OBJ_DATA *obj )
{
    static const struct { sh_int item_type, ftime; } float_time_table[] =
    {
	{ ITEM_KEY, 1 },	{ ITEM_MAYMUNCUK, 1 },	{ ITEM_ARMOR, 2 },
	{ ITEM_TREASURE, 2 },	{ ITEM_PILL, 2 },	{ ITEM_POTION, 3 },
	{ ITEM_TRASH, 3 },	{ ITEM_FOOD, 4 },	{ ITEM_CONTAINER, 5 },
	{ ITEM_CORPSE_NPC, 10 },{ ITEM_CORPSE_PC, 10 },	{ -1, 0 }
    };
    int i, ftime = 0;

    for (i = 0; float_time_table[i].item_type != -1; i++)
	if (float_time_table[i].item_type == obj->item_type)
	{
	    ftime = float_time_table[i].ftime;
	    break;
	}

    ftime = number_fuzzy( ftime ) ;

    return ( ftime < 0 ? 0 : ftime);
}

int attack_lookup  (const char *name)
{
    int att = TABLE_LOOKUP(name, attack_table, name, 1, -1);

    return att < 0 ? 0 : att;
}

/* wiznet_table indeksini döndürür (bayrak: wiznet_table[i].flag), yoksa -1 */
long wiznet_lookup (const char *name)
{
    return TABLE_LOOKUP(name, wiznet_table, name, 1, -1);
}

/* returns class number */
int class_lookup (const char *name)
{
    return TABLE_LOOKUP(name, class_table, name, 2, MAX_CLASS);
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
    /* DAM_* -> IMM_* eşlemesi; 0 = tabloda yok (DAM_LIGHT_V, DAM_TRAP_ROOM...) */
    static const long dam_to_imm[] =
    {
	[DAM_BASH] = IMM_BASH,		[DAM_PIERCE] = IMM_PIERCE,
	[DAM_SLASH] = IMM_SLASH,	[DAM_FIRE] = IMM_FIRE,
	[DAM_COLD] = IMM_COLD,		[DAM_LIGHTNING] = IMM_LIGHTNING,
	[DAM_ACID] = IMM_ACID,		[DAM_POISON] = IMM_POISON,
	[DAM_NEGATIVE] = IMM_NEGATIVE,	[DAM_HOLY] = IMM_HOLY,
	[DAM_ENERGY] = IMM_ENERGY,	[DAM_MENTAL] = IMM_MENTAL,
	[DAM_DISEASE] = IMM_DISEASE,	[DAM_DROWNING] = IMM_DROWNING,
	[DAM_LIGHT] = IMM_LIGHT,	[DAM_CHARM] = IMM_CHARM,
	[DAM_SOUND] = IMM_SOUND,
    };
    int immune, def;
    long bit;

    immune = -1;
    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if (dam_type <= 3)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
	    def = IS_VULNERABLE;
    }
    else /* magical attack */
    {
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
	    def = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    if (dam_type < 0 || dam_type >= (int) (sizeof dam_to_imm / sizeof dam_to_imm[0])
    ||  (bit = dam_to_imm[dam_type]) == 0)
	return def;

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,bit))
    {
	if (immune == IS_IMMUNE)
	    immune = IS_RESISTANT;
	else if (immune == IS_RESISTANT)
	    immune = IS_NORMAL;
	else
	    immune = IS_VULNERABLE;
    }

    if (!IS_NPC(ch) && get_curr_stat(ch, STAT_CHA) < 18
		&& dam_type == DAM_CHARM)
	immune = IS_VULNERABLE;

    if (immune == -1)
	return def;
    else
      	return immune;
}

/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
    int skill;

    if (sn == -1) /* shorthand for level based skills */
    {
	skill = ch->level * 5 / 2;
    }

    else if (sn < -1 || sn >= MAX_SKILL)
    {
	bug("Bad sn %d in get_skill.",sn);
	return 0;
    }

    else if (!IS_NPC(ch))
    {
	if (ch->level < skill_table[sn].skill_level[ch->iclass])
	    skill = 0;
	else
	    skill = ch->pcdata->learned[sn];
    }

    else /* mobiles */
    {

        if (skill_table[sn].spell_fun != spell_null)
	    skill = 40 + 2 * ch->level;

	else if (sn == gsn_sneak || sn == gsn_hide)
	    skill = ch->level + 20;

        else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
 	||       (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)))
	    skill = ch->level * 2;

 	else if (sn == gsn_shield_block)
	    skill = 10 + 2 * ch->level;

	else if (sn == gsn_second_attack )
	    skill = 30 + ch->level;

	else if (sn == gsn_third_attack
	&& (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF)))
	    skill = 30 + ch->level / 2;

	else if (sn == gsn_fourth_attack && IS_SET(ch->act,ACT_WARRIOR))
	    skill = 20 + ch->level / 2;

	else if (sn == gsn_second_weapon && IS_SET(ch->act,ACT_WARRIOR))
	    skill = 30 + ch->level / 2;

	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;

 	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = 10 + 3 * ch->level;

 	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
	    skill = 10 + 3 * ch->level;

	else if (sn == gsn_disarm
	     &&  (IS_SET(ch->off_flags,OFF_DISARM)
	     ||   IS_SET(ch->act,ACT_WARRIOR)
	     ||	  IS_SET(ch->act,ACT_THIEF)))
	    skill = 20 + 3 * ch->level;

	else if (sn == gsn_grip
	     &&  (IS_SET(ch->act,ACT_WARRIOR)
	     ||	  IS_SET(ch->act,ACT_THIEF)))
	    skill = ch->level;

	else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
	    skill = 3 * ch->level;

	else if (sn == gsn_kick)
	    skill = 10 + 3 * ch->level;

	else if (sn == gsn_backstab && IS_SET(ch->act,ACT_THIEF))
	    skill = 20 + 2 * ch->level;

  	else if (sn == gsn_rescue)
	    skill = 40 + ch->level;

	else if (sn == gsn_recall)
	    skill = 40 + ch->level;

	else if (sn == gsn_sword
	||  sn == gsn_dagger
	||  sn == gsn_spear
	||  sn == gsn_mace
	||  sn == gsn_axe
	||  sn == gsn_flail
	||  sn == gsn_whip
	||  sn == gsn_polearm
	||  sn == gsn_bow
	||  sn == gsn_arrow
	||  sn == gsn_lance)
	    skill = 40 + 5 * ch->level / 2;

	else
	   skill = 0;
    }

    if (ch->daze > 0)
    {
	if (sn >= 0 && skill_table[sn].spell_fun != spell_null)
	    skill /= 2;
	else
	    skill = 2 * skill / 3;
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	skill = 9 * skill / 10;

    if ( ch->hit < (ch->max_hit * 0.6))
	skill = 9 * skill / 10;

    return URANGE(0,skill,100);
}

/* for returning weapon information */
int get_weapon_sn(CHAR_DATA *ch, bool second)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_wield_char( ch, second );

    if (wield == NULL || wield->item_type != ITEM_WEAPON)
        sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
        default :               sn = -1;                break;
        case(WEAPON_SWORD):     sn = gsn_sword;         break;
        case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
        case(WEAPON_SPEAR):     sn = gsn_spear;         break;
        case(WEAPON_MACE):      sn = gsn_mace;          break;
        case(WEAPON_AXE):       sn = gsn_axe;           break;
        case(WEAPON_FLAIL):     sn = gsn_flail;         break;
        case(WEAPON_WHIP):      sn = gsn_whip;          break;
        case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
        case(WEAPON_BOW):   	sn = gsn_bow;       	break;
        case(WEAPON_ARROW):   	sn = gsn_arrow;       	break;
        case(WEAPON_LANCE):   	sn = gsn_lance;       	break;
   }
   return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;
	else
	    skill = 40 + 5 * ch->level / 2;
    }

    else
    {
	if (sn == -1)
	    skill = UMIN(3 * ch->level,100);
	else
	    skill = ch->pcdata->learned[sn];
    }

    if ( ch->hit < (ch->max_hit * 0.6))
        skill = 9 * skill / 10;

    return URANGE(0,skill,100);
}


/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
     int loc;
     OBJ_DATA *obj, *obj_next;
     AFFECT_DATA *af;

     if (IS_NPC(ch) || ch->in_room == NULL)
	return;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	ch->pcdata->true_sex = 0;

    ch->sex		= ch->pcdata->true_sex;
    ch->max_hit 	= ch->pcdata->perm_hit;
    ch->max_mana	= ch->pcdata->perm_mana;
    ch->max_move	= ch->pcdata->perm_move;

    reset_in_progress = TRUE;
    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
	affect_modify(ch, af, TRUE);
    }

    /* now start adding back the effects */
    for ( obj = ch->carrying; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	if ((loc = obj->wear_loc) != WEAR_NONE)
	{
	   obj->wear_loc = WEAR_NONE;
	   equip_char(ch, obj, loc);
	}
    }

    reset_in_progress = FALSE;
    /* make sure sex is RIGHT! */
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = ch->pcdata->true_sex;

  if (IS_SET(ch->act,PLR_GHOST))
  {
    ch->pcdata->ghost_mode_counter = 0;
    REMOVE_BIT(ch->act,PLR_GHOST);
    printf_to_char(ch,"Ete kemiğe büründüğünü hissediyorsun. Arkanı kollamaya başlasan iyi olur!\n\r");
    act ("$n ete kemiğe bürünüyor!",ch,NULL,NULL,TO_ROOM);
    while ( ch->affected )
      affect_remove( ch, ch->affected );
    ch->affected_by	= 0;
    ch->detection	= 0;
  }

}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    if ( ch->trust != 0 && IS_SET(ch->comm,COMM_TRUE_TRUST))
	return ch->trust;

    if ( IS_NPC(ch) && ch->level >= LEVEL_HERO )
	return LEVEL_HERO - 1;
    else
	return ch->level;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
  int max;

  if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
  {
    max = 25;
  }
  else
  {
    max = get_max_train(ch,stat);
    max = UMIN(max,25);
  }

  return URANGE(3,ch->perm_stat[stat] + ch->mod_stat[stat], max);
}


/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	return 25;

    max = ( race_table[ORG_RACE(ch)].stats[stat] + class_table[ch->iclass].stats[stat]);

    return UMIN(max,25);
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return MAX_WEAR + get_curr_stat(ch,STAT_DEX) - 10 + ch->size;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	    return 10000000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return (str_app[get_curr_stat(ch,STAT_STR)].carry + ch->level * 25 ) * 5;
}



/*
 * See if a string is one of the names of an object.
 */

bool is_name( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;


    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_prefix(string,name))
		return TRUE; /* full pattern match */

	    if (!str_prefix(part,name))
		break;
	}
    }
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
        AFFECT_DATA *paf, *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected;
             paf != NULL; paf = paf->next)
        {
	    af_new = new_affect();

            af_new->next = obj->affected;
            obj->affected = af_new;

	    af_new->where	= paf->where;
            af_new->type        = UMAX(0,paf->type);
            af_new->level       = paf->level;
            af_new->duration    = paf->duration;
            af_new->location    = paf->location;
            af_new->modifier    = paf->modifier;
            af_new->bitvector   = paf->bitvector;
        }
    }
}


/*
 * Irk değişimi: `from` ırkının bayraklarını kaldırıp `to` ırkınınkileri ekler.
 */
static void race_flags_apply(CHAR_DATA *ch, int from, int to)
{
    REMOVE_BIT(ch->affected_by, race_table[from].det);
    SET_BIT(ch->affected_by, race_table[to].det);
    REMOVE_BIT(ch->affected_by, race_table[from].aff);
    SET_BIT(ch->affected_by, race_table[to].aff);
    REMOVE_BIT(ch->imm_flags, race_table[from].imm);
    SET_BIT(ch->imm_flags, race_table[to].imm);
    REMOVE_BIT(ch->res_flags, race_table[from].res);
    SET_BIT(ch->res_flags, race_table[to].res);
    REMOVE_BIT(ch->vuln_flags, race_table[from].vuln);
    SET_BIT(ch->vuln_flags, race_table[to].vuln);
    ch->form	= race_table[to].form;
    ch->parts	= race_table[to].parts;
}

/* TO_RACE etkisi: modifier yeni ırktır */
static void race_affect_apply(CHAR_DATA *ch, AFFECT_DATA *paf)
{
    RACE(ch) = paf->modifier < MAX_PC_RACE ? paf->modifier : 1;
    race_flags_apply(ch, ORG_RACE(ch), RACE(ch));
}

/* `where` alanının yazdığı bayrak alanı; TO_RACE ve bilinmeyenler için NULL */
static long *flag_field_for(CHAR_DATA *ch, int where)
{
    switch (where)
    {
    case TO_AFFECTS:	return &ch->affected_by;
    case TO_IMMUNE:	return &ch->imm_flags;
    case TO_RESIST:	return &ch->res_flags;
    case TO_ACT_FLAG:	return &ch->act;
    case TO_VULN:	return &ch->vuln_flags;
    case TO_DETECTS:	return &ch->detection;
    }
    return NULL;
}

/* Elindeki eşya gücünü aşıyorsa yere bırakır */
static void drop_if_too_heavy(CHAR_DATA *ch, int iWear)
{
    OBJ_DATA *hold = get_eq_char(ch, iWear);

    if (hold == NULL
    ||  get_obj_weight(hold) <= str_app[get_curr_stat(ch, STAT_STR)].carry)
	return;

    act( "$p objesini bırakıyorsun.", ch, hold, NULL, TO_CHAR );
    act( "$n $p objesini bırakıyor.", ch, hold, NULL, TO_ROOM );
    obj_from_char( hold );
    obj_to_room( hold, ch->in_room );
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    long *field;
    int mod,i;

    mod = paf->modifier;

    if ( paf->where == TO_RACE )
    {
	if ( fAdd )
	    race_affect_apply(ch, paf);
	else
	{
	    race_flags_apply(ch, RACE(ch), ORG_RACE(ch));
	    RACE(ch) = ORG_RACE(ch);
	}
    }
    else if ( (field = flag_field_for(ch, paf->where)) != NULL )
    {
	if ( fAdd )
	{
	    SET_BIT(*field, paf->bitvector);
	    if (paf->where == TO_AFFECTS
	    &&  IS_SET(paf->bitvector, AFF_FLYING) && !IS_NPC(ch))
		REMOVE_BIT(ch->act,PLR_CHANGED_AFF);
	}
	else
	    REMOVE_BIT(*field, paf->bitvector);
    }

    if ( !fAdd )
	mod = 0 - mod;

    switch ( paf->location )
    {
    default:
	bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]	+= mod;	break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]	+= mod;	break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]	+= mod;	break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]	+= mod;	break;
    case APPLY_CHA:	      ch->mod_stat[STAT_CHA]	+= mod; break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:	ch->played += age_to_num( mod );	break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana		+= mod;	break;
    case APPLY_HIT:           ch->max_hit		+= mod;	break;
    case APPLY_MOVE:          ch->max_move		+= mod;	break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:
        for (i = 0; i < 4; i ++)
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
    case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
    case APPLY_SIZE:		ch->size		+= mod; break;
    case APPLY_SAVES:   ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_ROD:    ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_PETRI:  ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_BREATH: ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_SPELL:  ch->saving_throw		+= mod;	break;
    case APPLY_SPELL_AFFECT:  					break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC(ch) && !ch->extracted && !reset_in_progress )
    {
	static const int hands[] = { WEAR_BOTH, WEAR_RIGHT, WEAR_LEFT };
	static bool dropping;

	if ( !dropping )
	{
	    dropping = TRUE;
	    for ( i = 0; i < (int) (sizeof hands / sizeof hands[0]); i++ )
		drop_if_too_heavy( ch, hands[i] );
	    dropping = FALSE;
	}
    }

    return;
}


/* find an effect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
    AFFECT_DATA *paf_find;

    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
        if ( paf_find->type == sn )
	return paf_find;
    }

    return NULL;
}

/* listede where/bitvector çifti eşleşen ilk etki */
static AFFECT_DATA *affect_list_find(AFFECT_DATA *list, int where, int vector)
{
    AFFECT_DATA *paf;

    for (paf = list; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	    return paf;
    return NULL;
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch,int where,int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    long *field;

    if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
	return;

    paf = affect_list_find(ch->affected, where, vector);

    for (obj = ch->carrying; paf == NULL && obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc == -1 || obj->wear_loc == WEAR_STUCK_IN)
	    continue;

	paf = affect_list_find(obj->affected, where, vector);
	if (paf == NULL && !obj->enchanted)
	    paf = affect_list_find(obj->pIndexData->affected, where, vector);
    }

    if (paf == NULL)
	return;

    if (where == TO_RACE)
    {
	if (RACE(ch) == ORG_RACE(ch))
	    race_affect_apply(ch, paf);
    }
    else if ((field = flag_field_for(ch, where)) != NULL)
	SET_BIT(*field, vector);
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    /* apply any affect vectors to the object's extra_flags */
    if (paf->bitvector)
        switch (paf->where)
        {
        case TO_OBJECT:
    	    SET_BIT(obj->extra_flags,paf->bitvector);
	    break;
        case TO_WEAPON:
	    if (obj->item_type == ITEM_WEAPON)
	        SET_BIT(obj->value[4],paf->bitvector);
	    break;
        }


    return;
}



/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    int where;
    int vector;

    if ( ch->affected == NULL )
    {
	bug( "Affect_remove: no affect.", 0 );
	return;
    }

    affect_modify( ch, paf, FALSE );
    where = paf->where;
    vector = paf->bitvector;

    if ( !LIST_UNLINK(ch->affected, paf, next) )
    {
	bug( "Affect_remove: cannot find paf.", 0 );
	return;
    }

    free_affect(paf);

    affect_check(ch,where,vector);
    return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    int where, vector;
    if ( obj->affected == NULL )
    {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the object if needed */
    if (paf->bitvector)
	switch( paf->where)
        {
        case TO_OBJECT:
            REMOVE_BIT(obj->extra_flags,paf->bitvector);
            break;
        case TO_WEAPON:
            if (obj->item_type == ITEM_WEAPON)
                REMOVE_BIT(obj->value[4],paf->bitvector);
            break;
        }

    if ( !LIST_UNLINK(obj->affected, paf, next) )
    {
        bug( "Affect_remove_object: cannot find paf.", 0 );
        return;
    }

    free_affect(paf);

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_check(obj->carried_by,where,vector);
    return;
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    int level;

    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
      level = (paf->level + paf_old->level) / 2;
	    paf->level = (sh_int) level;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}


/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( !LIST_UNLINK(ch->carrying, obj, next_content) )
	bug( "Obj_from_char: obj not in list.", 0 );

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );
    return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:	return 3 * obj->value[type];
    case WEAR_HEAD:	return 2 * obj->value[type];
    case WEAR_LEGS:	return 2 * obj->value[type];
    case WEAR_FEET:	return     obj->value[type];
    case WEAR_HANDS:	return     obj->value[type];
    case WEAR_ARMS:	return     obj->value[type];
    case WEAR_FINGER:   return     obj->value[type];
    case WEAR_NECK:	return     obj->value[type];
    case WEAR_ABOUT:	return 2 * obj->value[type];
    case WEAR_WAIST:	return     obj->value[type];
    case WEAR_WRIST:	return     obj->value[type];
    case WEAR_LEFT:	return     obj->value[type];
    case WEAR_RIGHT:	return     obj->value[type];
    case WEAR_BOTH:	return     obj->value[type];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}



/* Eşyanın anti-iyi/kötü/tarafsız bayrağı karakterin yönelimiyle çelişiyor mu? */
bool obj_zaps_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    return ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	|| ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	|| ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) );
}

/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    int i;

    if (iWear == WEAR_STUCK_IN)
    {
	obj->wear_loc = iWear;
	return;
    }

    if ( count_worn( ch, iWear ) >= max_can_wear(ch, iWear) )
    {
	bug( "Equip_char: already equipped (%d).", iWear );
	return;
    }

    if ( obj_zaps_char( obj, ch ) )
    {
		/*
		* Thanks to Morgenes for the bug fix here!
		*/
		act("$p seni çarparak elinden düşüyor.", ch, obj, NULL, TO_CHAR );
		act("$p $m çarparak elinden düşüyor.",  ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		return;
    }


    for (i = 0; i < 4; i++)
    	ch->armor[i]      	-= apply_ac( obj, iWear,i );

    if ( get_light_char(ch) == NULL && ch->in_room != NULL
	&& ( (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
            || (iWear == WEAR_HEAD && IS_OBJ_STAT(obj, ITEM_GLOW)) ) )
	++ch->in_room->light;

    obj->wear_loc	 = iWear;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location != APPLY_SPELL_AFFECT )
	        affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
    	    affect_to_char ( ch, paf );
	else
	    affect_modify( ch, paf, TRUE );

    if (IS_SET(obj->progtypes,OPROG_WEAR))
      (obj->pIndexData->oprogs->wear_prog) (obj,ch);

    return;
}



/* Eşyanın APPLY_SPELL_AFFECT etkisiyle karaktere eklenmiş büyüyü söker */
static void strip_spell_affect( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *lpaf;

    for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf->next )
    {
	if ( lpaf->type == paf->type
	&&   lpaf->level == paf->level
	&&   lpaf->location == APPLY_SPELL_AFFECT )
	{
	    affect_remove( ch, lpaf );
	    return;
	}
    }
}

/* Bir etki listesindeki tüm etkileri karakterden geri alır */
static void unapply_affect_list( CHAR_DATA *ch, AFFECT_DATA *list )
{
    AFFECT_DATA *paf;

    for ( paf = list; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_SPELL_AFFECT )
	    strip_spell_affect( ch, paf );
	else
	{
	    affect_modify( ch, paf, FALSE );
	    affect_check( ch, paf->where, paf->bitvector );
	}
    }
}

/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    int i, old_wear;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    if ( obj->wear_loc == WEAR_STUCK_IN )
    {
	obj->wear_loc = WEAR_NONE;
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]	+= apply_ac( obj, obj->wear_loc,i );
    old_wear 		 = obj->wear_loc;
    obj->wear_loc	 = -1;

    if (!obj->enchanted)
	unapply_affect_list( ch, obj->pIndexData->affected );
    unapply_affect_list( ch, obj->affected );

    if ( get_light_char(ch) == NULL && ch->in_room != NULL
	&& ( (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
            || (old_wear == WEAR_HEAD && IS_OBJ_STAT(obj, ITEM_GLOW)) )
	&&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if (IS_SET(obj->progtypes,OPROG_REMOVE))
      (obj->pIndexData->oprogs->remove_prog) (obj,ch);

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}


/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{

    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
    if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
        obj->cost = 0;

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( !LIST_UNLINK(obj_from->contains, obj, next_content) )
    {
	bug( "Obj_from_obj: obj not found.", 0 );
	return;
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj )
		* WEIGHT_MULT(obj_from) / 100;
	}
    }

    return;
}

/*
 * Extract an object consider limit
 */
void extract_obj( OBJ_DATA *obj )
{
  extract_obj_1(obj,TRUE);
}

/*
 * Extract an object consider limit
 */
void extract_obj_nocount( OBJ_DATA *obj )
{
  extract_obj_1(obj,FALSE);
}

/*
 * Extract an obj from the world.
 */
void extract_obj_1( OBJ_DATA *obj, bool count )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;
    int i;

    if (obj->extracted)  /* if the object has already been extracted once */
      {
        bugf("Warning! Extraction of %s, vnum %d.", obj->name,
                obj->pIndexData->vnum);
        return; /* if it's already been extracted, something bad is going on */
      }
    else
      obj->extracted = TRUE;  /* if it hasn't been extracted yet, now
                               * it's being extracted. */

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );

    if ( (i = cabal_obj_index(obj->pIndexData->vnum)) > 0
    &&   cabal_table[i].obj_ptr == obj )
    {
        obj->pIndexData->count--;
        cabal_table[i].obj_ptr = NULL;
    }

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj_1( obj_content, count );
    }

    if (obj->pIndexData->vnum == OBJ_VNUM_MAGIC_JAR )
	{
	 CHAR_DATA *wch;

	 for ( wch = char_list; wch != NULL ; wch = wch->next )
	  {
	   if (IS_NPC(wch)) continue;
	   if (is_name(obj->name,wch->name) )
		{
		 REMOVE_BIT(wch->act,PLR_NO_EXP);
     send_to_char("Ruhunu yakalıyorsun...\n\r",wch);
		 break;
		}
	  }
	}
    if ( !LIST_UNLINK(object_list, obj, next) )
	bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );

    if (count)
      --obj->pIndexData->count;
    free_obj(obj);
    return;
}

void extract_char( CHAR_DATA *ch, bool fPull )
{
  extract_char_org( ch, fPull, TRUE );
  return;
}

void extract_char_nocount( CHAR_DATA *ch, bool fPull )
{
  extract_char_org( ch, fPull, FALSE );
  return;
}


/*
 * Extract a char from the world.
 */
void extract_char_org( CHAR_DATA *ch, bool fPull, bool Count )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( ch->in_room == NULL )
    {
	bug( "Extract_char: NULL.", 0 );
	return;
    }

    if (fPull) /* only for total extractions should it check */
    {
	if (ch->extracted)  /* if the char has already been extracted once */
	{
	    bugf("Warning! Extraction of %s.", ch->name);
	    return; /* if it's already been extracted, something bad is going on */
	}
	ch->extracted = TRUE;  /* if it hasn't been extracted yet, now
				* it's being extracted. */
    }

    nuke_pets(ch);
    ch->pet = NULL; /* just in case */

    if ( fPull )
	die_follower( ch );

    stop_fighting( ch, TRUE );

    char_from_room( ch );
    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( Count )
  	  extract_obj( obj );
	else
	  extract_obj_nocount( obj );
    }

    char_from_room( ch );

    if ( !fPull )
    {
        char_to_room(ch, get_room_index(hometown_table[ch->hometown].altar[align_index(ch)]));
	return;
    }

    if ( IS_NPC(ch) )
	--ch->pIndexData->count;

    if ( ch->desc != NULL && ch->desc->original != NULL )
    {
	do_return( ch, "" );
	ch->desc = NULL;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch->reply == ch )
	    wch->reply = NULL;
    }

    if ( !LIST_UNLINK(char_list, ch, next) )
    {
	bug( "Extract_char: char not found.", 0 );
	return;
    }

    if ( ch->desc != NULL )
	ch->desc->character = NULL;
    free_char( ch );
    return;
}


/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    number = number_argument( argument, arg );
    if ( is_self_keyword( arg ) )
	return ch;

    /* önce oda; get_char_room2 odada bulunan sayıyı number'dan düşer */
    if ( ( wch = get_char_room2( ch, ch->in_room, arg, &number ) ) != NULL )
	return wch;

    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL || wch->in_room == ch->in_room
	||   !can_see( ch, wch ) || !is_name( arg, wch->name ) )
	    continue;

	if ( ++count == number )
	    return wch;
    }

    return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * "numara.isim" biçimli aramanın ortak çekirdeği: listede görülebilen ve adı
 * uyan `number`. eşyayı döndürür. wear_filter: OBJ_FIND_ANY hepsi,
 * OBJ_FIND_CARRIED yalnızca envanter (giyilmemiş), OBJ_FIND_WORN yalnızca giyilen.
 */
#define OBJ_FIND_ANY		0
#define OBJ_FIND_CARRIED	1
#define OBJ_FIND_WORN		2

static OBJ_DATA *find_obj_in( CHAR_DATA *ch, OBJ_DATA *list, char *arg,
			      int number, int wear_filter, bool by_next )
{
    OBJ_DATA *obj;
    int count = 0;

    for ( obj = list; obj != NULL; obj = by_next ? obj->next : obj->next_content )
    {
	if ( wear_filter == OBJ_FIND_CARRIED && obj->wear_loc != WEAR_NONE )
	    continue;
	if ( wear_filter == OBJ_FIND_WORN && obj->wear_loc == WEAR_NONE )
	    continue;
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    int number = number_argument( argument, arg );

    return find_obj_in( ch, list, arg, number, OBJ_FIND_ANY, FALSE );
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int number = number_argument( argument, arg );

    return find_obj_in( ch, ch->carrying, arg, number, OBJ_FIND_CARRIED, FALSE );
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int number = number_argument( argument, arg );

    return find_obj_in( ch, ch->carrying, arg, number, OBJ_FIND_WORN, FALSE );
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    return find_obj_in( ch, object_list, arg, number, OBJ_FIND_ANY, TRUE );
}

/* deduct cost from a character */

void deduct_cost(CHAR_DATA *ch, int cost)
{

    ch->silver -= cost;

    if (ch->silver < 0)
    {
	bug("deduct costs: silver %d < 0",ch->silver);
	ch->silver = 0;
    }
}
/*
 * Create a 'money' obj.
 */
/*
 * Alan dosyasından gelen tanım şablonunda `spec` ("%s"/"%d") yerine `value`
 * yazar; şablon asla biçim dizgisi olarak kullanılmaz.
 */
void descr_subst( char *buf, size_t size, const char *tmpl, const char *spec, const char *value )
{
    const char *p = strstr( tmpl, spec );

    if ( p == NULL )
	snprintf( buf, size, "%s", tmpl );
    else
	snprintf( buf, size, "%.*s%s%s", (int) (p - tmpl), tmpl, value, p + strlen(spec) );
}

OBJ_DATA *create_money( int silver )
{
    char buf[MAX_STRING_LENGTH];
    char num[32];
    OBJ_DATA *obj;

    if ( silver < 0 || silver == 0 )
    {
	bug( "Create_money: zero or negative money.",silver);
	silver = UMAX(1,silver);
    }

    if (silver == 1)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
    }
    else
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
	snprintf(num, sizeof(num), "%d", silver);
	descr_subst( buf, sizeof(buf), obj->short_descr, "%d", num );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[0]           = silver;
        obj->cost               = silver;
	obj->weight		= silver/20;
    }

    return obj;
}


/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;

    if ( obj->item_type == ITEM_MONEY )
	number = 0;
    else
        number = 1;

    return number;
}

int get_obj_realnumber( OBJ_DATA *obj )
{
    int number = 1;

    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );

    return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    OBJ_DATA *tobj;

    weight = obj->weight;
    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
    int weight;

    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight( obj );

    return weight;
}

/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
/* RT changed so that WIZ_INVIS has levels */
    if ( ch == NULL || victim == NULL )
    {
	bug( "Can_see: NULL argument.", 0 );
	return FALSE;
    }

    if ( ch == victim )
	return TRUE;

    if ( get_trust(ch) < victim->invis_level)
	return FALSE;


    if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
	return FALSE;

    if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
    ||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ( ch->in_room == NULL )
        return FALSE;

    if ( room_is_dark( ch ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
    &&   !CAN_DETECT(ch, DETECT_INVIS) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_IMP_INVIS)
    &&   !CAN_DETECT(ch, DETECT_IMP_INVIS) )
	return FALSE;

    if (IS_AFFECTED(victim,AFF_CAMOUFLAGE) &&
        !CAN_DETECT(ch,ACUTE_VISION))
      return FALSE;

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   !CAN_DETECT(ch, DETECT_HIDDEN)
    &&   victim->fighting == NULL)
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_FADE)
    &&   !CAN_DETECT(ch, DETECT_FADE)
    &&   victim->fighting == NULL)
	return FALSE;

    if ( CAN_DETECT(victim, ADET_EARTHFADE))
	return FALSE;
    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !CAN_DETECT(ch, DETECT_INVIS) )
        return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_BURIED) && !IS_IMMORTAL(ch) )
        return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if ( room_is_dark( ch ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

    return FALSE;
}



/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj )
{
    switch ( obj->item_type )
    {
    case ITEM_LIGHT:		return "light";
    case ITEM_SCROLL:		return "scroll";
    case ITEM_WAND:		return "wand";
    case ITEM_STAFF:		return "staff";
    case ITEM_WEAPON:		return "weapon";
    case ITEM_TREASURE:		return "treasure";
    case ITEM_ARMOR:		return "armor";
    case ITEM_CLOTHING:		return "clothing";
    case ITEM_POTION:		return "potion";
    case ITEM_FURNITURE:	return "furniture";
    case ITEM_TRASH:		return "trash";
    case ITEM_CONTAINER:	return "container";
    case ITEM_DRINK_CON:	return "drink container";
    case ITEM_KEY:		return "key";
    case ITEM_MAYMUNCUK:		return "maymuncuk";
    case ITEM_FOOD:		return "food";
    case ITEM_MONEY:		return "money";
    case ITEM_BOAT:		return "boat";
    case ITEM_CORPSE_NPC:	return "npc corpse";
    case ITEM_CORPSE_PC:	return "pc corpse";
    case ITEM_FOUNTAIN:		return "fountain";
    case ITEM_PILL:		return "pill";
    case ITEM_MAP:		return "map";
    case ITEM_PORTAL:		return "portal";
    case ITEM_WARP_STONE:	return "warp stone";
    case ITEM_GEM:		return "gem";
    case ITEM_JEWELRY:		return "jewelry";
    case ITEM_JUKEBOX:		return "juke box";
    case ITEM_TATTOO:		return "tattoo";
    }

    bug( "Item_type_name: unknown type %d.", obj->item_type );
    return "(unknown)";
}



/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_CHA:		return "charisma";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_MANA:		return "mana";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_SIZE:		return "size";
    case APPLY_SAVES:		return "saves";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PETRI:	return "save vs petrification";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_SPELL_AFFECT:	return "none";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/* Bayrak tablosundaki adları ' ' ile ayırarak buf'un sonuna ekler */
static void flag_bits_append( char *buf, size_t size, const struct flag_type *table, long bits )
{
    size_t len = strlen( buf );

    for ( ; table->name != NULL && len < size; table++ )
	if ( bits & table->bit )
	    len += snprintf( buf + len, size - len, " %s", table->name );
}

/*
 * bits içinde ayarlı bayrakların adları (boşlukla ayrılmış), hiçbiri yoksa "none".
 * *_bit_name işlevlerinin ortak çekirdeği.
 */
const char *flag_bits_name( char *buf, size_t size, const struct flag_type *table, long bits )
{
    buf[0] = '\0';
    flag_bits_append( buf, size, table, bits );
    return buf[0] != '\0' ? buf + 1 : "none";
}

#define BIT_NAME_FUNC(fname, table) \
    char *fname( int bits ) \
    { \
	static char buf[512]; \
	return (char *) flag_bits_name( buf, sizeof buf, table, bits ); \
    }

static const struct flag_type affect_bit_names[] =
{
    { "blind",		AFF_BLIND,		TRUE },
    { "invisible",	AFF_INVISIBLE,		TRUE },
    { "imp_invis",	AFF_IMP_INVIS,		TRUE },
    { "fade",		AFF_FADE,		TRUE },
    { "scream",		AFF_SCREAM,		TRUE },
    { "bloodthirst",	AFF_BLOODTHIRST,	TRUE },
    { "stun",		AFF_STUN,		TRUE },
    { "sanctuary",	AFF_SANCTUARY,		TRUE },
    { "faerie_fire",	AFF_FAERIE_FIRE,	TRUE },
    { "infrared",	AFF_INFRARED,		TRUE },
    { "curse",		AFF_CURSE,		TRUE },
    { "poison",		AFF_POISON,		TRUE },
    { "prot_evil",	AFF_PROTECT_EVIL,	TRUE },
    { "prot_good",	AFF_PROTECT_GOOD,	TRUE },
    { "sleep",		AFF_SLEEP,		TRUE },
    { "sneak",		AFF_SNEAK,		TRUE },
    { "hide",		AFF_HIDE,		TRUE },
    { "charm",		AFF_CHARM,		TRUE },
    { "flying",		AFF_FLYING,		TRUE },
    { "pass_door",	AFF_PASS_DOOR,		TRUE },
    { "berserk",	AFF_BERSERK,		TRUE },
    { "calm",		AFF_CALM,		TRUE },
    { "haste",		AFF_HASTE,		TRUE },
    { "slow",		AFF_SLOW,		TRUE },
    { "weaken",		AFF_WEAKEN,		TRUE },
    { "plague",		AFF_PLAGUE,		TRUE },
    { "regeneration",	AFF_REGENERATION,	TRUE },
    { "camouflage",	AFF_CAMOUFLAGE,		TRUE },
    { "swim",		AFF_SWIM,		TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type detect_bit_names[] =
{
    { "detect_imp_inv",	DETECT_IMP_INVIS,	TRUE },
    { "detect_evil",	DETECT_EVIL,		TRUE },
    { "detect_good",	DETECT_GOOD,		TRUE },
    { "detect_invis",	DETECT_INVIS,		TRUE },
    { "detect_magic",	DETECT_MAGIC,		TRUE },
    { "detect_hidden",	DETECT_HIDDEN,		TRUE },
    { "dark_vision",	DARK_VISION,		TRUE },
    { "acute_vision",	ACUTE_VISION,		TRUE },
    { "fear",		ADET_FEAR,		TRUE },
    { "form_tree",	ADET_FORM_TREE,		TRUE },
    { "form_grass",	ADET_FORM_GRASS,	TRUE },
    { "web",		ADET_WEB,		TRUE },
    { "life",		DETECT_LIFE,		TRUE },
    { "detect_sneak",	DETECT_SNEAK,		TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type extra_bit_names[] =
{
    { "glow",		ITEM_GLOW,		TRUE },
    { "hum",		ITEM_HUM,		TRUE },
    { "dark",		ITEM_DARK,		TRUE },
    { "lock",		ITEM_LOCK,		TRUE },
    { "evil",		ITEM_EVIL,		TRUE },
    { "invis",		ITEM_INVIS,		TRUE },
    { "magic",		ITEM_MAGIC,		TRUE },
    { "nodrop",		ITEM_NODROP,		TRUE },
    { "bless",		ITEM_BLESS,		TRUE },
    { "anti-good",	ITEM_ANTI_GOOD,		TRUE },
    { "anti-evil",	ITEM_ANTI_EVIL,		TRUE },
    { "anti-neutral",	ITEM_ANTI_NEUTRAL,	TRUE },
    { "noremove",	ITEM_NOREMOVE,		TRUE },
    { "inventory",	ITEM_INVENTORY,		TRUE },
    { "nopurge",	ITEM_NOPURGE,		TRUE },
    { "vis_death",	ITEM_VIS_DEATH,		TRUE },
    { "rot_death",	ITEM_ROT_DEATH,		TRUE },
    { "no_locate",	ITEM_NOLOCATE,		TRUE },
    { "sell_extract",	ITEM_SELL_EXTRACT,	TRUE },
    { "burn_proof",	ITEM_BURN_PROOF,	TRUE },
    { "no_uncurse",	ITEM_NOUNCURSE,		TRUE },
    { "buried",		ITEM_BURIED,		TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type npc_act_names[] =
{
    { "npc",		ACT_IS_NPC,		TRUE },
    { "sentinel",	ACT_SENTINEL,		TRUE },
    { "scavenger",	ACT_SCAVENGER,		TRUE },
    { "aggressive",	ACT_AGGRESSIVE,		TRUE },
    { "stay_area",	ACT_STAY_AREA,		TRUE },
    { "wimpy",		ACT_WIMPY,		TRUE },
    { "pet",		ACT_PET,		TRUE },
    { "train",		ACT_TRAIN,		TRUE },
    { "practice",	ACT_PRACTICE,		TRUE },
    { "undead",		ACT_UNDEAD,		TRUE },
    { "hunter",		ACT_HUNTER,		TRUE },
    { "cleric",		ACT_CLERIC,		TRUE },
    { "mage",		ACT_MAGE,		TRUE },
    { "thief",		ACT_THIEF,		TRUE },
    { "warrior",	ACT_WARRIOR,		TRUE },
    { "no_align",	ACT_NOALIGN,		TRUE },
    { "no_purge",	ACT_NOPURGE,		TRUE },
    { "healer",		ACT_IS_HEALER,		TRUE },
    { "skill_train",	ACT_GAIN,		TRUE },
    { "update_always",	ACT_UPDATE_ALWAYS,	TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type plr_act_names[] =
{
    { "autoassist",	PLR_AUTOASSIST,		TRUE },
    { "autoexit",	PLR_AUTOEXIT,		TRUE },
    { "autoloot",	PLR_AUTOLOOT,		TRUE },
    { "autosac",	PLR_AUTOSAC,		TRUE },
    { "autoakce",	PLR_AUTOAKCE,		TRUE },
    { "autosplit",	PLR_AUTOSPLIT,		TRUE },
    { "wanted",		PLR_WANTED,		TRUE },
    { "no_title",	PLR_NO_TITLE,		TRUE },
    { "no_exp",		PLR_NO_EXP,		TRUE },
    { "holy_light",	PLR_HOLYLIGHT,		TRUE },
    { "no_cancel",	PLR_NOCANCEL,		TRUE },
    { "loot_corpse",	PLR_CANLOOT,		TRUE },
    { "no_summon",	PLR_NOSUMMON,		TRUE },
    { "no_follow",	PLR_NOFOLLOW,		TRUE },
    { "Cabal_LEADER",	PLR_CANINDUCT,		TRUE },
    { "ghost",		PLR_GHOST,		TRUE },
    { "remorted",	PLR_REMORTED,		TRUE },
    { "log",		PLR_LOG,		TRUE },
    { "frozen",		PLR_FREEZE,		TRUE },
    { "lefthand",	PLR_LEFTHAND,		TRUE },
    { "questor",	PLR_QUESTOR,		TRUE },
    { "VAMPIRE",	PLR_VAMPIRE,		TRUE },
    { "harakiri",	PLR_HARA_KIRI,		TRUE },
    { "blink_on",	PLR_BLINK_ON,		TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type comm_bit_names[] =
{
    { "quiet",		COMM_QUIET,		TRUE },
    { "deaf",		COMM_DEAF,		TRUE },
    { "no_wiz",		COMM_NOWIZ,		TRUE },
    { "no_auction",	COMM_NOAUCTION,		TRUE },
    { "no_gossip",	COMM_NOGOSSIP,		TRUE },
    { "no_question",	COMM_NOQUESTION,	TRUE },
    { "no_music",	COMM_NOMUSIC,		TRUE },
    { "no_quote",	COMM_NOQUOTE,		TRUE },
    { "compact",	COMM_COMPACT,		TRUE },
    { "brief",		COMM_BRIEF,		TRUE },
    { "prompt",		COMM_PROMPT,		TRUE },
    { "combine",	COMM_COMBINE,		TRUE },
    { "no_emote",	COMM_NOEMOTE,		TRUE },
    { "no_shout",	COMM_NOSHOUT,		TRUE },
    { "no_tell",	COMM_NOTELL,		TRUE },
    { "no_channels",	COMM_NOCHANNELS,	TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type imm_bit_names[] =
{
    { "summon",		IMM_SUMMON,		TRUE },
    { "charm",		IMM_CHARM,		TRUE },
    { "magic",		IMM_MAGIC,		TRUE },
    { "weapon",		IMM_WEAPON,		TRUE },
    { "blunt",		IMM_BASH,		TRUE },
    { "piercing",	IMM_PIERCE,		TRUE },
    { "slashing",	IMM_SLASH,		TRUE },
    { "fire",		IMM_FIRE,		TRUE },
    { "cold",		IMM_COLD,		TRUE },
    { "lightning",	IMM_LIGHTNING,		TRUE },
    { "acid",		IMM_ACID,		TRUE },
    { "poison",		IMM_POISON,		TRUE },
    { "negative",	IMM_NEGATIVE,		TRUE },
    { "holy",		IMM_HOLY,		TRUE },
    { "energy",		IMM_ENERGY,		TRUE },
    { "mental",		IMM_MENTAL,		TRUE },
    { "disease",	IMM_DISEASE,		TRUE },
    { "drowning",	IMM_DROWNING,		TRUE },
    { "light",		IMM_LIGHT,		TRUE },
    { "sound",		IMM_SOUND,		TRUE },
    { "iron",		IMM_IRON,		TRUE },
    { "wood",		IMM_WOOD,		TRUE },
    { "silver",		IMM_SILVER,		TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type wear_bit_names[] =
{
    { "take",		ITEM_TAKE,		TRUE },
    { "finger",		ITEM_WEAR_FINGER,	TRUE },
    { "neck",		ITEM_WEAR_NECK,		TRUE },
    { "torso",		ITEM_WEAR_BODY,		TRUE },
    { "head",		ITEM_WEAR_HEAD,		TRUE },
    { "legs",		ITEM_WEAR_LEGS,		TRUE },
    { "feet",		ITEM_WEAR_FEET,		TRUE },
    { "hands",		ITEM_WEAR_HANDS,	TRUE },
    { "arms",		ITEM_WEAR_ARMS,		TRUE },
    { "shield",		ITEM_WEAR_SHIELD,	TRUE },
    { "body",		ITEM_WEAR_ABOUT,	TRUE },
    { "waist",		ITEM_WEAR_WAIST,	TRUE },
    { "wrist",		ITEM_WEAR_WRIST,	TRUE },
    { "wield",		ITEM_WIELD,		TRUE },
    { "hold",		ITEM_HOLD,		TRUE },
    { "float",		ITEM_WEAR_FLOAT,	TRUE },
    { "tattoo",		ITEM_WEAR_TATTOO,	TRUE },
    { NULL, 0, FALSE }
};

/* poison ile edible birlikte yazılmaz (form_bit_name) */
static const struct flag_type form_bit_names[] =
{
    { "poison",		FORM_POISON,		TRUE },
    { "edible",		FORM_EDIBLE,		TRUE },
    { "magical",	FORM_MAGICAL,		TRUE },
    { "instant_rot",	FORM_INSTANT_DECAY,	TRUE },
    { "other",		FORM_OTHER,		TRUE },
    { "animal",		FORM_ANIMAL,		TRUE },
    { "sentient",	FORM_SENTIENT,		TRUE },
    { "undead",		FORM_UNDEAD,		TRUE },
    { "construct",	FORM_CONSTRUCT,		TRUE },
    { "mist",		FORM_MIST,		TRUE },
    { "intangible",	FORM_INTANGIBLE,	TRUE },
    { "biped",		FORM_BIPED,		TRUE },
    { "ciren",		FORM_CIREN,		TRUE },
    { "insect",		FORM_INSECT,		TRUE },
    { "spider",		FORM_SPIDER,		TRUE },
    { "crustacean",	FORM_CRUSTACEAN,	TRUE },
    { "worm",		FORM_WORM,		TRUE },
    { "blob",		FORM_BLOB,		TRUE },
    { "mammal",		FORM_MAMMAL,		TRUE },
    { "bird",		FORM_BIRD,		TRUE },
    { "reptile",	FORM_REPTILE,		TRUE },
    { "snake",		FORM_SNAKE,		TRUE },
    { "dragon",		FORM_DRAGON,		TRUE },
    { "amphibian",	FORM_AMPHIBIAN,		TRUE },
    { "fish",		FORM_FISH,		TRUE },
    { "cold_blooded",	FORM_COLD_BLOOD,	TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type part_bit_names[] =
{
    { "head",		PART_HEAD,		TRUE },
    { "arms",		PART_ARMS,		TRUE },
    { "legs",		PART_LEGS,		TRUE },
    { "heart",		PART_HEART,		TRUE },
    { "brains",		PART_BRAINS,		TRUE },
    { "guts",		PART_GUTS,		TRUE },
    { "hands",		PART_HANDS,		TRUE },
    { "feet",		PART_FEET,		TRUE },
    { "fingers",	PART_FINGERS,		TRUE },
    { "ears",		PART_EAR,		TRUE },
    { "eyes",		PART_EYE,		TRUE },
    { "long_tongue",	PART_LONG_TONGUE,	TRUE },
    { "eyestalks",	PART_EYESTALKS,		TRUE },
    { "tentacles",	PART_TENTACLES,		TRUE },
    { "fins",		PART_FINS,		TRUE },
    { "wings",		PART_WINGS,		TRUE },
    { "tail",		PART_TAIL,		TRUE },
    { "claws",		PART_CLAWS,		TRUE },
    { "fangs",		PART_FANGS,		TRUE },
    { "horns",		PART_HORNS,		TRUE },
    { "scales",		PART_SCALES,		TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type weapon_bit_names[] =
{
    { "flaming",	WEAPON_FLAMING,		TRUE },
    { "frost",		WEAPON_FROST,		TRUE },
    { "vampiric",	WEAPON_VAMPIRIC,	TRUE },
    { "sharp",		WEAPON_SHARP,		TRUE },
    { "vorpal",		WEAPON_VORPAL,		TRUE },
    { "two-handed",	WEAPON_TWO_HANDS,	TRUE },
    { "shocking",	WEAPON_SHOCKING,	TRUE },
    { "poison",		WEAPON_POISON,		TRUE },
    { "holy",		WEAPON_HOLY,		TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type cont_bit_names[] =
{
    { "closable",	CONT_CLOSEABLE,		TRUE },
    { "pickproof",	CONT_PICKPROOF,		TRUE },
    { "closed",		CONT_CLOSED,		TRUE },
    { "locked",		CONT_LOCKED,		TRUE },
    { NULL, 0, FALSE }
};

static const struct flag_type off_bit_names[] =
{
    { "area attack",	OFF_AREA_ATTACK,	TRUE },
    { "backstab",	OFF_BACKSTAB,		TRUE },
    { "bash",		OFF_BASH,		TRUE },
    { "berserk",	OFF_BERSERK,		TRUE },
    { "disarm",		OFF_DISARM,		TRUE },
    { "dodge",		OFF_DODGE,		TRUE },
    { "fade",		OFF_FADE,		TRUE },
    { "fast",		OFF_FAST,		TRUE },
    { "kick",		OFF_KICK,		TRUE },
    { "kick_dirt",	OFF_KICK_DIRT,		TRUE },
    { "parry",		OFF_PARRY,		TRUE },
    { "rescue",		OFF_RESCUE,		TRUE },
    { "tail",		OFF_TAIL,		TRUE },
    { "trip",		OFF_TRIP,		TRUE },
    { "crush",		OFF_CRUSH,		TRUE },
    { "assist_all",	ASSIST_ALL,		TRUE },
    { "assist_align",	ASSIST_ALIGN,		TRUE },
    { "assist_race",	ASSIST_RACE,		TRUE },
    { "assist_players",	ASSIST_PLAYERS,		TRUE },
    { "assist_guard",	ASSIST_GUARD,		TRUE },
    { "assist_vnum",	ASSIST_VNUM,		TRUE },
    { NULL, 0, FALSE }
};

/* Return ascii name of an affect bit vector. */
BIT_NAME_FUNC( affect_bit_name, affect_bit_names )
BIT_NAME_FUNC( detect_bit_name, detect_bit_names )
BIT_NAME_FUNC( extra_bit_name,  extra_bit_names  )
BIT_NAME_FUNC( comm_bit_name,   comm_bit_names   )
BIT_NAME_FUNC( imm_bit_name,    imm_bit_names    )
BIT_NAME_FUNC( wear_bit_name,   wear_bit_names   )
BIT_NAME_FUNC( part_bit_name,   part_bit_names   )
BIT_NAME_FUNC( weapon_bit_name, weapon_bit_names )
BIT_NAME_FUNC( cont_bit_name,   cont_bit_names   )
BIT_NAME_FUNC( off_bit_name,    off_bit_names    )

/* return ascii name of an act vector: "npc ..." ya da "player ..." */
char *act_bit_name( int act_flags )
{
    static char buf[512];

    if ( IS_SET(act_flags, ACT_IS_NPC) )
	return (char *) flag_bits_name( buf, sizeof buf, npc_act_names, act_flags );

    snprintf( buf, sizeof buf, "player" );
    flag_bits_append( buf, sizeof buf, plr_act_names, act_flags );
    return buf;
}

char *form_bit_name( int form_flags )
{
    static char buf[512];

    if ( form_flags & FORM_POISON )
	form_flags &= ~FORM_EDIBLE;
    return (char *) flag_bits_name( buf, sizeof buf, form_bit_names, form_flags );
}

int cabal_lookup (const char *argument)
{
   int cabal;

   for ( cabal = 0; cabal < MAX_CABAL; cabal++)
   {
        if (utf8_first_eq(argument, cabal_table[cabal].short_name)
        &&  !str_prefix( argument,cabal_table[cabal].short_name))
            return cabal;
   }

   return -1;
}


int isn_dark_safe( CHAR_DATA *ch)
{
    CHAR_DATA *rch;
    OBJ_DATA *light;
    int light_exist;

    if (!IS_VAMPIRE(ch))  return 0;

    if ( IS_SET(ch->in_room->room_flags, ROOM_DARK) )
	return 0;

    if ( weather_info.sunlight == SUN_LIGHT
    ||   weather_info.sunlight == SUN_RISE )
	return 2;

    light_exist = 0;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( (light = get_light_char(rch)) != NULL
	      && light->item_type == ITEM_LIGHT
	      && IS_OBJ_STAT(light,ITEM_MAGIC) )
	 {
	  light_exist = 1;
	  break;
	 }
    }

    return light_exist;
}


int ch_skill_nok_nomessage( CHAR_DATA *ch , int skill )
{
 if ( IS_NPC(ch) && RACE_OK(ch,skill) && CABAL_OK(ch,skill))
	return 0;
 if ( !IS_NPC(ch) && CLEVEL_OK(ch,skill) && RACE_OK(ch,skill) &&
	CABAL_OK(ch,skill) && ALIGN_OK(ch,skill)) return 0;
 return 1;
}

int ch_skill_nok( CHAR_DATA *ch, int skill )
{
   if (ch_skill_nok_nomessage(ch,skill))
   {
     send_to_char("Hı?\n\r",ch);
	 return 1;
   }
   return 0;
}

int affect_check_obj(CHAR_DATA *ch,int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if (vector == 0) 	return 0;

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc == -1 || obj->wear_loc == WEAR_STUCK_IN)
	    continue;

        for (paf = obj->affected; paf != NULL; paf = paf->next)
	    {
            if ( paf->bitvector == vector)
                return 1;
            }

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	    {
            if ( paf->bitvector == vector)
		return 1;
            }
    }
  return 0;
}

int count_charmed( CHAR_DATA *ch )
{
  CHAR_DATA *gch;
  int count = 0;

  for (gch = char_list; gch != NULL; gch = gch->next)
  {
      if ( IS_AFFECTED(gch,AFF_CHARM)
		&& gch->master == ch
		&& ch->pet != gch )
	  count++;
  }

  if (count >= MAX_CHARM(ch) )
  {
    send_to_char("Emrindekilerden fazlasını kontrol etmeye gücün yetmez!\n\r",ch);
    return count;
  }

  return 0;
}

void add_mind(CHAR_DATA *ch, char *str)
{
 char buf[MAX_STRING_LENGTH];

 if (!IS_NPC(ch) || ch->in_room == NULL) return;

 if (ch->in_mind == NULL)
  {
   snprintf(buf, sizeof(buf),"%d",ch->in_room->vnum);
   ch->in_mind = str_dup(buf);
  }
 if (!is_name(str,ch->in_mind))
  {
    snprintf(buf, sizeof(buf),"%s %s",ch->in_mind,str);
    free_string(ch->in_mind); ch->in_mind = str_dup(buf);
  }

}
void remove_mind(CHAR_DATA *ch, char *str)
{
 char buf[MAX_STRING_LENGTH];
 char buff[MAX_STRING_LENGTH];
 char arg[MAX_INPUT_LENGTH];
 char *mind = ch->in_mind;

 if (!IS_NPC(ch) || ch->in_room == NULL
	|| mind == NULL || !is_name(str,mind) ) return;

 buf[0] = '\0';
 do
  {
   mind = one_argument(mind,arg);
   if (!is_name(str,arg))
   {
    if (buf[0] == '\0') strcpy(buff,arg);
    else snprintf(buff, sizeof(buff),"%s %s",buf,arg);
    strcpy(buf,buff);
   }
  }
 while ( mind[0] != '\0' );

 do_say(ch,"Sonunda intikamımı aldım!");
 free_string(ch->in_mind);
 ch->in_mind = str_dup( buf );
 if (is_number(buf)) back_home(ch);
}

int opposite_door(int door)
{
  int opdoor;

  switch ( door )
   {
    case 0: opdoor=2;	break;
    case 1: opdoor=3;	break;
    case 2: opdoor=0;	break;
    case 3: opdoor=1;	break;
    case 4: opdoor=5;	break;
    case 5: opdoor=4;	break;
    default: opdoor=-1; break;
  }
  return opdoor;
}
void back_home( CHAR_DATA *ch )
{
 ROOM_INDEX_DATA *location;
 char arg[MAX_INPUT_LENGTH];

 if (!IS_NPC(ch) || ch->in_mind == NULL) return;

 one_argument(ch->in_mind,arg);
 if ( ( location = find_location( ch, arg ) ) == NULL )
  {
   bug("Mob cannot return to reset place",0);
   return;
  }

 if ( ch->fighting == NULL && location != ch->in_room )
  {
    act("$n nakil için dua ediyor.",ch,NULL,NULL,TO_ROOM);
   char_from_room( ch );
   char_to_room( ch, location );
   act("$n odada beliriyor.",ch,NULL,NULL,TO_ROOM);
   if (is_number(ch->in_mind))
   {
	free_string(ch->in_mind);
	ch->in_mind = NULL;
   }
 }
}

CHAR_DATA * find_char( CHAR_DATA *ch, char *argument,int door, int range )
{
 EXIT_DATA *pExit,*bExit;
 ROOM_INDEX_DATA *dest_room,*back_room;
 CHAR_DATA *target;
 int number = 0,opdoor;
 char arg[MAX_INPUT_LENGTH];

 number = number_argument(argument,arg);
 dest_room = ch->in_room;
 if ( (target = get_char_room2(ch,dest_room,arg,&number)) != NULL)
	return target;

 if ( (opdoor = opposite_door( door )) == -1)
  {
   bug("In find_char wrong door: %d",door);
   send_to_char("Onu orada görmüyorsun.\n\r",ch);
   return NULL;
 }
 while (range > 0)
 {
  range--;
  /* find target room */
  back_room = dest_room;
  if ( (pExit = dest_room->exit[door]) == NULL
      || (dest_room = pExit->u1.to_room) == NULL
      || IS_SET(pExit->exit_info,EX_CLOSED) )
   break;
  if ( (bExit = dest_room->exit[opdoor]) == NULL
      || bExit->u1.to_room != back_room)
   {
     send_to_char("Seçtiğin yol gücünün yoldan geçmesine izin vermiyor.\n\r",ch);
    return NULL;
   }
  if ((target = get_char_room2(ch,dest_room,arg,&number)) != NULL )
	return target;
 }

 send_to_char("Onu orada görmüyorsun.\n\r",ch);
 return NULL;
}

int check_exit( char *arg )
{
    int door = -1;

    if ( !str_cmp( arg, "k" ) || !str_cmp( arg, "kuzey" ) ) door = 0;
     else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "doğu"  ) ) door = 1;
     else if ( !str_cmp( arg, "g" ) || !str_cmp( arg, "güney" ) ) door = 2;
     else if ( !str_cmp( arg, "b" ) || !str_cmp( arg, "batı"  ) ) door = 3;
     else if ( !str_cmp( arg, "y" ) || !str_cmp( arg, "yukarı"    ) ) door = 4;
     else if ( !str_cmp( arg, "a" ) || !str_cmp( arg, "aşağı"  ) ) door = 5;

    return door;
}

/*
 * Find a char for spell usage.
 */
CHAR_DATA *get_char_spell( CHAR_DATA *ch, char *argument, int *door, int range )
{
 char buf[MAX_INPUT_LENGTH];
 int i;

 for(i=0;argument[i] != '\0' && argument[i] != '.';i++)
   buf[i] = argument[i];
 buf[i] = '\0';

 if (i == 0 || (*door = check_exit(buf)) == -1)
	return get_char_room(ch,argument);

 return find_char(ch,(argument+i+1),*door,range);
}

void path_to_track( CHAR_DATA *ch, CHAR_DATA *victim, int door)
{
  ROOM_INDEX_DATA *temp;
  EXIT_DATA *pExit;
  int opdoor;
  int range = 0;
  int i;

  ch->last_fight_time = current_time;
  if (!IS_NPC(victim)) victim->last_fight_time = current_time;

  if (IS_NPC(victim) && victim->position != POS_DEAD)
   {
    victim->last_fought = ch;

    if ( (opdoor = opposite_door( door )) == -1)
	{
	 bug("In path_to_track wrong door: %d",door);
	 return;
	}
    temp = ch->in_room;
    for(i=0; i< 1000; i++)
     {
      range++;
      if ( victim->in_room == temp ) break;
      if ((pExit = temp->exit[ door ]) == NULL
	  || (temp = pExit->u1.to_room) == NULL)
       {
	bug("In path_to_track: couldn't calculate range %d",range);
	return;
       }
      if ( range > 100)
       {
	bug("In path_to_track: range exceeded 100",0);
	return;
       }
     }

    temp = victim->in_room;
    while (--range > 0)
       {
        room_record(ch->name,temp, opdoor);
        if ((pExit = temp->exit[opdoor]) == NULL
	    || (temp = pExit->u1.to_room) == NULL )
	{
	 snprintf(log_buf, sizeof(log_buf),"Path to track: Range: %d Room: %d opdoor:%d",
		range,temp->vnum,opdoor);
	 bug(log_buf,0);
	 return;
	}
       }
    do_track(victim,"");
  }
 return;
}



/* new staff */

OBJ_DATA *get_wield_char( CHAR_DATA *ch, bool second )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_WEAPON )
	{
	 if (second)
	 {
	   if ((obj->wear_loc == WEAR_RIGHT && LEFT_HANDER(ch))
		|| (obj->wear_loc == WEAR_LEFT && RIGHT_HANDER(ch)))
	   return obj;
	 }
	 else
	 {
	   if ((obj->wear_loc == WEAR_RIGHT && RIGHT_HANDER(ch))
		|| (obj->wear_loc == WEAR_LEFT && LEFT_HANDER(ch))
		|| obj->wear_loc == WEAR_BOTH)
	   return obj;
	 }
	}
    }

    return NULL;
}


OBJ_DATA *get_shield_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( (obj->wear_loc == WEAR_LEFT || obj->wear_loc == WEAR_RIGHT ||
	     obj->wear_loc == WEAR_BOTH) && CAN_WEAR(obj,ITEM_WEAR_SHIELD))
	   return obj;
    }

    return NULL;
}


OBJ_DATA *get_hold_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( (obj->wear_loc == WEAR_LEFT || obj->wear_loc == WEAR_RIGHT ||
	     obj->wear_loc == WEAR_BOTH) && CAN_WEAR(obj,ITEM_HOLD))
	   return obj;
    }

    return NULL;
}


OBJ_DATA *get_light_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( (obj->item_type == ITEM_LIGHT
	     && obj->value[2] != 0
	     && ( obj->wear_loc == WEAR_LEFT
		|| obj->wear_loc == WEAR_RIGHT
		|| obj->wear_loc == WEAR_BOTH ))
	  || ( obj->wear_loc == WEAR_HEAD
	     && IS_OBJ_STAT(obj, ITEM_GLOW)) )

	   return obj;
    }

    return NULL;
}


bool is_wielded_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    OBJ_DATA *w;

    if (ch == NULL)
	return FALSE;

    for ( w = ch->carrying; w != NULL; w = w->next_content )
    {
	if ( (w->wear_loc == WEAR_LEFT || w->wear_loc == WEAR_RIGHT ||
	      w->wear_loc == WEAR_BOTH) && CAN_WEAR(w,ITEM_WIELD)
	      && w == obj)
	   return TRUE;
    }

    return FALSE;
}


bool is_equiped_n_char( CHAR_DATA *ch, int vnum, int iWear )
{
    OBJ_DATA *e;

    if (ch == NULL)
	return FALSE;

    for ( e = ch->carrying; e != NULL; e = e->next_content )
    {
	if ( e->wear_loc == iWear && e->pIndexData->vnum == vnum )
	   return TRUE;
    }

    return FALSE;
}


bool is_equiped_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    OBJ_DATA *e;

    if (ch == NULL)
	return FALSE;

    for ( e = ch->carrying; e != NULL; e = e->next_content )
    {
	if ( e->wear_loc == iWear && e == obj )
	   return TRUE;
    }

    return FALSE;
}



int count_worn( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;
    int count;

    if (ch == NULL)
	return FALSE;

    count = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	if ( obj->wear_loc == iWear)	count++;

    return count;
}



int max_can_wear( CHAR_DATA *ch, int i )
{
    if (IS_NPC(ch) || !IS_SET(ch->act, PLR_REMORTED))
	return ( i == WEAR_FINGER ? MAX_FINGER : \
                                  i == WEAR_STUCK_IN ? MAX_STUCK_IN : \
                                  i == WEAR_WRIST ? MAX_WRIST : \
                                  i == WEAR_TATTOO ? MAX_TATTOO : \
                                  i == WEAR_NECK ? MAX_NECK : 1 );
    else
	return ( i == WEAR_FINGER ? (MAX_FINGER + 2) : \
                                  i == WEAR_STUCK_IN ? MAX_STUCK_IN : \
                                  i == WEAR_WRIST ? MAX_WRIST : \
                                  i == WEAR_TATTOO ? MAX_TATTOO : \
                                  i == WEAR_NECK ? MAX_NECK : 1 );
}

int get_total_played( CHAR_DATA *ch )
{
    int l;
    int sum=0;

    if (IS_NPC(ch)) return 0;

    for( l=0; l<MAX_TIME_LOG; l++)
	sum += ch->pcdata->log_time[l];

    return (sum + parse_time_spec(current_time - ch->logon));
}

int check_time_sync( )
{
    char *strtime;
    char mytime[MAX_INPUT_LENGTH];
    char tword[MAX_INPUT_LENGTH];
    int thour, tmin;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';

    strtime = one_argument( strtime, tword);
    strtime = one_argument( strtime, tword);
    strtime = one_argument( strtime, tword);
    strtime = one_argument( strtime, mytime);

    tword[0] = mytime[0];
    tword[1] = mytime[1];
    tword[2] = '\0';
    thour = atoi( tword );

    tword[0] = mytime[3];
    tword[1] = mytime[4];
    tword[2] = '\0';
    tmin = atoi( tword );

    if (thour == 23 && tmin > 54)
	return (59 - tmin);
    return 0;
}

int parse_date( time_t t)
{
    char *strtime;
    char day[MAX_INPUT_LENGTH];
    char mon[MAX_INPUT_LENGTH];
    int d;

    strtime                    = ctime( &t );
    strtime[strlen(strtime)-1] = '\0';

    strtime = one_argument( strtime, mon);	/* temporary */
    strtime = one_argument( strtime, mon);
    strtime = one_argument( strtime, day);
    d = atoi( day );

    if (!str_cmp(mon, "Jan"))	;
    else if (!str_cmp(mon, "Feb"))
    {
	d += 31;
        if ( d > 59 ) d = 59;	/* discard 29th of February */
    }
    else if (!str_cmp(mon, "Mar"))	d += 59;
    else if (!str_cmp(mon, "Apr"))	d += 90;
    else if (!str_cmp(mon, "May"))	d += 120;
    else if (!str_cmp(mon, "Jun"))	d += 151;
    else if (!str_cmp(mon, "Jul"))	d += 181;
    else if (!str_cmp(mon, "Aug"))	d += 212;
    else if (!str_cmp(mon, "Sep"))	d += 242;
    else if (!str_cmp(mon, "Oct"))	d += 273;
    else if (!str_cmp(mon, "Nov"))	d += 303;
    else if (!str_cmp(mon, "Dec"))	d += 334;
    else return -1;

    return d;
}

int parse_time_spec( time_t t )
{
    int h, m, s;

    s = t % 60;
    m = t % 3600;
    h = ((t % 86400) - m) / 3600;
    m = (m - s) / 60;

    return ((h*60)+m);
}

int parse_time( time_t t )
{
    int h, m, s;

    s = t % 60;
    m = t % 3600;
    h = ((t % 86400) - m) / 3600;
    h = ( h + 16) % 24;
    m = (m - s) / 60;

    return ((h*60)+m);
}

bool room_has_exit( ROOM_INDEX_DATA *room )
{
  int i;

  if ( room == NULL )
    return FALSE;

  for(i=0;i<6;i++)
  {
    if (room->exit[i] != NULL)
      return TRUE;
  }

  return FALSE;
}
