/***************************************************************************
 *                                                                         *
 * Uzak Diyarlar a��k kaynak T�rk�e Mud projesidir.                        *
 * Oyun geli�tirmesi Jai ve Maru taraf�ndan y�netilmektedir.               *
 * Unutulmamas� gerekenler: Nir, Kame, Randalin, Nyah, Sint                          *
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
		do_say(mafya, (char*)"Lakap kald�rtmak:");
		do_say(mafya, (char*)"aran�yor:  20000 ak�e");
		do_say(mafya, (char*)"talep lakapkald�r <aran�yor>");
		return;
    }

	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

	if (!str_prefix(arg1,"lakapkald�r"))
	{
		if (!str_prefix(arg2,"aran�yor"))
		{
			cost = 20000;
			if ( !IS_SET(ch->act, PLR_WANTED) )
			{
				do_say(mafya, (char*)"Aranm�yorsun ki! Hatta olduk�a zarars�z bi tipe benziyorsun!");
				return;
			}
			if ( cost > ch->pcdata->bank_s )
			{
				do_say(mafya, (char*)"Bankada yeterli ak�en yok.");
				return;
			}
			REMOVE_BIT( ch->act, PLR_WANTED );
			ch->pcdata->bank_s 	    -= cost;
			mafya->silver  += cost;
			do_say(mafya, (char*)"Temiz i� oldu. Y�zba��n�n ofisindeki kay�tlar� sildik. Art�k aranm�yorsun!");
			do_say(mafya, (char*)"Yine bekleriz.");
			return;
		}
	}
	else
	{
		do_say(mafya, (char*)"Ne s�yledi�ini anlam�yorum, ama �unlar ilgini �ekebilir...");
		do_say(mafya, (char*)"Lakap kald�rtmak:");
		do_say(mafya, (char*)"aran�yor:  20000 ak�e");
		do_say(mafya, (char*)"talep lakapkald�r <aran�yor>");
		return;
	}
}
