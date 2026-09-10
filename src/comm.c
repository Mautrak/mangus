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
 *  Merc Diku vMud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

/*
 * This file contains the network glue, the main loop and the login
 * state machine (nanny).  OS-dependent socket code lives in platform.c.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 */


#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "merc.h"
#include "bot.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "password.h"
#include "turkish.h"
#include "utf8.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_skills	);
DECLARE_DO_FUN(do_outfit	);

bool cabal_area_check   (CHAR_DATA *ch);

/*
 * Telnet komut baytları. Sunucu yalnızca ECHO müzakeresi ve GA gönderir;
 * istemciden gelen her IAC dizisi girdi tamponundan ayıklanır (telnet_strip).
 */
#define TELNET_IAC	255
#define TELNET_DONT	254
#define TELNET_DO	253
#define TELNET_WONT	252
#define TELNET_WILL	251
#define TELNET_SB	250
#define TELNET_GA	249
#define TELNET_SE	240
#define TELOPT_ECHO	  1
const	char	echo_off_str	[] = { (char) TELNET_IAC, (char) TELNET_WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { (char) TELNET_IAC, (char) TELNET_WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { (char) TELNET_IAC, (char) TELNET_GA, '\0' };

/*
 * Çıktı tamponu üst sınırı: alloc_mem en fazla 32768-64 baytlık blok verir
 * (db.c rgSizeList); bir sonraki ikiye katlama oraya sığmaz.
 */
#define MAX_OUTBUF		32000
#define WRITE_BLOCK		4096

/*
 * Giriş (nanny) sabitleri.
 */
#define PROMPT_ENTER		"[Devam etmek için ENTER]\n\r"
#define SITE_INFO		"www.mangusmud.com"
#define PROMPT_CLASS		"Sınıfın ne olsun (bilgi: " SITE_INFO ")? "
#define PROMPT_RACE		"Irkınız nedir? (bilgi: " SITE_INFO ") "
#define PROMPT_NAME		"İsim: "
#define PROMPT_NAME_TAKEN	"Bu karakter oyunda, başka bir tane deneyin.\n\r" PROMPT_NAME
#define MAX_LOGIN_TRIES		3	/* üçüncü yanlış parolada bağlantı kapanır */
#define DISCORD_ID_MIN_LEN	18	/* Discord kullanıcı kimliği en az 18 hane */
#define RACE_LIST_PER_LINE	8
#define NEW_PC_RECALL_SKILL	75
#define NEW_PC_WEAPON_SKILL	40
#define NEW_PC_FAMILYA		75
#define NEW_PC_CHA		15
#define NEW_PC_BONUS_MINUTES	60	/* son 14 gün için birer saat oynama süresi */
#define NEW_PC_TRAINS		3
#define NEW_PC_PRACTICES	5
#define MAX_RACE_SKILLS		((int) (sizeof(race_table[0].skills) / sizeof(race_table[0].skills[0])))

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
time_t		    boot_time;		/* time of boot */
time_t		    current_time;	/* time of this pulse */

/*
 * Local functions.
 */
void	game_loop		( int control );
void	init_descriptor		( int control );
bool	read_from_descriptor	( DESCRIPTOR_DATA *d );
bool	write_to_descriptor	( int desc, char *txt, int length );
bool	check_parse_name	( char *name );
bool	check_reconnect		( DESCRIPTOR_DATA *d, char *name, bool fConn );
bool	check_playing		( DESCRIPTOR_DATA *d, char *name );
void	nanny			( DESCRIPTOR_DATA *d, char *argument );
bool	process_output		( DESCRIPTOR_DATA *d, bool fPrompt );
void	read_from_buffer	( DESCRIPTOR_DATA *d );
void	stop_idling		( CHAR_DATA *ch );
void    bust_a_prompt           ( CHAR_DATA *ch );
int 	log_area_popularity	( void );
int	ethos_check		( CHAR_DATA *ch );

static void	drop_link	( DESCRIPTOR_DATA *d );
static void	telnet_strip	( char *buf );
static int	write_partial	( int desc, const char *txt, int length );
static size_t	colour_expand	( char *dst, size_t dstsz, const char *src, CHAR_DATA *ch );
static void	log_player_ip	( CHAR_DATA *ch, const char *host );
static int	search_sockets	( DESCRIPTOR_DATA *inp );
static DESCRIPTOR_DATA *find_descriptor_by_name( const char *name, DESCRIPTOR_DATA *except );

/*
 * Sunucunun yazdığı dizinler yoksa oluşturulur.
 */
static void ensure_directories( void )
{
    static const char *dirs[] =
	{ "../player", "../gods", "../remort", "../log", "../log/ip", "../log/kanal",
	  "../log/events", "../data", NULL };
    int i;

    for ( i = 0; dirs[i] != NULL; i++ )
    {
	if ( !platform_mkdir( dirs[i] ) )
	{
	    fprintf( stderr, "Dizin oluşturulamadı: %s\n", dirs[i] );
	    exit( 1 );
	}
    }
}

int main( int argc, char **argv )
{
    int port;
    int control;

    boot_time = current_time = time( NULL );

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    port = 4000;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Kullanım: %s [port]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port 1024'ün üzerinde olmalı.\n" );
	    exit( 1 );
	}
    }

    ensure_directories( );
    ud_data_read( );

    /*
     * Run the game.
     */
    net_startup( );
    if ( ( control = net_listen( port ) ) < 0 )
	exit( 1 );
    boot_db( );
    bot_boot( );
    snprintf( log_buf, sizeof(log_buf), "Mangus %d portunda kullanıma hazır.", port );
    log_string( log_buf );
    game_loop( control );
    net_close( control );
    net_shutdown( );

    log_area_popularity( );
    log_string( "Oyun normal şekilde sonlandı." );
    return 0;
}

/*
 * Bağlantısı kopan / yazılamayan descriptor: oyundaki karakter kaydedilir,
 * bekleyen çıktı atılır ve soket kapatılır.
 */
static void drop_link( DESCRIPTOR_DATA *d )
{
    if ( d->character != NULL && d->character->level > 1 && d->connected == CON_PLAYING )
	save_char_obj( d->character );
    d->outtop = 0;
    close_socket( d );
}

