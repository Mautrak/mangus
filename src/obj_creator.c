/***************************************************************************
 *                                                                         *
 * Uzak Diyarlar açık kaynak Türkçe Mud projesidir.                        *
 * Oyun geliştirmesi Jai ve Maru tarafından yönetilmektedir.               *
 * Unutulmaması gerekenler: Nir, Kame, Randalin, Nyah, Sint                *
 *                                                                         *
 * Github  : https://github.com/yelbuke/UzakDiyarlar                       *
 * Web     : http://www.uzakdiyarlar.net                                   *
 * Discord : https://discord.gg/kXyZzv                                     *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"

#define ARRAY_LEN(a) ((int)(sizeof(a) / sizeof((a)[0])))

/* Bilinmeyen malzeme için varsayılanlar (eski davranış korunur). */
#define MATERIAL_DEFAULT_INDEX	66	/* "skin" */
#define MATERIAL_DEFAULT_DECAY	240

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
		return number_range( UMAX(1,(level+5)*3/2) , UMAX(1,(level+5)*5) );
	}
	if (location == APPLY_HITROLL || location == APPLY_DAMROLL)
	{
		return number_range( UMAX(1,(level+4)/5) , UMAX(1,(level+3)/2) );
	}
	return 1;
}

int obj_random_paf_find_available_location(OBJ_DATA *obj)
{
	AFFECT_DATA *paf;
	int random_number;
	int random_location = 0;
	bool found = FALSE;
	int i = 0;

	while(found == FALSE && ++i <= 10)
	{
		random_number = number_range(1,500);
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
			switch(number_range(1,5))
			{
				case 1:		random_location = APPLY_MOVE; break;
				case 2:
				case 3:		random_location = APPLY_HIT; break;
				case 4:
				case 5:		random_location = APPLY_MANA; break;
				default:	random_location = APPLY_HIT; break;
			}
		}
		else
		{
			switch(number_range(1,2))
			{
				case 1:		random_location = APPLY_HITROLL; break;
				case 2:		random_location = APPLY_DAMROLL; break;
				default:	random_location = APPLY_HITROLL; break;
			}
		}

		found = TRUE;

		for (paf = obj->affected; paf != NULL; paf = paf->next)
		{
			if(paf->location == random_location)
			{
				random_location = 0;
				found = FALSE;
			}
		}
	}

	return random_location;
}

/*
 * Direnç / zayıflık / bağışıklık bitleri: merc.h'de RES_x == VULN_x == IMM_x
 * olduğundan tek tablo yeter. Bağışıklıkta silah ve büyü hariç tutulur.
 */
static const long elem_bits[] =
{
	IMM_SUMMON, IMM_CHARM, IMM_MAGIC, IMM_WEAPON, IMM_BASH, IMM_PIERCE,
	IMM_SLASH, IMM_FIRE, IMM_COLD, IMM_LIGHTNING, IMM_ACID, IMM_POISON,
	IMM_NEGATIVE, IMM_HOLY, IMM_ENERGY, IMM_MENTAL, IMM_DISEASE,
	IMM_DROWNING, IMM_LIGHT, IMM_SOUND, IMM_WOOD, IMM_SILVER, IMM_IRON
};

static const long imm_bits[] =
{
	IMM_SUMMON, IMM_CHARM, IMM_BASH, IMM_PIERCE,
	IMM_SLASH, IMM_FIRE, IMM_COLD, IMM_LIGHTNING, IMM_ACID, IMM_POISON,
	IMM_NEGATIVE, IMM_HOLY, IMM_ENERGY, IMM_MENTAL, IMM_DISEASE,
	IMM_DROWNING, IMM_LIGHT, IMM_SOUND, IMM_WOOD, IMM_SILVER, IMM_IRON
};

/* Eşyanın 'where' türündeki etkilerinde henüz bulunmayan rastgele bir bit; en fazla 10 deneme. */
static long pick_free_bit(OBJ_DATA *obj, int where, const long *bits, int n)
{
	AFFECT_DATA *paf;
	long bit;
	int i;

	for (i = 0; i < 10; i++)
	{
		bit = bits[number_range(0, n - 1)];
		for (paf = obj->affected; paf != NULL; paf = paf->next)
			if (paf->where == where && IS_SET(paf->bitvector, bit))
				break;
		if (paf == NULL)
			return bit;
	}

	return 0;
}

int obj_random_paf_find_available_resistance(OBJ_DATA *obj)
{
	return (int) pick_free_bit(obj, TO_RESIST, elem_bits, ARRAY_LEN(elem_bits));
}

