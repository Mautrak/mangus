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
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "turkish.h"
#include "bot.h"
#include "utf8.h"
#include "password.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

/* command procedures needed */
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);
DECLARE_DO_FUN( do_murder	);
DECLARE_DO_FUN( do_say  	);
DECLARE_DO_FUN( do_scan2  	);
DECLARE_DO_FUN( do_affects_col 	);
DECLARE_DO_FUN( do_who_col	);


const char *	where_name	[] =
{
  "< parmak   >  ",
  "< boyun    >  ",
  "< gövde    >  ",
  "< kafa     >  ",
  "< bacaklar >  ",
  "< ayaklar  >  ",
  "< eller    >  ",
  "< kollar   >  ",
  "< vücut    >  ",
  "< bel      >  ",
  "< bilek    >  ",
  "< sol el   >",
  "< sağ el   >",
  "< çift el  >  ",
  "< süzülen  >  ",
  "< dövme    >  ",
  "< saplanmış>  "
};


extern const char *dir_name[];

/* for do_count */
int max_on = 0;
int max_on_so_far;



/*
 * Local functions.
 */
void	show_list_to_char	( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing );
void	show_char_to_char_0	( CHAR_DATA *victim, CHAR_DATA *ch );
void	show_char_to_char_1	( CHAR_DATA *victim, CHAR_DATA *ch );
void	show_char_to_char	( CHAR_DATA *list, CHAR_DATA *ch );
bool	check_blind		( CHAR_DATA *ch );
bool	show_cwear_to_char	( CHAR_DATA *ch, OBJ_DATA *obj );


/*
 * Yuva etiketi: sol/sağ el yuvalarında 'mark' (baskın el için '*') eklenir,
 * diğerleri tablodaki gibi basılır.
 */
static const char *where_label( int iWear, char mark, char *buf, size_t n )
{
    if ( iWear == WEAR_LEFT || iWear == WEAR_RIGHT )
	snprintf( buf, n, "%s%c ", where_name[iWear], mark );
    else
	snprintf( buf, n, "%s", where_name[iWear] );
    return buf;
}

/* Sözcüğe ünlü uyumlu belirtme ('yi) ya da bulunma ('de) eki takar. */
static const char *tr_suffixed( const char *word, char *buf, size_t n, bool locative )
{
    static const char *acc_vowel[] = { "ı", "i", "u", "ü" };
    static const char *loc_vowel[] = { "a", "e", "a", "e" };
    int cls = tr_vowel_class( word );
    size_t len = strlen( word );

    if ( cls < 0 )
	cls = 0;

    if ( locative )
    {
	/* sert ünsüzden (p ç t k s ş h f) sonra 't' */
	bool hard = ( len >= 1 && strchr( "ptkshf", word[len - 1] ) != NULL )
		 || ( len >= 2 && ( !strcmp( word + len - 2, "ç" ) || !strcmp( word + len - 2, "ş" ) ) );
	snprintf( buf, n, "%s'%c%s", word, hard ? 't' : 'd', loc_vowel[cls] );
    }
    else
	snprintf( buf, n, "%s'%s%s", word, tr_ends_with_vowel( word ) ? "y" : "", acc_vowel[cls] );
    return buf;
}

bool show_cwear_to_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
 char buf[MAX_STRING_LENGTH];

  where_label( obj->wear_loc,
	( (obj->wear_loc == WEAR_LEFT && LEFT_HANDER(ch))
	|| (obj->wear_loc == WEAR_RIGHT && RIGHT_HANDER(ch)) ) ? '*' : ' ',
	buf, sizeof(buf) );
  send_to_char(buf, ch );
  if ( can_see_obj( ch, obj ) )
	send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
  else
  send_to_char( "bir şey.\n\r", ch );
  send_to_char("\n\r", ch);

 return TRUE;
}



#define OBJ_VNUM_MONEY_MAX	5	/* bu vnum'a kadar olanlar para: durum/yaş gösterilmez */

/* Eşyanın etkilerini nitelik / yp-mana-zp / vuruş-zarar toplamlarına ayırır. */
static void sum_obj_applies( AFFECT_DATA *list, int *nitelik, int *yp, int *zz )
{
    AFFECT_DATA *paf;

    for ( paf = list; paf != NULL; paf = paf->next )
    {
	switch ( paf->location )
	{
	case APPLY_HITROLL: case APPLY_DAMROLL:
	    *zz += paf->modifier;
	    break;
	case APPLY_STR: case APPLY_INT: case APPLY_WIS:
	case APPLY_DEX: case APPLY_CON: case APPLY_CHA:
	    *nitelik += paf->modifier;
	    break;
	case APPLY_MANA: case APPLY_HIT: case APPLY_MOVE:
	    *yp += paf->modifier;
	    break;
	}
    }
}

/* Yıldız rengi: 0 beyaz, <= lo kırmızı, <= hi cam göbeği, üstü magenta. */
static const char *tier_color( int v, int lo, int hi )
{
    if ( v == 0 )	return CLR_WHITE;
    if ( v <= lo )	return CLR_RED;
    if ( v <= hi )	return CLR_CYAN;
    return CLR_MAGENTA;
}

/* Tampona ekler; taşarsa keser. */
static void bufcat( char *buf, size_t n, size_t *len, const char *str )
{
    *len += snprintf( buf + *len, n - *len, "%s", str );
    if ( *len >= n )
	*len = n - 1;
}

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static const struct { long bit; long detect; const char *color; const char *tag; } tag_table[] =
    {
	{ ITEM_BURIED, 0,            CLR_WHITE,  "[gömülü] "     },
	{ ITEM_INVIS,  0,            CLR_WHITE,  "[görünmez] "   },
	{ ITEM_EVIL,   DETECT_EVIL,  CLR_RED,    "[kızıl aura] " },
	{ ITEM_BLESS,  DETECT_GOOD,  CLR_BLUE,   "[mavi aura] "  },
	{ ITEM_MAGIC,  DETECT_MAGIC, CLR_YELLOW, "[büyülü] "     },
	{ ITEM_GLOW,   0,            CLR_CYAN,   "[parlayan] "   },
	{ ITEM_HUM,    0,            CLR_BROWN,  "[vızıldayan] " },
    };
    static char buf[MAX_STRING_LENGTH];
    char tmp[MAX_INPUT_LENGTH];
    size_t len = 0, i;
    int nitelik = 0, yp = 0, zz = 0;

    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
    ||  (obj->description == NULL || obj->description[0] == '\0'))
	return buf;

    sum_obj_applies( obj->enchanted ? obj->affected : obj->pIndexData->affected,
		     &nitelik, &yp, &zz );

    bufcat( buf, sizeof(buf), &len, CLR_WHITE );
    if ( IS_IMMORTAL(ch) )
    {
	snprintf( tmp, sizeof(tmp), "[%-5d]", obj->pIndexData->vnum );
	bufcat( buf, sizeof(buf), &len, tmp );
    }
    snprintf( tmp, sizeof(tmp), "[%s*%s*%s*%s]",
	tier_color( nitelik, 2, 4 ), tier_color( yp, 100, 300 ),
	tier_color( zz, 5, 25 ), CLR_WHITE );
    bufcat( buf, sizeof(buf), &len, tmp );

    for ( i = 0; i < sizeof(tag_table) / sizeof(tag_table[0]); i++ )
    {
	if ( IS_OBJ_STAT(obj, tag_table[i].bit)
	&&   ( tag_table[i].detect == 0 || CAN_DETECT(ch, tag_table[i].detect) ) )
	{
	    bufcat( buf, sizeof(buf), &len, tag_table[i].color );
	    bufcat( buf, sizeof(buf), &len, tag_table[i].tag );
	}
    }
    bufcat( buf, sizeof(buf), &len, CLR_WHITE_BOLD );

    if ( fShort )
    {
	if ( esya_kac_gunluk(obj->creation_time) > find_material_decay_days(obj) )
	    bufcat( buf, sizeof(buf), &len, CLR_RED_BOLD );
	bufcat( buf, sizeof(buf), &len, obj->short_descr );
	bufcat( buf, sizeof(buf), &len, CLR_WHITE_BOLD );

	snprintf( tmp, sizeof(tmp), " [%s%s%s]", CLR_GREEN, obj->material, CLR_WHITE_BOLD );
	bufcat( buf, sizeof(buf), &len, tmp );
	if ( obj->pIndexData->vnum > OBJ_VNUM_MONEY_MAX )
	{
	    snprintf( tmp, sizeof(tmp), " [%s%s%s] [%s%d%s]",
		CLR_GREEN, get_cond_alias(obj), CLR_WHITE_BOLD,
		CLR_GREEN, esya_kac_gunluk(obj->creation_time), CLR_WHITE_BOLD );
	    bufcat( buf, sizeof(buf), &len, tmp );
	}
    }
    else if ( obj->in_room != NULL && IS_WATER( obj->in_room ) )
    {
	size_t start = len;

	bufcat( buf, sizeof(buf), &len, obj->short_descr );
	utf8_upper_first( buf + start, sizeof(buf) - start );
	len = strlen( buf );
	switch ( dice(1,3) )
	{
	case 1:  bufcat( buf, sizeof(buf), &len, " suyun üstünde yüzüyor." ); break;
	case 2:  bufcat( buf, sizeof(buf), &len, " suda ilerliyor." );        break;
	default: bufcat( buf, sizeof(buf), &len, " suda ıslanıyor." );        break;
	}
    }
    else
	bufcat( buf, sizeof(buf), &len, obj->description );

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf();

    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= (char **)alloc_mem( count * sizeof(char *) );
    prgnShow    = (int *)alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ))
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );

	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if (prgpstrShow[iShow][0] == '\0')
	{
	    free_string(prgpstrShow[iShow]);
	    continue;
	}

	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		snprintf(buf, sizeof(buf), "(%2d) ", prgnShow[iShow] );
		add_buf(output,buf);
	    }
	    else
	    {
		add_buf(output,"     ");
	    }
	}
	add_buf(output,prgpstrShow[iShow]);
	add_buf(output,"\n\r");
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
      send_to_char( "Hiçbir şey.\n\r", ch );
    }
    page_to_char(buf_string(output),ch);

    /*
     * Clean up.
     */
    free_buf(output);
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}



/* Duruşa göre fiil ve mobilya bayrakları (uyuyor / dinleniyor / oturuyor / duruyor). */
static const char *pos_verb[] =
{
    [POS_SLEEPING] = "uyuyor",   [POS_RESTING]  = "dinleniyor",
    [POS_SITTING]  = "oturuyor", [POS_STANDING] = "duruyor",
};
static const int pos_at_bit[] =
{
    [POS_SLEEPING] = SLEEP_AT, [POS_RESTING] = REST_AT,
    [POS_SITTING]  = SIT_AT,   [POS_STANDING] = STAND_AT,
};
static const int pos_on_bit[] =
{
    [POS_SLEEPING] = SLEEP_ON, [POS_RESTING] = REST_ON,
    [POS_SITTING]  = SIT_ON,   [POS_STANDING] = STAND_ON,
};