void game_loop( int control )
{
    static int fds[NET_MAX_POLL];
    long long last_time;

    last_time = platform_now_us( );
    current_time = (time_t) ( last_time / 1000000 );

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;
	int count;

	/*
	 * Poll all active descriptors.
	 */
	fds[0] = control;
	count  = 1;
	for ( d = descriptor_list; d != NULL && count < NET_MAX_POLL; d = d->next )
	    fds[count++] = d->descriptor;

	if ( net_poll( fds, count, 0 ) < 0 )
	{
	    net_perror( "Game_loop: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( net_can_read( control ) )
	    init_descriptor( control );

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( d->outflow_error )
	    {
		drop_link( d );
		continue;
	    }

	    if ( net_can_read( d->descriptor ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    drop_link( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

		if (d->showstr_point)
		    show_string(d,d->incomm);
		else if ( d->connected == CON_PLAYING )
		    substitute_alias( d, d->incomm );
		else
		    nanny( d, d->incomm );

		d->incomm[0]	= '\0';
	    }
	}

	/*
	 * Autonomous game motion.
	 */
	update_handler( );

	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( d->outflow_error )
	    {
		drop_link( d );
		continue;
	    }

	    if ( ( d->fcommand || d->outtop > 0 ) && net_can_write( d->descriptor ) )
	    {
		if ( !process_output( d, TRUE ) )
		    drop_link( d );
	    }
	}

	/*
	 * Synchronize to a clock: sleep until the next pulse.
	 */
	{
	    long long next_pulse = last_time + 1000000 / PULSE_PER_SCD;
	    long long now = platform_now_us( );

	    if ( now < next_pulse )
		platform_sleep_us( next_pulse - now );
	}

	last_time    = platform_now_us( );
	current_time = (time_t) ( last_time / 1000000 );
    }
}

void init_descriptor( int control )
{
    char host[64];
    DESCRIPTOR_DATA *dnew;
    DESCRIPTOR_DATA *d;
    int desc;
    int count;

    if ( ( desc = net_accept( control, host, sizeof(host) ) ) < 0 )
	return;

    /*
     * Yoklama listesine (fds[0] dinleme soketi) sığmayan bağlantı hiç
     * okunamaz ve kapatılamazdı; doluysa baştan reddet.
     */
    for ( d = descriptor_list, count = 0; d != NULL; d = d->next )
	count++;
    if ( count + 1 >= NET_MAX_POLL )
    {
	write_to_descriptor( desc, "Sunucu dolu, lütfen daha sonra tekrar dene.\n\r", 0 );
	net_close( desc );
	snprintf( log_buf, sizeof(log_buf), "Sunucu dolu, bağlantı reddedildi: %s", host );
	log_string( log_buf );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    dnew = new_descriptor();

    dnew->descriptor	= desc;
    dnew->connected	= CON_GET_NAME;
    dnew->showstr_head	= NULL;
    dnew->showstr_point = NULL;
    dnew->outsize	= 2000;
    dnew->outbuf	= (char *)alloc_mem( dnew->outsize );
    dnew->host		= str_dup( host );

    snprintf( log_buf, sizeof(log_buf), "Sock.sinaddr:  %s", host );
    log_string( log_buf );

    /*
     * Ban check by site.
     */
    if ( check_ban(dnew->host,BAN_ALL))
    {
	write_to_descriptor( desc,
	    "Sitenizden yapılan bağlantılar engellenmiştir.\n\r", 0 );
	net_close( desc );
	free_descriptor(dnew);
	return;
    }

    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( dnew, help_greeting+1, 0 );
	else
	    write_to_buffer( dnew, help_greeting  , 0 );
    }
}



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
		"Kurbanın oyundan ayrıldı.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	snprintf(log_buf, sizeof(log_buf), "Bağlantı kapatılıyor: %s.", ch->name );
	log_string( log_buf );

	if (ch->pet &&
		( ch->pet->in_room == NULL
		|| ch->pet->in_room == get_room_index(ROOM_VNUM_LIMBO)) )
	{
		char_to_room( ch->pet, get_room_index(ROOM_VNUM_LIMBO));
		extract_char( ch->pet, TRUE);
	}
	if ( dclose->connected == CON_PLAYING )
	{
	    if (!IS_IMMORTAL(ch))
	       act( "$n bağlantısını kaybetti.", ch, NULL, NULL, TO_ROOM );
	    wiznet("$N bağlantısını kaybetti.",ch,NULL,WIZ_LINKS,0,0);
	    ch->desc = NULL;
	}
	else
	{
	    free_char( dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    net_close( dclose->descriptor );
    free_descriptor(dclose);
    return;
}


/*
 * Telnet komut dizilerini ham girdi tamponundan ayıklar (yerinde).
 * IAC IAC (gerçek 0xFF baytı) UTF-8'de geçersiz olduğundan atılır. Tamponun
 * sonunda yarım kalan dizi olduğu gibi bırakılır; bir sonraki okumada
 * tamamlanınca ayıklanır (temizlenmiş kısımda IAC kalmadığından yeniden
 * taramak zararsızdır).
 */
static void telnet_strip( char *buf )
{
    unsigned char *r = (unsigned char *) buf;
    unsigned char *w = r;
    bool partial = FALSE;

    while ( *r != '\0' )
    {
	unsigned char cmd;

	if ( *r != TELNET_IAC )
	{
	    *w++ = *r++;
	    continue;
	}

	cmd = r[1];
	if ( cmd == '\0' )
	{
	    partial = TRUE;
	    break;
	}

	if ( cmd == TELNET_IAC )
	{
	    r += 2;
	}
	else if ( cmd == TELNET_SB )
	{
	    unsigned char *p = r + 2;

	    while ( *p != '\0' && !( p[0] == TELNET_IAC && p[1] == TELNET_SE ) )
		p++;
	    if ( *p == '\0' )
	    {
		partial = TRUE;
		break;
	    }
	    r = p + 2;
	}
	else if ( cmd == TELNET_WILL || cmd == TELNET_WONT
	     ||   cmd == TELNET_DO   || cmd == TELNET_DONT )
	{
	    if ( r[2] == '\0' )
	    {
		partial = TRUE;
		break;
	    }
	    r += 3;
	}
	else
	{
	    /* iki baytlık komut: GA, NOP, AYT, ... */
	    r += 2;
	}
    }

    if ( partial )
	memmove( w, r, strlen( (char *) r ) + 1 );
    else
	*w = '\0';
}


bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    size_t iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	snprintf(log_buf, sizeof(log_buf), "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** BUNA BİR SON VER!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
    for ( ; ; )
    {
	int nRead;

	nRead = net_read( d->descriptor, d->inbuf + iStart, sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == NET_EOF )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( nRead == NET_WOULDBLOCK )
	    break;
	else
	{
	    net_perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }

    d->inbuf[iStart] = '\0';
    telnet_strip( d->inbuf );
    return TRUE;
}


/*
 * k. bayttan önceki kod noktasının başlangıcı (backspace bir harf siler).
 */
static int utf8_prev_boundary( const char *s, int k )
{
    while ( k > 0 && ( (unsigned char) s[k-1] & 0xC0 ) == 0x80 )
	k--;
    if ( k > 0 )
	k--;
    return k;
}

/*
 * Kesilmiş satırın sonunda yarım kalmış çok baytlı dizi varsa onu da atar.
 */
static int utf8_trim_partial( const char *s, int k )
{
    int lead = k;
    unsigned char c;
    int need;

    while ( lead > 0 && ( (unsigned char) s[lead-1] & 0xC0 ) == 0x80 )
	lead--;
    if ( lead == 0 )
	return k;
    c = (unsigned char) s[lead-1];
    need = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
    if ( k - ( lead - 1 ) < need )
	return lead - 1;
    return k;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;
    char buf[MAX_STRING_LENGTH];

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	unsigned char c = (unsigned char) d->inbuf[i];

	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Satır çok uzun.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    k = utf8_trim_partial( d->incomm, k );
	    break;
	}

	if ( c == '\b' && k > 0 )
	    k = utf8_prev_boundary( d->incomm, k );
	else if ( c >= 0x80 || isprint( c ) )
	    d->incomm[k++] = (char) c;
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /* Eski istemciler Latin-5 gönderebilir; her durumda geçerli UTF-8'e çevir. */
    utf8_from_latin5( d->incomm, sizeof(d->incomm) );

    /*
     * Deal with bozos with #repeat 1000 ...
     */

    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 25 )	/* corrected by chronos */
	    {
		snprintf(log_buf, sizeof(log_buf), "%s input spamming!", d->host );
		log_string( log_buf );
		if (d->character != NULL)
		{
		    snprintf(buf, sizeof(buf),"SPAM SPAM SPAM %s spamming, and OUT!",d->character->name);
		    wiznet(buf,d->character,NULL,WIZ_SPAM,0,get_trust(d->character));

		    snprintf(buf, sizeof(buf),"[%s]'s  Inlast:[%s] Incomm:[%s]!",
			d->character->name,d->inlast,d->incomm);
		    wiznet(buf,d->character,NULL,WIZ_SPAM,0,get_trust(d->character));

		    d->repeat = 0;

		    write_to_descriptor( d->descriptor,
			"\n\r*** BUNA BİR SON VER!!! ***\n\r", 0 );
		    close_socket( d );
		    return;
		}
	    }
	}
    }


    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	snprintf( d->incomm, sizeof(d->incomm), "%s", d->inlast );
    else
	snprintf( d->inlast, sizeof(d->inlast), "%s", d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}


/*
 * Kurbanın yara durumu (dövüş istemi).
 */
static const char *wound_text( int percent )
{
    static const struct { int pct; const char *txt; } table[] =
    {
	{ 100, "mükemmel durumda."	},
	{  90, "birkaç çiziği var."	},
	{  75, "birkaç kesiği var."	},
	{  50, "kanayan yaralarla kaplı." },
	{  30, "kan kaybediyor."	},
	{  15, "acı içinde bağırıyor."	},
	{   0, "acıyla sürünüyor."	},
    };
    size_t i;

    for ( i = 0; i < sizeof(table) / sizeof(table[0]); i++ )
	if ( percent >= table[i].pct )
	    return table[i].txt;
    return "ölmek üzere.";
}