int obj_random_paf_find_available_vulnerability(OBJ_DATA *obj)
{
	return (int) pick_free_bit(obj, TO_VULN, elem_bits, ARRAY_LEN(elem_bits));
}

int obj_random_paf_find_available_immunity(OBJ_DATA *obj)
{
	return (int) pick_free_bit(obj, TO_IMMUNE, imm_bits, ARRAY_LEN(imm_bits));
}

static void obj_add_random_affect(OBJ_DATA *obj, int where, int location, int modifier, long bits)
{
	AFFECT_DATA *paf = new_affect();

	paf->where	= where;
	paf->type	= -1;
	paf->level	= obj->level;
	paf->duration	= -1;
	paf->location	= location;
	paf->modifier	= modifier;
	paf->bitvector	= bits;
	paf->next	= obj->affected;
	obj->affected	= paf;
}

/* Rastgele etki almayan eşya türleri. */
static const int no_enchant_types[] =
{
	ITEM_TREASURE, ITEM_FURNITURE, ITEM_TRASH, ITEM_KEY, ITEM_MONEY,
	ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PROTECT, ITEM_MAP,
	ITEM_ROOM_KEY, ITEM_GEM, ITEM_JEWELRY, ITEM_JUKEBOX, ITEM_TATTOO,
	ITEM_MAYMUNCUK, ITEM_FOOD, ITEM_POTION, ITEM_DRINK_CON
};

/* Seviye üst sınırı -> etki şansı 1/N. */
static const struct { int below_level; int one_in; } enchant_chance_by_level[] =
{
	{ 10, 16 }, { 20, 14 }, { 30, 11 }, { 40, 9 },
	{ 50, 7 },  { 60, 6 },  { 70, 5 },  { INT_MAX, 4 }
};

void obj_random_paf(OBJ_DATA *obj)
{
	int location = 0, bitvector = 0;
	int random_number = 0;
	int i;

	if( !IS_SET( obj->wear_flags, ITEM_TAKE) )
		return;

	for (i = 0; i < ARRAY_LEN(no_enchant_types); i++)
		if (obj->item_type == no_enchant_types[i])
			return;

	for (i = 0; i < ARRAY_LEN(enchant_chance_by_level); i++)
		if (obj->level < enchant_chance_by_level[i].below_level)
			break;
	if (number_range(1, enchant_chance_by_level[i].one_in) != 1)
		return;

	while(number_range(1,10)<7)
	{
		obj->enchanted	= TRUE;
		random_number = number_range(1,100);
		if(random_number<=90)
		{
			location = obj_random_paf_find_available_location(obj);
			if(location == 0)
				continue;
			obj_add_random_affect(obj, TO_OBJECT, location,
				obj_random_paf_modifier(location, obj->level), 0);
		}
		else if(random_number<=92)
		{
			bitvector = obj_random_paf_find_available_vulnerability(obj);
			if(bitvector == 0)
				continue;
			obj_add_random_affect(obj, TO_VULN, 0, 0, bitvector);
		}
		else if(random_number<=99)
		{
			bitvector = obj_random_paf_find_available_resistance(obj);
			if(bitvector == 0)
				continue;
			obj_add_random_affect(obj, TO_RESIST, 0, 0, bitvector);
		}
		else
		{
			bitvector = obj_random_paf_find_available_immunity(obj);
			if(bitvector == 0)
				continue;
			obj_add_random_affect(obj, TO_IMMUNE, 0, 0, bitvector);
		}
	}
}

int obj_random_condition(void)
{
	int percent = number_percent();

	if(percent<=50)
		return 100;
	else if(percent<=70)
		return 90;
	else if(percent<=85)
		return 75;
	else if(percent<=90)
		return 50;
	else if(percent<=95)
		return 35;
	else
		return 20;
}

int obj_random_cost(int level)
{
	int min_cost = number_range(1,50);
	int max_cost = level * level * number_range(1,5);
	return UMAX( min_cost, max_cost );
}

/* Malzeme tablosunda ad araması; bulunamazsa -1. */
static int material_index_of(const char *material_name)
{
	int i;

	if (material_name == NULL)
		return -1;
	for (i = 0; material_table[i].name != NULL; i++)
		if (!strcmp(material_table[i].name, material_name))
			return i;
	return -1;
}

int find_material_index(char *material_name)
{
	int i = material_index_of(material_name);

	if (i < 0)
	{
		bugf( "Find material index: Unknown material name '%s'",
			material_name != NULL ? material_name : "(null)" );
		return MATERIAL_DEFAULT_INDEX;
	}
	return i;
}

