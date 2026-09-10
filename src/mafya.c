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
#include <stdlib.h>
#include <string.h>
#include "merc.h"

DECLARE_DO_FUN( do_say );

/* Mafyanın sunduğu hizmetler: "iste lakapkaldır <lakap>". */
struct mafya_hizmet
{
    const char *lakap;	/* argüman */
    int		bedel;	/* bankadan düşülen akçe */
    long	bit;	/* kaldırılan PLR_ biti */
};

static const struct mafya_hizmet mafya_hizmetleri[] =
{
    { "aranıyor", 20000, PLR_WANTED },
    { NULL, 0, 0 }
};

static CHAR_DATA *find_mafya( CHAR_DATA *ch )
{
    CHAR_DATA *mafya;

    for ( mafya = ch->in_room->people; mafya != NULL; mafya = mafya->next_in_room )
	if ( IS_NPC(mafya) && IS_SET(mafya->act, ACT_MAFYA) )
	    return mafya;

    send_to_char("Burada olmaz.\n\r", ch);
    return NULL;
}

static void mafya_menu( CHAR_DATA *mafya )
{
    const struct mafya_hizmet *h;
    char buf[MAX_INPUT_LENGTH];

    do_say(mafya, "Lakap kaldırtmak:");
    for ( h = mafya_hizmetleri; h->lakap != NULL; h++ )
    {
	snprintf(buf, sizeof(buf), "%s:  %d akçe", h->lakap, h->bedel);
	do_say(mafya, buf);
    }
    do_say(mafya, "talep lakapkaldır <aranıyor>");
}

void do_iste( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mafya;
    const struct mafya_hizmet *h;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
	return;

    mafya = find_mafya(ch);
    if ( mafya == NULL )
	return;

    if ( argument[0] == '\0' )
    {
	mafya_menu(mafya);
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( str_prefix(arg1, "lakapkaldır") || arg2[0] == '\0' )
    {
	do_say(mafya, "Ne söylediğini anlamıyorum, ama şunlar ilgini çekebilir...");
	mafya_menu(mafya);
	return;
    }

    for ( h = mafya_hizmetleri; h->lakap != NULL; h++ )
	if ( !str_prefix(arg2, h->lakap) )
	    break;

    if ( h->lakap == NULL )
    {
	do_say(mafya, "Ne söylediğini anlamıyorum, ama şunlar ilgini çekebilir...");
	mafya_menu(mafya);
	return;
    }

    if ( !IS_SET(ch->act, h->bit) )
    {
	do_say(mafya, "Aranmıyorsun ki! Hatta oldukça zararsız bi tipe benziyorsun!");
	return;
    }
    if ( h->bedel > ch->pcdata->bank_s )
    {
	do_say(mafya, "Bankada yeterli akçen yok.");
	return;
    }
    REMOVE_BIT( ch->act, h->bit );
    ch->pcdata->bank_s -= h->bedel;
    mafya->silver      += h->bedel;
    do_say(mafya, "Temiz iş oldu. Yüzbaşının ofisindeki kayıtları sildik. Artık aranmıyorsun!");
    do_say(mafya, "Yine bekleriz.");
}