/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    int n;

    /*
     * Bust a prompt. Önceki pulse'tan kalan (yazılamamış) veri tek başına
     * yeni bir istem gerektirmez.
     */
    if ( !merc_down && d->showstr_point )
    {
	if ( d->fcommand || d->outtop > d->outkept )
	    write_to_buffer( d, "\r" PROMPT_ENTER, 0 );
    }
    else if ( fPrompt && !merc_down && d->connected == CON_PLAYING
	 &&   ( d->fcommand || d->outtop > d->outkept ) )
    {
   	CHAR_DATA *ch;
	CHAR_DATA *victim;

	ch = d->character;

        /* battle prompt */
        if ((victim = ch->fighting) != NULL && can_see(ch,victim))
        {
            int percent;
	    char buf[MAX_STRING_LENGTH];

            if (victim->max_hit > 0)
                percent = victim->hit * 100 / victim->max_hit;
            else
                percent = -1;

            snprintf(buf, sizeof(buf),"%s %s \n\r",
	            IS_NPC(victim) ? victim->short_descr : victim->name, wound_text( percent ));
	    utf8_upper_first(buf, sizeof(buf));
            write_to_buffer( d, buf, 0);
        }


	ch = d->original ? d->original : d->character;
	if (!IS_SET(ch->comm, COMM_COMPACT) )
	    write_to_buffer( d, "\n\r", 2 );


        if ( IS_SET(ch->comm, COMM_PROMPT) )
            bust_a_prompt( d->character );

	if (IS_SET(ch->comm,COMM_TELNET_GA))
	    write_to_buffer(d,go_ahead_str,0);
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output. Gönderim tamponu dolarsa kalan veri bir sonraki
     * pulse'a bırakılır (yavaş istemci düşürülmez).
     */
    n = write_partial( d->descriptor, d->outbuf, d->outtop );
    if ( n < 0 )
    {
	d->outtop  = 0;
	d->outkept = 0;
	return FALSE;
    }
    if ( n < d->outtop )
    {
	memmove( d->outbuf, d->outbuf + n, d->outtop - n );
	d->outtop -= n;
	d->outbuf[d->outtop] = '\0';
    }
    else
	d->outtop = 0;
    d->outkept = d->outtop;
    return TRUE;
}

/*
 * Sınır denetimli dizgi ekleme: point'ten end'e kadar kopyalar.
 */
static char *append_bounded( char *point, const char *end, const char *i )
{
    while ( *i != '\0' && point < end )
	*point++ = *i++;
    return point;
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH * 4];
    const char *str;
    const char *i;
    char *point;
    const char *end = buf + sizeof(buf) - 1;
    CHAR_DATA *victim;

    /* İstem sabittir (suflör yalnızca açar/kapar); ch->prompt dosya uyumu için tutulur. */
    point = buf;
    str   = DEFAULT_PROMPT;

    while( *str != '\0' && point < end )
    {
	if( *str != '%' )
	{
	    *point++ = *str++;
	    continue;
	}
	++str;
	switch( *str )
	{
	    default :
		i = " "; break;
	    case '\0':
		i = ""; --str; break;
	    case 'o' :
		if ( ( victim = ch->fighting ) != NULL && victim->hit >= 0 )
		    snprintf(buf2, sizeof(buf2), "%d",((100 * victim->hit) / UMAX(1,victim->max_hit)));
		else
		    snprintf(buf2, sizeof(buf2),"0");
		i = buf2; break;
	    case 'h' :
		snprintf(buf2, sizeof(buf2), "%d", ch->hit );
		i = buf2; break;
	    case 'H' :
		snprintf(buf2, sizeof(buf2), "%d", ch->max_hit );
		i = buf2; break;
	    case 'm' :
		snprintf(buf2, sizeof(buf2), "%d", ch->mana );
		i = buf2; break;
	    case 'M' :
		snprintf(buf2, sizeof(buf2), "%d", ch->max_mana );
		i = buf2; break;
	    case 'v' :
		snprintf(buf2, sizeof(buf2), "%d", ch->move );
		i = buf2; break;
	    case 'V' :
		snprintf(buf2, sizeof(buf2), "%d", ch->max_move );
		i = buf2; break;
	    case '%' :
		i = "%"; break;
	}
	++str;
	point = append_bounded( point, end, i );
    }
    *point = '\0';
    colour_expand( buffer, sizeof(buffer), buf, ch );
    write_to_buffer( ch->desc, buffer, 0 );

    if (ch->prefix[0] != '\0')
	write_to_buffer(ch->desc,ch->prefix,0);
    return;
}


/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed. Taşmada bağlantı burada kapatılmaz
     * (çağıran d'yi kullanmaya devam eder); game_loop bayrağı görüp kapatır.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

        if (d->outsize >= MAX_OUTBUF)
	{
	    if ( !d->outflow_error )
		bug("Write_to_buffer: çıktı tamponu taştı, bağlantı kapatılacak.",0);
	    d->outflow_error = TRUE;
	    return;
 	}
	outbuf      = (char *)alloc_mem( 2 * d->outsize );
	memcpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    memcpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    d->outbuf[d->outtop] = '\0';
    return;
}


/*
 * Yazabildiği kadarını yazar; yazılan bayt sayısını, hatada -1 döndürür.
 */
static int write_partial( int desc, const char *txt, int length )
{
    int done = 0;

    while ( done < length )
    {
	int n = net_write( desc, txt + done, UMIN( length - done, WRITE_BLOCK ) );

	if ( n == NET_WOULDBLOCK || n == 0 )
	    break;
	if ( n < 0 )
	{
	    net_perror( "Write_to_descriptor" );
	    return -1;
	}
	done += n;
    }
    return done;
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    if ( length <= 0 )
	length = strlen(txt);

    return write_partial( desc, txt, length ) >= 0;
}


/*
 * Aynı hosttan ikinci bağlantı (ölümsüz, aynı karakter ya da ortak Discord
 * kimliği dışında) yasaktır.
 */
static int search_sockets( DESCRIPTOR_DATA *inp )
{
    DESCRIPTOR_DATA *d;

    if (IS_IMMORTAL(inp->character) )
	return 0;

    for(d=descriptor_list; d!=NULL; d=d->next)
    {
	if(!strcmp(inp->host, d->host))
	{
	    if ( d->character && inp->character )
	    {
		if (!str_cmp(inp->character->name,d->character->name))
		    continue;
		if (get_trust(inp->character) >= LEVEL_IMMORTAL || get_trust(d->character) >= LEVEL_IMMORTAL)
		    continue;
		if(inp->character->pcdata->discord_id[0] != '\0' && d->character->pcdata->discord_id[0] != '\0' && strcmp(inp->character->pcdata->discord_id,d->character->pcdata->discord_id))
		    continue;
	    }
	    return 1;
	}
    }
    return 0;
}


/*
 * Aynı adlı karakteri taşıyan (except dışındaki) descriptor.
 */
static DESCRIPTOR_DATA *find_descriptor_by_name( const char *name, DESCRIPTOR_DATA *except )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d != except
	&&   d->character != NULL
	&&   !str_cmp( name, d->original ? d->original->name : d->character->name ) )
	    return d;
    }
    return NULL;
}


/*
 * ---------------------------------------------------------------------
 * Giriş durum makinesi (nanny). Her CON_* durumu ayrı bir fonksiyondur;
 * nanny_table durumu fonksiyona bağlar.
 * ---------------------------------------------------------------------
 */
typedef void NANNY_FUN( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument );

/*
 * Karakteri at, isim sorusuna dön.
 */
static void nanny_restart( DESCRIPTOR_DATA *d, const char *prompt )
{
    if ( prompt != NULL )
	write_to_buffer( d, prompt, 0 );
    if ( d->character != NULL )
    {
	free_char( d->character );
	d->character = NULL;
    }
    d->connected = CON_GET_NAME;
}

/*
 * "yardım [konu]" alt komutu: konu verilmemişse topic, verilmişse o konu
 * gösterilir ve prompt yinelenir. Eşleşirse TRUE.
 */
static bool nanny_help( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument,
			const char *topic, const char *prompt )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;

    rest = one_argument( argument, arg );
    if ( str_cmp( arg, "yardım" ) && str_cmp( arg, "help" ) )
	return FALSE;

    if ( rest[0] == '\0' )
	do_help( ch, (char *) topic );
    else
	do_help( ch, rest );
    write_to_buffer( d, prompt, 0 );
    return TRUE;
}

/*
 * Irk listesi: Mangus Mud'un oyuncu ırkları, satır başına RACE_LIST_PER_LINE.
 */
static void nanny_race_list( DESCRIPTOR_DATA *d )
{
    int race;

    write_to_buffer(d,"Aşağıdakilerden birini seçebilirsin:\n\r  ",0);
    for ( race = 1; race_table[race].name[0] != NULL; race++ )
    {
	if (!race_table[race].pc_race)
	    break;
	if ( race > 1 && ( race - 1 ) % RACE_LIST_PER_LINE == 0 )
	    write_to_buffer(d,"\n\r  ",0);
	write_to_buffer(d,"(",0);
	write_to_buffer(d,race_table[race].name[1],0);
	write_to_buffer(d,") ",0);
    }
    write_to_buffer(d,"\n\r",0);
}

/*
 * Irk seçimi istemi (yeni karakter ve yeniyaşam ortak).
 */