/* " masa'da uyuyor." / " taht üzerinde oturuyor." / " çadır içinde dinleniyor." */
static void furniture_phrase( char *out, size_t n, OBJ_DATA *on, int pos )
{
    char tmp[MAX_INPUT_LENGTH];

    if ( IS_SET(on->value[2], pos_at_bit[pos]) )
	snprintf( out, n, " %s %s.", tr_suffixed( on->short_descr, tmp, sizeof(tmp), TRUE ), pos_verb[pos] );
    else if ( IS_SET(on->value[2], pos_on_bit[pos]) )
	snprintf( out, n, " %s üzerinde %s.", on->short_descr, pos_verb[pos] );
    else
	snprintf( out, n, " %s içinde %s.", on->short_descr, pos_verb[pos] );
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH],message[MAX_STRING_LENGTH],tmp[MAX_INPUT_LENGTH];

    buf[0] = '\0';

   /*
    * Quest staff
    */
   if (!IS_NPC(ch) && IS_NPC(victim)
	&& ch->pcdata->questmob > 0
	&& victim->pIndexData->vnum == ch->pcdata->questmob)
  {
		strcat( buf, "{R[hedef]{x ");
}
    if ( RIDDEN(victim)  ) 			strcat( buf, "[binek] "     );
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "[görünmez] "      );
    if ( IS_AFFECTED(victim,AFF_IMP_INVIS )   ) strcat( buf, "[gelişmiş] "   );
    if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "[WiZ] "	     );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "[saklı] "       );
    if ( IS_AFFECTED(victim, AFF_FADE)        ) strcat( buf, "[solmuş] "       );
    if ( IS_AFFECTED(victim, AFF_CAMOUFLAGE)  ) strcat( buf, "[kamufle] "       );
    if ( CAN_DETECT(victim, ADET_EARTHFADE)   ) strcat( buf, "[arz] "      );
    if ( IS_AFFECTED(victim, AFF_CHARM)
	&& victim->master == ch) 		strcat( buf, "[teshir] "    );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "{W[yarısaydam]{x ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "{M[pembe aura]{x "  );
    if ( IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)
    &&   CAN_DETECT(ch, DETECT_UNDEAD)     ) strcat( buf, "{W[hortlak]{x ");
    if ( IS_EVIL(victim)
    &&   CAN_DETECT(ch, DETECT_EVIL)     ) strcat( buf, "{R[kızıl aura]{x "   );
    if ( IS_GOOD(victim)
    &&   CAN_DETECT(ch, DETECT_GOOD)     ) strcat( buf, "{Y[altın aura]{x ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "{W[beyaz aura]{x " );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED ) )
						strcat( buf, "[SUÇLU] ");

    if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
    {
	strcat( buf, victim->long_descr );
	printf_to_char( ch, "%s", buf );
	return;
    }

    if ( IS_SET(ch->act,PLR_HOLYLIGHT) && is_affected(victim,gsn_doppelganger))
      {
        strcat( buf, "{"); strcat(buf, PERS(victim,ch)); strcat(buf, "} ");
      }

    if (is_affected(victim,gsn_doppelganger)  &&
        victim->doppel->long_descr[0] != '\0') {
      strcat( buf, victim->doppel->long_descr);
      send_to_char(buf, ch);
      return;
      }

    if (victim->long_descr[0] != '\0' &&
        !is_affected(victim,gsn_doppelganger)) {
      strcat( buf, victim->long_descr );
      send_to_char(buf, ch);
      return;
    }

    if (is_affected(victim, gsn_doppelganger))
      {
        strcat(buf, PERS(victim->doppel, ch ));
        if (!IS_NPC(victim->doppel) && !IS_SET(ch->comm, COMM_BRIEF))
          strcat(buf, victim->doppel->pcdata->title);
      }
   else
   {
     strcat( buf, PERS( victim, ch ) );
     if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF)
     &&   victim->position == POS_STANDING && ch->on == NULL )
 	 strcat( buf, victim->pcdata->title );
    }

    switch ( victim->position )
    {
      case POS_DEAD:
  		strcat( buf, " ÖLDÜ!!" );
  		break;
      case POS_MORTAL:
  		strcat( buf, " öldürücü yaralar almış." );
  		break;
      case POS_INCAP:
  		strcat( buf, " aciz durumda." );
  		break;
      case POS_STUNNED:
  		strcat( buf, " sersemlemiş yatıyor." );
  		break;
    case POS_SLEEPING:
    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
	if ( victim->on != NULL )
	    furniture_phrase( message, sizeof(message), victim->on, victim->position );
	else if ( victim->position == POS_STANDING && MOUNTED(victim) )
	    snprintf( message, sizeof(message), " burada, %s sürüyor.",
		tr_suffixed( PERS(MOUNTED(victim),ch), tmp, sizeof(tmp), FALSE ) );
	else if ( victim->position == POS_STANDING )
	    snprintf( message, sizeof(message), " burada." );
	else
	    snprintf( message, sizeof(message), " burada %s.", pos_verb[victim->position] );
	strcat( buf, message );
	break;
    case POS_FIGHTING:
    strcat( buf, " burada, " );
	if ( victim->fighting == NULL )
  strcat( buf, "havayla dövüşüyor??" );
	else if ( victim->fighting == ch )
  strcat( buf, "SENinle dövüşüyor!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, " ile dövüşüyor." );
	}
	else
  strcat( buf, "kim kaldıysa onunla dövüşüyor??" );
	break;
    }

    strcat( buf, "\n\r" );
    utf8_upper_first(buf, sizeof(buf));
    send_to_char( buf, ch );
    return;
}

/*
 * Bakan, hedefin gerçek durumunu seçebiliyor mu? Önce bilgelik zarı (ölümsüz
 * her zaman), sonra ırk irfanı; yaratıkların irfanı yoktur.
 */
static bool perceives_truly( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( number_range( 1, 26 ) < get_curr_stat( ch, STAT_WIS ) || IS_IMMORTAL(ch) )
	return TRUE;

    if ( IS_NPC(ch) || victim->race < 0 || victim->race >= MAX_RACE )
	return FALSE;

    return number_range( 1, 100 ) < ch->pcdata->familya[victim->race];
}

char *show_char_to_char_1_health_check(CHAR_DATA *ch , CHAR_DATA *victim)
{
  int percent;

  if ( perceives_truly( ch, victim ) )
  {
    if ( victim->max_hit > 0 )
      percent = ( 100 * victim->hit ) / victim->max_hit;
    else
      percent = -1;
  }
  else
  {
    percent = number_range(51,89);
  }

  /* vampire ... */
  if (percent < 90 && ch->iclass == CLASS_VAMPIRE && ch->level > 10)
  {
	   gain_condition(ch,COND_BLOODLUST,-1);
   }

  if (percent >= 100)
  {
    return "mükemmel durumda";
  }
  else if (percent >= 90)
  {
    return "birkaç çiziği var";
  }
  else if (percent >= 75)
  {
    return "birkaç küçük yarası var";
  }
  else if (percent >=  50)
  {
    return "kanayan yaralarla kaplanmış";
  }
  else if (percent >= 30)
  {
    return "kötü kanıyor";
  }
  else if (percent >= 15)
  {
    return "acı içinde kıvranıyor";
  }
  else if (percent >= 0 )
  {
    return "yerde sürünüyor";
  }

  return "ölmek üzere";
}

char *show_char_to_char_1_alignment(CHAR_DATA *ch , CHAR_DATA *victim)
{
  int alignment;

  if ( perceives_truly( ch, victim ) )
    alignment = victim->alignment;
  else
    alignment = number_range(-1000,1000);

  if (alignment >= 350)
    return "'iyi' olabilir";
  if (alignment <= -350)
    return "'kem' olabilir";
  return "'yansız' olabilir";
}



/* Yuva bakan için örtülü mü? Ölümsüz her şeyi görür. */
static bool slot_hidden( int iWear, bool about, bool hands, CHAR_DATA *viewer )
{
    if ( IS_IMMORTAL(viewer) )
	return FALSE;

    switch ( iWear )
    {
    case WEAR_FINGER:
	return hands;
    case WEAR_NECK: case WEAR_WRIST: case WEAR_TATTOO:
    case WEAR_BODY: case WEAR_LEGS:  case WEAR_ARMS: case WEAR_WAIST:
	return about;
    }
    return FALSE;
}

/* Tek ekipman satırı: yuva adı + (örtülü | görülen eşya | boş). */
static void show_eq_line( CHAR_DATA *ch, OBJ_DATA *obj, int iWear, bool hidden )
{
    char label[MAX_INPUT_LENGTH];

    where_label( iWear, ' ', label, sizeof(label) );
    if ( hidden )
	printf_to_char( ch, "%s {DÖrtülü{x\n\r", label );
    else if ( obj != NULL && can_see_obj( ch, obj ) )
	printf_to_char( ch, "%s %s\n\r", label, format_obj_to_char( obj, ch, TRUE ) );
    else
	printf_to_char( ch, "%s\n\r", label );
}

void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
  OBJ_DATA *obj;
  int iWear, i;
  CHAR_DATA *vict;
  bool about, hands;

  vict = is_affected(victim,gsn_doppelganger) ? victim->doppel : victim;

  if ( can_see( victim, ch ) )
  {
    if (ch == victim)
    {
      act( "$n kendisine bakıyor.",ch,NULL,NULL,TO_ROOM);
    }
    else
    {
      act("$n sana bakıyor.", ch, NULL, victim, TO_VICT    );
      act( "$n $E bakıyor.",  ch, NULL, victim, TO_NOTVICT );
    }
  }

  printf_to_char(ch,"[{c%s{x] [{c%s{x] [{c%s{x] [{c%s{x]\n\r\n\r",(vict->sex==SEX_MALE?"erkek":"kadın"),race_table[RACE(vict)].name[1], show_char_to_char_1_alignment(ch,victim),show_char_to_char_1_health_check(ch,victim));

  if ( vict->description[0] != '\0' )
  {
    printf_to_char(ch,"%s\n\r\n\r",vict->description);
  }
  else
  {
    printf_to_char(ch,"Onun hakkında ilgi çekici birşey bulamıyorsun.\n\r\n\r");
  }

  if ( MOUNTED(victim) )
  {
    printf_to_char(ch,"%s %s sürüyor.\n\r\n\r",PERS(victim,ch), PERS( MOUNTED(victim),ch));
  }
  if ( RIDDEN(victim) )
  {
    printf_to_char(ch,"%s %s tarafından sürülüyor.\n\r\n\r",PERS(victim,ch), PERS( RIDDEN(victim),ch));
  }

  act( "$S kullandıklarına göz atıyorsun:", ch, NULL, victim, TO_CHAR);
  printf_to_char( ch,"\n\r" );

  /* Pelerin gövde yuvalarını, eldiven parmakları örter. */
  about = get_eq_char( vict, WEAR_ABOUT ) != NULL;
  hands = get_eq_char( vict, WEAR_HANDS ) != NULL;

  for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
  {
    bool hidden = slot_hidden( iWear, about, hands, ch );
    bool multi  = ( iWear == WEAR_FINGER || iWear == WEAR_NECK
		 || iWear == WEAR_WRIST  || iWear == WEAR_TATTOO );
    bool listed = multi || iWear == WEAR_STUCK_IN
		 || iWear == WEAR_BODY || iWear == WEAR_LEGS
		 || iWear == WEAR_ARMS || iWear == WEAR_WAIST;

    if ( !listed )
    {
      show_eq_line( ch, get_eq_char( vict, iWear ), iWear, hidden );
      continue;
    }

    for ( obj = vict->carrying; obj != NULL; obj = obj->next_content )
    {
      if ( obj->wear_loc != iWear )
	continue;
      if ( iWear == WEAR_STUCK_IN && !can_see_obj( ch, obj ) )
	continue;
      show_eq_line( ch, obj, iWear, hidden );
    }

    if ( multi )
      for ( i = count_worn( vict, iWear ); i < max_can_wear( vict, iWear ); i++ )
	show_eq_line( ch, NULL, iWear, hidden );
  }

  if ( victim != ch
  &&   !IS_NPC(ch)
  &&   number_percent( ) < get_skill(ch,gsn_peek))
  {
    printf_to_char( ch,"\n\rEnvantere göz atıyorsun:\n\r" );
    check_improve(ch,gsn_peek,TRUE,4);
    show_list_to_char( vict->carrying, ch, TRUE, TRUE );
  }
  return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;
    int life_count=0;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( get_trust(ch) < rch->invis_level)
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
    send_to_char( "SENi izleyen kırmızı gözler görüyorsun!\n\r", ch );
	    if (!IS_IMMORTAL(rch)) life_count++;
	}
	else if (!IS_IMMORTAL(rch)) life_count++;
    }

    if (life_count && CAN_DETECT(ch,DETECT_LIFE) )
	{
	 char buf[MAX_STRING_LENGTH];
   snprintf(buf, sizeof(buf),"Odada %d yaşam formu seziyorsun.\n\r",life_count);
	 send_to_char(buf,ch);
	}
    return;
}



bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
      send_to_char("Hiçbir şey göremiyorsun!\n\r", ch );
	return FALSE;
    }

    return TRUE;
}

void do_clear( CHAR_DATA *ch, char *argument )
{
  if ( !IS_NPC(ch) )
    send_to_char("\033[0;0H\033[2J", ch );
  return;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    int lines;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("Uzun mesajları sayfalamıyorsun.\n\r",ch);
	else
	    printf_to_char(ch,"Sayfa başına %d satır gösteriyorsun.\n\r",
		    ch->lines + 2);
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("Bir sayı vermelisin.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Sayfalama kapatıldı.\n\r",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("Makul bir sayı vermelisin (10-100).\n\r",ch);
	return;
    }

    printf_to_char(ch,"Sayfa uzunluğu %d satıra ayarlandı.\n\r",lines);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;

    col = 0;

    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	snprintf(buf, sizeof(buf),"%-*s",utf8_width(social_table[iSocial].name, 12), social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

/* Bayrak aç/kapa komutlarının ortak gövdesi. */
static void toggle_flag( CHAR_DATA *ch, long *field, long bit,
			 const char *on_msg, const char *off_msg )
{
    if ( IS_SET(*field, bit) )
    {
	send_to_char( off_msg, ch );
	REMOVE_BIT(*field, bit);
    }
    else
    {
	send_to_char( on_msg, ch );
	SET_BIT(*field, bit);
    }
}

/* 'oto' listesi: etiket, act mi comm mu, bit. */
static const struct { const char *label; bool comm; long bit; } auto_table[] =
{
    { "otodestek", FALSE, PLR_AUTOASSIST },
    { "otoçıkış",  FALSE, PLR_AUTOEXIT   },
    { "otoakçe",   FALSE, PLR_AUTOAKCE   },
    { "otoyağma",  FALSE, PLR_AUTOLOOT   },
    { "otokurban", FALSE, PLR_AUTOSAC    },
    { "otodağıt",  FALSE, PLR_AUTOSPLIT  },
    { "kısa",      TRUE,  COMM_COMPACT   },
    { "suflör",    TRUE,  COMM_PROMPT    },
    { "bileşik",   TRUE,  COMM_COMBINE   },
};

void do_autolist(CHAR_DATA *ch, char *argument)
{
    size_t i;

    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("oto  durumu\n\r",ch);
    send_to_char("---------------------\n\r",ch);

    for ( i = 0; i < sizeof(auto_table) / sizeof(auto_table[0]); i++ )
    {
	long field = auto_table[i].comm ? ch->comm : ch->act;

	printf_to_char( ch, "%-*s%s\n\r", utf8_width( auto_table[i].label, 16 ),
	    auto_table[i].label, IS_SET(field, auto_table[i].bit) ? "AÇIK" : "KAPALI" );
    }

    if (IS_SET(ch->act,PLR_NOSUMMON))
    send_to_char("Ancak OK aralığının çağrı büyülerinden etkilenebilirsin.\n\r",ch);
    else
    send_to_char("Herkesin çağrı büyüsünden etkilenebilirsin.\n\r",ch);

    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("Takipçileri kabul etmiyorsun.\n\r",ch);
    else
	send_to_char("Takipçileri kabul ediyorsun.\n\r",ch);

    if (IS_SET(ch->act,PLR_NOCANCEL))
    send_to_char("Ancak OK aralığının iptal büyülerinden etkilenebilirsin.\n\r",ch);
    else
    send_to_char("Herkesin iptal büyüsünden etkilenebilirsin.\n\r",ch);
}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    toggle_flag( ch, &ch->act, PLR_AUTOASSIST,
	"Otodestek açıldı.\n\r", "Otodestek kaldırıldı.\n\r" );
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    toggle_flag( ch, &ch->act, PLR_AUTOEXIT,
	"Otoçıkış açıldı.\n\r", "Otoçıkış kaldırıldı.\n\r" );
}

void do_autoakce(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    toggle_flag( ch, &ch->act, PLR_AUTOAKCE,
	"Otoakçe açıldı.\n\r", "Otoakçe kaldırıldı.\n\r" );
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    toggle_flag( ch, &ch->act, PLR_AUTOLOOT,
	"Otoyağma açıldı.\n\r", "Otoyağma kaldırıldı.\n\r" );
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    toggle_flag( ch, &ch->act, PLR_AUTOSAC,
	"Otokurban açıldı.\n\r", "Otokurban kaldırıldı.\n\r" );
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    toggle_flag( ch, &ch->act, PLR_AUTOSPLIT,
	"Otodağıt açıldı.\n\r", "Otodağıt kaldırıldı.\n\r" );
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    toggle_flag( ch, &ch->comm, COMM_BRIEF,
	"Özet tanım açıldı.\n\r", "Özet tanım kapandı.\n\r" );
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    toggle_flag( ch, &ch->comm, COMM_COMPACT,
	"Kompakt modu açıldı.\n\r", "Kompakt modu kapatıldı.\n\r" );
}

void do_show(CHAR_DATA *ch, char *argument)
{
    toggle_flag( ch, &ch->comm, COMM_SHOW_AFFECTS,
	"Etkiler skor tablosunda gösterilecek.\n\r",
	"Etkiler skor tablosunda gösterilmeyecek.\n\r" );
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
  if ( argument[0] != '\0' )
  {
     send_to_char( "Bu komutla argüman kullanılmaz.\n\r", ch );
     return;
  }
  toggle_flag( ch, &ch->comm, COMM_PROMPT,
	"Suflör açıldı.\n\r", "Suflör kapatıldı.\n\r" );
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    toggle_flag( ch, &ch->comm, COMM_COMBINE,
	"Bileşik nesne açıldı.\n\r", "Bileşik nesne kaldırıldı.\n\r" );
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    toggle_flag( ch, &ch->act, PLR_CANLOOT,
	"Cesedin yağmalanabilir.\n\r", "Cesedin hırsızlara karşı güvende.\n\r" );
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    if ( IS_AFFECTED( ch, AFF_CHARM ) )  {
	send_to_char( "Sevgili efendinden ayrılmak istemiyorsun.\n\r",ch);
	return;
    }

    toggle_flag( ch, &ch->act, PLR_NOFOLLOW,
	"Takipçileri kabul etmiyorsun.\n\r", "Takipçileri kabul etmeye başladın.\n\r" );
    if ( IS_SET(ch->act, PLR_NOFOLLOW) )
	die_follower( ch );
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
	toggle_flag( ch, &ch->imm_flags, IMM_SUMMON,
	    "Çağrı büyülerine bağışıklısın.\n\r",
	    "Çağrı büyülerine bağışıklı değilsin.\n\r" );
    else
	toggle_flag( ch, &ch->act, PLR_NOSUMMON,
	    "Yalnız OK aralığının çağrı büyülerine açıksın.\n\r",
	    "Herkesin çağrı büyüsüne açıksın.\n\r" );
}

/* Sektör türünün oda başlığındaki Türkçe adı. */
static const char *sector_name_tr( int sector )
{
    static const char *names[] =
    {
	[SECT_INSIDE]       = "İçeri", [SECT_CITY]   = "Şehir",  [SECT_FIELD]  = "Ova",
	[SECT_FOREST]       = "Orman", [SECT_HILLS]  = "Tepe",   [SECT_MOUNTAIN] = "Dağ",
	[SECT_WATER_SWIM]   = "Su",    [SECT_WATER_NOSWIM] = "Su", [SECT_AIR]  = "Hava",
	[SECT_DESERT]       = "Çöl",   [SECT_MAX]    = "Zorlu",
    };

    if ( sector >= 0 && sector < (int) ( sizeof(names) / sizeof(names[0]) ) && names[sector] != NULL )
	return names[sector];
    return "*Bilinmeyen*";
}

/* Yön argümanı: dir_name'in ön eki (k, ku, kuzey...). Değilse -1. */
static int dir_lookup( const char *arg )
{
    int door;

    if ( arg[0] == '\0' )
	return -1;
    for ( door = 0; door < 6; door++ )
	if ( !str_prefix( arg, dir_name[door] ) )
	    return door;
    return -1;
}

/* 'bak' / 'bak auto': oda başlığı, tanım, çıkışlar, eşyalar, kişiler. */
static void look_room( CHAR_DATA *ch, bool full )
{
    printf_to_char( ch, "{y%s{x [%s] [{y%s{x]", ch->in_room->name,
	sector_name_tr( ch->in_room->sector_type ), ch->in_room->area->name );

    if (IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)))
      printf_to_char(ch," [%d]",ch->in_room->vnum);

    send_to_char( "\n\r", ch );

    if ( full || ( IS_PC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
    {
      send_to_char( "  ", ch );
      send_to_char( ch->in_room->description, ch );
    }

    if ( IS_PC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
    {
      send_to_char( "\n\r", ch );
      do_exits( ch, "auto" );
    }

    show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
    show_char_to_char( ch->in_room->people,   ch );
}

/* 'bak içine <taşıyıcı>' ve 'incele': taşıyıcının içi. */
static void look_in_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    switch ( obj->item_type )
    {
      default:
        send_to_char( "O bir taşıyıcı değil.\n\r", ch );
        break;

      case ITEM_DRINK_CON:
        if ( obj->value[1] <= 0 )
        {
          send_to_char( "İçi boş.\n\r", ch );
          break;
        }

        printf_to_char(ch,"%s %s sıvısıyla dolu.\n\r",obj->value[1] < obj->value[0] / 4 ? "Yarıdan azı" : obj->value[1] < 3 * obj->value[0] / 4 ? "Yarısı"     : "Yarıdan fazlası",liq_table[obj->value[2]].liq_color);
        break;

      case ITEM_CONTAINER:
      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
        if ( IS_SET(obj->value[1], CONT_CLOSED) )
        {
          send_to_char( "Kapalı.\n\r", ch );
          break;
        }

        act( "$p şunları içeriyor:", ch, obj, NULL, TO_CHAR );
        show_list_to_char( obj->contains, ch, TRUE, TRUE );
        break;
    }
}

/* Aşk iksiri: bakılan kişiye büyülenir. */
static void love_potion_trigger( CHAR_DATA *ch, CHAR_DATA *victim )
{
    AFFECT_DATA af;

    affect_strip(ch, gsn_love_potion);

    if (ch->master)
      stop_follower(ch);
    add_follower(ch, victim);
    ch->leader = victim;

    af.where = TO_AFFECTS;
    af.type = gsn_charm_person;
    af.level = ch->level;
    af.duration =  number_fuzzy(victim->level / 4);
    af.bitvector = AFF_CHARM;
    af.modifier = 0;
    af.location = 0;
    affect_to_char(ch, &af);

    act("$n sence de tatlı değil mi?", victim, NULL, ch, TO_VICT);
    act("$N büyülenmiş gözlerle sana bakıyor.",victim,NULL,ch,TO_CHAR);
    act("$N büyülenmiş gözlerle $e bakıyor.",victim,NULL,ch,TO_NOTVICT);
}

/* 'bak <kişi>' */
static void look_char( CHAR_DATA *ch, CHAR_DATA *victim )
{
    show_char_to_char_1( victim, ch );

    if ( is_affected( ch, gsn_love_potion ) && victim != ch )
	love_potion_trigger( ch, victim );
}

/*
 * 'bak <eşya/anahtar>': envanter ve odadaki eşyaların ekstra tanımları,
 * eşya tanımı, odanın ekstra tanımı. Bir şey basıldıysa (ya da sayı
 * uyuşmazlığı bildirildiyse) TRUE döner.
 */
static bool look_extra( CHAR_DATA *ch, char *arg3, int number )
{
    OBJ_DATA *obj;
    char *pdesc;
    int count = 0;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( !can_see_obj( ch, obj ) )
	    continue;

	if ( ( pdesc = get_extra_descr( arg3, obj->extra_descr ) ) != NULL
	||   ( pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr ) ) != NULL )
	{
	    if ( ++count == number )
	    {
		send_to_char( pdesc, ch );
		return TRUE;
	    }
	    continue;
	}

	if ( is_name( arg3, obj->name ) && ++count == number )
	{
	    send_to_char( "Özel bir şey görmüyorsun.\n\r", ch );
	    return TRUE;
	}
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    if ( ( pdesc = get_extra_descr( arg3, obj->extra_descr ) ) != NULL
	    &&   ++count == number )
	    {
		send_to_char( pdesc, ch );
		return TRUE;
	    }

	    if ( ( pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr ) ) != NULL
	    &&   ++count == number )
	    {
		send_to_char( pdesc, ch );
		return TRUE;
	    }
	}

	if ( is_name( arg3, obj->name ) && ++count == number )
	{
	    printf_to_char( ch, "%s\n\r", obj->description );
	    return TRUE;
	}
    }

    if ( ( pdesc = get_extra_descr( arg3, ch->in_room->extra_descr ) ) != NULL
    &&   ++count == number )
    {
	send_to_char( pdesc, ch );
	return TRUE;
    }

    if ( count > 0 && count != number )
    {
	if ( count == 1 )
	    printf_to_char( ch, "Sadece bir %s görüyorsun.\n\r", arg3 );
	else
	    printf_to_char( ch, "Ondan sadece %d tane görüyorsun.\n\r", count );
	return TRUE;
    }

    return FALSE;
}

