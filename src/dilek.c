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
#include "utf8.h"

DECLARE_DO_FUN( do_say );

/* Her dilek taşının bedeli. */
#define DILEK_QP_COST		3000
#define DILEK_SILVER_COST	500000

static const char DILEK_USAGE[] = "DİLEK komutları: SATINAL LİSTE ÖZELLİK.\n\r";

struct dilek_stone
{
    const char *name;		/* liste ve argüman anahtarı */
    int         flag;		/* pcdata->dilek biti */
    const char *colour;		/* taşın rengi (satın alma mesajı) */
    const char *ozellik;	/* 'dilek özellik' açıklaması */
};

static const struct dilek_stone dilek_table[] =
{
    { "tecrübe", DILEK_FLAG_TECRUBE, "mavi",
      "Tecrübe taşı ile dilenen dilekten sonra kişi her defasında iki kat tecrübe kazanır." },
    { "görev",   DILEK_FLAG_GOREV,   "kızıl",
      "Görev taşı ile dilenen dilekten sonra kişi her görev tamamlayışında iki kat GP kazanır." },
    { NULL, 0, NULL, NULL }
};

static const struct dilek_stone *dilek_lookup( const char *arg )
{
    const struct dilek_stone *stone;

    for ( stone = dilek_table; stone->name != NULL; stone++ )
	if ( is_name( (char *) arg, (char *) stone->name ) )
	    return stone;
    return NULL;
}

static void dilek_list( CHAR_DATA *ch, CHAR_DATA *wishmaster )
{
    const struct dilek_stone *stone;
    char label[MAX_INPUT_LENGTH];

    act( "$n $Z dilek taşlarının listesini istiyor.", ch, NULL, wishmaster, TO_ROOM );
    act( "$Z dilek taşlarının listesini istiyorsun.", ch, NULL, wishmaster, TO_CHAR );

    printf_to_char( ch, "Mevcut dilek taşları:\n\r\n\r" );
    for ( stone = dilek_table; stone->name != NULL; stone++ )
    {
	size_t n;

	snprintf( label, sizeof(label), "%s taşı", stone->name );
	for ( n = utf8_strlen( label ); n < 16 && strlen( label ) + 1 < sizeof(label); n++ )
	    strncat( label, ".", sizeof(label) - strlen( label ) - 1 );
	printf_to_char( ch, "  {C%s{x({y%-*s{x)....{R%d gp, %d akçe{x\n\r",
	    label, utf8_width( stone->name, 7 ), stone->name,
	    DILEK_QP_COST, DILEK_SILVER_COST );
    }
}

static void dilek_satin_al( CHAR_DATA *ch, CHAR_DATA *wishmaster, const struct dilek_stone *stone )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET( ch->pcdata->dilek, stone->flag ) )
    {
	do_say( wishmaster, "Bu dileği zaten dilemişsin." );
	return;
    }

    if ( ch->pcdata->questpoints < DILEK_QP_COST || ch->pcdata->bank_s < DILEK_SILVER_COST )
    {
	snprintf( buf, sizeof(buf), "Üzgünüm %s, o kadar puanın veya bankada akçen yok.", ch->name );
	do_say( wishmaster, buf );
	return;
    }

    ch->pcdata->questpoints -= DILEK_QP_COST;
    ch->pcdata->bank_s -= DILEK_SILVER_COST;
    SET_BIT( ch->pcdata->dilek, stone->flag );
    do_say( wishmaster, "İşte bu taş senin dileğin için..." );
    printf_to_char( ch, "\n\r" );
    act( "{C$N $t bir dilek taşına son şeklini verdikten sonra kuyuya atıyor.{x",
	ch, stone->colour, wishmaster, TO_CHAR );
    printf_to_char( ch, "{CKuyudan yükselen bir ışık yavaşça bedenini sarıp seninle bütünleşiyor.{x\n\r" );
}

void do_dilek( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *wishmaster;
    SPEC_FUN *spec_wishmaster;
    const struct dilek_stone *stone;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) || ch->in_room == NULL )
	return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	printf_to_char( ch, "%s", DILEK_USAGE );
	return;
    }

    spec_wishmaster = spec_lookup( "spec_wishmaster" );
    for ( wishmaster = ch->in_room->people; wishmaster != NULL; wishmaster = wishmaster->next_in_room )
	if ( IS_NPC( wishmaster ) && wishmaster->spec_fun == spec_wishmaster )
	    break;

    if ( wishmaster == NULL )
    {
	printf_to_char( ch, "Burada dilek dileyemezsin. Bir dilek taşı uzmanı bul.\n\r" );
	return;
    }

    if ( wishmaster->fighting != NULL )
    {
	printf_to_char( ch, "Dövüş bitene kadar bekle.\n\r" );
	return;
    }

    if ( !str_prefix( arg1, "liste" ) )
    {
	dilek_list( ch, wishmaster );
	return;
    }

    if ( !str_prefix( arg1, "özellik" ) )
    {
	if ( arg2[0] == '\0' || ( stone = dilek_lookup( arg2 ) ) == NULL )
	{
	    printf_to_char( ch, "Hangi dilek taşının özelliklerini öğrenmek istiyorsun?\n\r"
				"Kullanım: {Rdilek özellik <taş>{x\n\r" );
	    return;
	}
	printf_to_char( ch, "%s\n\r", stone->ozellik );
	return;
    }

    if ( !str_prefix( arg1, "satınal" ) )
    {
	if ( arg2[0] == '\0' || ( stone = dilek_lookup( arg2 ) ) == NULL )
	{
	    printf_to_char( ch, "Dilek satın almak için {Rdilek satınal <taş>{x kullanılabilir.\n\r" );
	    return;
	}
	dilek_satin_al( ch, wishmaster, stone );
	return;
    }

    printf_to_char( ch, "%s", DILEK_USAGE );
}