static void nanny_ask_race( DESCRIPTOR_DATA *d, CHAR_DATA *ch, const char *intro )
{
    char buf[MAX_STRING_LENGTH];

    snprintf(buf, sizeof(buf), intro, MAX_PC_RACE - 1);
    write_to_buffer( d, buf, 0);
    write_to_buffer( d, "\n\r", 0);
    do_help(ch,"ırklar");
    d->connected = CON_GET_NEW_RACE;
}

/*
 * Etkin iki kat TP/GP duyurusu.
 */
static void announce_events( CHAR_DATA *ch )
{
    if( ikikat_tp > 0 )
	printf_to_char( ch , "\n\r{Cİki kat TP kazanma etkinliği etkin. Kalan süre %d dakika.{x\n\r\n\r" , ikikat_tp );
    if( ikikat_gp > 0 )
	printf_to_char( ch , "\n\r{Cİki kat GP kazanma etkinliği etkin. Kalan süre %d dakika.{x\n\r\n\r" , ikikat_gp );
}

/*
 * Oyuncunun bağlandığı adresi ../log/ip/<isim> dosyasına ekler.
 */
static void log_player_ip( CHAR_DATA *ch, const char *host )
{
    FILE *fp;
    char path[MAX_INPUT_LENGTH];
    char stamp[32];

    snprintf( path, sizeof(path), "%s%s", IP_DIR, ch->name );
    if ( ( fp = fopen( path, "a" ) ) == NULL )
    {
	bug( "log_player_ip: ip günlüğü açılamadı.", 0 );
	return;
    }
    platform_time_str( current_time, stamp, sizeof(stamp) );
    fprintf( fp, "%s %s\n", stamp, host );
    fclose( fp );
}