/* 'bak <yön>' */
static void look_dir( CHAR_DATA *ch, int door )
{
    EXIT_DATA *pexit;

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Özel bir şey yok.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Özel bir şey yok.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    act( "$d kapalı.", ch, NULL, pexit->keyword, TO_CHAR );
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	    act( "$d açık.",   ch, NULL, pexit->keyword, TO_CHAR );
    }
}

void do_look( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int door;
  int number;

  if ( ch->desc == NULL )
    return;

  if ( ch->position < POS_SLEEPING )
  {
    send_to_char( "Yıldızlardan başka bir şey göremiyorsun!\n\r", ch );
    return;
  }

  if ( ch->position == POS_SLEEPING )
  {
    send_to_char( "Bir şey göremiyorsun, uyuyorsun!\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) )
  {
    send_to_char( "Bir şey göremiyorsun, körsün!\n\r", ch );
    return;
  }

  if ( IS_PC(ch)
      && !IS_SET(ch->act, PLR_HOLYLIGHT)
      && !IS_SET(ch->act,PLR_GHOST)
      && room_is_dark( ch ) )
  {
    send_to_char( "Zifiri karanlık ... \n\r", ch );
    show_char_to_char( ch->in_room->people, ch );
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  number = number_argument(arg1,arg3);

  if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
  {
    look_room( ch, arg1[0] == '\0' );
    return;
  }

  if ( !str_cmp( arg1, "in" ) || !str_cmp( arg1, "on" )
  ||   !str_prefix( arg1, "içine" ) || !str_prefix( arg1, "içinde" ) )
  {
    /* 'bak içine <taşıyıcı>' */
    if ( arg2[0] == '\0' )
    {
      send_to_char( "Neyin içine bakacaksın?\n\r", ch );
      return;
    }

    if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
    {
      send_to_char( "Onu görmüyorsun.\n\r", ch );
      return;
    }

    look_in_obj( ch, obj );
    return;
  }

  if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
  {
    look_char( ch, victim );
    return;
  }

  if ( look_extra( ch, arg3, number ) )
    return;

  if ( ( door = dir_lookup( arg1 ) ) < 0 )
  {
    send_to_char( "Onu görmüyorsun.\n\r", ch );
    return;
  }

  look_dir( ch, door );
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_look(ch,argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
      send_to_char("Neyi inceleyeceksin?\n\r", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_MONEY:
	    if (obj->value[0] == 0)
	    {
        if (obj->value[1] == 0)
        snprintf(buf, sizeof(buf),"Tuhaf...yığın içinde akçe yok.\n\r");
        else if (obj->value[1] == 1)
        snprintf(buf, sizeof(buf),"Vayy. Bir akçe.\n\r");
        else
        snprintf(buf, sizeof(buf),"Yığında %d akçe var.\n\r",obj->value[1]);
	    }
	    else if (obj->value[1] == 0)
	    {
		if (obj->value[0] == 1)
    snprintf(buf, sizeof(buf),"Vayy. Bir akçe.\n\r");
		else
    snprintf(buf, sizeof(buf),"Yığında %d akçe var.\n\r",obj->value[0]);
	    }
	    else
		snprintf(buf, sizeof(buf),
      "Yığında %d akçe var.\n\r",obj->value[1] + obj->value[0]);
	    send_to_char(buf,ch);
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    look_in_obj( ch, obj );
	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;
    size_t len;

    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

  if (fAuto)
  {
    snprintf(buf, sizeof(buf), "%s[", CLR_CYAN);
  }
  else if (IS_IMMORTAL(ch))
  {
    snprintf(buf, sizeof(buf),"%d nolu odadan çıkışlar:\n\r",ch->in_room->vnum);
  }
  else
  {
    snprintf(buf, sizeof(buf),"Görünen çıkışlar:\n\r");
  }

  found = FALSE;
  len = strlen( buf );
  for ( door = 0; door <= 5; door++ )
  {
    bool closed;

    if ( ( pexit = ch->in_room->exit[door] ) == NULL
    ||   pexit->u1.to_room == NULL
    ||   !can_see_room( ch, pexit->u1.to_room ) )
	continue;

    found  = TRUE;
    closed = IS_SET(pexit->exit_info, EX_CLOSED);

    if ( fAuto )
    {
	len += snprintf( buf + len, sizeof(buf) - len, " %s%s",
	    dir_name[door], closed ? "*" : "" );
	continue;
    }

    if ( closed )
	len += snprintf( buf + len, sizeof(buf) - len, "%-*s * (%s)",
	    utf8_width( capitalize( dir_name[door] ), 5 ), capitalize( dir_name[door] ),
	    pexit->keyword );
    else
	len += snprintf( buf + len, sizeof(buf) - len, "%-*s - %s",
	    utf8_width( capitalize( dir_name[door] ), 5 ), capitalize( dir_name[door] ),
	    room_dark( pexit->u1.to_room ) ? "Zifiri karanlık" : pexit->u1.to_room->name );

    if ( IS_IMMORTAL(ch) )
	len += snprintf( buf + len, sizeof(buf) - len, " (oda %d)\n\r", pexit->u1.to_room->vnum );
    else
	len += snprintf( buf + len, sizeof(buf) - len, "\n\r" );
    if ( len >= sizeof(buf) )
	len = sizeof(buf) - 1;
  }

    if ( !found )
	strcat( buf, fAuto ? " hiç" : "Hiç.\n\r" );

    if ( fAuto )
	{
	  strcat( buf, " ]\n\r" );
	  strcat( buf, CLR_WHITE_BOLD);
  }
    send_to_char( buf, ch );
    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int total_played;

    if (IS_NPC(ch))
    {
      printf_to_char(ch,"%ld akçen var.\n\r",ch->silver);
	return;
    }

    printf_to_char(ch,
    "%ld akçen ve %d Tecrübe Puanın var (seviye atlamaya %d).\n\r",ch->silver,ch->exp,
	(ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
    snprintf(buf, sizeof(buf),"Şimdiye kadar %3d %s and %3d %s öldürdün.\n\r",
		ch->pcdata->has_killed,
		IS_GOOD(ch) ? "iyi olmayan" :
				IS_EVIL(ch) ? "kem olmayan" : "yansız olmayan",
		ch->pcdata->anti_killed,
		IS_GOOD(ch) ? "iyi" :	IS_EVIL(ch) ? "kem" : "yansız");
    send_to_char(buf, ch);

    total_played = get_total_played(ch);
    snprintf(buf, sizeof(buf),
	"Son 14 günde, %d saat ve %d dakika oynadın.\n\r",
	(int) (total_played / 60), (total_played % 60) );
    send_to_char(buf, ch);
    if (IS_IMMORTAL(ch))
    {
	int l;

	for( l=0; l<MAX_TIME_LOG; l++)
	{
	   snprintf(buf, sizeof(buf), "Gün: %3d Oynama zamanı: %3d\n",
		ch->pcdata->log_date[l], ch->pcdata->log_time[l]);
	   send_to_char(buf, ch);
	}
    }

    return;
}

/* Bağışıklık / dayanıklılık / zayıflık satırları (skor ve yaratık skoru ortak). */
static const struct { const char *label; long imm, res, vuln; } irv_table[] =
{
    { "teshir",   IMM_CHARM,     RES_CHARM,     VULN_CHARM     },
    { "çağrı  ",  IMM_SUMMON,    RES_SUMMON,    VULN_SUMMON    },
    { "büyü  ",   IMM_MAGIC,     RES_MAGIC,     VULN_MAGIC     },
    { "silah ",   IMM_WEAPON,    RES_WEAPON,    VULN_WEAPON    },
    { "ezici ",   IMM_BASH,      RES_BASH,      VULN_BASH      },
    { "delici  ", IMM_PIERCE,    RES_PIERCE,    VULN_PIERCE    },
    { "kesici",   IMM_SLASH,     RES_SLASH,     VULN_SLASH     },
    { "emici  ",  IMM_DROWNING,  RES_DROWNING,  VULN_DROWNING  },
    { "ateş  ",   IMM_FIRE,      RES_FIRE,      VULN_FIRE      },
    { "ayaz  ",   IMM_COLD,      RES_COLD,      VULN_COLD      },
    { "şimşek",   IMM_LIGHTNING, RES_LIGHTNING, VULN_LIGHTNING },
    { "asit    ", IMM_ACID,      RES_ACID,      VULN_ACID      },
    { "zehir ",   IMM_POISON,    RES_POISON,    VULN_POISON    },
    { "negatif",  IMM_NEGATIVE,  RES_NEGATIVE,  VULN_NEGATIVE  },
    { "kutsal",   IMM_HOLY,      RES_HOLY,      VULN_HOLY      },
    { "enerji",   IMM_ENERGY,    RES_ENERGY,    VULN_ENERGY    },
    { "zihin ",   IMM_MENTAL,    RES_MENTAL,    VULN_MENTAL    },
    { "hastalık", IMM_DISEASE,   RES_DISEASE,   VULN_DISEASE   },
    { "ışık  ",   IMM_LIGHT,     RES_LIGHT,     VULN_LIGHT     },
    { "ses    ",  IMM_SOUND,     RES_SOUND,     VULN_SOUND     },
    { "tahta ",   IMM_WOOD,      RES_WOOD,      VULN_WOOD      },
    { "gümüş ",   IMM_SILVER,    RES_SILVER,    VULN_SILVER    },
    { "demir ",   IMM_IRON,      RES_IRON,      VULN_IRON      },
};
#define IRV_PER_ROW	6
#define IRV_LAST_PAD	13	/* eksik satırın kutu kenarına kadar dolgusu */

static void show_irv_rows( CHAR_DATA *ch, CHAR_DATA *who )
{
    const int count = (int) ( sizeof(irv_table) / sizeof(irv_table[0]) );
    int i;

    printf_to_char(ch,"{c| {wBağışıklıklar, Dayanıklılıklar, Zayıflıklar{c                         |{x\n\r");
    for ( i = 0; i < count; i++ )
    {
	printf_to_char( ch, "%s{c%s:%s%s%s",
	    ( i % IRV_PER_ROW == 0 ) ? "{c| " : " ",
	    irv_table[i].label,
	    ( who->imm_flags  & irv_table[i].imm  ) ? "{w+{x" : "{D-{x",
	    ( who->res_flags  & irv_table[i].res  ) ? "{w+{x" : "{D-{x",
	    ( who->vuln_flags & irv_table[i].vuln ) ? "{w+{x" : "{D-{x" );
	if ( i % IRV_PER_ROW == IRV_PER_ROW - 1 )
	    send_to_char( "{c|{x\n\r", ch );
    }
    if ( count % IRV_PER_ROW != 0 )
	printf_to_char( ch, "%*s{c|{x\n\r", IRV_LAST_PAD, "" );
}

static const char *sex_name( int sex, bool npc )
{
    switch ( sex )
    {
    case SEX_NEUTRAL: return "yok";
    case SEX_MALE:    return "erkek";
    case SEX_FEMALE:  return npc ? "dişi" : "kadın";
    }
    return "";
}

static const char *pk_flag_name( int oyuncu_katli )
{
    return oyuncu_katli == 0 ? "Hayır" : oyuncu_katli == 1 ? "Evet" : "Bug";
}

void do_score( CHAR_DATA *ch, char *argument )
{
  char dogumGunu[MAX_INPUT_LENGTH];
  char yonelim_etik[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim = NULL, *who;
  const char *sex, *oyuncukatli, *adrenalin;

  argument = one_argument(argument,arg);

  if(arg[0]!='\0' && !IS_IMMORTAL(ch))
  {
     printf_to_char(ch,"Bu komutla argüman kullanılmaz.\n\r");
     return;
  }

  if(arg[0]!='\0' && IS_IMMORTAL(ch))
  {
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
      if ( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
        printf_to_char(ch, "Oyunda değil.\n\r" );
        return;
      }
    }

    if ( IS_NPC(victim) )
    {
       mob_score(ch,victim);
       return;
    }
  }

  who         = ( victim != NULL ) ? victim : ch;
  sex         = sex_name( who->sex, FALSE );
  oyuncukatli = pk_flag_name( who->pcdata->oyuncu_katli );
  adrenalin   = ( who->last_fight_time != -1
		&& current_time - who->last_fight_time < FIGHT_DELAY_TIME ) ? "Evet" : "Hayır";
  game_time_to_string( who->pcdata->birth_time, dogumGunu );

  snprintf(yonelim_etik, sizeof(yonelim_etik), "%s/%s",
	IS_GOOD(who) ? "iyi" : IS_EVIL(who) ? "kem" : "yansız",
	who->ethos == 1 ? "tüze" : who->ethos == 2 ? "yansız" : "kaos" );

  printf_to_char(ch,"{c,---------------------------------------------------------------------,{w\n\r");
  printf_to_char(ch,"{c|{w%*s%-*s{cDiscord:%-*s|\n\r",utf8_width(who->name, 12), who->name,utf8_width(who->pcdata->title, 30), who->pcdata->title,utf8_width(who->pcdata->discord_id, 19), who->pcdata->discord_id);
  printf_to_char(ch,"{c|-------------------------,-------------------------------------------,{w\n\r");
  printf_to_char(ch,"{c| Irk     : {w%-*s{c | ZIRH         | PARA                       |\n\r",utf8_width(race_table[who->race].name[1], 13), race_table[who->race].name[1]);
  printf_to_char(ch,"{c| Yaş     : {w%-13d{c | Delici : {w%-4d{c| Akçe        : {w%-7ld{c      |\n\r",get_age(who),GET_AC(who,AC_PIERCE),who->silver);
  printf_to_char(ch,"{c| Cinsiyet: {w%-*s{c | Ezici  : {w%-4d{c| Akçe (Banka): {w%-7ld{c      |\n\r",utf8_width(sex, 13), sex,GET_AC(who,AC_BASH),who->pcdata->bank_s);
  printf_to_char(ch,"{c| Sınıf   : {w%-*s{c | Kesici : {w%-4d{c|                            |\n\r",utf8_width(class_table[who->iclass].name[1], 13), class_table[who->iclass].name[1],GET_AC(who,AC_SLASH));
  printf_to_char(ch,"{c| Yön/Etk : {w%-*s{c | Egzotik: {w%-4d{c|                            |\n\r",utf8_width(yonelim_etik, 13), yonelim_etik,GET_AC(who,AC_EXOTIC));
  printf_to_char(ch,"{c| Doğum   : {w%-*s{c  | Büyü K.: {w%-4d{c|                            |\n\r",utf8_width(dogumGunu, 12), dogumGunu,who->saving_throw);
  printf_to_char(ch,"{c|-------------------------'--------------|----------------------------,{w\n\r");
  printf_to_char(ch,"{c| Yp    : {w%-7d/%-7d{c | Güç: {w%-2d(%-2d){c  | Pratik : {w%-3d{c               |\n\r",who->hit,who->max_hit,who->perm_stat[STAT_STR],get_curr_stat(who,STAT_STR),who->practice);
  printf_to_char(ch,"{c| Mana  : {w%-7d/%-7d{c | Zek: {w%-2d(%-2d){c  | Eğitim : {w%-3d{c               |\n\r",who->mana, who->max_mana,who->perm_stat[STAT_INT],get_curr_stat(who,STAT_INT),who->train);
  printf_to_char(ch,"{c| Zp    : {w%-7d/%-7d{c | Bil: {w%-2d(%-2d){c  | Eşya   : {w%-3d / %-4d{c        |\n\r",who->move, who->max_move,who->perm_stat[STAT_WIS],get_curr_stat(who,STAT_WIS),who->carry_number, can_carry_n(who));
  printf_to_char(ch,"{c| Seviye: {w%-10d{c      | Çev: {w%-2d(%-2d){c  | Ağırlık: {w%-6ld / %-8d{c |\n\r",who->level,who->perm_stat[STAT_DEX],get_curr_stat(who,STAT_DEX),get_carry_weight(who), can_carry_w(who));
  printf_to_char(ch,"{c| Kalan : {w%-10d{c      | Bün: {w%-2d(%-2d){c  | GörevP : {w%-5d{c             |\n\r",(who->level + 1) * exp_per_level(who,who->pcdata->points) - who->exp,who->perm_stat[STAT_CON],get_curr_stat(who,STAT_CON),who->pcdata->questpoints);
  printf_to_char(ch,"{c| TP    : {w%-12ld{c    | Kar: {w%-2d(%-2d){c  | GörevZ : {w%-2d{c                |\n\r",who->exp,who->perm_stat[STAT_CHA],get_curr_stat(who,STAT_CHA),((IS_SET(who->act, PLR_QUESTOR))?(who->pcdata->countdown):(who->pcdata->nextquest)));
  printf_to_char(ch,"{c| Korkak: {w%-10d{c      | ZZ : {w%-3d{c     | GörevPr: {w%-2d{c                |\n\r",who->wimpy,GET_DAMROLL(who),who->pcdata->questpractice);
  printf_to_char(ch,"{c| Ölüm  : {w%-3d{c             | VZ : {w%-3d{c     | RolP   : {w%-6ld{c            |\n\r",who->pcdata->death,GET_HITROLL(who), who->pcdata->rk_puani);
  printf_to_char(ch,"{c| Din   : {w%-*s{c    | OK : {w%-*s{c  | DinP   : {w%-6ld{c            |\n\r",utf8_width(religion_table[who->religion].name, 12), religion_table[who->religion].name, utf8_width(oyuncukatli, 6), oyuncukatli, who->pcdata->din_puani);
  printf_to_char(ch,"{c|-------------------------'--------------'----------------------------|{w\n\r");
  show_irv_rows( ch, who );
  printf_to_char(ch,"{c|---------------------------------------------------------------------|{x\n\r");
  printf_to_char(ch,"{c| Susuzluk   : {w%-3d{c Açlık    : {w%-3d{c Memleket Özlemi: {w%-3d{c                |\n\r",who->pcdata->condition[COND_THIRST],who->pcdata->condition[COND_HUNGER],who->pcdata->condition[COND_DESIRE]);
  printf_to_char(ch,"{c| Kana susama: {w%-3d{c Sarhoşluk: {w%-3d{c Adrenalin      : {w%-*s{c              |\n\r",who->pcdata->condition[COND_BLOODLUST],who->pcdata->condition[COND_DRUNK],utf8_width(adrenalin, 5), adrenalin);
  printf_to_char(ch,"{c'---------------------------------------------------------------------'{x\n\r");
}

void mob_score(CHAR_DATA *ch,CHAR_DATA *mob)
{
	const char *sex = sex_name( mob->sex, TRUE );

	printf_to_char(ch,"{c,---------------------------------------------------------------------,\n\r");
	printf_to_char(ch,"{c| {w%-*s                                 {c                       |\n\r",utf8_width(mob->short_descr, 12), mob->short_descr);
	printf_to_char(ch,"{c|--------------------,------------------------------------------------|\n\r");
  printf_to_char(ch,"{c| Vnum    : {w%-8d{c | Güç: {w%-2d{c        | Delici : {w%-4d {c| Büyücü : %*s    |\n\r",mob->pIndexData->vnum, mob->perm_stat[STAT_STR],GET_AC(mob,AC_PIERCE), utf8_width((IS_SET(mob->act,ACT_MAGE))?"{w+{c":"", 1), (IS_SET(mob->act,ACT_MAGE))?"{w+{c":"");
  printf_to_char(ch,"{c| Irk     : {w%-*s{c | Zek: {w%-2d{c        | Ezici  : {w%-4d {c| Ermiş  : %*s    |\n\r",utf8_width(race_table[mob->race].name[1], 8), race_table[mob->race].name[1],mob->perm_stat[STAT_INT],GET_AC(mob,AC_BASH),utf8_width((IS_SET(mob->act,ACT_CLERIC))?"{w+{c":"", 1), (IS_SET(mob->act,ACT_CLERIC))?"{w+{c":"");
  printf_to_char(ch,"{c| Cinsiyet: {w%-*s{c | Bil: {w%-2d{c        | Kesici : {w%-4d {c| Savaşçı: %*s    |\n\r",utf8_width(sex, 8), sex,mob->perm_stat[STAT_WIS],GET_AC(mob,AC_SLASH),utf8_width((IS_SET(mob->act,ACT_WARRIOR))?"{w+{c":"", 1), (IS_SET(mob->act,ACT_WARRIOR))?"{w+{c":"");
  printf_to_char(ch,"{c| Sınıf   : {wmobil{c    | Çev: {w%-2d{c        | Egzotik: {w%-4d {c| Hırsız : %*s    |\n\r",mob->perm_stat[STAT_DEX],GET_AC(mob,AC_EXOTIC),utf8_width((IS_SET(mob->act,ACT_THIEF))?"{w+{c":"", 1), (IS_SET(mob->act,ACT_THIEF))?"{w+{c":"");
  printf_to_char(ch,"{c| Yönelim : {w%-8d{c | Bün: {w%-2d{c        | ZZ     : {w%-4d {c|               |\n\r",mob->alignment,mob->perm_stat[STAT_CON],GET_DAMROLL(mob));
  printf_to_char(ch,"{c|                    | Kar: {w%-2d{c        | VZ     : {w%-4d {c|               |\n\r",mob->perm_stat[STAT_CHA],GET_HITROLL(mob));
	printf_to_char(ch,"{c|--------------------|----------------|---------------'---------------'{x\n\r");
	printf_to_char(ch,"{c| Yp    : {w%-5d/%-5d{c| Akçe : {w%-7ld {c| Din: {w%-*s{c             |\n\r",mob->hit,  mob->max_hit,mob->silver,utf8_width(religion_table[mob->religion].name, 12), religion_table[mob->religion].name);
	printf_to_char(ch,"{c| Mana  : {w%-5d/%-5d{c| Beden: {w%-8d{c|                               |\n\r",mob->mana, mob->max_mana,mob->size);
	printf_to_char(ch,"{c| Zp    : {w%-5d/%-5d{c|                |                               |\n\r",mob->move, mob->max_move);
	printf_to_char(ch,"{c| Seviye: {w%-7ld{c    |                |                               |\n\r",mob->level);
	printf_to_char(ch,"{c|--------------------'------------------'-----------------------------,{x\n\r");
  show_irv_rows( ch, mob );
  printf_to_char(ch,"{c'---------------------------------------------------------------------'{x\n\r");
}

char *	const	month_name	[] =
{
	"Albars",
	"Kadimler",
    "Büyük Acı",
	"Zeytin",
	"Yılan",
	"Yelbüke",
	"Pusu",
	"Savaş",
	"Albastı",
	"Gölge",
	"Kara Ölüm",
	"Alacakaranlık",
};


#define COLOR_DAWN "\033[1;34;44m"
#define COLOR_MORNING "\033[1;37;44m"
#define COLOR_DAY "\033[1;33;44m"
#define COLOR_EVENING "\033[1;31;44m"
#define COLOR_NIGHT "\033[0;30;44m"

void do_time( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH - 20];

	printf_to_char( ch , "Yıl  : %d\n\r", time_info.year );
	printf_to_char( ch , "Ay   : %s (%d. ay)\n\r", month_name[time_info.month-1] , time_info.month );
	printf_to_char( ch , "Gün  : %d\n\r", time_info.day );
	printf_to_char( ch , "Saat : %d\n\r", time_info.hour );

    if ( !IS_SET(ch->in_room->room_flags,ROOM_INDOORS) ||
         IS_IMMORTAL(ch) )
    {
      snprintf(buf, sizeof(buf), "$C %s vakti. $c",
         (time_info.hour>=4 && time_info.hour<6)? "şafak":
         (time_info.hour>=6 && time_info.hour<10)? "sabah":
         (time_info.hour>=10 && time_info.hour<16)? "öğlen":
         (time_info.hour>=16 && time_info.hour<20)? "akşam":
         "gece" );

      act_color( buf, ch, NULL, NULL, TO_CHAR, POS_RESTING,
         (time_info.hour>=4 && time_info.hour<6)? COLOR_DAWN:
         (time_info.hour>=6 && time_info.hour<10)? COLOR_MORNING:
         (time_info.hour>=10 && time_info.hour<16)? COLOR_DAY:
         (time_info.hour>=16 && time_info.hour<20)? COLOR_EVENING:
         COLOR_NIGHT, CLR_NORMAL );
		printf_to_char(ch,"\n\r\n\r");
    }
    if ( !IS_IMMORTAL( ch ) )
	{
      return;
  }
    snprintf(buf2, sizeof(buf2), "%s", (char *) ctime( &boot_time ));
    snprintf(buf, sizeof(buf),"Mangus %s tarihinde başlatıldı.\n\rSistem zamanı, %s.\n\r",
	buf2, (char *) ctime( &current_time ) );
    send_to_char( buf, ch );

    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{

    static const char * sky_look[4] =
    {
      "Bulutsuz",
    	"Bulutlu",
    	"Yağmurlu",
    	"Şimşekler çakıyor"
    };

    if ( !IS_OUTSIDE(ch) )
    {
      printf_to_char(ch, "Tavanın ardındaki gökyüzünü göremiyorsun.\n\r" );
	return;
    }

    printf_to_char( ch, "%s ve %s.",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "ılık güney meltemi esiyor"
	: "soğuk kuzey borası esiyor"
	);
    return;
}



void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
		printf_to_char( ch, "%s\n\r", pHelp->keyword );
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		page_to_char( pHelp->text+1, ch );
	    else
		page_to_char( pHelp->text  , ch );
	    return;
	}
    }

    printf_to_char(ch, "Bu sözcükle alakalı yardım yok.\n\r" );
    return;
}


/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char output[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char titlebuf[MAX_STRING_LENGTH];
    char level_buf[MAX_STRING_LENGTH];
    char classbuf[MAX_STRING_LENGTH];
    char pk_buf[100];
    char act_buf[100];
    bool found = FALSE;
    char cabalbuf[MAX_STRING_LENGTH];
    static CHAR_DATA *who_dch[BOT_WHO_MAX], *who_wch[BOT_WHO_MAX];
    int who_n, who_i;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
      printf_to_char(ch,"Bir isim belirtmelisin.\n\r");
	return;
    }

    output[0] = '\0';

    who_n = bot_who_collect( who_dch, who_wch, BOT_WHO_MAX );
    for ( who_i = 0; who_i < who_n; who_i++ )
    {
	CHAR_DATA *wch;
	CHAR_DATA *dch = who_dch[who_i];
	char const *iclass;

 	if (!can_see(ch,dch))
	    continue;

	if ( ((IS_VAMPIRE( dch ) || IS_BEAR( dch ) ) && !IS_IMMORTAL(ch) && (ch != dch) ) )
	    continue;

	wch = who_wch[who_i];

 	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg,wch->name))
	{
	    found = TRUE;

	    /* work out the printing */

	    iclass = class_table[wch->iclass].who_name;
	    switch(wch->level)
	    {
        case MAX_LEVEL - 0 : iclass = "YRT"; break;
    		case MAX_LEVEL - 1 : iclass = "YRT";	break;
    		case MAX_LEVEL - 2 : iclass = "TNR";	break;
    		case MAX_LEVEL - 3 : iclass = "TNR";	break;
    		case MAX_LEVEL - 4 : iclass = "TNR";	break;
    		case MAX_LEVEL - 5 : iclass = "MLK";	break;
    		case MAX_LEVEL - 6 : iclass = "MLK";	break;
    		case MAX_LEVEL - 7 : iclass = "YML";	break;
    		case MAX_LEVEL - 8 : iclass = "YML";	break;
	    }

	    /* for cabals
	    if ((wch->cabal && (ch->cabal == wch->cabal ||
			       IS_TRUSTED(ch,LEVEL_IMMORTAL))) ||
                               wch->level >= LEVEL_HERO)
	      snprintf(cabalbuf, sizeof(cabalbuf), "[%s] ",cabal_table[wch->cabal].short_name);
	    else cabalbuf[0] = '\0';
		*/
	if (( wch->cabal && ch->cabal == wch->cabal) || IS_IMMORTAL(ch)
		|| ( IS_SET(wch->act , PLR_CANINDUCT) && wch->cabal == 1)
		|| wch->cabal == CABAL_HUNTER
		|| (wch->cabal == CABAL_RULER
		   && is_equiped_n_char(wch, OBJ_VNUM_RULER_BADGE, WEAR_NECK)) )
	{
	  snprintf(cabalbuf, sizeof(cabalbuf), "[%s%s%s] ",
		CLR_CYAN,
		cabal_table[wch->cabal].short_name,
		CLR_WHITE_BOLD
		);
	}
	else cabalbuf[0] = '\0';
	if (wch->cabal == 0) cabalbuf[0] = '\0';

	pk_buf[0] = '\0';
	if (!((ch==wch && ch->level < KIDEMLI_OYUNCU_SEVIYESI) || is_safe_nomessage(ch,wch)))
	 {
	  snprintf(pk_buf, sizeof(pk_buf),"%s(OK) %s",
		CLR_RED,CLR_WHITE_BOLD);
	 }
	act_buf[0] = '\0';
	snprintf(act_buf, sizeof(act_buf),"%s%s%s",CLR_WHITE,
		    IS_SET(wch->act, PLR_WANTED) ? "(ARANIYOR) " : "",CLR_WHITE_BOLD);

	if (IS_NPC(wch))
		snprintf(titlebuf, sizeof(titlebuf),"Tanrılara inanan.");
	else
		snprintf(titlebuf, sizeof(titlebuf),"%s%s%s",CLR_WHITE,wch->pcdata->title,CLR_WHITE_BOLD);
	/*
	 * Format it up.
	 */

	snprintf(level_buf, sizeof(level_buf), "%s%2d%s",CLR_CYAN,wch->level,CLR_WHITE_BOLD);
	snprintf(classbuf, sizeof(classbuf),"%s%s%s",CLR_YELLOW,iclass,CLR_WHITE_BOLD);
	    /* a little formatting */

	    if (IS_TRUSTED(ch,LEVEL_IMMORTAL) || ch==wch ||
                      wch->level >= LEVEL_HERO)

	      snprintf(buf, sizeof(buf), "[%2d %s %s] %s%s%s%s%s\n\r",
		      wch->level,
		      RACE(wch) < MAX_PC_RACE ?
		        race_table[RACE(wch)].who_name: "     ",
		      classbuf,
		      pk_buf,
		      cabalbuf,
		      act_buf,
		      wch->name,
		      titlebuf);


		else
	  snprintf(buf, sizeof(buf), "[%s %s    ] %s%s%s%s%s\n\r",
		(get_curr_stat(wch, STAT_CHA) < 18 ) ? level_buf : "  ",
 		      RACE(wch) < MAX_PC_RACE ?
 		        race_table[RACE(wch)].who_name: "     ",
 		      ( (ch==wch && ch->level < KIDEMLI_OYUNCU_SEVIYESI) ||
 		        is_safe_nomessage(ch,wch) ) ?
 		       "" : "(OK) ",
 		      cabalbuf,
 		      IS_SET(wch->act,PLR_WANTED) ? "(ARANIYOR) " : "",
 		      wch->name,
		      titlebuf);

	    strcat(output,buf);
	}
    }

    if (!found)
    {
      printf_to_char(ch,"Bu isimde birini göremiyorum??\n\r");
	return;
    }

    page_to_char(output,ch);
}

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    {
	static CHAR_DATA *who_dch[BOT_WHO_MAX], *who_wch[BOT_WHO_MAX];
	int who_n = bot_who_collect( who_dch, who_wch, BOT_WHO_MAX ), who_i;

	for ( who_i = 0; who_i < who_n; who_i++ )
	    if ( can_see( ch, who_dch[who_i] ) )
		count++;
    }

    max_on = UMAX(count,max_on);

    if (max_on == count)
        snprintf(buf, sizeof(buf),"Oyunda %d karakter var, bugünün rekoru.\n\r",
	    count);
    else
	snprintf(buf, sizeof(buf),"Oyunda %d karakter var, bugün en çok %d.\n\r",
	    count,max_on);

    send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Taşıdıkların:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  int iWear, bolgeye_giyilen_esya_sayisi, i;
  bool found;
  char buf[MAX_STRING_LENGTH];

  printf_to_char( ch , "Ekipmanların:\n\r" );

  found = FALSE;

  for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
  {
    if ( iWear==WEAR_FINGER || iWear==WEAR_NECK || iWear==WEAR_WRIST || iWear==WEAR_TATTOO )
    {
      for ( obj=ch->carrying; obj != NULL; obj = obj->next_content )
      {
        if ( obj->wear_loc == iWear && show_cwear_to_char( ch, obj ) )
          found = TRUE;
      }

      bolgeye_giyilen_esya_sayisi = max_can_wear(ch, iWear) - count_worn(ch, iWear);

      if( bolgeye_giyilen_esya_sayisi > 0 )
      {
        for( i=1 ; i <= bolgeye_giyilen_esya_sayisi ; i++ )
        {
          printf_to_char(ch,"%s\n\r",where_name[iWear]);
        }
      }
    }
    else if ( iWear == WEAR_STUCK_IN )
    {
      for ( obj=ch->carrying; obj != NULL; obj = obj->next_content )
      {
        if ( obj->wear_loc == iWear && show_cwear_to_char( ch, obj ) )
          found = TRUE;
      }
    }
    else
    {
      if ( ( obj = get_eq_char(ch,iWear)) != NULL && show_cwear_to_char( ch, obj ) )
      {
        found = TRUE;
      }
      else
      {
        printf_to_char( ch, "%s\n\r", where_label( iWear, ' ', buf, sizeof(buf) ) );
      }
    }
  }

  if ( !found )
    send_to_char( "Hiçbir şey.\n\r", ch );

  return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
      printf_to_char(ch, "Neyi neyle kıyaslayacaksın?\n\r" );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
      printf_to_char(ch, "Ona sahip değilsin.\n\r" );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
    printf_to_char(ch,"Kıyaslanabilecek bir şey giymiyorsun.\n\r");
	    return;
	}
    }

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == NULL )
    {
      printf_to_char(ch,"Ona sahip değilsin.\n\r");
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "$p ile kendisini kıyaslıyorsun... Hmm, aynı gibiler.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "$p ile $P kıyaslanamaz.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "$p ile $P kıyaslanamaz.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p ile $P aynı gibiler.";
	else if ( value1  > value2 ) msg = "$p daha iyi gibi.";
	else                         msg = "$p daha kötü gibi.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}

void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char pkbuf[100];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    static CHAR_DATA *who_dch[BOT_WHO_MAX], *who_wch[BOT_WHO_MAX];
    int who_n, who_i;
    bool found;
    bool fPKonly = FALSE;

    one_argument( argument, arg );

    if (!check_blind(ch))
      return;

    if (room_is_dark( ch ) && !IS_SET(ch->act, PLR_HOLYLIGHT))
    {
      send_to_char("Çok karanlık.\n\r",ch);
      return;
    }

    if (!str_cmp(arg, "tüze"))
    {
      if (IS_SET(ch->in_room->area->area_flag, AREA_PROTECTED))
	send_to_char("Bu bölge Tüze tarafından korunuyor!\n\r",ch);
      else
	send_to_char("Bu bölge korunmuyor.\n\r",ch);
      return;
    }

    if (!str_cmp(arg,"ok"))
      fPKonly = TRUE;

    snprintf(pkbuf, sizeof(pkbuf),"%s%s%s",CLR_RED,"(OK) ",CLR_WHITE_BOLD);

    if ( arg[0] == '\0' || fPKonly)
    {
	send_to_char( "Yakınındaki karakterler:\n\r", ch );
	found = FALSE;
	who_n = bot_who_collect( who_dch, who_wch, BOT_WHO_MAX );
	for ( who_i = 0; who_i < who_n; who_i++ )
	{
	    if ( ( victim = who_dch[who_i] ) != NULL
	    &&   !IS_NPC(victim)
	    && !(fPKonly && is_safe_nomessage(ch,victim))
	    &&   victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   can_see( ch, victim ) )

	    {
		found = TRUE;
		snprintf(buf, sizeof(buf), "%s%-*s %s\n\r",
		 (is_safe_nomessage(ch,
(is_affected(victim,gsn_doppelganger) && victim->doppel) ?
	victim->doppel : victim) || IS_NPC(victim)) ?
			"  " :  pkbuf,
	         utf8_width((is_affected(victim,gsn_doppelganger)
		  && !IS_SET(ch->act,PLR_HOLYLIGHT)) ?
		    victim->doppel->name : victim->name, 28), (is_affected(victim,gsn_doppelganger)
		  && !IS_SET(ch->act,PLR_HOLYLIGHT)) ?
		    victim->doppel->name : victim->name,
		 victim->in_room->name);
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Hiç\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_FADE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		snprintf(buf, sizeof(buf), "%-*s %s\n\r",
		    utf8_width(PERS(victim, ch), 28), PERS(victim, ch), victim->in_room->name);
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
  act( "Hiç $T bulamadın.", ch, NULL, arg, TO_CHAR );
    }

    return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    char *align;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
      printf_to_char(ch, "Kimi tartacaksın?\n\r" );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
      printf_to_char(ch, "Burada değil.\n\r" );
	return;
    }

    if (is_safe(ch,victim))
    {
      printf_to_char(ch,"Bunu düşünme bile.\n\r");
	return;
    }

    diff = victim->level - ch->level;

     if ( diff <= -10 ) msg = "$G çıplak ve silahsızken bile öldürürsün.";
else if ( diff <=  -5 ) msg = "$N sana antrenman bile yaptıramaz.";
else if ( diff <=  -2 ) msg = "$N kolayca öldürülebilir.";
else if ( diff <=   1 ) msg = "Tam dişine göre!";
else if ( diff <=   4 ) msg = "Kendini şanslı hissediyor musun?";
else if ( diff <=   9 ) msg = "$N acımasızca gülüyor.";
else                    msg = "İntihar günahtır!";

    if (IS_EVIL(ch) && IS_EVIL(victim))
      align = "$N seninle birlikte kötücül şekilde sırıtıyor.";
    else if (IS_GOOD(victim) && IS_GOOD(ch))
      align = "$N seni sıcak şekilde selamlıyor.";
    else if (IS_GOOD(victim) && IS_EVIL(ch))
      align = "$N sana gülümsüyor ve kem yoldan dönmen için dua ediyor.";
    else if (IS_EVIL(victim) && IS_GOOD(ch))
      align = "$N sana şeytani şekilde gülümsüyor.";
    else if (IS_NEUTRAL(ch) && IS_EVIL(victim))
      align = "$N yüzüne kötücül bir gülümseme yerleştiriyor.";
    else if (IS_NEUTRAL(ch) && IS_GOOD(victim))
      align = "$N mutlulukla gülümsüyor.";
    else if (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))
      align = "$N seninle ilgilenmiyor.";
    else
      align = "$N çok ilgisiz görünüyor.";

    act( msg, ch, NULL, victim, TO_CHAR );
    act( align, ch, NULL, victim, TO_CHAR);
    return;
}


