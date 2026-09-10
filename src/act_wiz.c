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
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "merc.h"
#include "utf8.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"


/* command procedures needed */
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_grant         );
DECLARE_DO_FUN(do_limited       );
DECLARE_DO_FUN(do_help	        );
DECLARE_DO_FUN(do_wear	        );

extern int max_on;

/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location	( CHAR_DATA *ch, char *arg );
bool write_to_descriptor  ( int desc, char *txt, int length );
void update_total_played        ( CHAR_DATA *ch);

/*
 * Ölümsüz komutlarının ortak iletileri ve yardımcıları.
 */
#define MSG_NOT_HERE	"Öyle biri yok.\n\r"
#define MSG_NOT_ON_NPC	"Yaratıklarda olmaz.\n\r"
#define MSG_NOT_ON_PC	"Oyuncularda olmaz.\n\r"
#define MSG_FAILED	"Başaramadın.\n\r"
#define MSG_OK		"Tamam.\n\r"
#define MSG_NO_LOCATION	"Öyle bir yer yok.\n\r"
#define MSG_PRIVATE_ROOM "O oda şu an özel.\n\r"

/* wiz_find_victim bayrakları */
#define WIZ_ALLOW_NPC	(A)	/* yaratık hedef alınabilir */
#define WIZ_NEED_TRUST	(B)	/* hedefin güveni ölümsüzünkinden düşük olmalı */
#define WIZ_SAVE	(C)	/* (ceza tablosu) işlem sonrası oyuncu kaydedilir */

/*
 * Hedef bulma iskeleti: boş argümanda 'usage', bulunamayan hedefte, yaratıkta
 * ve yetki yetmezliğinde ileti verip NULL döndürür.
 */
static CHAR_DATA *wiz_find_victim( CHAR_DATA *ch, char *arg, const char *usage, int flags )
{
    CHAR_DATA *victim;

    if ( arg[0] == '\0' )
    {
	send_to_char( usage, ch );
	return NULL;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( MSG_NOT_HERE, ch );
	return NULL;
    }

    if ( !IS_SET(flags, WIZ_ALLOW_NPC) && IS_NPC(victim) )
    {
	send_to_char( MSG_NOT_ON_NPC, ch );
	return NULL;
    }

    if ( IS_SET(flags, WIZ_NEED_TRUST) && get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( MSG_FAILED, ch );
	return NULL;
    }

    return victim;
}

/* Hedef özel bir odadaysa ve ölümsüz oraya giremiyorsa TRUE. */
static bool wiz_room_blocked( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
    if ( room == NULL || ch->in_room == room )
	return FALSE;
    return !is_room_owner( ch, room ) && room_is_private( room )
	&& !IS_TRUSTED( ch, IMPLEMENTOR );
}

/* time_t -> "YYYY-AA-GG SS:DD:ss" (statik tampon); 0 için "yok". */
static const char *fmt_time( time_t t )
{
    static char buf[32];
    struct tm *tm;

    if ( t == 0 )
	return "yok";
    tm = localtime( &t );
    if ( tm == NULL || strftime( buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm ) == 0 )
	return "?";
    return buf;
}

/* Bit tablosundaki adları boşlukla ayırarak yazar (statik tampon). */
static const char *flag_names( const struct flag_type *table, long flags )
{
    static char buf[MAX_STRING_LENGTH];
    size_t len = 0;
    int i;

    buf[0] = '\0';
    for ( i = 0; table[i].name != NULL; i++ )
    {
	if ( !IS_SET(flags, table[i].bit) )
	    continue;
	len += snprintf( buf + len, sizeof(buf) - len, "%s ", table[i].name );
	if ( len >= sizeof(buf) )
	    break;
    }
    return buf;
}

static const struct flag_type mprog_flag_names[] =
{
    { "bribe",	MPROG_BRIBE,	TRUE },
    { "speech",	MPROG_SPEECH,	TRUE },
    { "give",	MPROG_GIVE,	TRUE },
    { "death",	MPROG_DEATH,	TRUE },
    { "greet",	MPROG_GREET,	TRUE },
    { "entry",	MPROG_ENTRY,	TRUE },
    { "fight",	MPROG_FIGHT,	TRUE },
    { "area",	MPROG_AREA,	TRUE },
    { NULL,	0,		FALSE }
};

static const struct flag_type oprog_flag_names[] =
{
    { "get",		OPROG_GET,	TRUE },
    { "drop",		OPROG_DROP,	TRUE },
    { "sacrifice",	OPROG_SAC,	TRUE },
    { "give",		OPROG_GIVE,	TRUE },
    { "fight",		OPROG_FIGHT,	TRUE },
    { "death",		OPROG_DEATH,	TRUE },
    { "speech",		OPROG_SPEECH,	TRUE },
    { "area",		OPROG_AREA,	TRUE },
    { NULL,		0,		FALSE }
};

/*
 * Yükleme / kopyalama yetki eşikleri: güven düzeyine göre en yüksek eşya
 * seviyesi ve bedeli.
 */
static const struct { int trust; int max_level; int max_cost; } load_limits[] =
{
    { GOD,	INT_MAX,	INT_MAX },
    { IMMORTAL,	20,		1000 },
    { DEMI,	10,		500 },
    { ANGEL,	5,		250 },
    { AVATAR,	0,		100 }
};

static bool wiz_may_load( CHAR_DATA *ch, int level, int cost )
{
    size_t i;

    for ( i = 0; i < sizeof(load_limits) / sizeof(load_limits[0]); i++ )
	if ( IS_TRUSTED( ch, load_limits[i].trust )
	&&   level <= load_limits[i].max_level && cost <= load_limits[i].max_cost )
	    return TRUE;
    return FALSE;
}

/*
 * Ölümsüzün oda değiştirmesi: bamfout duyurusu, taşınma, bamfin duyurusu, bak.
 */
static void imm_announce( CHAR_DATA *ch, const char *custom, const char *dflt )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( get_trust( rch ) < ch->invis_level )
	    continue;
	if ( custom != NULL && custom[0] != '\0' )
	    act( "$t", ch, custom, rch, TO_VICT );
	else
	    act( dflt, ch, NULL, rch, TO_VICT );
    }
}

static void imm_move( CHAR_DATA *ch, ROOM_INDEX_DATA *to )
{
    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    imm_announce( ch, ch->pcdata != NULL ? ch->pcdata->bamfout : NULL,
	"$n leaves in a swirling mist." );
    char_from_room( ch );
    char_to_room( ch, to );
    imm_announce( ch, ch->pcdata != NULL ? ch->pcdata->bamfin : NULL,
	"$n appears in a swirling mist." );
    do_look( ch, "auto" );
}

/* Tam iyileştirme: kötü etkileri sök, yp/mp/zp doldur. */
static void restore_char( CHAR_DATA *ch, CHAR_DATA *victim )
{
    const sh_int strip_sns[] = { gsn_plague, gsn_poison, gsn_blindness, gsn_sleep, gsn_curse };
    size_t i;

    for ( i = 0; i < sizeof(strip_sns) / sizeof(strip_sns[0]); i++ )
	affect_strip( victim, strip_sns[i] );

    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    if ( victim->in_room != NULL )
	act( "$n has restored you.", ch, NULL, victim, TO_VICT );
}

/*
 * Bit açıp kapatan ceza komutları tablosu.
 */
enum { PEN_ACT, PEN_COMM };

struct penalty
{
    int		field;		/* PEN_ACT: act, PEN_COMM: comm */
    long	bit;
    const char *usage;
    const char *set_victim, *set_imm, *set_wiz;	/* *_wiz: "$N ... %s ..." (%s: kurban adı) */
    const char *clr_victim, *clr_imm, *clr_wiz;
    int		flags;		/* WIZ_ALLOW_NPC | WIZ_NEED_TRUST | WIZ_SAVE */
};

static void wiz_toggle_penalty( CHAR_DATA *ch, char *argument, const struct penalty *p )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    long *field;
    bool set;

    one_argument( argument, arg );
    if ( ( victim = wiz_find_victim( ch, arg, p->usage, p->flags ) ) == NULL )
	return;

    field = p->field == PEN_ACT ? &victim->act : &victim->comm;
    set = !IS_SET(*field, p->bit);
    if ( set )
	SET_BIT(*field, p->bit);
    else
	REMOVE_BIT(*field, p->bit);

    if ( set ? p->set_victim != NULL : p->clr_victim != NULL )
	send_to_char( set ? p->set_victim : p->clr_victim, victim );
    send_to_char( set ? p->set_imm : p->clr_imm, ch );
    if ( set ? p->set_wiz != NULL : p->clr_wiz != NULL )
    {
	snprintf( buf, sizeof(buf), set ? p->set_wiz : p->clr_wiz, victim->name );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
    }

    if ( IS_SET(p->flags, WIZ_SAVE) )
	save_char_obj( victim );
}