static void nanny_get_name( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    bool fOld;

    if ( argument[0] == '\0' )
    {
	close_socket( d );
	return;
    }

    argument[0] = UPPER(argument[0]);
    if ( !check_parse_name( argument ) )
    {
	write_to_buffer( d, "Kurallara uygun olmayan isim, başka bir tane deneyin.\n\r" PROMPT_NAME, 0 );
	return;
    }

    if ( bot_name_taken( argument ) )
    {
	write_to_buffer( d, PROMPT_NAME_TAKEN, 0 );
	return;
    }

    fOld = load_char_obj( d, argument );
    ch   = d->character;

    if (get_trust(ch) < LEVEL_IMMORTAL)
    {
	if ( check_ban(d->host,BAN_PLAYER))
	{
	    write_to_buffer( d,"Sitenizden oyuncu girişi engellenmiştir.\n\r",0);
	    close_socket( d );
	    return;
	}

	if(search_sockets(d))
	{
	    write_to_buffer(d, "Çift karakterle oynamak yasak.\n\r", 0);
	    close_socket(d);
	    return;
	}
    }

    if ( IS_SET(ch->act, PLR_DENY) )
    {
	snprintf(log_buf, sizeof(log_buf), "Denying access to %s@%s.", argument, d->host );
	log_string( log_buf );
	write_to_buffer( d, "Erişiminiz engellendi.\n\r", 0 );
	close_socket( d );
	return;
    }

    if ( check_reconnect( d, argument, FALSE ) )
    {
	fOld = TRUE;
    }
    else
    {
	if ( wizlock && !IS_HERO(ch))
	{
	    write_to_buffer( d, "Oyun ölümsüzler tarafından kilitlenmiş.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if ( !IS_IMMORTAL(ch) && !IS_SET(ch->act,PLR_CANINDUCT) )
	{
	    if (iNumPlayers >= max_oldies && fOld)
	    {
		snprintf(buf, sizeof(buf),
		    "\n\rŞu anda en fazla %i oyuncudan %i tanesi oyunda.\n\r"
		    "Lütfen biraz sonra tekrar dene.\n\r", max_oldies, iNumPlayers);
		write_to_buffer(d, buf, 0);
		close_socket(d);
		return;
	    }

	    if (iNumPlayers >= max_newbies && !fOld)
	    {
		snprintf(buf, sizeof(buf),
		    "\n\rŞu anda %i oyuncu oyunda. Yeni karakter yaratılması oyuncu sayısı\n\r"
		    "%i'nin altındayken mümkündür. Lütfen biraz sonra tekrar dene.\n\r",
		    iNumPlayers, max_newbies);
		write_to_buffer(d, buf, 0);
		close_socket(d);
		return;
	    }
	}
    }

    if ( fOld )
    {
	/* Old player */
	write_to_buffer( d, echo_off_str, 0 );
	write_to_buffer( d, "Parola: ", 0 );
	d->connected = CON_GET_OLD_PASSWORD;
	return;
    }

    /* New player */
    if (newlock)
    {
	write_to_buffer( d, "Yeni karakter girişi kapalıdır.\n\r", 0 );
	close_socket( d );
	return;
    }

    if (check_ban(d->host,BAN_NEWBIES))
    {
	write_to_buffer(d, "Sitenizden yeni karakter yaratılamamaktadır.\n\r",0);
	close_socket(d);
	return;
    }

    if ( find_descriptor_by_name( argument, d ) != NULL )
    {
	nanny_restart( d, PROMPT_NAME_TAKEN );
	return;
    }

    do_help(ch,"isim");
    snprintf(buf, sizeof(buf), "\n\rDoğru anladım mı, %s (E/H)? ", argument );
    write_to_buffer( d, buf, 0 );
    d->connected = CON_CONFIRM_NEW_NAME;
}

static void nanny_old_password( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int obj_count;
    int obj_count2;
    bool fOld;

    write_to_buffer( d, "\n\r", 2 );

    if ( !pwd_check( ch->pcdata->pwd, argument ) )
    {
	write_to_buffer( d, "Yanlış şifre.\n\r", 0 );
	snprintf(buf, sizeof(buf), "Wrong password by %s@%s", ch->name, d->host);
	log_string(buf);
	if ( ++d->login_tries >= MAX_LOGIN_TRIES )
	{
	    close_socket( d );
	}
	else
	{
	    write_to_buffer( d, "Şifre: ", 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	}
	return;
    }

    if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
    {
	write_to_buffer( d, "Uyarı! Parola boş!\n\r",0 );
	write_to_buffer( d, "Lütfen durumu 'hata' komutuyla bildir.\n\r",0);
	write_to_buffer( d, "Düzeltmek için 'parola null <yeni parola>' yaz.\n\r",0);
    }

    write_to_buffer( d, echo_on_str, 0 );

    if ( check_reconnect( d, ch->name, TRUE ) )
	return;

    if ( check_playing( d, ch->name ) )
	return;

    /* Count objects in loaded player file */
    for (obj = ch->carrying,obj_count = 0; obj != NULL; obj = obj->next_content)
	obj_count += get_obj_realnumber(obj);

    snprintf(buf, sizeof(buf), "%s", ch->name);

    free_char(ch);
    fOld = load_char_obj( d, buf );
    ch   = d->character;

    if (!fOld)
    {
	write_to_buffer(d, "Yeni karakter yaratmak için yeniden giriş yap.\n\r", 0);
	close_socket(d);
	return;
    }

    /* Eski crypt(3) özetini yeni biçime yükselt; oyuncu kaydedilince kalıcı olur. */
    if ( pwd_is_legacy( ch->pcdata->pwd ) )
    {
	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd = str_dup( pwd_hash( argument ) );
    }

    /* Count objects in refreshed player file */
    for (obj = ch->carrying,obj_count2 = 0; obj != NULL; obj = obj->next_content)
	obj_count2 += get_obj_realnumber(obj);

    snprintf(log_buf, sizeof(log_buf), "%s@%s baglandi.", ch->name, d->host );
    log_string( log_buf );

    if ( IS_HERO(ch) )
    {
	do_help( ch, "imotd" );
	d->connected = CON_READ_IMOTD;
    }
    else
    {
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
    }

    /* This player tried to use the clone cheat --
     * Log in once, connect a second time and enter only name,
     * drop all and quit with first character, finish login with second.
     * This clones the player's inventory.
     */
    if (obj_count != obj_count2)
    {
	snprintf(log_buf, sizeof(log_buf), "%s@%s tried to use the clone cheat.", ch->name, d->host );
	log_string( log_buf );
	send_to_char("Tanrılar yaptığına kaş çatıyor.\n\r",ch);
    }
}

/*
 * Yeniyaşam sonrası sınıf seçimi: yalnızca büyücü sınıfları seçilebilir.
 */
static void nanny_new_classes( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int iClass;

    iClass = class_lookup(argument);
    if ( nanny_help( d, ch, argument, "new classes",
		     "Sınıfınız nedir (bilgi için: " SITE_INFO ")? " ) )
	return;

    if ( iClass == -1 )
    {
	write_to_buffer( d,
	    "Bu bir sınıf değil.\n\r"
	    "Sınıfınız NEDİR (Gan/Biçimci/Ögeci)? ", 0 );
	return;
    }

    if (iClass != CLASS_INVOKER
    &&  iClass != CLASS_TRANSMUTER
    &&  iClass != CLASS_ELEMENTALIST )
    {
	write_to_buffer(d, "Bu sınıf yasaklı.\n\rBaşka bir tane seçin:",0);
	return;
    }

    ch->iclass = iClass;

    ch->pcdata->points = class_table[iClass].points
	    + class_table[ORG_RACE(ch)].points;
    snprintf(buf, sizeof(buf), "Artık sınıfın %s.\n\r", class_table[iClass].name[1]);
    write_to_buffer(d, buf, 0 );
    write_to_buffer( d, PROMPT_ENTER, 0 );
    d->connected = CON_READ_MOTD;
}

/* RT code for breaking link */
static void nanny_break_connect( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d_old, *d_old_next;

    switch( *argument )
    {
    case 'e' : case 'E':
	for ( d_old = descriptor_list; d_old != NULL; d_old = d_old_next )
	{
	    d_old_next = d_old->next;
	    if (d_old == d || d_old->character == NULL)
		continue;

	    if (str_cmp(ch->name,d_old->character->name))
		continue;

	    close_socket(d_old);
	}
	if (check_reconnect(d,ch->name,TRUE))
	    return;
	nanny_restart( d, "Tekrar bağlanılamadı.\n\r" PROMPT_NAME );
	break;

    case 'h' : case 'H':
	nanny_restart( d, PROMPT_NAME );
	break;

    default:
	write_to_buffer(d,"Lütfen cevap verin (E-H). ",0);
	break;
    }
}

static void nanny_confirm_new_name( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    switch ( *argument )
    {
    case 'e': case 'E':
	write_to_buffer( d, echo_off_str, 0 );
	snprintf(buf, sizeof(buf), "\n\rTeşekkürler.\n\r%s karakteri için bir şifre girin: ",
	    ch->name );
	write_to_buffer( d, buf, 0 );
	d->connected = CON_GET_NEW_PASSWORD;
	break;

    case 'h': case 'H':
	nanny_restart( d, "O halde uygun bir isim gir: " );
	break;

    default:
	write_to_buffer( d, "Cevabın nedir ( E - H )? ", 0 );
	break;
    }
}

static void nanny_get_new_password( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    const char *pwdnew;

    write_to_buffer( d, "\n\r", 2 );

    if ( strlen(argument) < 5 )
    {
	write_to_buffer( d,
	    "Şifre en az 5 karakter uzunluğunda olmalıdır.\n\rŞifre: ", 0 );
	return;
    }

    pwdnew = pwd_hash( argument );
    if ( strchr( pwdnew, '~' ) != NULL )
    {
	write_to_buffer( d,
	    "Girdiğiniz şifre kabul edilebilir değil.\n\rLütfen işlemi tekrarlayın.\n\rŞifre: ", 0 );
	return;
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd	= str_dup( pwdnew );
    write_to_buffer( d, "Lütfen şifreyi tekrar girin: ", 0 );
    d->connected = CON_CONFIRM_NEW_PASSWORD;
}

static void nanny_confirm_new_password( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    write_to_buffer( d, "\n\r", 2 );

    if ( !pwd_check( ch->pcdata->pwd, argument ) )
    {
	write_to_buffer( d, "\n\rGirilen şifreler eşleşmiyor.\n\rLütfen işlemi tekrarlayın.\n\rŞifre: ", 0 );
	d->connected = CON_GET_NEW_PASSWORD;
	return;
    }

    write_to_buffer( d, echo_on_str, 0 );
    nanny_ask_race( d, ch, "Mangus Mud %d farklı ırka ev sahipliği yapar. Irkların özeti:" );
}

static void nanny_remorting( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    SET_BIT( ch->act, PLR_REMORTED );
    nanny_ask_race( d, ch, "Bildiğin gibi Mangus Mud'da %d farklı ırk bulunmaktadır:" );
}

static void nanny_get_new_race( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int race, i;

    one_argument(argument,arg);

    if (!str_cmp(arg,"yardım"))
    {
	argument = one_argument(argument,arg);
	if (argument[0] == '\0')
	{
	    write_to_buffer( d, "Aşağıda ırk listesi verilmiştir. Lütfen seçiniz:\n\n\r", 0);
	    do_help(ch,"ırklar");
	}
	else
	{
	    do_help(ch,argument);
	    write_to_buffer(d, PROMPT_RACE,0);
	}
	return;
    }

    race = race_lookup(argument);

    if (race == 0 || !race_table[race].pc_race)
    {
	write_to_buffer(d,"\n\rGeçerli bir ırk seçmedin.\n\r",0);
	nanny_race_list( d );
	write_to_buffer(d, PROMPT_RACE,0);
	return;
    }

    ch->pcdata->race = race;
    RACE(ch) = race;
    for (i=0; i < MAX_STATS;i++)
	ch->mod_stat[i] = 0;

    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;

    ch->detection   = ch->affected_by|race_table[race].det;
    ch->affected_by = ch->affected_by|race_table[race].aff;
    ch->imm_flags	= ch->imm_flags|race_table[race].imm;
    ch->res_flags	= ch->res_flags|race_table[race].res;
    ch->vuln_flags	= ch->vuln_flags|race_table[race].vuln;
    ch->form	= race_table[race].form;
    ch->parts	= race_table[race].parts;

    /* add skills */
    for (i = 0; i < MAX_RACE_SKILLS; i++)
    {
	if (race_table[race].skills[i] == NULL)
	    break;
	ch->pcdata->learned[skill_lookup(race_table[race].skills[i])] = 100;
    }
    /* add cost */

    ch->pcdata->points = race_table[race].points;

    ch->size = race_table[race].size;

    ch->pcdata->familya[race] = NEW_PC_FAMILYA;

    write_to_buffer( d, "\n\rIrk seçimi tamam.\n\rPeki karakterin cinsiyeti ne olsun ( E - K )? ", 0 );
    d->connected = CON_GET_NEW_SEX;
}

static void nanny_get_new_sex( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    size_t len;
    int iClass;

    switch ( argument[0] )
    {
    case 'e': case 'E': ch->sex = SEX_MALE;
			ch->pcdata->true_sex = SEX_MALE;
			break;
    case 'k': case 'K': ch->sex = SEX_FEMALE;
			ch->pcdata->true_sex = SEX_FEMALE;
			break;
    default:
	write_to_buffer( d, "Seçimin geçerli bir cinsiyet değil.\n\rKarakterin cinsiyeti ne olsun ( E - K )? ", 0 );
	return;
    }

    do_help(ch,"sınıflar");

    len = (size_t) snprintf( buf, sizeof(buf), "Bir sınıf seçin:\n\r[ " );
    for ( iClass = 0; iClass < MAX_CLASS && len < sizeof(buf); iClass++ )
	len += (size_t) snprintf( buf + len, sizeof(buf) - len, "%s ", class_table[iClass].name[1] );
    if ( len < sizeof(buf) )
	snprintf( buf + len, sizeof(buf) - len, "]\n\r " );
    write_to_buffer( d, buf, 0 );
    write_to_buffer(d, PROMPT_CLASS,0);
    d->connected = CON_GET_NEW_CLASS;
}

static void nanny_get_new_class( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int iClass, i;

    iClass = class_lookup(argument);
    if ( nanny_help( d, ch, argument, "sınıflar", PROMPT_CLASS ) )
	return;

    if ( iClass == -1 )
    {
	write_to_buffer( d, "\n\rBu bir sınıf değil.\n\rKarakterinin sınıfı ne olsun? ", 0 );
	return;
    }

    ch->iclass = iClass;

    ch->pcdata->points += class_table[iClass].points;
    snprintf(buf, sizeof(buf), "Tebrikler! Karakterin %s sınıfından.\n\r", class_table[iClass].name[1]);
    write_to_buffer(d, buf, 0 );

    for (i=0; i < MAX_STATS; i++)
	ch->perm_stat[i] = get_max_train(ch,i);

    ch->perm_stat[STAT_CHA] = NEW_PC_CHA;

    write_to_buffer( d, "\n\r", 2 );
    write_to_buffer( d, "Sıra geldi karakterin için yönelim seçmeye. Yönelim, basit\n\r",0);
    write_to_buffer( d, "bir ifadeyle karakterin topluma ve doğaya karşı davranış\n\r",0);
    write_to_buffer( d, "biçimini belirler. Ayrıntılı bilgiye siteden ulaşabilirsin.\n\r\n\r",0);
    write_to_buffer( d, "Üç çeşit yönelim vardır:\n\r",0);
    write_to_buffer( d, "iyi, yansız ve kem\n\r\n\r",0);
    write_to_buffer( d, "Karakterinin yöneliminin ne olmasını istiyorsun ( i - y - k )? ",0);
    d->connected = CON_GET_ALIGNMENT;
}

static void nanny_get_alignment( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    uint32_t first;

    utf8_decode( argument, &first );
    switch( utf8_fold_cp( first ) )
    {
    case 'i' :
	ch->alignment = 1000;
	write_to_buffer(d, "Karakterinin yönelimi 'iyi'.\n\r",0);
	break;
    case 'y' :
	ch->alignment = 0;
	write_to_buffer(d, "Karakterinin yönelimi 'yansız'.\n\r",0);
	break;
    case 'k' :
	ch->alignment = -1000;
	write_to_buffer(d, "Karakterinin yönelimi 'kem'.\n\r",0);
	break;
    default:
	write_to_buffer(d,"Geçerli bir yönelim değil.\n\r",0);
	write_to_buffer(d,"Karakterinin yöneliminin ne olmasını istiyorsun ( i - y - k )? ",0);
	return;
    }
    write_to_buffer( d, "\n\r" PROMPT_ENTER, 0 );
    d->ethos_intro = TRUE;	/* ilk ENTER'da etik açıklaması gösterilir */
    ch->hometown = 0;
    d->connected = CON_GET_ETHOS;
}

static void nanny_get_ethos( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    const char *align = IS_GOOD(ch) ? "iyi" : IS_EVIL(ch) ? "kem" : "yansız";

    if ( d->ethos_intro )
    {
	d->ethos_intro = FALSE;
	if (!ethos_check(ch))
	{
	    write_to_buffer( d, "Sıra geldi karakterin için etik seçmeye. Etik, basit\n\r",0);
	    write_to_buffer( d, "bir ifadeyle karakterin kanunlara karşı davranış\n\r",0);
	    write_to_buffer( d, "biçimini belirler. Ayrıntılı bilgiye siteden ulaşabilirsin.\n\r\n\r",0);
	    write_to_buffer( d, "Üç çeşit etik vardır:\n\r",0);
	    write_to_buffer( d, "Tüze: kanunların toplum yaşamı için vazgeçilmez olduğuna inanır.\n\r",0);
	    write_to_buffer( d, "Yansız: kanunların varlığı veya yokluğuyla ilgilenmez.\n\r",0);
	    write_to_buffer( d, "Kaos: kanunların özgürlüğün ve iradenin önünde engel olduğuna inanır.\n\r\n\r",0);
	    write_to_buffer( d, "Karakterinin etiğinin ne olmasını istiyorsun ( t - y - k )? ",0);
	    d->connected = CON_GET_ETHOS;
	    return;
	}
    }
    else
    {
	switch(argument[0])
	{
	case 'T': case 't':
	    snprintf(buf, sizeof(buf),"\n\rArtık felsefen tüze-%s.\n\r", align);
	    write_to_buffer(d, buf, 0);
	    ch->ethos = 1;
	    break;
	case 'Y': case 'y':
	    snprintf(buf, sizeof(buf),"\n\rArtık felsefen yansız-%s.\n\r", align);
	    write_to_buffer(d, buf, 0);
	    ch->ethos = 2;
	    break;
	case 'K': case 'k':
	    snprintf(buf, sizeof(buf),"\n\rArtık felsefen kaos-%s.\n\r", align);
	    write_to_buffer(d, buf, 0);
	    ch->ethos = 3;
	    break;
	default:
	    write_to_buffer(d, "\n\rGeçerli bir etik seçmedin.\n\r", 0);
	    write_to_buffer(d, "Etiğin ne olsun, (T/Y/K) (bilgi: " SITE_INFO ")?",0);
	    return;
	}
    }
    write_to_buffer( d, "\n\r" PROMPT_ENTER, 0 );
    d->connected = CON_CREATE_DONE;
}

static void nanny_create_done( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    snprintf(log_buf, sizeof(log_buf), "%s@%s new player.", ch->name, d->host );
    log_string( log_buf );
    group_add(ch);
    ch->pcdata->learned[gsn_recall] = NEW_PC_RECALL_SKILL;
    write_to_buffer( d, "\n\r", 2 );
    do_help(ch,"genel");
    write_to_buffer( d, PROMPT_ENTER, 0 );
    d->connected = CON_READ_NEWBIE;
}

static void nanny_read_newbie( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    write_to_buffer( d, "\n\r", 2 );
    do_help( ch, "motd" );
    d->connected = CON_READ_MOTD;
}

static void nanny_read_imotd( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    write_to_buffer(d,"\n\r",2);
    do_help( ch, "motd" );
    d->connected = CON_READ_MOTD;
}

/*
 * Yeni karakterin ilk giriş hazırlığı.
 */
static void nanny_first_login( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    SET_BIT(ch->act,PLR_AUTOEXIT);
    SET_BIT(ch->act,PLR_AUTOAKCE);
    ch->level	= 1;
    ch->exp     = base_exp(ch,ch->pcdata->points);
    ch->hit	= ch->max_hit;
    ch->mana	= ch->max_mana;
    ch->move	= ch->max_move;
    ch->pcdata->perm_hit  = ch->max_hit;
    ch->pcdata->perm_mana = ch->max_mana;
    ch->pcdata->perm_move = ch->max_move;
    ch->train	+= NEW_PC_TRAINS;
    ch->practice += NEW_PC_PRACTICES;
    ch->pcdata->death = 0;

    snprintf(buf, sizeof(buf), "%s", title_table [ch->iclass] [ch->level]);
    set_title( ch, buf );

    obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);
    obj_to_char(create_object(get_obj_index(OBJ_VNUM_NMAP1),0),ch);
    obj_to_char(create_object(get_obj_index(OBJ_VNUM_NMAP2),0),ch);
    obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP_SM),0),ch);

    ch->pcdata->learned[get_weapon_sn(ch,FALSE)] = NEW_PC_WEAPON_SKILL;

    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
    send_to_char("\n\r",ch);
    do_help(ch, "yeni oyuncu");
    send_to_char("\n\r",ch);

    /* son 14 gun icin birer saat oynama suresi bonus olarak verilsin. */
    init_play_log( ch->pcdata, current_time, NEW_PC_BONUS_MINUTES );

    do_outfit(ch,"");
}