#define TITLE_MAX_LEN	45	/* lakap üst sınırı (bayt) */

void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    snprintf( buf, sizeof(buf), "%s%s",
	( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' ) ? " " : "",
	title );

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}


void do_titl( CHAR_DATA *ch, char *argument)
{
   printf_to_char( ch,"Lakabını değiştirmek istiyorsan 'lakap' komutunu eksiksiz yazmalısın.\n\r" );
}

void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
        return;

    if ( CANT_CHANGE_TITLE(ch) )
	{
         send_to_char( "Lakabını değiştiremezsin.\n\r", ch );
         return;
	}

  if ( argument[0] == '\0' )
  {
    printf_to_char(ch, "İsteğin üzerine lakabın sıfırlandı.\n\r");
    set_title( ch, "" );

      return;
  }

    if (!str_cmp(argument, "sıfırla"))
    {
        char buf[MAX_STRING_LENGTH];
        snprintf(buf, sizeof(buf), ", %s", title_table[ch->iclass][ch->level]);
        set_title(ch, buf);
        printf_to_char(ch, "Lakabın sıfırlandı.\n\r");
        return;
    }

    /* Bayt sınırı UTF-8 harf ortasında kesmesin */
    if ( strlen(argument) > TITLE_MAX_LEN )
	utf8_truncate( argument, TITLE_MAX_LEN );

    smash_tilde( argument );
    set_title( ch, argument );
    printf_to_char(ch, "Tamam.\n\r");
}


