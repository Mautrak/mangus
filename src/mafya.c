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
#include "magic.h"

DECLARE_DO_FUN( do_say );

CHAR_DATA *  find_mafya	args( ( CHAR_DATA * ch ) );

CHAR_DATA * find_mafya ( CHAR_DATA *ch )
{
    CHAR_DATA * mafya;

    for ( mafya = ch->in_room->people; mafya != NULL; mafya = mafya->next_in_room ) {
	if (!IS_NPC(mafya))
	    continue;

        if ( IS_NPC(mafya) && IS_SET(mafya->act, ACT_MAFYA) )
	    return mafya;
    }

    if ( mafya == NULL ) {
	send_to_char("Burada olmaz.\n\r", ch);
	return NULL;
    }

    return NULL;
}

void do_iste( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA * mafya;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int	cost;

	mafya = find_mafya(ch);
	if (!mafya)
		return;

	if (argument[0] == '\0')
	{
		do_say(mafya, (char*)"Lakap kaldırtmak:");
		do_say(mafya, (char*)"aranıyor:  20000 akçe");
		do_say(mafya, (char*)"talep lakapkaldır <aranıyor>");
		return;
    }

	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

	if (!str_prefix(arg1,"lakapkaldır"))
	{
		if (!str_prefix(arg2,"aranıyor"))
		{
			cost = 20000;
			if ( !IS_SET(ch->act, PLR_WANTED) )
			{
				do_say(mafya, (char*)"Aranmıyorsun ki! Hatta oldukça zararsız bi tipe benziyorsun!");
				return;
			}
			if ( cost > ch->pcdata->bank_s )
			{
				do_say(mafya, (char*)"Bankada yeterli akçen yok.");
				return;
			}
			REMOVE_BIT( ch->act, PLR_WANTED );
			ch->pcdata->bank_s 	    -= cost;
			mafya->silver  += cost;
			do_say(mafya, (char*)"Temiz iş oldu. Yüzbaşının ofisindeki kayıtları sildik. Artık aranmıyorsun!");
			do_say(mafya, (char*)"Yine bekleriz.");
			return;
		}
	}
	else
	{
		do_say(mafya, (char*)"Ne söylediğini anlamıyorum, ama şunlar ilgini çekebilir...");
		do_say(mafya, (char*)"Lakap kaldırtmak:");
		do_say(mafya, (char*)"aranıyor:  20000 akçe");
		do_say(mafya, (char*)"talep lakapkaldır <aranıyor>");
		return;
	}
}