static void nanny_read_motd( DESCRIPTOR_DATA *d, CHAR_DATA *ch, char *argument )
{
    int i;

    write_to_buffer( d, "\n\rMangus'a hoşgeldin. Ölümün tadını çıkar!!...\n\r", 0 );
    ch->next	= char_list;
    char_list	= ch;
    d->connected	= CON_PLAYING;
    announce_events( ch );

    if (!IS_NPC(ch) && strlen(ch->pcdata->discord_id) < DISCORD_ID_MIN_LEN)
    {
	printf_to_char( ch , "\n\r{CDiscord ID'niz kayıtlı değil. 'discord' komutuyla discord kullanıcı ID'nizi kaydediniz.{x\n\r\n\r" );
    }

    log_player_ip( ch, d->host );
    cevrimici_oyuncu_sayisi( );
    iNumPlayers++;

    if ( ch->level == 0 )
    {
	nanny_first_login( ch );
    }
    else if ( ch->in_room != NULL )
    {
	if ( cabal_area_check(ch) )
	{
	    i = IS_GOOD(ch) ? 0 : IS_EVIL(ch) ? 2 : 1;
	    char_to_room(ch, get_room_index(hometown_table[ch->hometown].altar[i]));
	}
	else char_to_room( ch, ch->in_room );
    }
    else if ( IS_IMMORTAL(ch) )
    {
	char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
    }
    else
    {
	char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
    }

    reset_char(ch);
    if  (!IS_IMMORTAL(ch))
	act( "$n mud'a giriş yaptı.", ch, NULL,NULL, TO_ROOM );

    wiznet("$N gerçekliğe giriş yaptı.",ch,NULL,WIZ_LOGINS,0,0);

    if ( ch->exp < (exp_per_level(ch,ch->pcdata->points) * ch->level ) )
    {
	ch->exp = (ch->level) * (exp_per_level(ch,ch->pcdata->points));
    }
    else if ( ch->exp > (exp_per_level(ch,ch->pcdata->points) * (ch->level + 1)) )
    {
	ch->exp = (ch->level + 1) * (exp_per_level(ch,ch->pcdata->points));
	ch->exp -= 10;
    }

    if (IS_QUESTOR(ch) && ch->pcdata->questmob == 0)
    {
	ch->pcdata->nextquest = ch->pcdata->countdown;
	REMOVE_BIT(ch->act,PLR_QUESTOR);
    }
    if(!IS_NPC(ch) && ch->pcdata->familya[ch->pcdata->race] < NEW_PC_FAMILYA)
    {
	ch->pcdata->familya[ch->pcdata->race] = NEW_PC_FAMILYA;
    }

    if (IS_SET(ch->act,PLR_NO_EXP))	REMOVE_BIT(ch->act,PLR_NO_EXP);
    if (IS_SET(ch->act,PLR_CHANGED_AFF)) REMOVE_BIT(ch->act,PLR_CHANGED_AFF);

    for (i = 0; i < MAX_STATS; i++)
    {
	if ( ch->perm_stat[i] > get_max_train(ch,i) )
	{
	    ch->train += ( ch->perm_stat[i] - get_max_train(ch,i) );
	    ch->perm_stat[i] = get_max_train(ch,i);
	}
    }

    do_look( ch, "auto" );

    if (ch->pet != NULL)
    {
	char_to_room(ch->pet,ch->in_room);
	act("$n mud'a giriş yaptı.",ch->pet,NULL,NULL,TO_ROOM);
    }

    if (ch->pcdata->confirm_delete)
    {
	send_to_char("Karakterine bir miktar bonus oynama zamanı verildi.\n\r",ch);
	ch->pcdata->confirm_delete = FALSE;
    }
}