int find_material_decay_days(OBJ_DATA *obj)
{
	int i = material_index_of(obj->material);

	if (i < 0)
	{
		bugf( "Find material decay days: Unknown material name for %d '%s'",
			obj->pIndexData->vnum, obj->material != NULL ? obj->material : "(null)" );
		return MATERIAL_DEFAULT_DECAY;
	}
	if (IS_WEAPON_STAT(obj, WEAPON_KATANA))
		return material_table[i].decay_pt * 5;
	return material_table[i].decay_pt;
}

/* Hacim tabloları (birim: malzeme ağırlık puanıyla çarpılır). */
struct volume_range { int key; int lo, hi; };

static const struct volume_range armor_volume[] =
{
	{ ITEM_WEAR_FINGER,	1,	3 },
	{ ITEM_WEAR_NECK,	10,	60 },
	{ ITEM_WEAR_BODY,	700,	1125 },
	{ ITEM_WEAR_HEAD,	175,	450 },
	{ ITEM_WEAR_LEGS,	300,	625 },
	{ ITEM_WEAR_FEET,	175,	375 },
	{ ITEM_WEAR_HANDS,	80,	250 },
	{ ITEM_WEAR_ARMS,	200,	375 },
	{ ITEM_WEAR_SHIELD,	300,	625 },
	{ ITEM_WEAR_ABOUT,	125,	500 },
	{ ITEM_WEAR_WAIST,	150,	300 },
	{ ITEM_WEAR_WRIST,	10,	60 },
	{ ITEM_HOLD,		10,	60 },
	{ ITEM_NO_SAC,		300,	625 },
	{ ITEM_WEAR_FLOAT,	20,	200 },
	{ ITEM_WEAR_TATTOO,	1,	2 }
};

static const struct volume_range weapon_volume[] =
{
	{ WEAPON_EXOTIC,	125,	250 },
	{ WEAPON_SWORD,		125,	250 },
	{ WEAPON_DAGGER,	20,	70 },
	{ WEAPON_SPEAR,		20,	70 },
	{ WEAPON_MACE,		125,	250 },
	{ WEAPON_AXE,		150,	300 },
	{ WEAPON_FLAIL,		125,	250 },
	{ WEAPON_WHIP,		20,	70 },
	{ WEAPON_POLEARM,	125,	250 },
	{ WEAPON_BOW,		20,	70 },
	{ WEAPON_ARROW,		2,	6 },
	{ WEAPON_LANCE,		125,	250 }
};

static const struct volume_range item_volume[] =
{
	{ ITEM_FOOD,		1,	8 },
	{ ITEM_PILL,		1,	8 },
	{ ITEM_POTION,		1,	8 },
	{ ITEM_SCROLL,		1,	8 },
	{ ITEM_MAP,		1,	8 },
	{ ITEM_CLOTHING,	30,	60 },
	{ ITEM_CONTAINER,	5,	50 },
	{ ITEM_DRINK_CON,	5,	50 },
	{ ITEM_CORPSE_NPC,	60000,	140000 },
	{ ITEM_CORPSE_PC,	60000,	140000 },
	{ ITEM_TRASH,		1,	8 },
	{ ITEM_WARP_STONE,	1,	8 },
	{ ITEM_TREASURE,	1,	8 },
	{ ITEM_MAYMUNCUK,	1,	3 },
	{ ITEM_PORTAL,		500000,	600000 },
	{ ITEM_TATTOO,		1,	2 },
	{ ITEM_PROTECT,		1,	2 },
	{ ITEM_FOUNTAIN,	100000,	200000 },
	{ ITEM_GEM,		1,	8 },
	{ ITEM_JEWELRY,		1,	8 },
	{ ITEM_JUKEBOX,		80000,	150000 },
	{ ITEM_BOAT,		3000,	5000 },
	{ ITEM_FURNITURE,	15000,	25000 },
	{ ITEM_KEY,		1,	8 },
	{ ITEM_ROOM_KEY,	1,	8 },
	{ ITEM_MONEY,		1,	8 },
	{ ITEM_LIGHT,		1,	10 },
	{ ITEM_WAND,		10,	30 },
	{ ITEM_STAFF,		125,	250 }
};