void do_cabal_scan( CHAR_DATA *ch, char *argument )
{
 int i;
 char buf1[MAX_INPUT_LENGTH];
 char buf2[MAX_INPUT_LENGTH];
 OBJ_DATA *in_obj;
 int show;

 if ( !IS_IMMORTAL(ch) &&
	(ch->cabal == CABAL_NONE || IS_NPC(ch)) )
 {
    send_to_char("You are not a cabal member yet.\n\r", ch);
    return;
 }

  for(i=1;i<MAX_CABAL;i++)
  {
    if (IS_IMMORTAL(ch) || ch->cabal == i)
	show = 1;
    else show = 0;
    snprintf(buf1, sizeof(buf1), " Cabal: %-*s, room %4d, item %4d, ptr: %-*s ",
	utf8_width(cabal_table[i].short_name, 11), cabal_table[i].short_name,
        cabal_table[i].room_vnum,
        cabal_table[i].obj_vnum,
	utf8_width(cabal_table[i].obj_ptr != NULL ?
	cabal_table[i].obj_ptr->short_descr : "(NULL)", 20), cabal_table[i].obj_ptr != NULL ?
	cabal_table[i].obj_ptr->short_descr : "(NULL)");
    if ( cabal_table[i].obj_ptr!=NULL )
    {
	for ( in_obj = cabal_table[i].obj_ptr;
		in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
		;
	if (in_obj->carried_by)
	 snprintf(buf2, sizeof(buf2),"\n\r\t\tcarried_by: %s\n\r",PERS(in_obj->carried_by,ch));
	else
	{
	 snprintf(buf2, sizeof(buf2),"\n\r\t\t\t\t\tin_room: %s\n\r", in_obj->in_room != NULL ?
		in_obj->in_room->name : "BUG!!" );
	 if (in_obj->in_room != NULL
		&& in_obj->in_room->vnum == cabal_table[ch->cabal].room_vnum)
	    show = 1;
	}
    }

    if (show)
    {
	send_to_char(buf1, ch);
	send_to_char(buf2, ch);
    }
  }
  return;
}

/* objlist ölçütleri: where >= 0 ise bit etkisi, değilse konum listesi + enaz. */
struct objlist_crit
{
    const char *name;
    int		where;
    int		locations[7];	/* -1 ile biter */
};

static const struct objlist_crit objlist_crits[] =
{
    { "imm",	 TO_IMMUNE, { -1 } },
    { "res",	 TO_RESIST, { -1 } },
    { "yp",	 -1, { APPLY_HIT, -1 } },
    { "mp",	 -1, { APPLY_MANA, -1 } },
    { "zp",	 -1, { APPLY_MOVE, -1 } },
    { "zz",	 -1, { APPLY_DAMROLL, -1 } },
    { "vz",	 -1, { APPLY_HITROLL, -1 } },
    { "nitelik", -1, { APPLY_STR, APPLY_INT, APPLY_DEX, APPLY_WIS, APPLY_CON, APPLY_CHA, -1 } },
    { NULL,	 -1, { -1 } }
};

static bool objlist_match( const AFFECT_DATA *paf, const struct objlist_crit *c, int enaz )
{
    int i;

    if ( c->where >= 0 )
	return paf->bitvector && paf->where == c->where;

    for ( i = 0; c->locations[i] != -1; i++ )
	if ( paf->location == c->locations[i] && paf->modifier >= enaz )
	    return TRUE;
    return FALSE;
}

/* "<arg> bir sayı olmalıdır" denetimiyle isteğe bağlı sayısal argüman. */
static bool objlist_number( CHAR_DATA *ch, const char *arg, const char *label, int *out )
{
    if ( arg[0] == '\0' )
	return TRUE;
    if ( !is_number( (char *) arg ) )
    {
	printf_to_char( ch, "%s argümanı bir sayı olmalıdır.\n\r", label );
	return FALSE;
    }
    *out = atoi( arg );
    return TRUE;
}

void do_objlist( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    const struct objlist_crit *crit;
    OBJ_DATA *obj;
    AFFECT_DATA *paf;
    int altseviye = -1;
    int ustseviye = -1;
    int enaz = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Argumanlar: <imm|res|yp|mp|zp|vz|zz|nitelik> <altseviye> <ustseviye> <enaz>\n\r", ch );
	return;
    }

    if ( !objlist_number( ch, arg2, "altseviye", &altseviye )
    ||   !objlist_number( ch, arg3, "ustseviye", &ustseviye )
    ||   !objlist_number( ch, arg4, "enaz", &enaz ) )
	return;

    for ( crit = objlist_crits; crit->name != NULL; crit++ )
	if ( !str_cmp( arg1, crit->name ) )
	    break;
    if ( crit->name == NULL )
    {
	do_objlist( ch, "" );
	return;
    }

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( altseviye >= 0 && obj->level < altseviye )
	    continue;
	if ( ustseviye >= 0 && obj->level > ustseviye )
	    continue;
	if ( !obj->pIndexData->random_object )
	    continue;

	for ( paf = obj->affected; paf != NULL; paf = paf->next )
	{
	    if ( objlist_match( paf, crit, enaz ) )
	    {
		spell_identify( 0, 0, ch, obj, 0 );
		send_to_char( "\n\r", ch );
		break;
	    }
	}
    }
}

void do_limited( CHAR_DATA *ch, char *argument )
{
  extern int top_obj_index;
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *obj_index;
  BUFFER *output;
  char  buf[MAX_STRING_LENGTH];
  int	lCount = 0;
  int	ingameCount;
  int 	nMatch;
  int 	vnum;

  if ( argument[0] != '\0' )
  {
    obj_index = get_obj_index( atoi(argument) );
    if ( obj_index == NULL )  {
      send_to_char( "Bulunamadı.\n\r", ch);
      return;
    }
    if ( obj_index->limit == -1 )  {
      send_to_char( "Bu sınırlı bir eşya değil.\n\r", ch );
      return;
    }
    snprintf(buf, sizeof(buf), "%-*s [%5d]  Sınır: %3d  Şu an: %3d\n\r",
		   utf8_width(obj_index->short_descr, 35), obj_index->short_descr,
		   obj_index->vnum,
	           obj_index->limit,
		   obj_index->count);
    utf8_upper_first(buf, sizeof(buf));
    send_to_char( buf, ch );
    ingameCount = 0;
    for ( obj=object_list; obj != NULL; obj=obj->next )
    {
	if ( obj->pIndexData->vnum != obj_index->vnum )
	    continue;
	ingameCount++;
	if ( obj->carried_by != NULL )
	    printf_to_char( ch, "Taşıyan: %s\n\r", obj->carried_by->name );
	else if ( obj->in_room != NULL )
	    printf_to_char( ch, "Oda: %s [%d]\n\r", obj->in_room->name, obj->in_room->vnum );
	else if ( obj->in_obj != NULL )
	    printf_to_char( ch, "İçinde: %s [%d]\n\r", obj->in_obj->short_descr, obj->in_obj->pIndexData->vnum );
	else
	    send_to_char( "Hiçlikte.\n\r", ch );
    }
    printf_to_char( ch, "  Oyunda %d bulundu, %d oyuncu dosyalarında olmalı.\n\r",
	ingameCount, obj_index->count - ingameCount );
    return;
  }

  nMatch = 0;
  output = new_buf();
  for ( vnum = 0; nMatch < top_obj_index; vnum++ )
      if ( ( obj_index = get_obj_index( vnum ) ) != NULL )
      {
        nMatch++;
	if ( obj_index->limit != -1 )  {
	  lCount++;
          snprintf(buf, sizeof(buf), "%-*s [%5d]  Sınır: %3d  Şu an: %3d\n\r",
		   utf8_width(obj_index->short_descr, 37), obj_index->short_descr,
		   obj_index->vnum,
	           obj_index->limit,
		   obj_index->count);
	  utf8_upper_first(buf, sizeof(buf));
	  add_buf( output, buf );
	}
      }
  snprintf(buf, sizeof(buf), "\n\r%d / %d eşya sınırlı.\n\r", lCount, nMatch );
  add_buf( output, buf );
  page_to_char( buf_string(output), ch );
  free_buf( output );
}

static void wiznet_set( CHAR_DATA *ch, bool on )
{
    if ( on )
    {
	send_to_char("Wiznet'e hoş geldin!\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
    }
    else
    {
	send_to_char("Wiznet'ten çıkıyorsun.\n\r",ch);
	REMOVE_BIT(ch->wiznet,WIZ_ON);
    }
}

/* only_set: yalnızca açık olanlar; değilse güven düzeyine açık tüm seçenekler. */
static void wiznet_list( CHAR_DATA *ch, bool only_set )
{
    BUFFER *out = new_buf();
    int flag;

    if ( only_set && !IS_SET(ch->wiznet,WIZ_ON) )
	add_buf( out, "off " );

    for ( flag = 0; wiznet_table[flag].name != NULL; flag++ )
    {
	if ( only_set ? IS_SET(ch->wiznet, wiznet_table[flag].flag)
		      : wiznet_table[flag].level <= get_trust(ch) )
	{
	    add_buf( out, (char *) wiznet_table[flag].name );
	    add_buf( out, " " );
	}
    }
    add_buf( out, "\n\r" );
    send_to_char( buf_string(out), ch );
    free_buf( out );
}

void do_wiznet( CHAR_DATA *ch, char *argument )
{
    int flag;

    if ( argument[0] == '\0' )
    {
	wiznet_set( ch, !IS_SET(ch->wiznet,WIZ_ON) );
      	return;
    }

    if (!str_prefix(argument,"on"))
    {
	wiznet_set( ch, TRUE );
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	wiznet_set( ch, FALSE );
	return;
    }

    if (!str_prefix(argument,"status"))
    {
	send_to_char("Wiznet durumu:\n\r",ch);
	wiznet_list( ch, TRUE );
	return;
    }

    if (!str_prefix(argument,"show"))
    {
	send_to_char("Kullanabileceğin wiznet seçenekleri:\n\r",ch);
	wiznet_list( ch, FALSE );
	return;
    }

    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
	send_to_char("Öyle bir seçenek yok.\n\r",ch);
	return;
    }

    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {
	printf_to_char( ch, "Artık wiznet'te %s görmeyeceksin.\n\r", wiznet_table[flag].name );
	REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
    }
    else
    {
	printf_to_char( ch, "Artık wiznet'te %s göreceksin.\n\r", wiznet_table[flag].name );
    	SET_BIT(ch->wiznet,wiznet_table[flag].flag);
    }
}

void wiznet(const char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level)
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->connected == CON_PLAYING
	&&  IS_IMMORTAL(d->character)
	&&  IS_SET(d->character->wiznet,WIZ_ON)
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch)
        {
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
	  	send_to_char("--> ",d->character);
            act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
        }
    }

    return;
}