static const struct
{
    int		con;
    NANNY_FUN *	fn;
} nanny_table[] =
{
    { CON_GET_NAME,		nanny_get_name		},
    { CON_GET_OLD_PASSWORD,	nanny_old_password	},
    { CON_NEW_CLASSES,		nanny_new_classes	},
    { CON_BREAK_CONNECT,	nanny_break_connect	},
    { CON_CONFIRM_NEW_NAME,	nanny_confirm_new_name	},
    { CON_GET_NEW_PASSWORD,	nanny_get_new_password	},
    { CON_CONFIRM_NEW_PASSWORD,	nanny_confirm_new_password },
    { CON_REMORTING,		nanny_remorting		},
    { CON_GET_NEW_RACE,		nanny_get_new_race	},
    { CON_GET_NEW_SEX,		nanny_get_new_sex	},
    { CON_GET_NEW_CLASS,	nanny_get_new_class	},
    { CON_GET_ALIGNMENT,	nanny_get_alignment	},
    { CON_GET_ETHOS,		nanny_get_ethos		},
    { CON_CREATE_DONE,		nanny_create_done	},
    { CON_READ_NEWBIE,		nanny_read_newbie	},
    { CON_READ_IMOTD,		nanny_read_imotd	},
    { CON_READ_MOTD,		nanny_read_motd		},
    { -1,			NULL			}
};

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    int i;

    while ( isspace( (unsigned char) *argument ) )
	argument++;

    for ( i = 0; nanny_table[i].fn != NULL; i++ )
    {
	if ( nanny_table[i].con == d->connected )
	{
	    (*nanny_table[i].fn)( d, d->character, argument );
	    return;
	}
    }

    bug( "Nanny: bad d->connected %d.", d->connected );
    close_socket( d );
}


/*
 * Parse a name for acceptability.
 *
 * Ad politikası: yalnızca ASCII harf, 2-12 karakter. Ad oyuncu dosyasının adı
 * olur (capitalize ASCII kalır) ve kabuk/dosya sistemine güvenle gider; Türkçe
 * harfli adlar bilerek kabul edilmez.
 */
bool check_parse_name( char *name )
{
    size_t len = strlen(name);

    /*
     * Reserved words.
     */
    if ( is_name( name,
	"tümü oto immortal ölümsüz self someone something the you demise balance circle loner honor") )
	return FALSE;

    /*
     * Length restrictions.
     */
    if ( len < 2 || len > 12 )
	return FALSE;

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	const char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	size_t total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    unsigned char c = (unsigned char) *pc;

	    if ( c >= 0x80 || !isalpha( c ) )
		return FALSE;

	    if ( isupper( c ) ) /* ugly anti-caps hack */
	    {
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( tolower( c ) != 'i' && tolower( c ) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > len / 2 && len < 3))
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !IS_BOT(ch)
	&&   !str_cmp( name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char(
		    "Tekrar bağlanıyor. Kaçırdığın konuşmalar için 'tekrarla' diyebilirsin.\n\r", ch );
		if (!IS_IMMORTAL(ch))
		    act( "$n yeniden bağlandı.", ch, NULL, NULL, TO_ROOM );
		if ( get_light_char(ch) != NULL )
		    --ch->in_room->light;

		snprintf(log_buf, sizeof(log_buf), "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		wiznet("$N bağlantısını yeniledi.",
		    ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "Bu karakter zaten oyunda.\n\r",0);
	    write_to_buffer( d, "Yine de bağlanmak istiyor musunuz (E/H)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n hiçlikten döndü.", ch, NULL, NULL, TO_ROOM );
    return;
}


/*
 * ---------------------------------------------------------------------
 * Renk kodları ({x, {r ...). Tek açılım fonksiyonu; hedef tampon sınırı
 * her zaman bilinir, dizgi sonundaki '{' NUL ötesine geçmez.
 * ---------------------------------------------------------------------
 */
static const struct
{
    char	key;
    const char *code;
} colour_table[] =
{
    { 'x', CLEAR	},
    { 'b', C_BLUE	},
    { 'c', C_CYAN	},
    { 'g', C_GREEN	},
    { 'm', C_MAGENTA	},
    { 'r', C_RED	},
    { 'w', C_WHITE	},
    { 'y', C_YELLOW	},
    { 'B', C_B_BLUE	},
    { 'C', C_B_CYAN	},
    { 'G', C_B_GREEN	},
    { 'M', C_B_MAGENTA	},
    { 'R', C_B_RED	},
    { 'W', C_B_WHITE	},
    { 'Y', C_B_YELLOW	},
    { 'D', C_D_GREY	},
    { '*', "\007"	},
    { '/', "\012"	},
    { '{', "{"		},
    { '\0', NULL	}
};

/*
 * Renk kodunun ANSI karşılığı; yaratıklar için boş, bilinmeyen kod CLEAR.
 */
static const char *colour_code( char type, CHAR_DATA *ch )
{
    int i;

    if ( IS_NPC( ch ) )
	return "";
    for ( i = 0; colour_table[i].code != NULL; i++ )
	if ( colour_table[i].key == type )
	    return colour_table[i].code;
    return CLEAR;
}

/*
 * src içindeki {x kodlarını açarak dst'ye (en fazla dstsz bayt, NUL dahil)
 * yazar; yazılan uzunluğu döndürür.
 */
static size_t colour_expand( char *dst, size_t dstsz, const char *src, CHAR_DATA *ch )
{
    size_t n = 0;

    if ( dstsz == 0 )
	return 0;
    if ( src != NULL )
    {
	for ( ; *src != '\0'; src++ )
	{
	    if ( *src == '{' )
	    {
		const char *code;
		size_t len;

		src++;
		if ( *src == '\0' )
		    break;
		code = colour_code( *src, ch );
		len  = strlen( code );
		if ( n + len >= dstsz )
		    break;
		memcpy( dst + n, code, len );
		n += len;
		continue;
	    }
	    if ( n + 1 >= dstsz )
		break;
	    dst[n++] = *src;
	}
    }
    dst[n] = '\0';
    return n;
}



/*
 * Write to one char, new colour version, by Lope.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    char buf[ MAX_STRING_LENGTH*4 ];

    if( txt && ch->desc )
    {
	colour_expand( buf, sizeof(buf), txt, ch );
	write_to_buffer( ch->desc, buf, 0 );
    }
    return;
}

/*
 * Write to one char with color
 */
void send_ch_color( const char *format, CHAR_DATA *ch, int min, ... )
{
    char buf[MAX_STRING_LENGTH];
    const char *str;
    const char *i;
    char *point;
    const char *end = buf + sizeof(buf) - 3;
    int n;
    va_list colors; /* variable arg list of colors */

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL || ch->desc == NULL)
	return;

    va_start(colors,min);

    point   = buf;
    str     = format;
    while ( *str != '\0' && point < end )
    {
	if ( *str != '$' )
	{
	    *point++ = *str++;
	    continue;
	}
	++str;

	switch ( *str )
	{
	default:  bug( "Act: bad code %d.", *str );
		  i = " <@@@> ";		break;
	case '\0': i = ""; --str;		break;
	case 'C':
	    i = va_arg(colors,char *);
	    break;
	case 'c':
	    i = CLR_NORMAL ;
	    break;
	}
	++str;
	point = append_bounded( point, end, i );
    }
    va_end(colors);

    *point++ = '\n';
    *point++ = '\r';
    *point   = '\0';

    /* fix for color prefix and capitalization */
    if (buf[0] == '\033')
    {
	for(n = 1; buf[n] != 'm' && buf[n] != '\0'; n++) ;
	if ( buf[n] == 'm' )
	    utf8_upper_first(buf + n + 1, sizeof(buf) - n - 1);
    }
    else utf8_upper_first(buf, sizeof(buf));
    write_to_buffer( ch->desc, buf, 0 );
    return;
}