void do_description( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );

    	if (argument[0] == '-')
    	{
            int len;
            bool found = FALSE;

            if (ch->description == NULL || ch->description[0] == '\0')
            {
              printf_to_char(ch,"Çıkartılacak satır kalmadı.\n\r");
                return;
            }

	    snprintf( buf, sizeof(buf), "%s", ch->description );

            for (len = strlen(buf); len > 0; len--)
            {
                if (buf[len] == '\r')
                {
                    if (!found)  /* back it up */
                    {
                        if (len > 0)
                            len--;
                        found = TRUE;
                    }
                    else /* found the second one */
                    {
                        buf[len + 1] = '\0';
			free_string(ch->description);
			ch->description = str_dup(buf);
      send_to_char( "Tanımın:\n\r", ch );
      send_to_char( ch->description ? ch->description : "(Hiç).\n\r", ch );
                        return;
                    }
                }
            }
            buf[0] = '\0';
	    free_string(ch->description);
	    ch->description = str_dup(buf);
      printf_to_char(ch,"Tanım silindi.\n\r");
	    return;
        }
	if ( argument[0] == '+' )
	{
	    if ( ch->description != NULL )
		strcat( buf, ch->description );
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}

	if ( strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2 )
	{

    printf_to_char(ch, "Tanım çok uzun.\n\r" );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->description );

	ch->description = str_dup( buf );
  if (!IS_NPC(ch) && (strlen(ch->description)>=350) && (IS_SET(ch->act, PLR_NO_DESCRIPTION)))
    REMOVE_BIT(ch->act, PLR_NO_DESCRIPTION);
  if (!IS_NPC(ch) && (strlen(ch->description)<350) && (!IS_SET(ch->act, PLR_NO_DESCRIPTION)))
      SET_BIT(ch->act, PLR_NO_DESCRIPTION);
    }

    send_to_char( "Tanımın:\n\r", ch );
    send_to_char( ch->description ? ch->description : "(Hiç).\n\r", ch );

    return;
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    snprintf(buf, sizeof(buf), "%d/%d Yp %d/%d Mp %d/%d Zp %d Tp",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );
    do_say( ch, buf );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf2[10*MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int sn;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	int col;

	col    = 0;
        strcpy( buf2, "" );
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name[0] == NULL )
		break;
	    if ( ch->level < skill_table[sn].skill_level[ch->iclass] ||
		!RACE_OK(ch,sn) ||
(skill_table[sn].cabal != ch->cabal && skill_table[sn].cabal != CABAL_NONE)
	      )
		continue;

    if(ch->pcdata->learned[sn]<75)
    snprintf(buf, sizeof(buf), "{r%-*s %3d%%  {x",utf8_width(skill_table[sn].name[1], 18), skill_table[sn].name[1], ch->pcdata->learned[sn]);
  else if(ch->pcdata->learned[sn]>=75 && ch->pcdata->learned[sn]<85)
    snprintf(buf, sizeof(buf), "{g%-*s %3d%%  {x",utf8_width(skill_table[sn].name[1], 18), skill_table[sn].name[1], ch->pcdata->learned[sn]);
  else if(ch->pcdata->learned[sn]>=85 && ch->pcdata->learned[sn]<100)
    snprintf(buf, sizeof(buf), "{G%-*s %3d%%  {x",utf8_width(skill_table[sn].name[1], 18), skill_table[sn].name[1], ch->pcdata->learned[sn]);
  else if(ch->pcdata->learned[sn]==100)
    snprintf(buf, sizeof(buf), "{C%-*s %3d%%  {x",utf8_width(skill_table[sn].name[1], 18), skill_table[sn].name[1], ch->pcdata->learned[sn]);
  else
    snprintf(buf, sizeof(buf), "%-*s %3d%%  ",utf8_width(skill_table[sn].name[1], 18), skill_table[sn].name[1], ch->pcdata->learned[sn]);
	    strcat( buf2, buf );
	    if ( ++col % 3 == 0 )
		strcat( buf2, "\n\r" );
	}

	if ( col % 3 != 0 )
	    strcat( buf2, "\n\r" );

	snprintf(buf, sizeof(buf), "%d pratik seansı var.\n\r",
	    ch->practice );
	strcat( buf2, buf );

	if ( IS_IMMORTAL( ch ) )
	  page_to_char( buf2, ch );
	else
	  send_to_char( buf2, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int adept;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "Rüyalarında mı?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
    printf_to_char(ch, "Burada yapamazsın.\n\r" );
	    return;
	}

	if ( ch->practice <= 0 )
	{
    printf_to_char(ch, "Pratik seansın yok.\n\r" );
	    return;
	}

	if ( ( sn = find_spell( ch,argument ) ) < 0
	|| ( !IS_NPC(ch)
	&&   (ch->level < skill_table[sn].skill_level[ch->iclass]
 	|| !RACE_OK(ch,sn) ||
(skill_table[sn].cabal != ch->cabal && skill_table[sn].cabal != CABAL_NONE) )))
	{
    printf_to_char(ch, "Onu pratik edemezsin.\n\r" );
	    return;
	}

	if (!str_cmp("vampir",skill_table[sn].name[1]) )
	{
	 send_to_char( "Bunu yalnızca görevciden alabilirsin.\n\r",ch);
	 return;
	}

	adept = IS_NPC(ch) ? 100 : class_table[ch->iclass].skill_adept;

	if ( ch->pcdata->learned[sn] >= adept )
	{
    printf_to_char( ch, "%s konusunu çoktan öğrendin.\n\r",
  skill_table[sn].name[1] );
	}
	else
	{
	    if (!ch->pcdata->learned[sn]) ch->pcdata->learned[sn] = 1;
	    ch->practice--;
	    ch->pcdata->learned[sn] +=
		int_app[get_curr_stat(ch,STAT_INT)].learn /
	        UMAX(skill_table[sn].rating[ch->iclass],1);
	    if ( ch->pcdata->learned[sn] < adept )
	    {
        act( "$T pratik ediyorsun.",
		    ch, NULL, skill_table[sn].name[1], TO_CHAR );
        act( "$n $T pratik ediyor.",
		    ch, NULL, skill_table[sn].name[1], TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
    act( "$T konusunu öğrendin.",
		    ch, NULL, skill_table[sn].name[1], TO_CHAR );
        act( "$n $T konusunu öğrendi.",
		    ch, NULL, skill_table[sn].name[1], TO_ROOM );
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{

    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( ch->iclass == CLASS_SAMURAI && ch->level >= 10 )
	{
    printf_to_char( ch, "Korkaklık pek sana göre değil!\n\r" );

	 if (ch->wimpy != 0) ch->wimpy = 0;
	 return;
	}

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else  wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
      printf_to_char(ch, "Cesaretin bilgeliğini aşıyor.\n\r" );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
      printf_to_char(ch, "Bu kadar korkaklık seni hasta eder.\n\r" );
	return;
    }

    ch->wimpy	= wimpy;

    printf_to_char( ch, "Korkaklık %d Yp'ye ayarlandı.\n\r", wimpy );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      printf_to_char(ch, "Yazım: şifre <eski> <yeni>.\n\r" );
	return;
    }

    if ( !pwd_check( ch->pcdata->pwd, arg1 ) )
    {
	WAIT_STATE( ch, 40 );
  printf_to_char(ch, "Yanlış şifre.  10 saniye bekle.\n\r" );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
      printf_to_char(ch, "Yeni parola en az 5 karakter uzunluğunda olmalı.\n\r" );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = (char *) pwd_hash( arg2 );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
    printf_to_char(ch,
  "Kabul edilemez bir parola, tekrar deneyin.\n\r" );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Tamam.\n\r", ch );
    return;
}

/* RT configure command */

void do_scan(CHAR_DATA *ch, char *argument)
{
  char dir[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *exit;	/* pExit */
  int door;
  int range;
  char buf[MAX_STRING_LENGTH];
  int i;
  CHAR_DATA *person;
  int numpeople;

  one_argument(argument,dir);

 if (dir[0] == '\0')

   {
	do_scan2(ch,"");
	return;
   }

  if ( ( door = dir_lookup( dir ) ) < 0 )
    {
      send_to_char("Bu bir yön değil.\n\r",ch);
      return;
    }

  printf_to_char(ch, "%s yönünü tarıyorsun.\n\r", dir_name[door]);
  snprintf(buf, sizeof(buf), "$n %s yönünü tarıyor.", dir_name[door]);
  act(buf,ch,NULL,NULL,TO_ROOM);

  if (!check_blind(ch))
    return;

  range = 1 + (ch->level)/10;

  in_room = ch->in_room;

  for (i=1; i <= range; i++)
    {
      if ( (exit = in_room->exit[door]) == NULL
	  || (to_room = exit->u1.to_room) == NULL
	  || IS_SET(exit->exit_info,EX_CLOSED) )
	return;

      for (numpeople = 0,person = to_room->people; person != NULL;
	   person = person->next_in_room)
	if (can_see(ch,person)) numpeople++;

      if (numpeople)
	{
	  snprintf(buf, sizeof(buf), "***** Uzaklık %d *****\n\r",i);
	  send_to_char(buf,ch);
	  show_char_to_char(to_room->people, ch);
	  send_to_char("\n\r", ch);
	}
      in_room = to_room;
    }
}

void do_request( CHAR_DATA *ch, char *argument)
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA  *obj;
  AFFECT_DATA af;

  if ( is_affected(ch, gsn_reserved))
	{
    send_to_char("Tekrar talep etmek için biraz bekle.\n\r",ch);
	 return;
	}


  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (IS_NPC(ch))
	return;

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "Kimden neyi talep edeceksin?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
      send_to_char("Burada değil.\n\r", ch );
      return;
    }

  if (!IS_NPC(victim))
    {
      send_to_char("Neden kendin sormuyorsun?\n\r", ch);
      return;
    }

  if (!IS_GOOD(ch))
    {
      do_say(victim, "Böyle ahlaksız birine hiçbir şey vermem.");
      return;
    }

  if (ch->move < (50 + ch->level))
    {
      do_say(victim, "Yorgun görünüyorsun. Önce dinlenmelisin.");
      return;
    }

  WAIT_STATE(ch, PULSE_VIOLENCE);
  ch->move -= 10;
  ch->move = UMAX(ch->move, 0);

  if (victim->level >= ch->level + 10 || victim->level >= ch->level * 2)
    {
      do_say(victim, "Henüz erken, çocuğum.");
      return;
    }

  if (( ( obj = get_obj_carry(victim , arg1 ) ) == NULL
      && (obj = get_obj_wear(victim, arg1)) == NULL)
      || IS_SET(obj->extra_flags, ITEM_INVENTORY))
    {
      do_say(victim, "Üzgünüm o şeyden bende yok.");
      return;
    }

  if (!IS_GOOD(victim))
    {
      do_say(victim, "Sana birşey vermek zorunda değilim!");
      do_murder(victim, ch->name);
      return;
    }

  if ( obj->wear_loc != WEAR_NONE )
    unequip_char(victim, obj);

  if ( !can_drop_obj( ch, obj ) )
    {
      do_say(victim, "Üzgünüm o lanetli şeyi kendimden ayıramıyorum.");
      return;
    }

  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
      send_to_char("Ellerin dolu.\n\r", ch);
      return;
    }

  if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
      send_to_char( "Bu kadar ağırlık taşıyamazsın.\n\r", ch);
      return;
    }

  if ( !can_see_obj( ch, obj ) )
    {
      act("O nesneyi görmüyorsun.", ch, NULL, victim, TO_CHAR );
      return;
    }

  obj_from_char( obj );
  obj_to_char( obj, ch );
  act( "$n $Z $p talep ediyor.", ch, obj, victim, TO_NOTVICT );
  act( "Sen $Z $p talep ediyorsun.",   ch, obj, victim, TO_CHAR    );
  act( "$n senden $p talep ediyor.", ch, obj, victim, TO_VICT    );



  if (IS_SET(obj->progtypes,OPROG_GIVE))
    (obj->pIndexData->oprogs->give_prog) (obj,ch,victim);

  if (IS_SET(victim->progtypes,MPROG_GIVE))
    (victim->pIndexData->mprogs->give_prog) (victim,ch,obj);



  ch->move -= (50 + ch->level);
  ch->move = UMAX(ch->move, 0);
  ch->hit -= 3 * (ch->level / 2);
  ch->hit = UMAX(ch->hit, 0);

  act("$S inancına şükranlık duyuyorsun.", ch, NULL, victim,TO_CHAR);
  send_to_char("Ve dünyadaki iyiliğin ışığından mutluluk duyuyorsun.\n\r",ch);

  af.type = gsn_reserved;
  af.where = TO_AFFECTS;
  af.level = ch->level;
  af.duration = ch->level / 10;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  affect_to_char ( ch,&af );

  return;
}

