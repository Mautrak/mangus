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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"

int hitroll_damroll_hesapla(int level)
{
	int seviyenin_yarisi    = UMAX(1, level / 2);
	int seviyenin_onda_biri = UMAX(1, level / 10);

	return number_range( UMAX(1, seviyenin_yarisi - seviyenin_onda_biri),
			     UMAX(1, seviyenin_yarisi + 2 * seviyenin_onda_biri) );
}

/* Seviyeye göre hasar zarı: out[DICE_NUMBER], out[DICE_TYPE], out[DICE_BONUS]. */
static void damage_dice(int level, int out[3])
{
	int type, number, bonus;

	type   = level*7/4;
	number = UMIN(type/8 + 1, 5);
	type   = UMAX(2, type/number);
	bonus  = UMAX(0, level*9/4 - number*type);

	out[DICE_NUMBER] = number;
	out[DICE_TYPE]   = type;
	out[DICE_BONUS]  = bonus;
}

int damage_dice_0(int level)
{
	int d[3];
	damage_dice(level, d);
	return d[DICE_NUMBER];
}

int damage_dice_1(int level)
{
	int d[3];
	damage_dice(level, d);
	return d[DICE_TYPE];
}

int damage_dice_2(int level)
{
	int d[3];
	damage_dice(level, d);
	return d[DICE_BONUS];
}

/* Rastgele yaratığın saldırı türü; attack_table'da ada göre bulunur. */
int dam_type_dice(void)
{
	static const char *const dam_names[] =
		{ "slash", "pound", "pierce", "beating", "punch", "slap", "crush" };
	static int dam_index[sizeof(dam_names) / sizeof(dam_names[0])];
	static bool ready = FALSE;
	size_t i;

	if (!ready)
	{
		for (i = 0; i < sizeof(dam_names) / sizeof(dam_names[0]); i++)
			dam_index[i] = attack_lookup(dam_names[i]);
		ready = TRUE;
	}

	return dam_index[number_range(0, (int)(sizeof(dam_names) / sizeof(dam_names[0])) - 1)];
}

int ac_dice(int i,int level)
{
	if (i == AC_PIERCE || i == AC_BASH || i == AC_SLASH)
		return interpolate( level, 100, -100);
	return interpolate( level, 100, 0);
}

int position_dice(void)
{
	int dice = number_range( 1, 100 );

	if(dice < 50)
		return POS_STANDING;
	else if(dice < 75)
		return POS_SITTING;
	else if(dice < 90)
		return POS_RESTING;
	else
		return POS_SLEEPING;
}

int sex_dice(void)
{
	return number_range(1,2);
}

/*
 * Rastgele ırk. Düşük seviyede ilk 26, yüksek seviyede ilk 31 ırk aday olur
 * (tablonun sonundaki "unique" ve nöbetçi hariç); humanoid istenirse yalnızca
 * humanoid ırklar arasından seçilir.
 */
sh_int race_dice(int level, bool humanoid)
{
	int hi = level < 40 ? 26 : 31;
	int candidates[MAX_RACE];
	int n = 0, irace;

	for (irace = 1; irace <= hi && irace < MAX_RACE && race_table[irace].name[0] != NULL; irace++)
		if (!humanoid || race_table[irace].humanoid)
			candidates[n++] = irace;

	if (n == 0)
	{
		bug("race_dice: uygun ırk yok, insan seçildi.", 0);
		return 1;
	}

	return (sh_int) candidates[number_range(0, n - 1)];
}