/*
 * Page to one char, new colour version, by Lope.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    char buf[ MAX_STRING_LENGTH * 4 ];
    size_t len;

    if( txt && ch->desc )
    {
	len = colour_expand( buf, sizeof(buf), txt, ch );
	if ( ch->desc->showstr_head != NULL )
	    free_string( ch->desc->showstr_head );
	ch->desc->showstr_head  = (char*)alloc_mem( (int) len + 1 );
	memcpy( ch->desc->showstr_head, buf, len + 1 );
	ch->desc->showstr_point = ch->desc->showstr_head;
	show_string( ch->desc, "" );
    }
    return;
}

/* string pager: showstr_point'ten en fazla ch->lines satır yazar. */
void show_string(struct descriptor_data *d, char *input)
{
    char buf[MAX_INPUT_LENGTH];
    char *start, *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
	    free_string(d->showstr_head);
	    d->showstr_head = 0;
	}
    	d->showstr_point  = 0;
	return;
    }

    if (d->character)
	show_lines = d->character->lines;
    else
	show_lines = 0;

    start = d->showstr_point;
    for (scan = start; ; scan++)
    {
	if ((*scan == '\n' || *scan == '\r') && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    if ( scan > start )
		write_to_buffer(d, start, (int) (scan - start));
	    d->showstr_point = scan;
	    for (chk = scan; isspace((unsigned char) *chk); chk++)
	        ;
	    if (!*chk)
	    {
		if (d->showstr_head)
		{
		    free_string(d->showstr_head);
		    d->showstr_head = 0;
		}
		d->showstr_point  = 0;
	    }
	    return;
	}
    }
}



void act (const char *format, CHAR_DATA *ch, const void *arg1,
		const void *arg2, int type)
{
    act_color(format,ch,arg1,arg2,type,POS_RESTING);
}

void act_color( const char *format1, CHAR_DATA *ch, const void *arg1,
	      const void *arg2, int type, int min_pos, ... )
{
    extern const char * dir_name[];
    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    char buffer[ MAX_STRING_LENGTH*4 ];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i = NULL;
    char *point;
    const char *end = buf + sizeof(buf) - 3;
    int n;
    va_list colors;

    /*
     * Discard null and zero-length messages.
     */
    if ( format1 == NULL  || format1[0] == '\0'  )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }

	if (vch->in_room == NULL)
	    return;

        to = vch->in_room->people;
    }

    va_start(colors,min_pos);

    for ( ; to != NULL; to = to->next_in_room )
    {
	va_list colors_copy;

        if ( to->desc == NULL || to->position < min_pos )
            continue;

        if ( type == TO_CHAR && to != ch )
            continue;
        if ( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if ( type == TO_ROOM && to == ch )
            continue;
        if ( type == TO_NOTVICT && (to == ch || to == vch) )
            continue;

	/* Renk argümanları her alıcı için baştan okunur. */
	va_copy(colors_copy, colors);

	point   = buf;
	str     = format1;
	while ( *str != '\0' && point < end )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;
	    i = " <@@@> ";

	    switch ( *str )
	    {
	    default:  bug( "Act: bad code %d.", *str );
		      i = " <@@@> ";				break;
	    case '\0': i = ""; --str;				break;
	    /* Thx alex for 't' idea */
	    case 't': i = arg1 != NULL ? (const char *) arg1 : "";	break;
	    case 'T': i = arg2 != NULL ? (const char *) arg2 : "";	break;
	    case 'W': i = dir_name[atoi((const char *)arg2)];	break;

	    /* Türkçe ek kodu:
	       s : birisinin   m : birisini   n : biri
	       e : birisine    y : birisinde  z : birisinden */
	    case 's':/*-un*/
	    case 'm':/*-u*/
	    case 'e':/*-e*/
	    case 'y':/*-de*/
	    case 'z':/*-den*/
		i=ekler (to,ch, str);
		break;
	    case 'S':
	    case 'M':
	    case 'E':
	    case 'Y':
	    case 'Z':
		i=ekler (to,vch, str);
		break;
	    case 'n': i = PERS( ch,  to  );
		break;
	    case 'N': i = PERS( vch, to  );
		break;
	    case 'C':
		i = va_arg(colors_copy,char *);
		break;
	    case 'c':
		i = CLR_NORMAL ;
		break;
	    case 'p':
		i = can_see_obj( to, obj1 )
			? obj1->short_descr
			: "birşey";
		break;

	    case 'P':
		i = can_see_obj( to, obj2 )
			? obj2->short_descr
			: "birşey";
		break;

	    case 'd':
		if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		{
		    i = "kapı";
		}
		else
		{
		    one_argument( (char *) arg2, fname );
		    i = fname;
		}
		break;
	    }

	    ++str;
	    point = append_bounded( point, end, i );
	}
	va_end(colors_copy);

	*point++ = '\n';
	*point++ = '\r';
	*point	 = '\0';
	/* fix for color prefix and capitalization */
	if (buf[0] == '\033')
	{
	    for(n = 1; buf[n] != 'm' && buf[n] != '\0'; n++) ;
	    if ( buf[n] == 'm' )
		utf8_upper_first(buf + n + 1, sizeof(buf) - n - 1);
	}
	else utf8_upper_first(buf, sizeof(buf));
	colour_expand( buffer, sizeof(buffer), buf, to );
	write_to_buffer( to->desc, buffer, 0 );
    }
    va_end(colors);
    return;
}

void act_new( const char *format, CHAR_DATA *ch, const void *arg1,
	      const void *arg2, int type, int min_pos)
{
    act_color(format,ch,arg1,arg2,type,min_pos);
    return;
}



/*
 *  writes bug directly to user screen.
 */

void dump_to_scr( char *text )
{
    fputs( text, stdout );
    fflush( stdout );
}


int log_area_popularity(void)
{
    FILE *fp;
    AREA_DATA *area;
    char stamp[32];
    extern AREA_DATA *area_first;

    if ( ( fp = fopen( AREASTAT_FILE, "w" ) ) == NULL )
    {
	bug( "log_area_popularity: " AREASTAT_FILE " açılamadı.", 0 );
	return 0;
    }
    platform_time_str( boot_time, stamp, sizeof(stamp) );
    fprintf(fp,"\nBooted %s\nArea popularity statistics (in char * ticks)\n", stamp );

    for (area = area_first; area != NULL; area = area->next) {
      if (area->count >= 5000000)
        fprintf(fp,"%-60s overflow\n",area->name);
      else
        fprintf(fp,"%-60s %lu\n",area->name,area->count);
    }
    fclose(fp);

    return 1;
}


/*
 * Adbolar (paladin) etiği tüzedir; seçim sorulmaz.
 */
int ethos_check(CHAR_DATA *ch)
{
    DESCRIPTOR_DATA *d = ch->desc;

    if ( ch->iclass == CLASS_PALADIN )
    {
	ch->ethos = 1;
	write_to_buffer( d, "Etiğin Tüze.\n\r", 0 );
	return 1;
    }
    return 0;
}


/* source: EOD, by John Booth <???> */

void printf_to_char (CHAR_DATA *ch, const char *fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;
	va_start (args, fmt);
	vsnprintf (buf, sizeof(buf), fmt, args);
	va_end (args);

	send_to_char (buf, ch);
}

/*
 * bug() ilk argümanını biçim dizgisi saydığından hazır metin ona verilmez;
 * aynı önekle doğrudan günlüğe yazılır.
 */
void bugf (const char *fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	char line [MAX_STRING_LENGTH + 16];
	va_list args;
	va_start (args, fmt);
	vsnprintf (buf, sizeof(buf), fmt, args);
	va_end (args);

	snprintf (line, sizeof(line), "[*****] BUG: %s", buf);
	log_string (line);
}