void do_tick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )  {
	send_to_char("tick area : area update\n\r",ch);
	send_to_char("tick char : char update\n\r",ch);
	send_to_char("tick obj  : obj  update\n\r",ch);
	send_to_char("tick room : room update\n\r",ch);
	send_to_char("tick track: track update\n\r",ch);
	return;
    }
    if (is_name(arg, "area") )  {
	area_update( );
	send_to_char("Area updated.\n\r", ch );
	return;
    }
    if (is_name(arg, "char player") )  {
	char_update( );
	send_to_char("Players updated.\n\r", ch );
	return;
    }
    if (is_name(arg, "obj") )  {
	obj_update( );
	send_to_char("Obj updated.\n\r", ch );
	return;
    }
    if (is_name(arg, "room") )  {
	room_update( );
	send_to_char("Room updated.\n\r", ch );
	return;
    }
    if (is_name(arg, "track") )  {
	track_update( );
	send_to_char("Track updated.\n\r", ch );
	return;
    }
    do_tick(ch,"");
    return;
}

/* equips a character */
/* Okul eşyası verir; taşıma sınırı dolduysa FALSE. */
static bool outfit_give( CHAR_DATA *ch, int vnum, bool zero_cost )
{
    OBJ_DATA *obj;

    if ( ch->carry_number + 1 > can_carry_n(ch) )
    {
	send_to_char( "Bu kadar çok eşya taşıyamazsın.\n\r", ch );
	return FALSE;
    }

    obj = create_object( get_obj_index(vnum), 0 );
    if ( zero_cost )
	obj->cost = 0;
    obj->condition = 100;
    obj_to_char( obj, ch );
    return TRUE;
}

/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    if ((ch->level > 5 || IS_NPC(ch)) && !IS_IMMORTAL(ch))
    {
	send_to_char("Kendin bul!\n\r",ch);
	return;
    }

    if ( get_light_char( ch ) == NULL && !outfit_give( ch, OBJ_VNUM_SCHOOL_BANNER, TRUE ) )
	return;

    if ( get_eq_char( ch, WEAR_BODY ) == NULL && !outfit_give( ch, OBJ_VNUM_SCHOOL_VEST, TRUE ) )
	return;

    if ( get_wield_char( ch, FALSE ) == NULL
    &&   !outfit_give( ch, class_table[ch->iclass].weapon, FALSE ) )
	return;

    if ( !outfit_give( ch, OBJ_VNUM_SCHOOL_SHIELD, TRUE ) )
	return;

    send_to_char("Tanrılar sana bazı eşyalar bahşediyor.\n\r",ch);
    send_to_char("Taşıdığın eşyaları görüntülemek için 'envanter' yaz.\n\r",ch);
    send_to_char("Eşyalarını giymek için 'giy <eşya adı>' yazmayı dene.\n\r\n\r",ch);
}


/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    static const struct penalty p =
    {
	PEN_COMM, COMM_NOCHANNELS, "Kimin kanalları?\n\r",
	"Tanrılar kanal ayrıcalıklarını geri aldı.\n\r", "NOCHANNELS açıldı.\n\r", "$N %s'ın kanallarını kapattı.",
	"Tanrılar kanal ayrıcalıklarını geri verdi.\n\r", "NOCHANNELS kaldırıldı.\n\r", "$N %s'ın kanallarını açtı.",
	WIZ_NEED_TRUST
    };
    wiz_toggle_penalty( ch, argument, &p );
}


/* out'a en fazla n bayt ekler (tampon sınırlı). */
static void smote_append( char *out, size_t size, size_t *len, const char *s, size_t n )
{
    if ( *len + n >= size )
	n = size - 1 - *len;
    memcpy( out + *len, s, n );
    *len += n;
    out[*len] = '\0';
}

/* text içindeki 'name' geçişlerini "you", "name's" geçişlerini "your" yapar. */
static void smote_replace( char *out, size_t size, const char *text, const char *name )
{
    size_t nlen = strlen( name ), len = 0;
    const char *p = text, *hit;

    out[0] = '\0';
    while ( ( hit = strstr( p, name ) ) != NULL )
    {
	smote_append( out, size, &len, p, (size_t)(hit - p) );
	if ( hit[nlen] == '\'' && hit[nlen + 1] == 's' )
	{
	    smote_append( out, size, &len, "your", 4 );
	    p = hit + nlen + 2;
	}
	else
	{
	    smote_append( out, size, &len, "you", 3 );
	    p = hit + nlen;
	}
    }
    smote_append( out, size, &len, p, strlen( p ) );
}

void do_smote(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char temp[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "Duygularını gösteremezsin.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Ne yapıyorsun?\n\r", ch );
        return;
    }

    if (strstr(argument,ch->name) == NULL)
    {
	send_to_char("smote içinde kendi adın geçmeli.\n\r",ch);
	return;
    }

    send_to_char(argument,ch);
    send_to_char("\n\r",ch);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;

	if ( strstr( argument, vch->name ) == NULL )
	{
	    send_to_char(argument,vch);
	    send_to_char("\n\r",vch);
	    continue;
	}

	smote_replace( temp, sizeof(temp), argument, vch->name );
	send_to_char(temp,vch);
	send_to_char("\n\r",vch);
    }
}

/* bamfin/bamfout ayarı: boş argüman gösterir, argüman ölümsüzün adını içermeli. */
static void set_bamf( CHAR_DATA *ch, char *argument, char **field, const char *label )
{
    if ( IS_NPC(ch) )
	return;

    smash_tilde( argument );

    if (argument[0] == '\0')
    {
	printf_to_char( ch, "%s iletin: %s\n\r", label, *field );
	return;
    }

    if ( strstr(argument,ch->name) == NULL)
    {
	send_to_char("Kendi adın geçmeli.\n\r",ch);
	return;
    }

    free_string( *field );
    *field = str_dup( argument );
    printf_to_char( ch, "%s iletin artık: %s\n\r", label, *field );
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
	set_bamf( ch, argument, &ch->pcdata->bamfin, "Geliş (poofin)" );
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
	set_bamf( ch, argument, &ch->pcdata->bamfout, "Gidiş (poofout)" );
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( ( victim = wiz_find_victim( ch, arg, "Kime giriş yasağı?\n\r", WIZ_NEED_TRUST ) ) == NULL )
	return;

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "Girişin yasaklandı!\n\r", victim );
    snprintf(buf, sizeof(buf),"$N %s'ın girişini yasakladı.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( MSG_OK, ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_quit( victim, "" );
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d, *d_next;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if (is_number(arg))
    {
	int desc;

	desc = atoi(arg);
    	for ( d = descriptor_list; d != NULL; d = d_next )
    	{
	    d_next = d->next;
            if ( d->descriptor == desc )
            {
            	close_socket( d );
            	send_to_char( "Ok.\n\r", ch );
            	return;
            }
	}
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}


void do_duyuru( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char eventbuf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
        send_to_char( "Neyi duyuracaksın?\n\r", ch );
        return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected == CON_PLAYING )
        {
            printf_to_char(d->character,"{RDUYURU: {G%s{x\n\r",argument);
        }
    }

    /* event */
    snprintf(eventbuf, sizeof(eventbuf),"DUYURU: %s", argument);
    write_event_log(eventbuf);

    return;
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d, *d_next;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		snprintf(buf, sizeof(buf), "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( room_is_private( location )
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( MSG_PRIVATE_ROOM, ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch, *wch_next;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location )
    &&  get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( MSG_PRIVATE_ROOM, ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument, FALSE );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Nereye gidiyorsun?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( MSG_NO_LOCATION, ch );
	return;
    }

    imm_move( ch, location );
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Nereye gidiyorsun?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( MSG_NO_LOCATION, ch );
        return;
    }

    if (!room_is_private( location ))
    {
        send_to_char( "O oda özel değil, goto kullan.\n\r", ch );
        return;
    }

    imm_move( ch, location );
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }

   if (!str_cmp(arg,"obj"))
   {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }

   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}





void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_HISTORY_DATA *rh;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

