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

/***************************************************************************
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"


BAN_DATA *ban_list;

void save_bans(void)
{
    BAN_DATA *pban;
    FILE *fp;
    bool found = FALSE;

    fclose( fpReserve );
    if ( ( fp = fopen( BAN_FILE, "w" ) ) == NULL )
    {
	perror( BAN_FILE );
	bug( "save_bans: yasak dosyası yazılamadı.", 0 );
    }
    else
    {
	for ( pban = ban_list; pban != NULL; pban = pban->next )
	{
	    if ( IS_SET(pban->ban_flags, BAN_PERMANENT) )
	    {
		found = TRUE;
		fprintf( fp, "%-20s %-2d %s\n", pban->name, pban->level,
		    print_flags(pban->ban_flags) );
	    }
	}
	fclose( fp );
	if ( !found )
	    remove( BAN_FILE );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    if ( fpReserve == NULL )
	bug( "save_bans: null dosyası açılamadı.", 0 );
}

void load_bans(void)
{
    FILE *fp;
    BAN_DATA *ban_last;

    if ( ( fp = fopen( BAN_FILE, "r" ) ) == NULL )
        return;

    ban_last = NULL;
    for ( ; ; )
    {
        BAN_DATA *pban;
        if ( feof(fp) )
        {
            fclose( fp );
            return;
        }

        pban = new_ban();

        pban->name = str_dup(fread_word(fp));
	pban->level = fread_number(fp);
	pban->ban_flags = fread_flag(fp);
	fread_to_eol(fp);

        if (ban_list == NULL)
	    ban_list = pban;
	else
	    ban_last->next = pban;
	ban_last = pban;
    }
}

bool check_ban(char *site,int type)
{
    BAN_DATA *pban;
    char host[MAX_STRING_LENGTH];
    size_t i;

    /* Host adları ASCII'dir: tümünü küçük harfe çevir. */
    for ( i = 0; site[i] != '\0' && i < sizeof(host) - 1; i++ )
	host[i] = LOWER(site[i]);
    host[i] = '\0';

    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if(!IS_SET(pban->ban_flags,type))
	    continue;

	if (IS_SET(pban->ban_flags,BAN_PREFIX)
	&&  IS_SET(pban->ban_flags,BAN_SUFFIX)
	&&  strstr(pban->name,host) != NULL)
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_PREFIX)
	&&  !str_suffix(pban->name,host))
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_SUFFIX)
	&&  !str_prefix(pban->name,host))
	    return TRUE;
    }

    return FALSE;
}

/*
 * Adı eşleşen yasakları listeden çıkarır.
 * Dönüş: BAN_RM_NONE (eşleşme yok), BAN_RM_HIGHER (daha yüksek seviyeli
 * biri koymuş, hiçbir şey silinmedi), BAN_RM_DONE (silindi).
 */
#define BAN_RM_NONE   0
#define BAN_RM_HIGHER 1
#define BAN_RM_DONE   2

static int ban_remove( const char *name, int trust )
{
    BAN_DATA *pban, *prev, *next;
    int result = BAN_RM_NONE;

    for ( pban = ban_list; pban != NULL; pban = pban->next )
	if ( !str_cmp( name, pban->name ) && pban->level > trust )
	    return BAN_RM_HIGHER;

    prev = NULL;
    for ( pban = ban_list; pban != NULL; pban = next )
    {
	next = pban->next;
	if ( str_cmp( name, pban->name ) )
	{
	    prev = pban;
	    continue;
	}

	if ( prev == NULL )
	    ban_list = next;
	else
	    prev->next = next;
	free_ban( pban );
	result = BAN_RM_DONE;
    }

    return result;
}

void ban_site(CHAR_DATA *ch, char *argument, bool fPerm)
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char *name;
    BUFFER *buffer;
    BAN_DATA *pban;
    bool prefix = FALSE,suffix = FALSE;
    int type;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' )
    {
	if (ban_list == NULL)
	{
	    send_to_char("Şu an yasaklı site yok.\n\r",ch);
	    return;
  	}
	buffer = new_buf();

        add_buf(buffer,"Yasaklı siteler seviye tür     durum\n\r");
        for (pban = ban_list;pban != NULL;pban = pban->next)
        {
	    snprintf(buf2, sizeof(buf2),"%s%s%s",
		IS_SET(pban->ban_flags,BAN_PREFIX) ? "*" : "",
		pban->name,
		IS_SET(pban->ban_flags,BAN_SUFFIX) ? "*" : "");
	    snprintf(buf, sizeof(buf),"%-12s    %-3d  %-7s  %s\n\r",
		buf2, pban->level,
		IS_SET(pban->ban_flags,BAN_NEWBIES) ? "newbies" :
		IS_SET(pban->ban_flags,BAN_PLAYER)  ? "player" :
		IS_SET(pban->ban_flags,BAN_PERMIT)  ? "permit"  :
		IS_SET(pban->ban_flags,BAN_ALL)     ? "all"	: "",
	    	IS_SET(pban->ban_flags,BAN_PERMANENT) ? "kalıcı" : "geçici");
	    add_buf(buffer,buf);
        }

        page_to_char( buf_string(buffer), ch );
	free_buf(buffer);
        return;
    }

    /* find out what type of ban */
    if (arg2[0] == '\0' || !str_prefix(arg2,"all"))
	type = BAN_ALL;
    else if (!str_prefix(arg2,"newbies"))
	type = BAN_NEWBIES;
    else if (!str_prefix(arg2,"player"))
	type = BAN_PLAYER;
    else if (!str_prefix(arg2,"permit"))
	type = BAN_PERMIT;
    else
    {
	send_to_char("Geçerli yasak türleri: all, newbies, player, permit.\n\r",
	    ch);
	return;
    }

    name = arg1;

    if (name[0] == '*')
    {
	prefix = TRUE;
	name++;
    }

    if (name[0] != '\0' && name[strlen(name) - 1] == '*')
    {
	suffix = TRUE;
	name[strlen(name) - 1] = '\0';
    }

    if (name[0] == '\0')
    {
	send_to_char("Bir şeyi yasaklaman gerekiyor.\n\r",ch);
	return;
    }

    if ( ban_remove( name, get_trust(ch) ) == BAN_RM_HIGHER )
    {
	send_to_char( "Bu yasak daha yüksek seviyeli biri tarafından koyulmuş.\n\r", ch );
	return;
    }

    pban = new_ban();
    pban->name = str_dup(name);
    pban->level = get_trust(ch);

    /* set ban type */
    pban->ban_flags = type;

    if (prefix)
	SET_BIT(pban->ban_flags,BAN_PREFIX);
    if (suffix)
	SET_BIT(pban->ban_flags,BAN_SUFFIX);
    if (fPerm)
	SET_BIT(pban->ban_flags,BAN_PERMANENT);

    pban->next  = ban_list;
    ban_list    = pban;
    save_bans();
    printf_to_char( ch, "%s yasaklandı.\n\r", pban->name );
}

void do_ban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,FALSE);
}

void do_permban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,TRUE);
}

void do_allow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Hangi site yasak listesinden çıkarılsın?\n\r", ch );
        return;
    }

    switch ( ban_remove( arg, get_trust(ch) ) )
    {
    case BAN_RM_HIGHER:
	send_to_char( "Bu yasağı kaldıracak kadar güçlü değilsin.\n\r", ch );
	return;
    case BAN_RM_DONE:
	printf_to_char( ch, "%s üzerindeki yasak kaldırıldı.\n\r", arg );
	save_bans();
	return;
    default:
	send_to_char( "Bu site yasaklı değil.\n\r", ch );
	return;
    }
}