void do_detect_hidden( CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;


  if (IS_NPC(ch) ||
      ch->level < skill_table[gsn_detect_hidden].skill_level[ch->iclass] )
    {
      send_to_char("Hı?\n\r", ch );
      return;
    }

    if ( CAN_DETECT(ch, DETECT_HIDDEN) )
    {
      send_to_char("Zaten olabildiğince tetiktesin. \n\r",ch);
	return;
    }
    if ( number_percent( ) > get_skill( ch, gsn_detect_hidden ) )  {
      send_to_char(
          "Dikkatle gölgelerin içine bakıyorsun, fakat birşey göremiyorsun.\n\r", ch );
      return;
    }
    af.where     = TO_DETECTS;
    af.type      = gsn_detect_hidden;
    af.level     = ch->level;
    af.duration  = ch->level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = DETECT_HIDDEN;
    affect_to_char( ch, &af );
    send_to_char( "Dikkatin artıyor.\n\r", ch );
    return;
}


void do_bear_call( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *gch;
  CHAR_DATA *bear;
  CHAR_DATA *bear2;
  AFFECT_DATA af;
  int i;

  if (IS_NPC(ch) ||
      ch->level < skill_table[gsn_bear_call].skill_level[ch->iclass] )
    {
      send_to_char( "Hı?\n\r", ch );
      return;
    }

    send_to_char("Ayıları yardımına çağırıyorsun.\n\r",ch);
    act("$n ayıları çağırıyor.",ch,NULL,NULL,TO_ROOM);

  if (is_affected(ch, gsn_bear_call))
    {
      send_to_char( "Onları kontrol edecek gücü nasıl çağıracaksın?\n\r", ch);
      return;
    }
  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  gch->pIndexData->vnum == MOB_VNUM_BEAR)
	{
    send_to_char( "Sahip olduğun ayıyla bir sorunun mu var?",ch);
	  return;
	}
    }

  if ( ch->in_room != NULL && IS_SET(ch->in_room->room_flags, ROOM_NO_MOB) )
  {
    send_to_char( "Ayılar seni dinlemiyor.\n\r", ch );
     return;
  }

  if ( number_percent( ) > get_skill( ch, gsn_bear_call) )
  {
    send_to_char( "Ayılar seni dinlemiyor.\n\r", ch );
	check_improve(ch,gsn_bear_call,TRUE,1);
	return;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)      ||
       IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)   ||
       IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)  ||
       (ch->in_room->exit[0] == NULL &&
          ch->in_room->exit[1] == NULL &&
          ch->in_room->exit[2] == NULL &&
          ch->in_room->exit[3] == NULL &&
          ch->in_room->exit[4] == NULL &&
          ch->in_room->exit[5] == NULL) ||

         ( ch->in_room->sector_type != SECT_FIELD &&
           ch->in_room->sector_type != SECT_FOREST &&
           ch->in_room->sector_type != SECT_MOUNTAIN &&
           ch->in_room->sector_type != SECT_HILLS ) )
  {
    send_to_char( "Yardımına gelen ayı olmuyor.\n\r", ch );
    return;
  }

  if ( ch->mana < 125 )
  {
    send_to_char("Bu iş için yeterli manan yok.\n\r", ch );
     return;
  }
  ch->mana -= 125;

  check_improve(ch,gsn_bear_call,TRUE,1);
  bear = create_mobile( get_mob_index(MOB_VNUM_BEAR), NULL );

  for (i=0;i < MAX_STATS; i++)
    {
      bear->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);
    }

  bear->max_hit = IS_NPC(ch)? ch->max_hit : ch->pcdata->perm_hit;
  bear->hit = bear->max_hit;
  bear->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  bear->mana = bear->max_mana;
  bear->alignment = ch->alignment;
  bear->level = UMIN(100,1 * ch->level-2);
  for (i=0; i < 3; i++)
    bear->armor[i] = interpolate(bear->level,100,-100);
  bear->armor[3] = interpolate(bear->level,100,0);
  bear->sex = ch->sex;
  bear->silver = 0;

  bear2 = create_mobile(bear->pIndexData, NULL);
  clone_mobile(bear,bear2);

  SET_BIT(bear->affected_by, AFF_CHARM);
  SET_BIT(bear2->affected_by, AFF_CHARM);
  bear->master = bear2->master = ch;
  bear->leader = bear2->leader = ch;

  char_to_room(bear,ch->in_room);
  char_to_room(bear2,ch->in_room);
  send_to_char( "İki ayı yardımına geliyor!\n\r",ch);
  act("$s yardımına iki ayı geliyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = gsn_bear_call;
  af.level              = ch->level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

}


void do_identify( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	int cost = 100;

	if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
	{
		send_to_char( "Bunu taşımıyorsun.\n\r", ch );
		return;
	}

  // AVCI (TURCCAR) kabali uyeleri de tanimlama yapabilsin.
  // Ancak bunun icin para vs odemeyecek, kendi irfanlariyla tanimlama yapacaklar.
  // Ilerde bu isin icine (avcilar icin) karizma veya bilgelik etkisi de ekleyelim.
  // ya da siger oyuncular avci kabalindan biri odadayken tanimlama yapabilsinler
  // ve odemeyi avci kabali uyesine yapsinlar.
  if(ch->cabal != CABAL_HUNTER)
  {
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
      if (IS_NPC(rch) && rch->pIndexData->vnum == MOB_VNUM_SAGE)
      {
        break;
      }
    }

    if (!rch)
    {
      send_to_char(  "Buralarda o şeyden anlayan yok.\n\r", ch);
      return;
    }
    
    if(number_percent()<= get_skill(ch, gsn_haggle))
    {
      if(number_percent()>90)
      {
        cost = 10;
      }
      else
      {
        cost = 40;
      }
    }

    if (IS_IMMORTAL(ch))
    {
      act( "$n sana bakıyor!\n\r", rch, obj, ch, TO_VICT );
    }
    else if (ch->silver < cost)
    {
      act( "$n $p'yi tanımlamaya devam ediyor.",rch, obj, 0, TO_ROOM );
      printf_to_char(ch,"Yeterli akçen yok.\n\r");
      return;
    }
    else
    {
      ch->silver -= cost;
      printf_to_char(ch,"Aldığın hizmet için %d akçe ödüyorsun.\n\r", cost);
    }

    act( "$n $p üzerine bilge bir bakış fırlatıyor.", rch, obj, 0, TO_ROOM );
  }
	spell_identify( 0, 0, ch, obj ,0);
}


void do_affects_col(CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA *paf, *paf_last = NULL;
  char buf[MAX_STRING_LENGTH];
  bool found, firstAffect;

  found = FALSE;
  firstAffect = TRUE;

  if ( ch->affected != NULL )
  {
    found = TRUE;
    send_to_char( "Şunlardan etkileniyorsun:\n\r", ch );
    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
      if (paf_last != NULL && paf->type == paf_last->type)
      {
        if( firstAffect == TRUE )
        {
          buf[0] = '\0';
          firstAffect = FALSE;
        }
        else
        {
          snprintf(buf, sizeof(buf), "                      ");
        }
      }
      else
      {
        snprintf(buf, sizeof(buf), "%sEtki%s: %s%-*s%s",
          CLR_RED,CLR_WHITE_BOLD,CLR_YELLOW,
          utf8_width(skill_table[paf->type].name[1], 16), skill_table[paf->type].name[1],CLR_WHITE_BOLD);
      }

      send_to_char( buf, ch );

      if ( paf->location != APPLY_NONE && paf->modifier != 0 )
      {
        printf_to_char( ch,
          ": %s%s%s yönünü %s%d%s değiştirir",
          CLR_MAGENTA,affect_loc_name( paf->location ),CLR_WHITE_BOLD,
          CLR_MAGENTA,paf->modifier,CLR_WHITE_BOLD);
        if ( paf->duration == -1 || paf->duration == -2)
        {
          printf_to_char( ch,"%ssürekli%s\n\r" ,CLR_CYAN,CLR_WHITE);
        }
        else
        {
          printf_to_char( ch,"%s%d%s saat\n\r",CLR_MAGENTA,paf->duration ,CLR_WHITE_BOLD);
        }
      }
      else if (paf->bitvector)
      {
        switch(paf->where)
        {
            case TO_AFFECTS:
                printf_to_char( ch,": %s%s%s etkisi ekler, ",CLR_MAGENTA,affect_bit_name(paf->bitvector),CLR_WHITE_BOLD);
                if ( paf->duration == -1 || paf->duration == -2)
                {
                  printf_to_char( ch,"%ssürekli%s\n\r" ,CLR_CYAN,CLR_WHITE);
                }
                else
                {
                  printf_to_char( ch,"%s%d%s saat\n\r",CLR_MAGENTA,paf->duration ,CLR_WHITE_BOLD);
                }
                break;
            case TO_OBJECT:
                printf_to_char( ch,": %s%s%s eşya özelliği ekler, ",CLR_MAGENTA,extra_bit_name(paf->bitvector),CLR_WHITE_BOLD);
                if ( paf->duration == -1 || paf->duration == -2)
                {
                  printf_to_char( ch,"%ssürekli%s\n\r" ,CLR_CYAN,CLR_WHITE);
                }
                else
                {
                  printf_to_char( ch,"%s%d%s saat\n\r",CLR_MAGENTA,paf->duration ,CLR_WHITE_BOLD);
                }
                break;
            case TO_WEAPON:
                printf_to_char( ch,": %s%s%s silah özelliği ekler, ",CLR_MAGENTA,weapon_bit_name(paf->bitvector),CLR_WHITE_BOLD);
                if ( paf->duration == -1 || paf->duration == -2)
                {
                  printf_to_char( ch,"%ssürekli%s\n\r" ,CLR_CYAN,CLR_WHITE);
                }
                else
                {
                  printf_to_char( ch,"%s%d%s saat\n\r",CLR_MAGENTA,paf->duration ,CLR_WHITE_BOLD);
                }
                break;
            case TO_IMMUNE:
                printf_to_char( ch,": %s%s%s bağışıklığı ekler, ",CLR_MAGENTA,imm_bit_name(paf->bitvector),CLR_WHITE_BOLD);
                if ( paf->duration == -1 || paf->duration == -2)
                {
                  printf_to_char( ch,"%ssürekli%s\n\r" ,CLR_CYAN,CLR_WHITE);
                }
                else
                {
                  printf_to_char( ch,"%s%d%s saat\n\r",CLR_MAGENTA,paf->duration ,CLR_WHITE_BOLD);
                }
                break;
            case TO_RESIST:
                printf_to_char( ch,": %s%s%s direnci ekler, ",CLR_MAGENTA,imm_bit_name(paf->bitvector),CLR_WHITE_BOLD);
                if ( paf->duration == -1 || paf->duration == -2)
                {
                  printf_to_char( ch,"%ssürekli%s\n\r" ,CLR_CYAN,CLR_WHITE);
                }
                else
                {
                  printf_to_char( ch,"%s%d%s saat\n\r",CLR_MAGENTA,paf->duration ,CLR_WHITE_BOLD);
                }
                break;
            case TO_VULN:
                printf_to_char( ch,": %s%s%s dayanıksızlığı ekler, ",CLR_MAGENTA,imm_bit_name(paf->bitvector),CLR_WHITE_BOLD);
                if ( paf->duration == -1 || paf->duration == -2)
                {
                  printf_to_char( ch,"%ssürekli%s\n\r" ,CLR_CYAN,CLR_WHITE);
                }
                else
                {
                  printf_to_char( ch,"%s%d%s saat\n\r",CLR_MAGENTA,paf->duration ,CLR_WHITE_BOLD);
                }
                break;
            case TO_DETECTS:
                printf_to_char( ch,": %s%s%s saptaması ekler, ",CLR_MAGENTA,detect_bit_name(paf->bitvector),CLR_WHITE_BOLD);
                if ( paf->duration == -1 || paf->duration == -2)
                {
                  printf_to_char( ch,"%ssürekli%s\n\r" ,CLR_CYAN,CLR_WHITE);
                }
                else
                {
                  printf_to_char( ch,"%s%d%s saat\n\r",CLR_MAGENTA,paf->duration ,CLR_WHITE_BOLD);
                }
                break;
            case TO_ACT_FLAG:
            case TO_RACE:
                break;
            default:
                printf_to_char( ch,": %s%d-%d%s bilinmeyen etkisi ekler, ",CLR_MAGENTA,paf->where,paf->bitvector,CLR_WHITE_BOLD);
                if ( paf->duration == -1 || paf->duration == -2)
                {
                  printf_to_char( ch,"%ssürekli%s\n\r" ,CLR_CYAN,CLR_WHITE);
                }
                else
                {
                  printf_to_char( ch,"%s%d%s saat\n\r",CLR_MAGENTA,paf->duration ,CLR_WHITE_BOLD);
                }
                break;
        }
      }
      else
      {
        printf_to_char( ch,": %sBilinmeyen etki%s\n\r",CLR_MAGENTA,CLR_WHITE_BOLD);
      }
      paf_last = paf;
    }
  }

  if (!IS_NPC(ch) && (IS_SET(ch->act,PLR_GHOST)))
  {
    found = TRUE;
    printf_to_char(ch,"{rHayalet{w: %d saat",ch->pcdata->ghost_mode_counter);
  }

  if (found == FALSE)
  {
    send_to_char("Hiçbir şeyin etkisinde değilsin.\n\r",ch);
  }

  return;
}