/*    if (!is_room_owner(ch,location) && ch->in_room != location  */
    if ( ch->in_room != location
    &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if (ch->in_room->affected_by)
    {
	snprintf(buf, sizeof(buf), "Affected by %s\n\r",
	    raffect_bit_name(ch->in_room->affected_by));
	send_to_char(buf,ch);
    }

    if (ch->in_room->room_flags)
    {
	snprintf(buf, sizeof(buf), "Roomflags %s\n\r",
	    flag_room_name(ch->in_room->room_flags));
	send_to_char(buf,ch);
    }

    snprintf(buf, sizeof(buf), "Name: '%s'\n\rArea: '%s'\n\rOwner: '%s'\n\r",
	location->name,
	location->area->name ,
	location->owner );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf),
	"Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf),
	"Room flags: %ld.\n\rDescription:\n\r%s",
	location->room_flags,
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    snprintf(buf, sizeof(buf),
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }
    send_to_char("Tracks:\n\r",ch);
    for (rh = location->history;rh != NULL;rh = rh->next) {
      snprintf(buf, sizeof(buf),"%s took door %i.\n\r",rh->name,rh->went);
      send_to_char(buf,ch);
    }
    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Ne bu dünyada ne de diğerinde böyle bir şey yok.\n\r", ch );
	return;
    }

    snprintf(buf, sizeof(buf), "Name(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Vnum: %d  Type: %s  Material: %s\n\r",
	obj->pIndexData->vnum, item_type_name(obj), obj->material );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Format: %s  Random: %s  Resets: %d\n\r",
	obj->pIndexData->new_format ? "new" : "old", obj->pIndexData->random_object ? "yes" : "no", obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Wear bits: %s\n\rExtra bits: %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Number: %d/%d  Weight: %d/%d/%d gr.\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Level: %d  Cost: %d  Condition: %d  Timer: %d Count: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer, obj->pIndexData->count );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf),
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" :
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "birisi",
	obj->wear_loc );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Values: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );

    /* now give out vital statistics as per identify */

    switch ( obj->item_type )
    {
    	case ITEM_SCROLL:
    	case ITEM_POTION:
    	case ITEM_PILL:
	    snprintf(buf, sizeof(buf), "Level %d spells of:", obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[1]].name[1], ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[2]].name[1], ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name[1], ch );
	    	send_to_char( "'", ch );
	    }

	    if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	    {
		send_to_char(" '",ch);
		send_to_char(skill_table[obj->value[4]].name[1],ch);
		send_to_char("'",ch);
	    }

	    send_to_char( ".\n\r", ch );
	break;

    	case ITEM_WAND:
    	case ITEM_STAFF:
	    snprintf(buf, sizeof(buf), "Has %d(%d) charges of level %d",
	    	obj->value[1], obj->value[2], obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name[1], ch );
	    	send_to_char( "'", ch );
	    }

	    send_to_char( ".\n\r", ch );
	break;

	case ITEM_DRINK_CON:
	    snprintf(buf, sizeof(buf),"It holds %s-colored %s.\n\r",
		liq_table[obj->value[2]].liq_color,
		liq_table[obj->value[2]].liq_name);
	    send_to_char(buf,ch);
	    break;


    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC):
		    send_to_char("exotic\n\r",ch);
		    break;
	    	case(WEAPON_SWORD):
		    send_to_char("sword\n\r",ch);
		    break;
	    	case(WEAPON_DAGGER):
		    send_to_char("dagger\n\r",ch);
		    break;
	    	case(WEAPON_SPEAR):
		    send_to_char("spear/staff\n\r",ch);
		    break;
	    	case(WEAPON_MACE):
		    send_to_char("mace/club\n\r",ch);
		    break;
	   	case(WEAPON_AXE):
		    send_to_char("axe\n\r",ch);
		    break;
	    	case(WEAPON_FLAIL):
		    send_to_char("flail\n\r",ch);
		    break;
	    	case(WEAPON_WHIP):
		    send_to_char("whip\n\r",ch);
		    break;
	    	case(WEAPON_POLEARM):
		    send_to_char("polearm\n\r",ch);
		    break;
	    	case(WEAPON_BOW):
		    send_to_char("bow\n\r",ch);
		    break;
	    	case(WEAPON_ARROW):
		    send_to_char("arrow\n\r",ch);
		    break;
	    	case(WEAPON_LANCE):
		    send_to_char("lance\n\r",ch);
		    break;
	    	default:
		    send_to_char("unknown\n\r",ch);
		    break;
 	    }
	    if (obj->pIndexData->new_format)
	    	snprintf(buf, sizeof(buf),"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
	    	snprintf(buf, sizeof(buf), "Damage is %d to %d (average %d)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );

	    snprintf(buf, sizeof(buf),"Damage noun is %s.\n\r",
		attack_table[obj->value[3]].noun);
	    send_to_char(buf,ch);

	    if (obj->value[4])  /* weapon flags */
	    {
	        snprintf(buf, sizeof(buf),"Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }
	break;

    	case ITEM_ARMOR:
	    snprintf(buf, sizeof(buf),
	    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	break;

        case ITEM_CONTAINER:
            snprintf(buf, sizeof(buf),"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
            send_to_char(buf,ch);
            if (obj->value[4] != 100)
            {
                snprintf(buf, sizeof(buf),"Weight multiplier: %d%%\n\r",
		    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;
    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	snprintf(buf, sizeof(buf), "Affects %s by %d, level %d",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    snprintf(buf, sizeof(buf),", %d hours.\n\r",paf->duration);
	else
	    snprintf(buf, sizeof(buf),".\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS:
		    snprintf(buf, sizeof(buf),"Adds %s affect.\n",
			affect_bit_name(paf->bitvector));
		    break;
                case TO_WEAPON:
                    snprintf(buf, sizeof(buf),"Adds %s weapon flags.\n",
                        weapon_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    snprintf(buf, sizeof(buf),"Adds %s object flag.\n",
			extra_bit_name(paf->bitvector));
		    break;
		case TO_IMMUNE:
		    snprintf(buf, sizeof(buf),"Adds immunity to %s.\n",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_RESIST:
		    snprintf(buf, sizeof(buf),"Adds resistance to %s.\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_VULN:
		    snprintf(buf, sizeof(buf),"Adds vulnerability to %s.\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_DETECTS:
		    snprintf(buf, sizeof(buf),"Adds %s detection.\n\r",
			detect_bit_name(paf->bitvector));
		    break;
		default:
		    snprintf(buf, sizeof(buf),"Unknown bit %d: %d\n\r",
			paf->where,paf->bitvector);
		    break;
	    }
	    send_to_char(buf,ch);
	}
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	snprintf(buf, sizeof(buf), "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    snprintf(buf, sizeof(buf),"Adds %s affect.\n",
                        affect_bit_name(paf->bitvector));
                    break;
                case TO_OBJECT:
                    snprintf(buf, sizeof(buf),"Adds %s object flag.\n",
                        extra_bit_name(paf->bitvector));
                    break;
                case TO_IMMUNE:
                    snprintf(buf, sizeof(buf),"Adds immunity to %s.\n",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_RESIST:
                    snprintf(buf, sizeof(buf),"Adds resistance to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_VULN:
                    snprintf(buf, sizeof(buf),"Adds vulnerability to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_DETECTS:
                    snprintf(buf, sizeof(buf),"Adds %s detection.\n\r",
                        detect_bit_name(paf->bitvector));
                    break;
                default:
                    snprintf(buf, sizeof(buf),"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }
    printf_to_char( ch, "Object progs: %s\n\r",
	obj->pIndexData->progtypes != 0 ? flag_names( oprog_flag_names, obj->progtypes ) : "" );
    snprintf(buf, sizeof(buf),"Damage condition : %d (%s) ", obj->condition,
			get_cond_alias(obj) );
    send_to_char(buf,ch);
    send_to_char("\n\r",ch);
    return;
}


void do_mobstat( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *gch;

  one_argument( argument, arg1 );

  if ( arg1[0] == '\0' )
  {
    printf_to_char(ch,"Eksik argüman.\n\r");
    return;
  }

  for ( gch = char_list; gch != NULL; gch = gch->next )
  {
      if (!IS_NPC(gch))
          continue;
      if (gch->level == atoi(arg1))
        printf_to_char(ch,"Level: %-3d  Damroll: %-4d  Hitroll: %-4d  Yp: %-6d Mp: %-6d Zp: %-6d\n\r",atoi( arg1 ),gch->damroll,gch->hitroll,gch->hit,gch->mana,gch->move);

  }
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    snprintf(buf, sizeof(buf), "Name: [%s] Reset Zone: [%s] Logon: %s\n\r",
	victim->name,
	(IS_NPC(victim) &&victim->zone) ? victim->zone->name : "?",
	fmt_time( victim->logon ) );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf),
	"Vnum: %d  Format: %s  Race: %s(%s)  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? "NPC" : "PC",
	race_table[RACE(victim)].name[1],race_table[ORG_RACE(victim)].name[1],
	sex_table[victim->sex].name,
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char( buf, ch );

    if (IS_NPC(victim))
    {
	snprintf(buf, sizeof(buf),"Count: %d  Killed: %d  ---  Status: %d  Cabal: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed,
	    victim->status, victim->cabal);
	send_to_char(buf,ch);
    }

    snprintf(buf, sizeof(buf),
"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d) Cha: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON),
	victim->perm_stat[STAT_CHA],
	get_curr_stat(victim,STAT_CHA) );
    send_to_char( buf, ch );


    snprintf(buf, sizeof(buf), "Yp: %d/%d  Mp: %d/%d  Zp: %d/%d  Practices: %d\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	IS_NPC(ch) ? 0 : victim->practice );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) )
      snprintf(buf2, sizeof(buf2), "%d", victim->alignment );
    else  {
      snprintf(buf2, sizeof(buf2), "%s",
	victim->ethos==1?"Law-":
	victim->ethos==2?"Neut-":
	victim->ethos==3?"Cha-":"none-" );
      strcat( buf2,
	IS_GOOD(victim)?"Good":
	IS_NEUTRAL(victim)?"Neut":
	IS_EVIL(victim)?"Evil":"Other" );
    }
    snprintf(buf, sizeof(buf),"It belives the religion of %s.\n\r",
	IS_NPC(victim) ? "None" : religion_table[victim->religion].name);
    send_to_char(buf,ch);
    snprintf(buf, sizeof(buf),
	"Lv: %d  Class: %s  Align: %s  Silver: %ld  Exp: %d\n\r",
	victim->level,
	IS_NPC(victim) ? "mobile" : class_table[victim->iclass].name[1],
	buf2,
	victim->silver, victim->exp );
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf),"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    snprintf(buf, sizeof(buf),
	"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	size_table[victim->size].name, position_table[victim->position].name,
	victim->wimpy );
    send_to_char( buf, ch );

    if (IS_NPC(victim))
    {
	snprintf(buf, sizeof(buf), "Damage: %dd%d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	send_to_char(buf,ch);
    }
    snprintf(buf, sizeof(buf), "Fighting: %s Death: %d Carry number: %d  Carry weight: %ld\n\r",
	victim->fighting ? victim->fighting->name : "(none)"
	,IS_NPC(victim) ? 0 : victim->pcdata->death,
	victim->carry_number, get_carry_weight(victim) / 10 );
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	snprintf(buf, sizeof(buf),
"Thirst: %d  Hunger: %d  Full: %d  Drunk: %d Bloodlust: %d Desire: %d\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_HUNGER],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->pcdata->condition[COND_BLOODLUST],
	    victim->pcdata->condition[COND_DESIRE] );
	send_to_char( buf, ch );
    }


    if (!IS_NPC(victim))
    {
    	snprintf(buf, sizeof(buf),
	    "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
	    get_age(victim),
	    (int) (victim->played + current_time - victim->logon) / 3600,
	    victim->pcdata->last_level,
	    victim->timer );
    	send_to_char( buf, ch );
    }

    snprintf(buf, sizeof(buf), "Act: %s\n\r",act_bit_name(victim->act));
    send_to_char(buf,ch);

    if (victim->comm)
    {
    	snprintf(buf, sizeof(buf),"Comm: %s\n\r",comm_bit_name(victim->comm));
    	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
    	snprintf(buf, sizeof(buf), "Offense: %s\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	snprintf(buf, sizeof(buf), "Immune: %s\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }

    if (victim->res_flags)
    {
	snprintf(buf, sizeof(buf), "Resist: %s\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	snprintf(buf, sizeof(buf), "Vulnerable: %s\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    if (victim->detection)
    {
     snprintf(buf, sizeof(buf), "Detection: %s\n\r",
	(victim->detection) ? detect_bit_name(victim->detection) : "(none)");
     send_to_char(buf,ch);
    }

    snprintf(buf, sizeof(buf), "Form: %s\n\rParts: %s\n\r",
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	snprintf(buf, sizeof(buf), "Affected by %s\n\r",
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }

    snprintf(buf, sizeof(buf), "Master: %s  Leader: %s  Pet: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)");
    send_to_char( buf, ch );

    snprintf(buf, sizeof(buf), "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	snprintf(buf, sizeof(buf),"Mobile has special procedure %s.\n\r",
		spec_name(victim->spec_fun));
	send_to_char(buf,ch);
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	snprintf(buf, sizeof(buf),
	    "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	    skill_table[(int) paf->type].name[1],
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    affect_bit_name( paf->bitvector ),
	    paf->level
	    );
	send_to_char( buf, ch );
    }

    if (!(IS_NPC(victim)))
       {
	if (IS_SET(victim->act,PLR_QUESTOR))
	{
	 snprintf(buf, sizeof(buf),"Questgiver: %d QuestPnts: %d	Questnext: %d\n\r",
		victim->pcdata->questgiver,victim->pcdata->questpoints,
		victim->pcdata->nextquest);
	 send_to_char(buf, ch);
	 snprintf(buf, sizeof(buf),"QuestCntDown: %d	Questmob: %d\n\r",
		victim->pcdata->countdown,victim->pcdata->questmob);
	 send_to_char(buf, ch);
	}
	if  (!IS_SET(victim->act,PLR_QUESTOR))
	{
	 snprintf(buf, sizeof(buf),"QuestPnts: %d	Questnext: %d    NOT QUESTING\n\r",
		victim->pcdata->questpoints,victim->pcdata->nextquest);
	 send_to_char(buf, ch);
	}
       }

    if ( IS_NPC(victim) && victim->pIndexData->progtypes != 0 )
	printf_to_char( ch, "Mobile progs: %s\n\r", flag_names( mprog_flag_names, victim->progtypes ) );
    printf_to_char( ch, "Last fought: %*s  Last fight time: %s\n\r",
	utf8_width(victim->last_fought!=NULL?victim->last_fought->name:"none", 10), victim->last_fought!=NULL?victim->last_fought->name:"none",
	fmt_time( victim->last_fight_time ) );
    snprintf(buf, sizeof(buf), "In_mind: [%s] Hunting: [%s]\n\r",
		victim->in_mind != NULL ? victim->in_mind : "none",
		victim->hunting != NULL ? victim->hunting->name : "none");
    send_to_char(buf,ch);
    return;
}

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mfind(ch,string);
	return;
    }

    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		snprintf(buf, sizeof(buf), "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		snprintf(buf, sizeof(buf), "[%5d] %s%s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr,
	(IS_OBJ_STAT(pObjIndex,ITEM_GLOW) && CAN_WEAR(pObjIndex,ITEM_WEAR_HEAD))
		? " [parlayan]" : "" );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\n\r", ch );

    return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name )
        ||   ch->level < obj->level)
            continue;

        found = TRUE;
        number++;

        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;

        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
            snprintf(buf, sizeof(buf), "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            snprintf(buf, sizeof(buf), "%3d) %s is in %s [Room %d]\n\r",
                number, obj->short_descr,in_obj->in_room->name,
	   	in_obj->in_room->vnum);
	else
            snprintf(buf, sizeof(buf), "%3d) %s is somewhere\n\r",number, obj->short_descr);

        utf8_upper_first(buf, sizeof(buf));
        add_buf(buffer,buf);

        if (number >= max_found)
            break;
    }

    if ( !found )
        send_to_char( "Ne bu dünyada ne de diğerinde böyle bir şey yok.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	DESCRIPTOR_DATA *d;

	/* show characters logged */

	buffer = new_buf();
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->character != NULL && d->connected == CON_PLAYING
	    &&  d->character->in_room != NULL && can_see(ch,d->character)
	    &&  can_see_room(ch,d->character->in_room))
	    {
		victim = d->character;
		count++;
		if (d->original != NULL)
		    snprintf(buf, sizeof(buf),"%3d) %s (in the body of %s) is in %s [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
		else
		    snprintf(buf, sizeof(buf),"%3d) %s is in %s [%d]\n\r",
			count, victim->name,victim->in_room->name,
			victim->in_room->vnum);
		add_buf(buffer,buf);
	    }
	}

        page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( victim->in_room != NULL
	&&   is_name( argument, victim->name ) )
	{
	    found = TRUE;
	    count++;
	    snprintf(buf, sizeof(buf), "%3d) [%5d] %-*s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		utf8_width(IS_NPC(victim) ? victim->short_descr : victim->name, 28), IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name);
	    add_buf(buffer,buf);
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
    	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (ch->invis_level < LEVEL_HERO)
    snprintf(buf, sizeof(buf), "Shutdown by %s.", ch->name );
    append_file( ch, (char*)SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    if (ch->invis_level < LEVEL_HERO)
    	do_duyuru( ch, buf );
    reboot_uzakdiyarlar(FALSE);
    return;
}

void do_protect( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Protect whom from snooping?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
	send_to_char("You can't find them.\n\r",ch);
	return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("Your snoop-proofing was just removed.\n\r",victim);
	REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
    else
    {
	act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("You are now immune to snooping.\n\r",victim);
	SET_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
}



void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	wiznet("$N stops being such a snoop.",
		ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( wiz_room_blocked( ch, victim->in_room ) )
    {
        send_to_char("O karakter özel bir odada.\n\r",ch);
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch )
    ||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    snprintf(buf, sizeof(buf),"$N starts snooping on %s",
	(IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if ( wiz_room_blocked( ch, victim->in_room ) )
    {
	send_to_char("O karakter özel bir odada.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    snprintf(buf, sizeof(buf),"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char(
"You return to your original body. Type replay to see any missed tells.\n\r",
	ch );
    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    snprintf(buf, sizeof(buf),"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc                  = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    return wiz_may_load( ch, obj->level, obj->cost );
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,0);
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ( !wiz_may_load( ch, mob->level, 0 ) )
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData, NULL);
	clone_mobile(mob,clone);

	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
	snprintf(buf, sizeof(buf),"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }
    /* echo syntax */
    do_load(ch,"");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex , NULL);
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    snprintf(buf, sizeof(buf),"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }

    level = get_trust(ch); /* default */

    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE)
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  printf_to_char(victim,"%s tried to purge you!\n\r",ch->name);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    	if (victim->level > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}


void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	printf_to_char( ch, "Level must be 0 (reset) or 1 to %d.\n\r", MAX_LEVEL );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	    restore_char( ch, vch );

        snprintf(buf, sizeof(buf),"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
        send_to_char("Room restored.\n\r",ch);
        return;
    }

    if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
    {
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;
	    if (victim == NULL || IS_NPC(victim))
		continue;
	    restore_char( ch, victim );
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( MSG_NOT_HERE, ch );
	return;
    }

    restore_char( ch, victim );
    snprintf(buf, sizeof(buf),"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( MSG_OK, ch );
}


void do_freeze( CHAR_DATA *ch, char *argument )
{
    static const struct penalty p =
    {
	PEN_ACT, PLR_FREEZE, "Kimi donduracaksın?\n\r",
	"Artık HİÇBİR şey yapamazsın!\n\r", "FREEZE açıldı.\n\r", "$N %s'ı dondurdu.",
	"Yeniden oynayabilirsin.\n\r", "FREEZE kaldırıldı.\n\r", "$N %s'ın buzunu çözdü.",
	WIZ_NEED_TRUST | WIZ_SAVE
    };
    wiz_toggle_penalty( ch, argument, &p );
}



void do_log( CHAR_DATA *ch, char *argument )
{
    /* Seviye denetimi yok: tanrılar herkesi günlükleyebilir. */
    static const struct penalty p =
    {
	PEN_ACT, PLR_LOG, "Kimi günlükleyeceksin?\n\r",
	NULL, "LOG açıldı.\n\r", NULL,
	NULL, "LOG kaldırıldı.\n\r", NULL,
	0
    };
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( !str_cmp( arg, "all" ) )
    {
	fLogAll = !fLogAll;
	send_to_char( fLogAll ? "Log ALL açık.\n\r" : "Log ALL kapalı.\n\r", ch );
	return;
    }

    wiz_toggle_penalty( ch, argument, &p );
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    static const struct penalty p =
    {
	PEN_COMM, COMM_NOEMOTE, "Kimin duyguları?\n\r",
	"Duygularını gösteremezsin!\n\r", "NOEMOTE açıldı.\n\r", "$N %s'ın duygularını kapattı.",
	"Yeniden duygu gösterebilirsin.\n\r", "NOEMOTE kaldırıldı.\n\r", "$N %s'ın duygularını açtı.",
	WIZ_ALLOW_NPC | WIZ_NEED_TRUST
    };
    wiz_toggle_penalty( ch, argument, &p );
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    static const struct penalty p =
    {
	PEN_COMM, COMM_NOSHOUT, "Kimin haykırışı?\n\r",
	"Haykıramazsın!\n\r", "NOSHOUT açıldı.\n\r", "$N %s'ın haykırışını kapattı.",
	"Yeniden haykırabilirsin.\n\r", "NOSHOUT kaldırıldı.\n\r", "$N %s'ın haykırışını açtı.",
	WIZ_NEED_TRUST
    };
    wiz_toggle_penalty( ch, argument, &p );
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    static const struct penalty p =
    {
	PEN_COMM, COMM_NOTELL, "Kimin fısıltısı?\n\r",
	"Fısıldayamazsın!\n\r", "NOTELL açıldı.\n\r", "$N %s'ın fısıltısını kapattı.",
	"Yeniden fısıldayabilirsin.\n\r", "NOTELL kaldırıldı.\n\r", "$N %s'ın fısıltısını açtı.",
	WIZ_ALLOW_NPC | WIZ_NEED_TRUST
    };
    wiz_toggle_penalty( ch, argument, &p );
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
    {
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;

    if ( newlock )
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }

    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name[0] == NULL )
		break;
	    snprintf(buf, sizeof(buf), "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name[1] );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	snprintf(buf, sizeof(buf), "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name[1] );
	send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;

    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }
    /* echo syntax */
    do_set(ch,"");
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( ( skill_table[sn].name[0] != NULL )
		&& ( (victim->cabal == skill_table[sn].cabal )
		|| (skill_table[sn].cabal == CABAL_NONE) )
		&& ( RACE_OK(victim,sn) )
		)
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}


void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    snprintf( arg3, sizeof(arg3), "%s", argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title spec\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }

    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "description" ) )
    	{
    	    free_string(victim->description);
    	    victim->description = str_dup(arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
    	}
    }

    if (!str_prefix(type,"object"))
    {
    	/* string an obj */

   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Ne bu dünyada ne de diğerinde böyle bir şey yok.\n\r", ch );
	    return;
    	}

        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }


    /* echo bad use message */
    do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    snprintf( arg3, sizeof(arg3), "%s", argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    extra wear level weight cost timer\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Ne bu dünyada ne de diğerinde böyle bir şey yok.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(50,value);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }

    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    snprintf( arg3, sizeof(arg3), "%s", argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( wiz_room_blocked( ch, location ) )
    {
        send_to_char( MSG_PRIVATE_ROOM, ch );
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}



void do_sockets( CHAR_DATA *ch, char *argument )
{
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;

    count	= 0;
    buf[0]	= '\0';

    one_argument(argument,arg);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
		if ( d->character != NULL && can_see( ch, d->character )
		&& (arg[0] == '\0' || is_name(arg,d->character->name)
				   || (d->original && is_name(arg,d->original->name))))
		{
			count++;
			sprintf( buf + strlen(buf), "[%3d %2d] %s@%s\n\r",
			d->descriptor,
			d->connected,
			d->original  ? d->original->name  : d->character ? d->character->name : "(none)",
			d->host
			);
		}
    }
    if (count == 0)
    {
	send_to_char("No one by that name is connected.\n\r",ch);
	return;
    }

    snprintf(buf2, sizeof(buf2), "%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat(buf,buf2);
    page_to_char( buf, ch );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    snprintf(buf, sizeof(buf), "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument, TRUE );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }

        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
	    &&	 vch->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument, FALSE );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }

        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
            &&   vch->level >= LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument, FALSE );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( wiz_room_blocked( ch, victim->in_room ) )
	{
	    send_to_char("O karakter özel bir odada.\n\r",ch);
	    return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument, FALSE );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    /* take the default path */

      if ( ch->invis_level)
      {
	  ch->invis_level = 0;
	  act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
      else
      {
	  ch->invis_level = get_trust(ch);
	  act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
	  ch->reply = NULL;
          ch->invis_level = level;
          act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    }

    return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    /* take the default path */

      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You are no longer cloaked.\n\r", ch );
      }
      else
      {
          ch->incog_level = get_trust(ch);
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    }

    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
	if (ch->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->prefix);
	ch->prefix = str_dup("");
	return;
    }

    if (ch->prefix[0] != '\0')
    {
	snprintf(buf, sizeof(buf),"Prefix changed to %s.\r\n",argument);
	free_string(ch->prefix);
    }
    else
    {
	snprintf(buf, sizeof(buf),"Prefix set to %s.\r\n",argument);
    }

    ch->prefix = str_dup(argument);
    send_to_char(buf,ch);
}



void do_grant( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( ( victim = wiz_find_victim( ch, arg, "Kime üye alma yetkisi?\n\r", 0 ) ) == NULL )
	return;

    if (IS_SET(victim->act,PLR_CANINDUCT))
    {
      REMOVE_BIT(victim->act, PLR_CANINDUCT);
      send_to_char( "You have the lost the power to INDUCT.\n\r",victim );
      send_to_char( "INDUCT powers removed.\n\r", ch );
    }
    else {
      SET_BIT(victim->act, PLR_CANINDUCT);
      send_to_char( "You have been given the power to INDUCT.\n\r",victim);
      send_to_char( "INDUCT powers given.\n\r", ch);
    }

    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > 100 )
    {
	send_to_char( "Level must be 1 to 100.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }


    /* Level counting */
    if (ch->level <= 5 || ch->level > LEVEL_HERO)
    {
      if (5 < level && level <= LEVEL_HERO)
	total_levels += level - 5;
    }
    else
    {
      if (5 < level && level <= LEVEL_HERO)
	total_levels += level - ch->level;
      else total_levels -= (ch->level - 5);
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
	temp_prac = victim->practice;
	victim->level    = 1;
	victim->exp      = exp_to_level(victim,victim->pcdata->points);
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	advance_level( victim );
	victim->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	send_to_char( "You raise a level!!  ", victim );
	victim->exp += exp_to_level(victim,victim->pcdata->points);
	victim->level += 1;
	advance_level( victim );
    }
    victim->trust = 0;
    save_char_obj(victim);
    return;
}

void do_ikikat( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char eventbuf[MAX_STRING_LENGTH];
  int value;

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0')
  {
    send_to_char("Kullanım:\n\r",ch);
    send_to_char("  ikikat <tp | gp> <dakika>\n\r",ch);
    send_to_char("Dakika olarak 0 veya pozitif bir değer verilmelidir.\n\r",ch);

    if(ikikat_gp >= 0)
        printf_to_char(ch,"İki kat GP etkinliğinin bitmesine kalan: %d.\n\r", ikikat_gp);
    if(ikikat_tp >= 0)
        printf_to_char(ch,"İki kat TP etkinliğinin bitmesine kalan: %d.\n\r", ikikat_tp);
    return;
  }

  value = is_number( arg2 ) ? atoi( arg2 ) : 0;

  if (value < 0 )
  {
    send_to_char("Kullanım:\n\r",ch);
    send_to_char("  ikikat <tp | gp> <dakika>\n\r",ch);
    send_to_char("Dakika olarak 0 veya pozitif bir değer verilmelidir.\n\r",ch);
    return;
  }

  if ( !str_cmp( arg1, "tp" ) )
  {
    ikikat_tp = value;
    if (value != 0)
    {
      printf_to_char(ch,"İki kat TP %d dakikalığına açıldı.", value);
      /* event */
      snprintf(eventbuf, sizeof(eventbuf),"İki kat TP %d dakikalığına açıldı.", value);
	  write_event_log(eventbuf);
    }
    else
    {
      printf_to_char(ch,"İki kat TP kapatıldı.");
      /* event */
	  write_event_log("İki kat TP etkinliği kapatıldı.");
    }
    return;
  }

  if ( !str_cmp( arg1, "gp" ) )
  {
    ikikat_gp = value;
    if (value != 0)
    {
      printf_to_char(ch,"İki kat GP %d dakikalığına açıldı.", value);
      /* event */
      snprintf(eventbuf, sizeof(eventbuf),"İki kat GP %d dakikalığına açıldı.", value);
	  write_event_log(eventbuf);
    }
    else
    {
      printf_to_char(ch,"İki kat GP kapatıldı.");
      /* event */
	  write_event_log("İki kat TP etkinliği kapatıldı.");
    }
    return;
  }

  return;

}

void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value,sn;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    snprintf( arg3, sizeof(arg3), "%s", argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    str int wis dex con cha sex class level rkp\n\r",ch );
	send_to_char( "    race silver bank hp mana move practice align\n\r",	ch );
	send_to_char( "    train thirst drunk full hometown ethos\n\r",	ch );

/*** Added By KIO ***/
	send_to_char( "    questp questt relig bloodlust desire\n\r",	ch );
/*** Added By KIO ***/
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "str" ) )
    {
    	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
    	{
        printf_to_char(ch,"Güç 3 ile %d arasında olabilir.\n\r",get_max_train(victim,STAT_STR));
        return;
    	}

    	victim->perm_stat[STAT_STR] = value;
      printf_to_char(ch,"Tamam.\n\r");
    	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
    	if ( value < 3 || value > get_max_train(victim,STAT_INT) )
    	{
        printf_to_char(ch,"Zeka 3 ile %d arasında olabilir.\n\r",get_max_train(victim,STAT_INT));
        return;
    	}

    	victim->perm_stat[STAT_INT] = value;
      printf_to_char(ch,"Tamam.\n\r");
    	return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
    	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
    	{
        printf_to_char(ch,"Bilgelik 3 ile %d arasında olabilir.\n\r",get_max_train(victim,STAT_WIS));
        return;
    	}

    	victim->perm_stat[STAT_WIS] = value;
      printf_to_char(ch,"Tamam.\n\r");
    	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
    	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
    	{
        printf_to_char(ch,"Çeviklik 3 ile %d arasında olabilir.\n\r",get_max_train(victim,STAT_DEX));
        return;
    	}

    	victim->perm_stat[STAT_DEX] = value;
      printf_to_char(ch,"Tamam.\n\r");
    	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
    	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
    	{
        printf_to_char(ch,"Bünye 3 ile %d arasında olabilir.\n\r",get_max_train(victim,STAT_CON));
        return;
    	}

    	victim->perm_stat[STAT_CON] = value;
      printf_to_char(ch,"Tamam.\n\r");
    	return;
    }

    if ( !str_cmp( arg2, "cha" ) )
    {
    	if ( value < 3 || value > get_max_train(victim,STAT_CHA) )
    	{
        printf_to_char(ch,"Karizma 3 ile %d arasında olabilir.\n\r",get_max_train(victim,STAT_CHA));
        return;
    	}

    	victim->perm_stat[STAT_CHA] = value;
      printf_to_char(ch,"Tamam.\n\r");
    	return;
    }

/*** Added By KIO  ***/
    if ( !str_cmp( arg2, "questp" ) )
    {
	 if ( value == -1) value = 0;
	 if (!IS_NPC(victim)) victim->pcdata->questpoints = value;
	return;
    }
    if ( !str_cmp( arg2, "questt" ) )
    {
	 if ( value == -1) value = 30;
	 if (!IS_NPC(victim))
    {
      victim->pcdata->nextquest = value;
      if (value == 0)
      {
        victim->pcdata->questgiver = 0;
        victim->pcdata->questmob = 0;
        victim->pcdata->questroom = 0;
      }
    }

	return;
    }
    if ( !str_cmp( arg2, "relig" ) )
    {
	 if ( value == -1) value = 0;
	 victim->religion = value;
	return;
    }
/*** Added By KIO ***/



    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	if (( victim->iclass == 0) || ( victim->iclass == 8) )
	{
	    send_to_char( "You can't change their sex.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int iclass;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no class.\n\r",ch);
	    return;
	}

	iclass = class_lookup(arg3);
	if ( iclass == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

        	strcpy( buf, "Possible classes are: " );
        	for ( iclass = 0; iclass < MAX_CLASS; iclass++ )
        	{
            	    if ( iclass > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[iclass].name[1] );
        	}
            strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->iclass = iclass;
	victim->exp = victim->level * exp_per_level(victim, 0);
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Level range is 0 to 100.\n\r", ch );
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !IS_NPC(victim) && !str_prefix( arg2, "rkp" ) )
    {
	victim->pcdata->rk_puani += value;
	return;
    }

    if ( !str_prefix( arg2, "silver" ) )
    {
	victim->silver = value;
	return;
    }
	
	if ( !str_prefix( arg2, "bank" ) )
    {
	victim->pcdata->bank_s = value;
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 30000 )
	{
	    send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 60000 )
	{
	    send_to_char( "Mana range is 0 to 60,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 60000 )
	{
	    send_to_char( "Move range is 0 to 60,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 250 )
	{
	    send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 50 )
	{
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	send_to_char("Remember to check their hometown.\n\r", ch);
	return;
    }

    if (!str_prefix(arg2, "ethos"))
      {
	if (IS_NPC(victim))
	  {
	    send_to_char("Mobiles don't have an ethos.\n\r", ch);
	    return;
	  }
	if (value < 0 || value > 3)
	  {
	send_to_char("The values are Lawful - 1, Neutral - 2, Chaotic - 3\n\r",
			ch);
	    return;
	  }

	victim->ethos = value;
	return;
      }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_prefix( arg2, "bloodlust" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_BLOODLUST] = value;
	return;
    }

    if ( !str_prefix( arg2, "desire" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DESIRE] = value;
	return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim))
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( ( skill_table[sn].name[0] != NULL )
		&& !RACE_OK(victim,sn)	)
		victim->pcdata->learned[sn]	= 0;

	    if ( ( skill_table[sn].name[0] != NULL )
		&&  (ORG_RACE(victim) == skill_table[sn].race )
		)
		victim->pcdata->learned[sn]	= 70;
	}

	if (ORG_RACE(victim) == RACE(victim)) RACE(victim) = race;
	if (IS_NPC(victim)) victim->pIndexData->race = race; else victim->pcdata->race = race;

	victim->exp = victim->level * exp_per_level(victim, 0);
	return;
    }


    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}

void do_induct( CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char *arg2;
  char buf[MAX_STRING_LENGTH];
  char *cabal;
  CHAR_DATA *victim;
  int sn;
  int i, prev_cabal=0;

  cabal = NULL;
  argument = one_argument( argument, arg1);
  arg2 = argument;

  if (arg1[0] == '\0'|| *arg2 == '\0')
    {
      send_to_char("Usage: induct <player> <cabal>\n\r", ch);
      return;
    }


  if ((victim = get_char_world(ch, arg1)) == NULL)
    {
      send_to_char("That player isn't on.\n\r", ch);
      return;
    }

  if (IS_NPC(victim))
    {
      act("$N is not smart enough to join a cabal.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if ( CANT_CHANGE_TITLE( victim) )
   {
     act("$N has tried to join a cabal, but failed.",ch,NULL,victim,TO_CHAR);
     return;
   }

  if ( (i = cabal_lookup(arg2) ) == -1 )
	{
      send_to_char("I've never heard of that cabal.\n\r",ch);
      return;
	}

  if (victim->pcdata->oyuncu_katli == 0 )
  {
    act("Fakat $N oyuncu katlini kabul etmiyor!",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if (victim->iclass == CLASS_WARRIOR  && i == CABAL_SHALAFI )
  {
    act("Fakat $N pis bir büyücü!",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (i == CABAL_RULER && get_curr_stat(victim,STAT_INT) < 19)
  {
   act("$N Tüze için yeterince zeki değil!",ch,NULL,victim,TO_CHAR);
   return;
  }

  if ( IS_TRUSTED(ch,LEVEL_IMMORTAL) ||
	(IS_SET(ch->act,PLR_CANINDUCT) &&
	      ((i==CABAL_NONE && (ch->cabal == victim->cabal))
	       ||
	       (i!=CABAL_NONE && ch->cabal==i && victim->cabal==CABAL_NONE))) )
	    {
	      prev_cabal = victim->cabal;
	      victim->cabal = i;
	      REMOVE_BIT(victim->act,PLR_CANINDUCT);
	      cabal = (char*)cabal_table[i].long_name;
	    }
  else {
	    send_to_char( "You do not have that power.\n\r",ch);
	    return;
	}

  /* set cabal skills to 70, remove other cabal skills */
  for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if ( (victim->cabal) && (skill_table[sn].cabal == victim->cabal))
	victim->pcdata->learned[sn] = 70;
      else if ( skill_table[sn].cabal != CABAL_NONE &&
	       victim->cabal != skill_table[sn].cabal )
	victim->pcdata->learned[sn] = 0;
    }


  snprintf(buf, sizeof(buf), "$n has been inducted into %s.", cabal);
  act(buf,victim, NULL, NULL, TO_NOTVICT);
  snprintf(buf, sizeof(buf), "You have been inducted into %s.", cabal);
  act(buf, victim, NULL, NULL, TO_CHAR);
  if (ch->in_room != victim->in_room)
    {
      snprintf(buf, sizeof(buf), "%s has been inducted into %s.\n\r",
	      IS_NPC(victim) ? victim->short_descr : victim->name, cabal);
      send_to_char(buf, ch);
    }
  if (victim->cabal == CABAL_NONE && prev_cabal != CABAL_NONE )
    {
     	char name[100];

 	switch( prev_cabal )
	{
	 default:
	  return;
	 case CABAL_BATTLE:
	  snprintf(name, sizeof(name),"The LOVER OF MAGIC.");
	  break;
	 case CABAL_SHALAFI:
	  snprintf(name, sizeof(name),"The HATER OF MAGIC.");
	  break;
	 case CABAL_KNIGHT:
	  snprintf(name, sizeof(name),"The UNHONOURABLE FIGHTER.");
	  break;
	 case CABAL_INVADER:
	 case CABAL_CHAOS:
	 case CABAL_LIONS:
	 case CABAL_HUNTER:
	 case CABAL_RULER:
	  snprintf(name, sizeof(name),"NO MORE CABALS.");
	  break;
	}
	set_title( victim, name );
	SET_BIT(victim->act, PLR_NO_TITLE);
    }
}


void do_smite(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  if (argument[0] == '\0')
    {
      send_to_char("You are so frustrated you smite yourself!  OWW!\n\r",
		ch);
      return;
    }

  if ((victim = get_char_world(ch, argument)) == NULL)
    {
      send_to_char("You'll have to smite them some other day.\n\r", ch);
      return;
    }

  if (IS_NPC(victim))
    {
      send_to_char("That poor mob never did anything to you.\n\r", ch);
      return;
    }

  if (get_trust(victim) > get_trust(ch))
    {
      send_to_char("How dare you!\n\r", ch);
      return;
    }

  if (victim->position < POS_SLEEPING)
    {
      send_to_char("Take pity on the poor thing.\n\r", ch);
      return;
    }

  act("A bolt comes down out of the heavens and smites you!", victim, NULL,
	ch, TO_CHAR);
  act("You reach down and smite $n!", victim, NULL, ch, TO_VICT);
  act("A bolt from the heavens smites $n!", victim, NULL, ch, TO_NOTVICT);
  victim->hit = victim->hit / 2;
  return;
}

void do_popularity( CHAR_DATA *ch, char *argument )
{
char buf[4 * MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
AREA_DATA *area;
extern AREA_DATA *area_first;
int i;

    snprintf(buf, sizeof(buf),"Area popularity statistics (in char * ticks)\n\r" );

    for (area = area_first,i=0; area != NULL; area = area->next,i++) {
      if (area->count >= 5000000)
        snprintf(buf2, sizeof(buf2),"%-*s overflow       ",utf8_width(area->name, 20), area->name);
      else
        snprintf(buf2, sizeof(buf2),"%-*s %-8lu       ",utf8_width(area->name, 20), area->name,area->count);
      if ( i % 2 == 0)
	strcat( buf, "\n\r" );
      strcat( buf, buf2 );
    }
    strcat( buf, "\n\r\n\r");
    page_to_char( buf, ch );
    return;
}

void do_ititle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )  {
	send_to_char( "Change whose title to what?\n\r", ch );
	return;
    }

    victim = get_char_world( ch, arg );
    if (victim == NULL)  {
	send_to_char( "Nobody is playing with that name.\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change the title to what?\n\r", ch );
	return;
    }

    if ( strlen(argument) > 45 )
	argument[45] = '\0';

    smash_tilde( argument );
    set_title( victim, argument );
    send_to_char( "Ok.\n\r", ch );
}

bool check_parse_name (char* name);

void do_rename (CHAR_DATA* ch, char* argument)
{
	char old_name[MAX_INPUT_LENGTH],
	     new_name[MAX_INPUT_LENGTH],
	     strsave [MAX_INPUT_LENGTH];
  OBJ_DATA *obj=NULL, *obj_next;
	CHAR_DATA* victim;
	FILE* file;

	argument = one_argument(argument, old_name);
	one_argument (argument, new_name);

	if (!old_name[0])
	{
		send_to_char ("Rename who?\n\r",ch);
		return;
	}

	victim = get_char_world (ch, old_name);

	if (!victim)
	{
		send_to_char ("There is no such a person online.\n\r",ch);
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char ("You cannot use Rename on NPCs.\n\r",ch);
		return;
	}

	if ( (victim != ch) && (get_trust (victim) >= get_trust (ch)) )
	{
		send_to_char ("You failed.\n\r",ch);
		return;
	}

	if (!victim->desc || (victim->desc->connected != CON_PLAYING) )
	{
		send_to_char ("This player has lost his link or is inside a pager or the like.\n\r",ch);
		return;
	}

	if (!new_name[0])
	{
		send_to_char ("Rename to what new name?\n\r",ch);
		return;
	}

/*
	if (victim->cabal)
	{
	 send_to_char ("This player is member of a cabal, remove him from there first.\n\r",ch);
	 return;
	}
*/

	new_name[0] = UPPER(new_name[0]);
	if (!check_parse_name(new_name))
	{
		send_to_char ("The new name is illegal.\n\r",ch);
		return;
	}

        snprintf(strsave, sizeof(strsave), "%s%s", PLAYER_DIR, capitalize( new_name ) );

	fclose (fpReserve);
	file = fopen (strsave, "r");
	if (file)
	{
		send_to_char ("A player with that name already exists!\n\r",ch);
		fclose (file);
    	fpReserve = fopen( NULL_FILE, "r" );
		return;
	}
   	fpReserve = fopen( NULL_FILE, "r" );

	/* Check .gz file ! */
        snprintf(strsave, sizeof(strsave), "%s%s.gz", PLAYER_DIR, capitalize( new_name ) );

	fclose (fpReserve);
	file = fopen (strsave, "r");
	if (file)
	{
		send_to_char ("A player with that name already exists in a compressed file!\n\r",ch);
		fclose (file);
    	fpReserve = fopen( NULL_FILE, "r" );
		return;
	}
   	fpReserve = fopen( NULL_FILE, "r" );

	if (get_char_world(ch,new_name))
	{
		send_to_char ("A player with the name you specified already exists!\n\r",ch);
		return;
	}

	snprintf(strsave, sizeof(strsave), "%s%s", PLAYER_DIR, capitalize( victim->name ) );

  for(obj = object_list; obj != NULL; obj = obj_next)
  {
    obj_next = obj->next;
    if ( gorev_ekipmani_mi( obj ) )
    {
      if(strstr( obj->short_descr, victim->name))
      {
        extract_obj( obj );
      }
    }
  }

  /*
   * NOTE: Players who are level 1 do NOT get saved under a new name
   */
	free_string (victim->name);
	victim->name = str_dup (capitalize(new_name));

	save_char_obj (victim);

	remove( strsave );
	send_to_char ("Character renamed.\n\r",ch);
	victim->position = POS_STANDING;
	act ("$n has renamed you to $N!",ch,NULL,victim,TO_VICT);

}

void do_notitle( CHAR_DATA *ch, char *argument )
{
    static const struct penalty p =
    {
	PEN_ACT, PLR_NO_TITLE, "Kullanım:\n\r  notitle <oyuncu>\n\r",
	"Artık ünvanını değiştiremezsin.\n\r", MSG_OK, NULL,
	"Ünvanını yeniden değiştirebilirsin.\n\r", MSG_OK, NULL,
	0
    };

    if ( !IS_IMMORTAL(ch) )
        return;
    wiz_toggle_penalty( ch, argument, &p );
}


void do_noaffect( CHAR_DATA *ch, char * argument)
{
  AFFECT_DATA *paf,*paf_next;
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

	 if ( !IS_IMMORTAL(ch) )
        	return;

	one_argument( argument, arg );
	if ( ( victim = wiz_find_victim( ch, arg, "Kullanım:\n\r  noaffect <oyuncu>\n\r", WIZ_ALLOW_NPC ) ) == NULL )
	    return;


	for ( paf = victim->affected; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration >= 0 )
	    {
              if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, victim );
			send_to_char( "\n\r", victim );
		    }

		affect_remove( victim, paf );
	    }
	}

}

void do_affrooms(CHAR_DATA *ch, char *argument)
 {
    ROOM_INDEX_DATA *room;
    ROOM_INDEX_DATA *room_next;
    char buf[MAX_STRING_LENGTH];
    int count=0;

    if ( !top_affected_room)
	{
	 send_to_char("No affected room.\n\r",ch);
	}
    for ( room = top_affected_room; room ; room = room_next )
    {
	room_next = room->aff_next;
	count++;
	snprintf(buf, sizeof(buf),"%d) [Vnum : %5d] %s\n\r",
		count, room->vnum , room->name);
	send_to_char(buf,ch);
    }
  return;
 }

void do_find( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    char buf[MAX_STRING_LENGTH];
    char lbuf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' )
    {
	send_to_char( "Ok. But what I should find?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    snprintf(buf, sizeof(buf),"%s.\n\r",find_way(ch,ch->in_room,location));
    send_to_char(buf,ch);
    snprintf(lbuf, sizeof(lbuf),"From %d to %d: %s",ch->in_room->vnum,location->vnum,buf);
    log_string(lbuf);
    return;
}


void do_reboot( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument(argument,arg);

 if (arg[0] == '\0')
    {
      send_to_char("Usage: reboot now\n\r",ch);
      send_to_char("Usage: reboot <ticks to reboot>\n\r",ch);
      send_to_char("Usage: reboot cancel\n\r",ch);
      send_to_char("Usage: reboot status\n\r",ch);
      return;
    }

    if (is_name(arg,"cancel"))
     {
      if (time_sync)
      {
	send_to_char("Time synchronization is activated, you cannot cancel the reboot.\n\r",ch);
	return;
      }
      reboot_counter = -1;
      send_to_char("Reboot canceled.\n\r",ch);
      return;
    }

    if (is_name(arg, "now"))
     {
      reboot_uzakdiyarlar(TRUE);
      return;
    }

    if (is_name(arg, "status"))
    {
      if (time_sync)
      {
	send_to_char("Time synchronization is activated.\n\r",ch);
	return;
      }
      if (reboot_counter == -1)
	snprintf(buf, sizeof(buf), "Only time synchronization reboot is activated.\n\r");
      else
	snprintf(buf, sizeof(buf),"Reboot in %i minutes.\n\r",reboot_counter);
      send_to_char(buf,ch);
      return;
    }

    if (is_number(arg))
    {
      if (time_sync)
      {
	send_to_char("Time synchronization is activated, you cannot change the reboot.\n\r",ch);
	return;
      }
     reboot_counter = atoi(arg);
     printf_to_char(ch,"{cMangus %i dakika sonra yeniden başlatılacak.{x\n\r",reboot_counter);
     return;
    }

 do_reboot(ch,"");
}


void reboot_uzakdiyarlar( bool fmessage )
{
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;

    snprintf(log_buf, sizeof(log_buf), "Oyun yeniden baslatiliyor.");
    log_string(log_buf);
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	if (fmessage)
	   write_to_buffer(d,"***** MANGUS ŞİMDİ YENİDEN BAŞLATILIYOR! *****\n\r",0);
        if (d->character != NULL)
	{
	   update_total_played(d->character);
	   save_char_obj(d->character);
	}
    	close_socket(d);
    }

    /* soketsiz oyuncular (botlar) da kaydedilsin; etkinlik sayaçları kalıcı olsun */
    {
	CHAR_DATA *ch;

	for ( ch = char_list; ch != NULL; ch = ch->next )
	    if ( IS_BOT(ch) )
	    {
		update_total_played( ch );
		save_char_obj( ch );
	    }
    }
    ud_data_write();
    return;
}



void do_maximum( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument(argument,arg);

  if (arg[0] == '\0')
    {
      send_to_char("Usage: maximum status\n\r",ch);
      send_to_char("Usage: maximum reset\n\r",ch);
      send_to_char("Usage: maximum newbies <number of newbies>\n\r",ch);
      send_to_char("Usage: maximum oldies <number of oldies>\n\r",ch);
      return;
    }

  if (is_name(arg, "status"))
    {
      snprintf(buf, sizeof(buf), "Maximum oldies allowed: %d.\n\r", max_oldies);
      send_to_char(buf,ch);
      snprintf(buf, sizeof(buf),"Maximum newbies allowed: %d.\n\r", max_newbies);
      send_to_char(buf,ch);
      snprintf(buf, sizeof(buf),"Current number of players: %d.\n\r", iNumPlayers);
      send_to_char(buf,ch);
      return;
    }

  if (is_name(arg, "reset"))
    {
      max_newbies = MAX_NEWBIES;
      max_oldies = MAX_OLDIES;
      snprintf(buf, sizeof(buf),"Maximum newbies and oldies have been reset.\n\r");
      send_to_char(buf,ch);
      do_maximum( ch, "status" );
      return;
    }

  if (is_name(arg, "newbies"))
    {
      argument = one_argument(argument, arg);
      if ( !is_number(arg) )
	{
	 do_maximum(ch, "");
	 return;
	}
      max_newbies = atoi(arg);
      if ( max_newbies < 0 )
        snprintf(buf, sizeof(buf), "No newbies are allowed!!!\n\r");
      else
        snprintf(buf, sizeof(buf),"Now maximum newbies allowed: %d.\n\r", max_newbies);
      send_to_char(buf,ch);
      return;
    }

  if (is_name(arg, "oldies"))
    {
      argument = one_argument(argument, arg);
      if ( !is_number(arg) )
	{
	 do_maximum(ch, "");
	 return;
	}
      max_oldies = atoi(arg);
      if ( max_oldies < 0 )
        snprintf(buf, sizeof(buf), "No oldies are allowed!!!\n\r");
      else
        snprintf(buf, sizeof(buf),"Now maximum oldies allowed: %d.\n\r", max_oldies);
      send_to_char(buf,ch);
      return;
    }

 do_maximum( ch, "" );
}