int obj_random_weight(int vnum, int item_type, int weapon_type, int material, int wear_flags)
{
	long hacim = 0;
	int i;

	switch ( item_type )
	{
		case ITEM_ARMOR:
			for (i = 0; i < ARRAY_LEN(armor_volume); i++)
				if (wear_flags & armor_volume[i].key)
					break;
			if (i < ARRAY_LEN(armor_volume))
				hacim = number_range(armor_volume[i].lo, armor_volume[i].hi);
			else
			{
				bugf( "Random volume: vnum %d armor has no known wear flag", vnum );
				hacim = number_range(300,625);
			}
			break;
		case ITEM_WEAPON:
			for (i = 0; i < ARRAY_LEN(weapon_volume); i++)
				if (weapon_type == weapon_volume[i].key)
					break;
			if (i < ARRAY_LEN(weapon_volume))
				hacim = number_range(weapon_volume[i].lo, weapon_volume[i].hi);
			else
			{
				bugf( "Random volume: vnum %d has unknown weapon type %d", vnum, weapon_type );
				hacim = number_range(125,250);
			}
			break;
		default:
			for (i = 0; i < ARRAY_LEN(item_volume); i++)
				if (item_type == item_volume[i].key)
					break;
			if (i < ARRAY_LEN(item_volume))
				hacim = number_range(item_volume[i].lo, item_volume[i].hi);
			else
			{
				bugf( "Random volume: vnum %d has unknown obj type %d", vnum, item_type );
				hacim = number_range(8,1000);
			}
			break;
	}

	if (material < 0 || material >= MAX_MATERIALS || material_table[material].name == NULL)
		material = MATERIAL_DEFAULT_INDEX;

	return material_table[material].weight_pt * hacim;
}

/* Tabloda 'ok_field' alanı doğru olan malzemelerden rastgele biri; yoksa "iron". */
static const char *pick_material(size_t ok_field)
{
	int i, n = 0, pick;

	for (i = 0; material_table[i].name != NULL; i++)
		if (*(const bool *)((const char *)&material_table[i] + ok_field))
			n++;

	if (n == 0)
	{
		bug("pick_material: uygun malzeme yok.", 0);
		return "iron";
	}

	pick = number_range(1, n);
	for (i = 0; material_table[i].name != NULL; i++)
		if (*(const bool *)((const char *)&material_table[i] + ok_field) && --pick == 0)
			return material_table[i].name;

	return "iron";
}

/* Sabit malzemeli eşya türleri. */
static const struct { int item_type; const char *material; } fixed_material[] =
{
	{ ITEM_POTION,		"glass" },
	{ ITEM_CORPSE_NPC,	"flesh" },
	{ ITEM_CORPSE_PC,	"flesh" },
	{ ITEM_TRASH,		"vellum" },
	{ ITEM_WARP_STONE,	"gem" },
	{ ITEM_TREASURE,	"gem" },
	{ ITEM_MAYMUNCUK,	"iron" },
	{ ITEM_PORTAL,		"energy" },
	{ ITEM_TATTOO,		"energy" },
	{ ITEM_PROTECT,		"energy" },
	{ ITEM_FOUNTAIN,	"marble" },
	{ ITEM_GEM,		"gem" },
	{ ITEM_JEWELRY,		"gem" },
	{ ITEM_JUKEBOX,		"hardwood" },
	{ ITEM_BOAT,		"hardwood" },
	{ ITEM_FURNITURE,	"hardwood" },
	{ ITEM_KEY,		"iron" },
	{ ITEM_ROOM_KEY,	"iron" },
	{ ITEM_MONEY,		"silver" }
};

static const char *const light_materials[] = { "energy", "glass", "wood", "oil" };
static const char *const wand_materials[]  = { "wood", "softwood", "hardwood" };

void obj_random_material(OBJ_DATA *obj)
{
	const char *name = NULL;
	int i;

	switch ( obj->item_type )
	{
		case ITEM_ARMOR:
			name = pick_material(IS_SET(obj->wear_flags, ITEM_WEAR_ABOUT)
				? offsetof(struct material_type, textile_ok)
				: offsetof(struct material_type, armor_ok));
			break;
		case ITEM_WEAPON:
			name = pick_material(offsetof(struct material_type, weapon_ok));
			break;
		case ITEM_FOOD:
		case ITEM_PILL:
			name = pick_material(offsetof(struct material_type, food_ok));
			break;
		case ITEM_SCROLL:
		case ITEM_MAP:
			name = pick_material(offsetof(struct material_type, scroll_ok));
			break;
		case ITEM_CLOTHING:
			name = pick_material(offsetof(struct material_type, textile_ok));
			break;
		case ITEM_CONTAINER:
		case ITEM_DRINK_CON:
			name = pick_material(offsetof(struct material_type, container_ok));
			break;
		case ITEM_LIGHT:
			name = light_materials[number_range(0, ARRAY_LEN(light_materials) - 1)];
			break;
		case ITEM_WAND:
		case ITEM_STAFF:
			name = wand_materials[number_range(0, ARRAY_LEN(wand_materials) - 1)];
			break;
		default:
			for (i = 0; i < ARRAY_LEN(fixed_material); i++)
				if (obj->item_type == fixed_material[i].item_type)
				{
					name = fixed_material[i].material;
					break;
				}
			if (name == NULL)
				name = "iron";
			break;
	}

	obj->material = str_dup(name);
}