void do_familya(CHAR_DATA *ch, char *argument )
{
	int sn,col;
	col    = 0;
	printf_to_char(ch,"Irklara ilişkin irfanın:\n\r\n\r");
	for(sn=0;sn<MAX_RACE;sn++)
	{
		if(race_table[sn].name[1] == NULL)
			continue;
		if(str_cmp(race_table[sn].name[1],"unique"))
		{
			printf_to_char(ch,"%-*s %3d%%  ",utf8_width(race_table[sn].name[1], 18), race_table[sn].name[1], ch->pcdata->familya[sn]);
			if ( ++col % 3 == 0 )
				printf_to_char(ch, "\n\r" );
		}
	}
	if ( col % 3 != 0 )
		printf_to_char(ch, "\n\r" );
	return;
}


void do_lion_call( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *gch;
  CHAR_DATA *bear;
  CHAR_DATA *bear2;
  AFFECT_DATA af;
  int i;

  if (IS_NPC(ch) ||
      ch->level < skill_table[gsn_lion_call].skill_level[ch->iclass] )
    {
      send_to_char( "Huh?\n\r", ch );
      return;
    }

    send_to_char("Aslanları yardımına çağırıyorsun.\n\r",ch);
    act("$n aslanları yardımına çağırıyor.",ch,NULL,NULL,TO_ROOM);

  if (is_affected(ch, gsn_lion_call))
    {
      send_to_char( "Onları kontrol edecek gücü nasıl çağıracaksın?\n\r", ch);
      return;
    }
  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  gch->pIndexData->vnum == MOB_VNUM_LION)
	{
    send_to_char(  "Sahip olduğun aslanla bir sorunun mu var?",ch);
	  return;
	}
    }

  if ( ch->in_room != NULL && IS_SET(ch->in_room->room_flags, ROOM_NO_MOB) )
  {
    send_to_char(  "Hiçbir aslan seni dinleyemez.\n\r", ch );
     return;
  }

  if ( number_percent( ) > get_skill( ch, gsn_lion_call) )
  {
    send_to_char("Hiçbir aslan seni dinlemiyor.\n\r", ch );
	return;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)      ||
       IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)   ||
       IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)  ||
       (ch->in_room->exit[0] == NULL &&
          ch->in_room->exit[1] == NULL &&
          ch->in_room->exit[2] == NULL &&
          ch->in_room->exit[3] == NULL &&
          ch->in_room->exit[4] == NULL &&
          ch->in_room->exit[5] == NULL) ||
         ( ch->in_room->sector_type != SECT_FIELD &&
           ch->in_room->sector_type != SECT_FOREST &&
           ch->in_room->sector_type != SECT_MOUNTAIN &&
           ch->in_room->sector_type != SECT_HILLS ) )
  {
    send_to_char("Hiçbir aslan yardımına gelmiyor.\n\r", ch );
    return;
  }

  if ( ch->mana < 125 )
  {
    send_to_char(  "Aslan çağırmak için yeterli manan yok.\n\r", ch );
     return;
  }
  ch->mana -= 125;

  bear = create_mobile( get_mob_index(MOB_VNUM_LION), NULL );

  for (i=0;i < MAX_STATS; i++)
    {
      bear->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);
    }

  bear->max_hit = IS_NPC(ch)? ch->max_hit : ch->pcdata->perm_hit;
  bear->hit = bear->max_hit;
  bear->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  bear->mana = bear->max_mana;
  bear->alignment = ch->alignment;
  bear->level = UMIN(100,1 * ch->level-2);
  for (i=0; i < 3; i++)
    bear->armor[i] = interpolate(bear->level,100,-100);
  bear->armor[3] = interpolate(bear->level,100,0);
  bear->sex = ch->sex;
  bear->silver = 0;

  bear2 = create_mobile(bear->pIndexData, NULL);
  clone_mobile(bear,bear2);

  SET_BIT(bear->affected_by, AFF_CHARM);
  SET_BIT(bear2->affected_by, AFF_CHARM);
  bear->master = bear2->master = ch;
  bear->leader = bear2->leader = ch;

  char_to_room(bear,ch->in_room);
  char_to_room(bear2,ch->in_room);
  send_to_char("Yardımına iki aslan geliyor!\n\r",ch);
  act("$s yardımına iki aslan geliyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = gsn_lion_call;
  af.level              = ch->level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

}

/* object condition aliases */
char *get_cond_alias( OBJ_DATA *obj)
{
 char *stat;
 int istat;

 istat = obj->condition;

 if      ( istat >  90 ) stat = "kusursuz";
 else if ( istat >= 80 ) stat = "iyi";
 else if ( istat >= 60 ) stat = "fena değil";
 else if ( istat >= 40 ) stat = "vasat";
 else if ( istat >= 20 ) stat = "hasarlı";
 else                    stat = "kırılgan";

 return stat;
}

/* room affects */
void do_raffects(CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char buf[MAX_STRING_LENGTH];

    if (ch->in_room->affected != NULL )
    {
      send_to_char( "Oda aşağıdaki büyülerden etkileniyor:\n\r", ch );
	for ( paf = ch->in_room->affected; paf != NULL; paf = paf->next )
	{
	    if (paf_last != NULL && paf->type == paf_last->type)
		if (ch->level >= 20 )
		    snprintf(buf, sizeof(buf), "                      ");
		else
		    continue;
	    else
      snprintf(buf, sizeof(buf), "Büyü: %-*s", utf8_width(skill_table[paf->type].name[1], 15), skill_table[paf->type].name[1]);

	    send_to_char( buf, ch );

	    if ( ch->level >= 20 )
	    {
		snprintf(buf, sizeof(buf),
      ": %s yönünü %d değiştirir ",
		    raffect_loc_name( paf->location ),
		    paf->modifier);
		send_to_char( buf, ch );
		if ( paf->duration == -1 || paf->duration == -2 )
    snprintf(buf, sizeof(buf), "sürekli" );
		else
    snprintf(buf, sizeof(buf), "%d saat", paf->duration );
		send_to_char( buf, ch );
	    }
	    send_to_char( "\n\r", ch );
	    paf_last = paf;
	}
    }
    else
    send_to_char("Oda hiçbir büyünün etkisinde değil.\n\r",ch);

    return;
}



/*
 * New 'who_col' command by chronos
 */
void do_who_col( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char cabalbuf[MAX_STRING_LENGTH];
    char titlebuf[MAX_STRING_LENGTH];
    char classbuf[100];
    char output[4 * MAX_STRING_LENGTH];
    char pk_buf[100];
    char act_buf[100];
    char level_buf[100];
    static CHAR_DATA *who_dch[BOT_WHO_MAX], *who_wch[BOT_WHO_MAX];
    int who_n, who_i;
    int iClass;
    int iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int vnum;
    int count;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fImmortalOnly;
    bool fPKRestrict;
    bool fRulerRestrict;
    bool fChaosRestrict;
    bool fShalafiRestrict;
    bool fInvaderRestrict;
    bool fBattleRestrict;
    bool fKnightRestrict;
    bool fLionsRestrict;
    bool fTattoo;

    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fClassRestrict = FALSE;
    fRaceRestrict = FALSE;
    fPKRestrict = FALSE;
    fImmortalOnly  = FALSE;
    fBattleRestrict = FALSE;
    fChaosRestrict = FALSE;
    fRulerRestrict = FALSE;
    fInvaderRestrict = FALSE;
    fShalafiRestrict = FALSE;
    fKnightRestrict = FALSE;
    fLionsRestrict = FALSE;
    vnum = 0;
    fTattoo = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
	rgfRace[iRace] = FALSE;


    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if (!str_cmp(arg,"pk"))
	  {
	    fPKRestrict = TRUE;
	    break;
	  }

	if (!str_cmp(arg,"tüze"))
	  {
	    if (ch->cabal != CABAL_RULER && !IS_IMMORTAL(ch))
	      {
		send_to_char("Bu kabala üye değilsin!\n\r",ch);
		return;
	      }
	    else
	      {
		fRulerRestrict = TRUE;
		break;
	      }
	  }
	if (!str_cmp(arg,"tılsım"))
	  {
	    if (ch->cabal != CABAL_SHALAFI && !IS_IMMORTAL(ch))
	      {
		send_to_char("Bu kabala üye değilsin!\n\r",ch);
		return;
	      }
	    else
	      {
		fShalafiRestrict = TRUE;
		break;
	      }
	  }
	if (!str_cmp(arg,"öfke"))
	  {
	    if (ch->cabal != CABAL_BATTLE && !IS_IMMORTAL(ch))
	      {
		send_to_char("Bu kabala üye değilsin!\n\r",ch);
		return;
	      }
	    else
	      {
		fBattleRestrict = TRUE;
		break;
	      }
	  }
	if (!str_cmp(arg,"istila"))
	  {
	    if (ch->cabal != CABAL_INVADER && !IS_IMMORTAL(ch))
	      {
		send_to_char("Bu kabala üye değilsin!\n\r",ch);
		return;
	      }
	    else
	      {
		fInvaderRestrict = TRUE;
		break;
	      }
	  }
	if (!str_cmp(arg,"kaos"))
	  {
	    if (ch->cabal != CABAL_CHAOS && !IS_IMMORTAL(ch))
	      {
		send_to_char("Bu kabala üye değilsin!\n\r",ch);
		return;
	      }
	    else
	      {
		fChaosRestrict = TRUE;
		break;
	      }
	  }
	if (!str_cmp(arg,"şövalye"))
	  {
	    if (ch->cabal != CABAL_KNIGHT && !IS_IMMORTAL(ch))
	      {
		send_to_char("Bu kabala üye değilsin!\n\r",ch);
		return;
	      }
	    else
	      {
		fKnightRestrict = TRUE;
		break;
	      }
	  }
	if (!str_cmp(arg,"aslan"))
	  {
	    if (ch->cabal != CABAL_LIONS && !IS_IMMORTAL(ch))
	      {
		send_to_char("Bu kabala üye değilsin!\n\r",ch);
		return;
	      }
	    else
	      {
		fLionsRestrict = TRUE;
		break;
	      }
	  }
	if (!str_cmp(arg,"dövme"))
	  {
	    if (get_eq_char(ch,WEAR_TATTOO) == NULL)
	      {
		send_to_char("Henüz dövmen yok!\n\r",ch);
		return;
	      }
	    else
	      {
		fTattoo = TRUE;
		vnum = get_eq_char(ch,WEAR_TATTOO)->pIndexData->vnum;
		break;
	      }
	  }


	if ( is_number( arg ) && IS_IMMORTAL(ch))
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Bu özellik ölümsüzler içindir.\n\r",ch);
		return;
	    }
	}
	else
	{

	    /*
	     * Look for classes to turn on.
	     */
	    if ( arg[0] == 'i' )
	    {
		fImmortalOnly = TRUE;
	    }
	    else
	    {
	      iClass = class_lookup(arg);
	      if (iClass == -1 || !IS_IMMORTAL(ch))
	      	{
	          iRace = race_lookup(arg);

	          if (iRace == 0 || iRace >= MAX_PC_RACE)
		    {
		      send_to_char("Geçerli bir ırk değil.\n\r",ch);
		      return;
		    }
	          else
		    {
			fRaceRestrict = TRUE;
			rgfRace[iRace] = TRUE;
		    }
		}
	      else
		{
		  fClassRestrict = TRUE;
		  rgfClass[iClass] = TRUE;
		}
	    }
	  }
      }

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output[0] = '\0';
    who_n = bot_who_collect( who_dch, who_wch, BOT_WHO_MAX );
    for ( who_i = 0; who_i < who_n; who_i++ )
    {
	CHAR_DATA *wch;
	CHAR_DATA *dch = who_dch[who_i];
	OBJ_DATA *tattoo;
	char const *iclass;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if ( !can_see( ch, dch ) )
	    continue;

	if ( ((IS_VAMPIRE( dch ) || IS_BEAR( dch )) && !IS_IMMORTAL(ch) && (ch != dch) ) )
	    continue;

	wch   = who_wch[who_i];
	if (!can_see(ch, wch)) /* can't see switched wizi imms */
	  continue;

	if ( wch->level < iLevelLower
	||   wch->level > iLevelUpper
	|| ( fImmortalOnly  && wch->level < LEVEL_HERO )
	|| ( fClassRestrict && !rgfClass[wch->iclass])
	|| ( fRaceRestrict && !rgfRace[RACE(wch)])
        || ( fPKRestrict && is_safe_nomessage(ch,wch) )
	|| ( fTattoo && ( ( tattoo = get_eq_char( wch, WEAR_TATTOO ) ) == NULL
			|| tattoo->pIndexData->vnum != vnum ) )
	    || (fRulerRestrict && wch->cabal != CABAL_RULER )
	    || (fChaosRestrict && wch->cabal != CABAL_CHAOS)
	    || (fBattleRestrict && wch->cabal != CABAL_BATTLE)
	    || (fInvaderRestrict && wch->cabal != CABAL_INVADER)
	    || (fShalafiRestrict && wch->cabal != CABAL_SHALAFI)
	    || (fKnightRestrict && wch->cabal != CABAL_KNIGHT)
	    || (fLionsRestrict && wch->cabal != CABAL_LIONS))
	    continue;

	nMatch++;

	/*
	 * Figure out what to print for class.
	 */
	iclass = class_table[wch->iclass].who_name;
	switch ( wch->level )
	{
	default: break;
            {
              case MAX_LEVEL - 0 : iclass = "YRT";     break;
              case MAX_LEVEL - 1 : iclass = "YRT";     break;
              case MAX_LEVEL - 2 : iclass = "TNR";     break;
              case MAX_LEVEL - 3 : iclass = "TNR";     break;
              case MAX_LEVEL - 4 : iclass = "TNR";     break;
              case MAX_LEVEL - 5 : iclass = "MLK";     break;
              case MAX_LEVEL - 6 : iclass = "MLK";     break;
              case MAX_LEVEL - 7 : iclass = "YML";     break;
              case MAX_LEVEL - 8 : iclass = "YML";     break;
            }
	}

	/* for cabals
	if ((wch->cabal && (wch->cabal == ch->cabal ||
			   IS_TRUSTED(ch,LEVEL_IMMORTAL))) ||
                           wch->level >= LEVEL_HERO)
	*/
	if ( (wch->cabal && ch->cabal ==  wch->cabal) || IS_IMMORTAL(ch)
		|| ( IS_SET(wch->act , PLR_CANINDUCT) && wch->cabal == 1)
		|| wch->cabal == CABAL_HUNTER
		|| (wch->cabal == CABAL_RULER
		   && is_equiped_n_char(wch, OBJ_VNUM_RULER_BADGE, WEAR_NECK)) )
	{
	  snprintf(cabalbuf, sizeof(cabalbuf), "[%s%s%s] ",
		CLR_CYAN,
		cabal_table[wch->cabal].short_name,
		CLR_WHITE_BOLD	);
	}
	else cabalbuf[0] = '\0';
	if (wch->cabal == 0) cabalbuf[0] = '\0';

	pk_buf[0] = '\0';
	if (!((ch==wch && ch->level< KIDEMLI_OYUNCU_SEVIYESI ) || is_safe_nomessage(ch,wch)))
	  snprintf(pk_buf, sizeof(pk_buf),"%s(OK) %s", CLR_RED , CLR_WHITE_BOLD );

	act_buf[0] = '\0';
	snprintf(act_buf, sizeof(act_buf),"%s%s%s", CLR_WHITE ,
		    IS_SET(wch->act, PLR_WANTED) ? "(ARANIYOR) " : "",
	        CLR_WHITE_BOLD );

	if (IS_NPC(wch))
		snprintf(titlebuf, sizeof(titlebuf),"Tanrılara inanan.");
	else
		snprintf(titlebuf, sizeof(titlebuf),"%s", wch->pcdata->title );
	/*
	 * Format it up.
	 */

	snprintf(level_buf, sizeof(level_buf), "%s%3d%s", CLR_CYAN,wch->level, CLR_WHITE_BOLD);
	snprintf(classbuf, sizeof(classbuf),"%s%s%s",CLR_YELLOW,iclass, CLR_WHITE_BOLD);

	if (IS_TRUSTED(ch,LEVEL_IMMORTAL) || ch==wch ||
                   wch->level >= LEVEL_HERO)

	  snprintf(buf, sizeof(buf), "[%3d %*s %*s] %s%s%s%s%s\n\r",
	    wch->level,
	    utf8_width(RACE(wch) < MAX_PC_RACE ? race_table[RACE(wch)].who_name
				    : "     ", 8), RACE(wch) < MAX_PC_RACE ? race_table[RACE(wch)].who_name
				    : "     ",
	    utf8_width(classbuf, 3), classbuf,
	    pk_buf,
	    cabalbuf,
	    act_buf,
	    wch->name,
	    titlebuf);

	else
/*	  snprintf(buf, sizeof(buf), "[%s %s %s] %s%s%s%s%s\n\r",	*/
	  snprintf(buf, sizeof(buf), "[%3s %*s    ] %s%s%s%s%s\n\r",
		(get_curr_stat(wch, STAT_CHA) < 18 ) ? level_buf : "  ",
	    utf8_width(RACE(wch) < MAX_PC_RACE ? race_table[RACE(wch)].who_name : "     ", 8),
	    RACE(wch) < MAX_PC_RACE ? race_table[RACE(wch)].who_name
				    : "     ",
/*	    classbuf, 	*/
	    pk_buf,
	    cabalbuf,
	    act_buf,
	    wch->name,
	    titlebuf);

	strcat(output,buf);
    }

    count = who_n;

    max_on = UMAX(count,max_on);
    snprintf(buf2, sizeof(buf2), "\n\rOyuncular: %d, bugün: %d, en çok:%d.\n\r",
		nMatch,max_on,max_on_so_far );
    strcat(output,buf2);
    page_to_char( output, ch );
    return;
}


