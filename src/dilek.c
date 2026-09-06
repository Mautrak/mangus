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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_say );


void do_dilek(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *wishmaster;
	char buf [MAX_STRING_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

	if (arg1[0] == '\0')
    {
        printf_to_char(ch,"DİLEK komutları: ALIŞ LİSTE ÖZELLİK.\n\r");
        return;
    }

	for ( wishmaster = ch->in_room->people; wishmaster != NULL; wishmaster = wishmaster->next_in_room )
    {
        if (!IS_NPC( wishmaster ))
			continue;
        if (wishmaster->spec_fun == spec_lookup( (char*)"spec_wishmaster" ))
			break;
    }

    if (wishmaster == NULL || wishmaster->spec_fun != spec_lookup( (char*)"spec_wishmaster" ))
    {
        printf_to_char(ch,"Burada dilek dileyemezsin. Bir dilek taşı uzmanı bul.\n\r");
        return;
    }

	if ( wishmaster->fighting != NULL)
    {
        printf_to_char(ch,"Dövüş bitene kadar bekle.\n\r");
        return;
    }

	if (!strcmp(arg1, "liste"))
	{
		act( "$n $Z dilek taşlarının listesini istiyor.", ch, NULL, wishmaster, TO_ROOM);
		act ("$Z dilek taşlarının listesini istiyorsun.",ch, NULL, wishmaster, TO_CHAR);

		printf_to_char(ch, "Mevcut dilek taşları:\n\r\n\r");
		printf_to_char(ch, "  {Ctecrübe taşı{x....({ytecrübe{x)....{R3000 gp, 500000 akçe{x\n\r");
		printf_to_char(ch, "  {Cgörev taşı{x......({ygörev  {x)....{R3000 gp, 500000 akçe{x\n\r");
		return;
	}
	else if (!strcmp(arg1, "özellik"))
	{
		if (arg2[0] == '\0')
		{
			printf_to_char(ch,"Hangi dilek taşının özelliklerini öğrenmek istiyorsun?\n\rKullanım: {Rdilek özellik <taş>{x\n\r");
			return;
		}
		else if (is_name(arg2, (char*)"tecrübe"))
		{
			printf_to_char(ch,"Tecrübe taşı ile dilenen dilekten sonra kişi her defasında iki kat tecrübe kazanır.\n\r");
			return;
		}
		else if (is_name(arg2, (char*)"görev"))
		{
			printf_to_char(ch,"Görev taşı ile dilenen dilekten sonra kişi her görev tamamlayışında iki kat GP kazanır.\n\r");
			return;
		}
		else
		{
			printf_to_char(ch,"Hangi dilek taşının özelliklerini öğrenmek istiyorsun?\n\rKullanım: {Rdilek özellik <taş>{x\n\r");
			return;
		}
	}
	else if (!strcmp(arg1, "satınal"))
	{
		if (arg2[0] == '\0')
		{
			printf_to_char( ch , "Dilek satın almak için {Rdilek satınal <taş>{x kullanılabilir.\n\r");
			return;
		}
		else if (is_name(arg2, (char*)"tecrübe"))
		{
			if(IS_SET(ch->pcdata->dilek,DILEK_FLAG_TECRUBE))
			{
				do_say(wishmaster,(char*)"Bu dileği zaten dilemişsin.");
				return;
			}
			if (ch->pcdata->questpoints >= 3000 && ch->pcdata->bank_s >= 500000)
			{
				ch->pcdata->questpoints -= 3000;
				ch->pcdata->bank_s -= 500000;
				SET_BIT(ch->pcdata->dilek,DILEK_FLAG_TECRUBE);
				do_say(wishmaster,(char*)"İşte bu taş senin dileğin için...");
				printf_to_char( ch , "\n\r{CHüssam mavi bir dilek taşına son şeklini verdikten sonra kuyuya atıyor.{x\n\r");
				printf_to_char( ch , "{CKuyudan yükselen bir ışık yavaşça bedenini sarıp seninle bütünleşiyor.{x\n\r");
				return;
			}
			else
			{
				sprintf(buf, "Üzgünüm %s, o kadar puanın veya bankada akçen yok.",ch->name);
				do_say(wishmaster,buf);
				return;
			}
		}
		else if (is_name(arg2, (char*)"görev"))
		{
			if(IS_SET(ch->pcdata->dilek,DILEK_FLAG_GOREV))
			{
				do_say(wishmaster,(char*)"Bu dileği zaten dilemişsin.");
				return;
			}
			if (ch->pcdata->questpoints >= 3000 && ch->pcdata->bank_s >= 500000)
			{
				ch->pcdata->questpoints -= 3000;
				ch->pcdata->bank_s -= 500000;
				SET_BIT(ch->pcdata->dilek,DILEK_FLAG_GOREV);
				do_say(wishmaster,(char*)"İşte bu taş senin dileğin için...");
				printf_to_char( ch , "\n\r{CHüssam kızıl bir dilek taşına son şeklini verdikten sonra kuyuya atıyor.{x\n\r");
				printf_to_char( ch , "{CKuyudan yükselen bir ışık yavaşça bedenini sarıp seninle bütünleşiyor.{x\n\r");
				return;
			}
			else
			{
				sprintf(buf, "Üzgünüm %s, o kadar puanın veya bankada akçen yok.",ch->name);
				do_say(wishmaster,buf);
				return;
			}
		}
		else
		{
			printf_to_char( ch , "Dilek satın almak için {Rdilek satınal <taş>{x kullanılabilir.\n\r");
			return;
		}
	}
	else
    {
        printf_to_char(ch,"DİLEK komutları: SATINAL LİSTE ÖZELLİK.\n\r");
        return;
    }
}