static const long random_extra_flags[] =
{
	ITEM_GLOW, ITEM_HUM, ITEM_DARK, ITEM_LOCK, ITEM_EVIL, ITEM_INVIS,
	ITEM_MAGIC, ITEM_NODROP, ITEM_BLESS, ITEM_ANTI_GOOD, ITEM_ANTI_EVIL,
	ITEM_ANTI_NEUTRAL, ITEM_NOREMOVE, ITEM_NOPURGE, ITEM_NOSAC,
	ITEM_NOLOCATE, ITEM_MELT_DROP, ITEM_BURN_PROOF
};

static const long random_weapon_flags[] =
{
	WEAPON_FLAMING, WEAPON_FROST, WEAPON_VAMPIRIC, WEAPON_SHARP,
	WEAPON_VORPAL, WEAPON_TWO_HANDS, WEAPON_SHOCKING, WEAPON_POISON,
	WEAPON_HOLY
};

/* Her bayrak için ayrı %chance şansı (eski davranış: extra < 2, silah <= 2). */
static int random_flags(const long *flags, int n, int chance)
{
	int newflag = 0, i;

	for (i = 0; i < n; i++)
		if (number_percent() < chance)
			SET_BIT(newflag, flags[i]);

	return newflag;
}

int obj_random_extra_flag(void)
{
	return random_flags(random_extra_flags, ARRAY_LEN(random_extra_flags), 2);
}

int obj_random_weapon_flag(void)
{
	return random_flags(random_weapon_flags, ARRAY_LEN(random_weapon_flags), 3);
}

const char *obj_random_wand_potion_spell(void)
{
	static int count = 0;

	if (count == 0)
		while (wand_spell_table[count].name != NULL)
			count++;

	return wand_spell_table[number_range(0, count - 1)].name;
}

static void set_obj_strings(OBJ_DATA *obj, const char *name, const char *shrt, const char *lng)
{
	free_string( obj->name );
	obj->name = str_dup( name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( shrt );
	free_string( obj->description );
	obj->description = str_dup( lng );
}

static const struct weapon_name_type
{
	int type;
	const char *name, *shrt, *lng;
} weapon_names[] =
{
	{ WEAPON_EXOTIC,	"egzotik silah",	"egzotik bir silah",	"Egzotik bir silah burada duruyor." },
	{ WEAPON_SWORD,		"kılıç silah",		"bir kılıç",		"Bir kılıç burada duruyor." },
	{ WEAPON_DAGGER,	"hançer silah",		"bir hançer",		"Bir hançer burada duruyor." },
	{ WEAPON_SPEAR,		"mızrak silah",		"bir mızrak",		"Bir mızrak burada duruyor." },
	{ WEAPON_MACE,		"topuz silah",		"bir topuz",		"Bir topuz burada duruyor." },
	{ WEAPON_AXE,		"balta silah",		"bir balta",		"Bir balta burada duruyor." },
	{ WEAPON_FLAIL,		"döven silah",		"bir döven",		"Bir döven burada duruyor." },
	{ WEAPON_WHIP,		"kırbaç silah",		"bir kırbaç",		"Bir kırbaç burada duruyor." },
	{ WEAPON_POLEARM,	"teber silah",		"bir teber",		"Bir teber burada duruyor." },
	{ WEAPON_BOW,		"yay silah",		"bir yay",		"Bir yay burada duruyor." },
	{ WEAPON_ARROW,		"ok silah",		"bir ok",		"Bir ok burada duruyor." },
	{ WEAPON_LANCE,		"kargı silah",		"bir kargı",		"Bir kargı burada duruyor." },
	{ -1,			"bilinmeyen silah",	"bilinmeyen bir silah",	"Bilinmeyen bir tür silah burada duruyor." }
};

void obj_random_name(OBJ_DATA *obj)
{
	const struct weapon_name_type *w;

	if(obj == NULL || obj->item_type != ITEM_WEAPON)
		return;

	for (w = weapon_names; w->type != -1 && w->type != obj->value[0]; w++)
		;
	set_obj_strings(obj, w->name, w->shrt, w->lng);
}