void do_camp( CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA af,af2;

  if (IS_NPC(ch) ||
      ch->level < skill_table[gsn_camp].skill_level[ch->iclass] )
    {
      send_to_char( "Hı?\n\r", ch );
      return;
    }

  if (is_affected(ch, gsn_camp))
    {
      send_to_char("Yeni kamp bölgeleri kurmak için yeterli enerjin yok.\n\r", ch);
      return;
    }


  if ( number_percent( ) > get_skill( ch, gsn_camp) )
  {
    send_to_char("Kamp kurmayı başaramadın.\n\r", ch );
	check_improve(ch,gsn_camp,TRUE,4);
	return;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)      ||
       IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)   ||
       IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)  ||
         ( ch->in_room->sector_type != SECT_FIELD &&
           ch->in_room->sector_type != SECT_FOREST &&
           ch->in_room->sector_type != SECT_MOUNTAIN &&
           ch->in_room->sector_type != SECT_HILLS ) )
  {
    send_to_char( "Kamp kurmak için yeterli yaprak yok.\n\r", ch );
    return;
  }

  if ( ch->mana < 150 )
  {
    send_to_char(  "Kamp kurmak için yeterli manan yok.\n\r", ch );
     return;
  }

  check_improve(ch,gsn_camp,TRUE,4);
  ch->mana -= 150;

  WAIT_STATE(ch,skill_table[gsn_camp].beats);

  send_to_char("Kamp kurmayı başardın.\n\r",ch);
  act("$n kamp kurmayı başardı.",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = gsn_camp;
  af.level              = ch->level;
  af.duration           = 12;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  af2.where		= TO_ROOM_CONST;
  af2.type               = gsn_camp;
  af2.level              = ch->level;
  af2.duration           = ch->level / 20;
  af2.bitvector          = 0;
  af2.modifier           = 2 * ch->level;
  af2.location           = APPLY_ROOM_HEAL;
  affect_to_room(ch->in_room, &af2);

  af2.modifier           = ch->level;
  af2.location           = APPLY_ROOM_MANA;
  affect_to_room(ch->in_room, &af2);

}


void do_demand( CHAR_DATA *ch, char *argument)
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA  *obj;
  int chance;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (IS_NPC(ch))
	return;

  if (ch->iclass != CLASS_ANTI_PALADIN)
    {
      send_to_char("Bunu yapamazsın.\n\r", ch);
      return;
    }

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "Kimden neyi talep edeceksin?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
      send_to_char( "Burada değil.\n\r", ch );
      return;
    }

  if (!IS_NPC(victim))
    {
      send_to_char( "Neden ondan istemeyi denemiyorsun?\n\r", ch);
      return;
    }

  WAIT_STATE(ch, PULSE_VIOLENCE);

  chance = IS_EVIL(victim) ? 10 : IS_GOOD(victim) ? -5 : 0;
  chance += (get_curr_stat(ch,STAT_CHA) - 15) * 10;
  chance += ch->level - victim->level;

  if (victim->level >= ch->level + 10 || victim->level >= ch->level * 2)
	chance = 0;

  if ( number_percent() > chance )
	{
    do_say(victim,"Sana birşey vermek zorunda değilim.");
         do_murder(victim, ch->name);
	 return;
	}

  if (( ( obj = get_obj_carry(victim , arg1 ) ) == NULL
      && (obj = get_obj_wear(victim, arg1)) == NULL)
      || IS_SET(obj->extra_flags, ITEM_INVENTORY))
    {
      do_say(victim,"Üzgünüm, o şeyden bende yok.");
      return;
    }


  if ( obj->wear_loc != WEAR_NONE )
    unequip_char(victim, obj);

  if ( !can_drop_obj( ch, obj ) )
    {
      do_say(victim,"O lanetli, kendimden ayıramıyorum. Affedin, efendim.");
	  if(number_percent()<30)
	  {
		printf_to_char(ch,"Rol yapma puanının arttığını hissediyorsun.\n\r");
		ch->pcdata->rk_puani += 1;
	  }
      return;
    }

  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
      send_to_char( "Ellerin dolu.\n\r", ch);
      return;
    }

  if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
      send_to_char( "O kadar ağırlık taşıyamazsın.\n\r", ch);
      return;
    }

  if ( !can_see_obj( ch, obj ) )
    {
      act( "Onu görmüyorsun.", ch, NULL, victim, TO_CHAR );
      return;
    }

  obj_from_char( obj );
  obj_to_char( obj, ch );
  act( "$n $Z $p talep ediyor.", ch, obj, victim, TO_NOTVICT );
  act( "Sen $Z $p talep ediyorsun.",   ch, obj, victim, TO_CHAR    );
  act( "$n senden $p talep ediyor.", ch, obj, victim, TO_VICT    );



  if (IS_SET(obj->progtypes,OPROG_GIVE))
  {
    (obj->pIndexData->oprogs->give_prog) (obj,ch,victim);
	}

  if (IS_SET(victim->progtypes,MPROG_GIVE))
  {
    (victim->pIndexData->mprogs->give_prog) (victim,ch,obj);
}

    send_to_char( "Gücün arzın ürpermesine neden oluyor.\n\r",ch);
	if(number_percent()<30)
	  {
		printf_to_char(ch,"Rol yapma puanının arttığını hissediyorsun.\n\r");
		ch->pcdata->rk_puani += 1;
	  }

  return;
}



void do_control( CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  argument = one_argument( argument, arg );

  if (IS_NPC(ch) ||
      ch->level < skill_table[gsn_control_animal].skill_level[ch->iclass] )
    {
      send_to_char("Hı?\n\r", ch );
      return;
    }

  if ( arg[0] == '\0' )
    {
      send_to_char("Neyi, kimi?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
      send_to_char("Burada değil.\n\r", ch );
      return;
    }

  if (race_table[ORG_RACE(victim)].pc_race)
    {
      send_to_char("Bunu moblar üzerinde denemelisin.\n\r", ch);
      return;
    }

  if (is_safe(ch,victim)) return;

  if (count_charmed(ch))  return;

  WAIT_STATE(ch, PULSE_VIOLENCE);

  chance = get_skill(ch,gsn_control_animal);

  chance += (get_curr_stat(ch,STAT_CHA) - 20) * 5;
  chance += (ch->level - victim->level) * 3;
  chance +=
   (get_curr_stat(ch,STAT_INT) - get_curr_stat(victim,STAT_INT)) * 5;

 if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||	 number_percent() > chance
    ||   ch->level < ( victim->level + 2 )
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   (IS_NPC(victim) && victim->pIndexData->pShop != NULL) )
	{
	 check_improve(ch,gsn_control_animal,FALSE,2);
	 do_say(victim,"Seni izlemek istemiyorum.");
         do_murder(victim, ch->name);
	 return;
	}

  check_improve(ch,gsn_control_animal,TRUE,2);

  if ( victim->master )
	stop_follower( victim );
  SET_BIT(victim->affected_by,AFF_CHARM);
  victim->master = victim->leader = ch;

  act( "$n sence de şirin değil mi?", ch, NULL, victim, TO_VICT );
  if ( ch != victim )
  act("$N etkilenmiş gözlerle sana bakıyor.",ch,NULL,victim,TO_CHAR);

  return;
}


void do_make_arrow( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *arrow;
  AFFECT_DATA tohit,todam,saf;
  int count,color,mana,wait;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char *str;

  if (IS_NPC(ch)) return;
  if (ch_skill_nok_nomessage(ch,gsn_make_arrow))
    {
      send_to_char("Ok yapmayı bilmiyorsun.\n\r", ch );
      return;
    }

  if ( ch->in_room->sector_type != SECT_FIELD &&
       ch->in_room->sector_type != SECT_FOREST &&
       ch->in_room->sector_type != SECT_HILLS )
  {
    send_to_char( "Yeterince odun bulamadın.\n\r", ch );
    return;
  }

  mana = skill_table[gsn_make_arrow].min_mana;
  wait = skill_table[gsn_make_arrow].beats;

  argument = one_argument(argument, arg);
  if (arg[0] == '\0') color = 0;
  else if (!str_prefix(arg,"yeşil")) color = gsn_green_arrow;
  else if (!str_prefix(arg,"kızıl")) color = gsn_red_arrow;
  else if (!str_prefix(arg,"beyaz")) color = gsn_white_arrow;
  else if (!str_prefix(arg,"mavi")) color = gsn_blue_arrow;
  else
  {
    send_to_char(  "Bu tür bir ok yapmayı bilmiyorsun.\n\r", ch );
     return;
  }

  if (color != 0 )
  {
   mana += skill_table[color].min_mana;
   wait += skill_table[color].beats;
  }

  if ( ch->mana < mana )
  {
    send_to_char("Bu tür bir ok yapmak için yeterince enerjin yok.\n\r", ch );
     return;
  }
  ch->mana -= mana;
  WAIT_STATE(ch,wait);

  send_to_char("Ok yapmaya başlıyorsun!\n\r",ch);
  act("$n ok yapmaya başladı!",ch,NULL,NULL,TO_ROOM);
  for(count=0; count < (ch->level/5); count++)
  {
   if ( number_percent( ) > get_skill( ch, gsn_make_arrow) )
   {
     send_to_char("Yapmaya çalıştığın oku kırdın.\n\r", ch );
	check_improve(ch,gsn_make_arrow,FALSE,3);
	continue;
   }
   send_to_char( "Ok yaptın.\n\r",ch );
   check_improve(ch,gsn_make_arrow,TRUE,3);

   arrow = create_object(get_obj_index(OBJ_VNUM_RANGER_ARROW),ch->level);
   arrow->level = ch->level;
   arrow->value[1] = ch->level / 10;
   arrow->value[2] = ch->level / 10;

   tohit.where		    = TO_OBJECT;
   tohit.type               = gsn_make_arrow;
   tohit.level              = ch->level;
   tohit.duration           = -1;
   tohit.location           = APPLY_HITROLL;
   tohit.modifier           = ch->level / 10;
   tohit.bitvector          = 0;
   affect_to_obj(arrow,&tohit);

   todam.where		   = TO_OBJECT;
   todam.type               = gsn_make_arrow;
   todam.level              = ch->level;
   todam.duration           = -1;
   todam.location           = APPLY_DAMROLL;
   todam.modifier           = ch->level / 10;
   todam.bitvector          = 0;
   affect_to_obj(arrow,&todam);

   if ( color != 0 )
   {
    saf.where		   = TO_WEAPON;
    saf.type               = color;
    saf.level              = ch->level;
    saf.duration           = -1;
    saf.location           = 0;
    saf.modifier           = 0;

    if (color == gsn_green_arrow)
	{
	 saf.bitvector	= WEAPON_POISON;
	 str = "yeşil";
	}
     else if (color == gsn_red_arrow)
	{
	 saf.bitvector	= WEAPON_FLAMING;
	 str = "kızıl";
	}
     else if (color == gsn_white_arrow)
	{
	 saf.bitvector	= WEAPON_FROST;
	 str = "beyaz";
	}
     else
	{
	 saf.bitvector	= WEAPON_SHOCKING;
	 str = "mavi";
	}
   }
   else str = "tahta";

   fill_template( buf, sizeof(buf), arrow->name, str );
   free_string( arrow->name );
   arrow->name = str_dup( buf );

   fill_template( buf, sizeof(buf), arrow->short_descr, str );
   free_string( arrow->short_descr );
   arrow->short_descr = str_dup( buf );

   fill_template( buf, sizeof(buf), arrow->description, str );
   free_string( arrow->description );
   arrow->description = str_dup( buf );

   if (color !=0 ) affect_to_obj(arrow,&saf);
   obj_to_char(arrow,ch);
   arrow = NULL;
  }

}



void do_make_bow( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *bow;
  AFFECT_DATA tohit,todam;
  int mana,wait;

  if (IS_NPC(ch)) return;

  if (ch_skill_nok_nomessage(ch,gsn_make_bow))
    {
      send_to_char("Yay yapmayı bilmiyorsun.\n\r", ch );
      return;
    }

  if ( ch->in_room->sector_type != SECT_FIELD &&
       ch->in_room->sector_type != SECT_FOREST &&
       ch->in_room->sector_type != SECT_HILLS )
  {
    send_to_char("Yeterince odun bulamadın.\n\r", ch );
    return;
  }

  mana = skill_table[gsn_make_bow].min_mana;
  wait = skill_table[gsn_make_bow].beats;

  if ( ch->mana < mana )
  {
    send_to_char( "Yay yapmak için yeterli enerjin yok.\n\r", ch);
     return;
  }
  ch->mana -= mana;
  WAIT_STATE(ch,wait);

  if ( number_percent( ) > get_skill( ch, gsn_make_bow) )
   {
     send_to_char( "Yapmaya çalıştığın yayı kırdın.\n\r", ch );
	check_improve(ch,gsn_make_bow,FALSE,1);
	return;
   }
   send_to_char("Yay yaptın.\n\r",ch );
  check_improve(ch,gsn_make_bow,TRUE,1);

  bow = create_object(get_obj_index(OBJ_VNUM_RANGER_BOW),ch->level);
  bow->level = ch->level;
  bow->value[1] = 3 + ch->level / 12;
  bow->value[2] = 4 + ch->level / 12;

  tohit.where		    = TO_OBJECT;
  tohit.type               = gsn_make_bow;
  tohit.level              = ch->level;
  tohit.duration           = -1;
  tohit.location           = APPLY_HITROLL;
  tohit.modifier           = ch->level / 10;
  tohit.bitvector          = 0;
  affect_to_obj(bow,&tohit);

  todam.where		   = TO_OBJECT;
  todam.type               = gsn_make_bow;
  todam.level              = ch->level;
  todam.duration           = -1;
  todam.location           = APPLY_DAMROLL;
  todam.modifier           = ch->level / 10;
  todam.bitvector          = 0;
  affect_to_obj(bow,&todam);

  obj_to_char(bow,ch);
}


void do_make( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument,arg);
  if (arg[0] == '\0')
  {
    send_to_char("Ok ya da yay yapabilirsin.\n\r",ch);
   return;
  }

  if (!str_prefix(arg,"ok")) do_make_arrow(ch,argument);
  else if (!str_prefix(arg,"yay")) do_make_bow(ch,argument);
  else do_make(ch,"");
}


void do_nocancel(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    toggle_flag( ch, &ch->act, PLR_NOCANCEL,
	"Başkalarının iptal büyülerini kabul etmiyorsun.\n\r",
	"Başkalarının iptal büyülerini kabul ediyorsun.\n\r" );
}


void do_diril( CHAR_DATA *ch, char *argument )
{
  if(IS_NPC(ch))
  {
    return;
  }

  if (IS_SET(ch->act, PLR_GHOST))
  {
    REMOVE_BIT(ch->act,PLR_GHOST);
    printf_to_char(ch,"Ete kemiğe büründüğünü hissediyorsun. Arkanı kollamaya başlasan iyi olur!\n\r");
    act ("$n ete kemiğe bürünüyor!",ch,NULL,NULL,TO_ROOM);
    while ( ch->affected )
      affect_remove( ch, ch->affected );
    ch->affected_by	= 0;
    ch->detection	= 0;
  }
  else
  {
    printf_to_char(ch,"Zaten oldukça dirisin!\n\r");
  }

  return;
}

void do_discord( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	FILE *fp;
	char line[30];
	int is_found;

	argument = one_argument(argument,arg);
	if (arg[0] == '\0')
	{
		printf_to_char(ch,"Bu karakterin bağlı olduğu discord kullanıcısının 19 karakterlik ID'sini vermelisin.\n\r");
		printf_to_char(ch,"Örneğin: discord 123456789123456789\n\r");
		return;
	}

	if (strlen(arg) != 18 && strlen(arg) != 19)
	{
		printf_to_char(ch,"Discord kullanıcısının ID'si 18 veya 19 karakterden oluşmalıdır.\n\r");
		printf_to_char(ch,"Örneğin: discord 123456789123456789\n\r");
		return;
	}
	
	if ( ( fp = fopen( "../data/discord_users", "r" ) ) == NULL )
	{
		printf_to_char(ch,"İşlem yapılamadı, daha sonra tekrar deneyiniz.\n\r");
		return;
	}

	is_found = 0;
	
	while(fgets (line, 30, fp) != NULL)
	{
		line[strcspn(line, "\r\n")] = 0;
		if (!str_cmp(arg,line))
		{
			is_found = 1;
		}
	}
	
	if(is_found == 1)
	{
		free_string( ch->pcdata->discord_id );
		ch->pcdata->discord_id = str_dup( arg );
		printf_to_char(ch,"Discord kullanıcı ID'si kaydedildi.\n\r");
	}
	else
	{
		printf_to_char(ch,"Discord Mangus loncasında %s ID'li bir üyemiz bulunmuyor. Discord ID'si kontrol ederek tekrar deneyin.\n\r",arg);
	}

	return;
}
