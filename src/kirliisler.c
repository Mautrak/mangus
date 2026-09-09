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
#include "utf8.h"

#define ZAR_MIN		1000
#define ZAR_MAX		500000

/* Oynanabilen zar türleri. */
static const struct
{
    const char *ad;
    int		yuz;
    const char *aciklama;
} zar_turleri[] =
{
    { "altı",	6,	"Altılık zar" },
    { "on",	10,	"Onluk zar" },
    { "yirmi",	20,	"Yirmilik zar" },
    { "yüz",	100,	"Yüzlük zar" },
    { NULL,	0,	NULL }
};

#define ZAR_KULLANIM "Hangi zardan ne kadar oynayacaksın? Mevcutları görmek için {Rzar liste{x yaz.\n\r"

void do_zar (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *duzenbaz;
	SPEC_FUN *duzenbaz_fn;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int i, zar_tipi, ch_zar, mekanci_zar;
	long yatirilan;

	duzenbaz_fn = spec_lookup( "spec_duzenbaz" );
	for ( duzenbaz = ch->in_room->people; duzenbaz != NULL; duzenbaz = duzenbaz->next_in_room )
	{
		if (!IS_NPC(duzenbaz))
			continue;
		if (duzenbaz_fn != NULL && duzenbaz->spec_fun == duzenbaz_fn)
			break;
	}

	if (duzenbaz == NULL)
	{
		send_to_char("Odada bu işlerden anlayan bir düzenbaz göremiyorum.\n\r", ch);
		return;
	}

	if (ch->silver < ZAR_MIN)
	{
		printf_to_char (ch,"Zar oynamak için en az %d akçen ihtiyacın var.\n\r", ZAR_MIN);
		return;
	}

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if( arg1[0]=='\0' )
	{
		send_to_char (ZAR_KULLANIM, ch);
		return;
	}
	if(!str_cmp(arg1,"liste"))
	{
		send_to_char ("Zar türleri:\n\r", ch);
		send_to_char ("Zar Tipi  Açıklama\n\r", ch);
		send_to_char ("--------  ------------\n\r", ch);
		for ( i = 0; zar_turleri[i].ad != NULL; i++ )
			printf_to_char (ch, "%-*s  %s\n\r",
				utf8_width(zar_turleri[i].ad, 8), zar_turleri[i].ad,
				zar_turleri[i].aciklama);
		send_to_char ("\n\rOynamak için {Gzar <zar_tipi> <yatırılan_akçe>{x\n\r", ch);
		return;
	}
	if( arg2[0]=='\0' )
	{
		send_to_char (ZAR_KULLANIM, ch);
		return;
	}
	zar_tipi = 0;
	for ( i = 0; zar_turleri[i].ad != NULL; i++ )
		if (!str_cmp(arg1, zar_turleri[i].ad))
		{
			zar_tipi = zar_turleri[i].yuz;
			break;
		}
	if ( zar_tipi == 0 )
	{
		send_to_char (ZAR_KULLANIM, ch);
		return;
	}
	yatirilan = advatoi (arg2);
	if(ch->silver<yatirilan)
	{
		send_to_char ("Bu oyun lafla değil akçeyle oynanır!\n\r", ch);
		return;
	}
	if( yatirilan > ZAR_MAX || yatirilan < ZAR_MIN )
	{
		send_to_char ("Zar en az 1000, en çok 500,000 akçeyle oynanır.\n\r", ch);
		return;
	}
	ch->silver -= yatirilan;
	ch_zar = 0;
	mekanci_zar = 0;

	while(mekanci_zar==ch_zar)
	{
		ch_zar=number_range( 1, zar_tipi );
		mekanci_zar=number_range( 1, zar_tipi );
	}
	WAIT_STATE (ch, PULSE_VIOLENCE);

	printf_to_char(ch,"Mekancı %d yüzlü bir zarı sallayıp atıyor... %d geldi.\n\r",zar_tipi,mekanci_zar);
	printf_to_char(ch,"%d yüzlü bir zarı sallayıp atıyorsun... %d geldi.\n\r",zar_tipi,ch_zar);
	act ("$n ve mekancı ellerindeki zarları sallayarak masaya atıyorlar.\n\r", ch,NULL,NULL,TO_ROOM);
	if (mekanci_zar > ch_zar)
	{
		send_to_char ("Kaybettin.\n\r", ch);
		act ("$n kaybetti.", ch, NULL, NULL, TO_ROOM);
		return;
	}
	/* ch kazandı: yatırdığının iki katını alır */
	printf_to_char (ch, "%ld akçe KAZANDIN.\n\r", yatirilan);
	act ("$n zar attı ve kazandı. Vay beee...\n\r", ch, NULL, NULL, TO_ROOM);
	ch->silver += 2 * yatirilan;
}
