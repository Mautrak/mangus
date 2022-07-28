/***************************************************************************
 *                                                                         *
 * Uzak Diyarlar a��k kaynak T�rk�e Mud projesidir.                        *
 * Oyun geli�tirmesi Jai ve Maru taraf�ndan y�netilmektedir.               *
 * Unutulmamas� gerekenler: Nir, Kame, Nyah, Sint                          *
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
#include <ctype.h>
#include "merc.h"
#include "interp.h"


void do_zar (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int zar_tipi, ch_zar, mekanci_zar;
	long yatirilan,kazanc;

	if (!IS_SET(ch->in_room->room_flags, ROOM_KUMAR))
    {
      printf_to_char (ch,"Kumarhanede de�ilsin.\n\r");
      return;
    }
	if (ch->silver < 5)
    {
        printf_to_char (ch,"Zar oynamak i�in en az 5 ak�en ihtiyac�n var.\n\r");
        return;
    }

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if( arg1[0]=='\0' )
	{
		printf_to_char (ch,"Hangi zardan ne kadar oynayacaks�n? Mevcutlar� g�rmek i�in {Rzar liste{x yaz.\n\r");
        return;
	}
	if(!str_cmp(arg1,"liste"))
	{
		printf_to_char (ch,"Zar t�rleri:\n\r");
		printf_to_char (ch,"Zar Tipi  A��klama\n\r");
		printf_to_char (ch,"--------  ------------\n\r");
		printf_to_char (ch,"alt�      Alt�l�k zar\n\r");
		printf_to_char (ch,"on        Onluk zar\n\r");
		printf_to_char (ch,"yirmi     Yirmilik zar\n\r");
		printf_to_char (ch,"y�z       Y�zl�k zar\n\r\n\r");
		printf_to_char (ch,"Oynamak i�in {Gzar <zar_tipi> <yat�r�lan_ak�e>\n\r");
		return;
	}
	if( arg2[0]=='\0' )
	{
		printf_to_char (ch,"Hangi zardan ne kadar oynayacaks�n? Mevcutlar� g�rmek i�in {Rzar liste{x yaz.\n\r");
        return;
	}
	zar_tipi=0;
	if(!str_cmp(arg1,"alt�"))
		zar_tipi=6;
	else if(!str_cmp(arg1,"on"))
		zar_tipi=10;
	else if(!str_cmp(arg1,"yirmi"))
		zar_tipi=20;
	else if(!str_cmp(arg1,"y�z"))
		zar_tipi=100;
	else
	{
		printf_to_char (ch,"Hangi zardan ne kadar oynayacaks�n? Mevcutlar� g�rmek i�in {Rzar liste{x yaz.\n\r");
        return;
	}
	yatirilan=advatoi (arg2);
	if(ch->silver<yatirilan)
	{
		send_to_char ("Bu oyun lafla de�il ak�eyle oynan�r!\n\r", ch);
        	return;
	}
	if( (yatirilan>50000)||(yatirilan<5) )
	{
		send_to_char ("Zar en az 5, en �ok 50,000 ak�eyle oynan�r.\n\r", ch);
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

	printf_to_char(ch,"Mekanc� %d y�zl� bir zar� sallay�p at�yor... %d geldi.\n\r",zar_tipi,mekanci_zar);
	printf_to_char(ch,"%d y�zl� bir zar� sallay�p at�yorsun... %d geldi.\n\r",zar_tipi,ch_zar);
    act ("$n ve mekanc� ellerindeki zarlar� sallayarak masaya at�yorlar.\n\r", ch,NULL,NULL,TO_ROOM);
	if (mekanci_zar > ch_zar)
    {
        printf_to_char (ch,"Kaybettin.\n\r");
        act ("$n kaybetti.", ch, NULL, NULL, TO_ROOM);
        return;
    }
	/* ch kazand� */
	kazanc=2*yatirilan;
    printf_to_char (ch, "%d ak�e KAZANDIN.\n\r", yatirilan);
    act ("$n zar att� ve kazand�. Vay beee...\n\r", ch, NULL, NULL, TO_ROOM);
    ch->silver += kazanc;
    return;
}
