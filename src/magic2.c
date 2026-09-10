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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"

DECLARE_DO_FUN(do_scan2);
/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_murder	);
DECLARE_DO_FUN(do_kill		);

void	raw_kill(CHAR_DATA *victim);
int	find_door	( CHAR_DATA *ch, char *arg );
int	check_exit	( char *argument );

extern char *target_name;


ROOM_INDEX_DATA * check_place( CHAR_DATA *ch, char *argument )
{
 EXIT_DATA *pExit;
 ROOM_INDEX_DATA *dest_room;
 int number,door;
 int range = (ch->level / 10) + 1;
 char arg[MAX_INPUT_LENGTH];

 number = number_argument(argument,arg);
 if ( (door = check_exit( arg )) == -1) return NULL;

 dest_room = ch->in_room;
 while (number > 0)
 {
  number--;
  if ( --range < 1 ) return NULL;
  if ( (pExit = dest_room->exit[door]) == NULL
      || (dest_room = pExit->u1.to_room) == NULL
      || IS_SET(pExit->exit_info,EX_CLOSED) )
   break;
  if ( number < 1)    return dest_room;
 }
 return NULL;
}


/*
 * ---- Ortak yardımcılar (Grup 03) ----
 */

/* gsn_* karşılığı olmayan büyüleri bir kez arayıp saklar. */
static int cached_sn( sh_int *cache, const char *name )
{
    if ( *cache <= 0 )
	*cache = skill_lookup( name );
    return *cache;
}

/* Karaktere tek etki; tüm alanlar zorunlu (where unutulamaz). */
static void char_affect( CHAR_DATA *ch, int where, int sn, int level,
			 int duration, int location, int modifier, int bits )
{
    AFFECT_DATA af = {0};

    af.where     = where;
    af.type      = sn;
    af.level     = level;
    af.duration  = duration;
    af.location  = location;
    af.modifier  = modifier;
    af.bitvector = bits;
    affect_to_char( ch, &af );
}

/* Nesneye kalıcı (duration -1) tek etki. */
static void obj_affect( OBJ_DATA *obj, int sn, int level, int location, int modifier )
{
    AFFECT_DATA af = {0};

    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.location  = location;
    af.modifier  = modifier;
    af.bitvector = 0;
    affect_to_obj( obj, &af );
}

/* Büyünün yeniden kullanım bekleme etkisi sürüyor mu? Sürüyorsa mesajı basar. */
static bool spell_busy( CHAR_DATA *ch, int sn, const char *msg )
{
    if ( !is_affected( ch, sn ) )
	return FALSE;
    send_to_char( msg, ch );
    return TRUE;
}

/* Büyücüye 'duration' saatlik yeniden kullanım bekleme etkisi ekler. */
static void spell_set_cooldown( CHAR_DATA *ch, int sn, int level, int duration )
{
    char_affect( ch, TO_AFFECTS, sn, level, duration, APPLY_NONE, 0, 0 );
}

/* Kurban hâlâ ch'nin odasında mı? (Öldü/çıkarıldıysa işaretçiye dokunulmaz.) */
static bool still_in_room( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *rch;

    if ( ch->in_room == NULL )
	return FALSE;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
	if ( rch == victim )
	    return TRUE;
    return FALSE;
}

/* Şifa: yp'yi max ile sınırlı artırır, standart mesajları basar. */
static void heal_victim( CHAR_DATA *ch, CHAR_DATA *victim, int amount )
{
    victim->hit = UMIN( victim->hit + amount, victim->max_hit );
    update_pos( victim );
    send_to_char( "Sıcak bir duygu vücudunu sarıyor.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Tamam.\n\r", ch );
}

/*
 * Uzaktan bakma: karakter geçici olarak odaya taşınıp baktırılır ve geri
 * getirilir; binek bayrakları korunur. (act_info.c'de taşımasız bir
 * show_room() olsa buna gerek kalmazdı.)
 */
static void scry_room( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
    ROOM_INDEX_DATA *old;
    bool mounted;

    if ( room == NULL || ch->in_room == NULL )
    {
	send_to_char( "Orayı göremiyorsun.\n\r", ch );
	return;
    }
    if ( ch->in_room == room )
    {
	do_look( ch, "auto" );
	return;
    }
    mounted = MOUNTED( ch ) != NULL;
    old = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, room );
    do_look( ch, "auto" );
    char_from_room( ch );
    char_to_room( ch, old );
    if ( mounted && ch->mount != NULL )
    {
	ch->riding = TRUE;
	ch->mount->riding = TRUE;
    }
}

/* Dizgi alanındaki "%s" yerine name koyar (alan verisi biçim olarak geçmez). */
static void personalize_str( char **field, const char *name )
{
    char buf[MAX_STRING_LENGTH];

    str_fill_name( buf, sizeof(buf), *field, name );
    free_string( *field );
    *field = str_dup( buf );
}

/* Yaratığın kısa/uzun/açıklama dizgilerini name ile kişiselleştirir. */
static void personalize_mob( CHAR_DATA *mob, const char *name )
{
    personalize_str( &mob->short_descr, name );
    personalize_str( &mob->long_descr, name );
    personalize_str( &mob->description, name );
}

/* Prototipteki ilk ekstra açıklamayı name ile doldurup nesneye kopyalar. */
static void personalize_extra( OBJ_DATA *obj, const char *name )
{
    EXTRA_DESCR_DATA *ed, *src = obj->pIndexData->extra_descr;
    char buf[MAX_STRING_LENGTH];

    if ( src == NULL )
	return;
    str_fill_name( buf, sizeof(buf), src->description, name );
    ed = new_extra_descr();
    ed->keyword = str_dup( src->keyword );
    ed->description = str_dup( buf );
    ed->next = NULL;
    obj->extra_descr = ed;
}

/* Sahipli özel nesne (ruh kavanozu, ateş kalkanı): owner/from/altar/pit. */
static OBJ_DATA *create_owned_obj( CHAR_DATA *ch, int vnum )
{
    OBJ_DATA *obj = create_object( get_obj_index( vnum ), 0 );
    int i = align_index( ch );

    obj->owner = str_dup( ch->name );
    obj->from  = str_dup( ch->name );
    obj->altar = hometown_table[ch->hometown].altar[i];
    obj->pit   = hometown_table[ch->hometown].pit[i];
    obj->level = ch->level;
    obj->cost  = 0;
    return obj;
}

/* Portal/nexus hedefi uygun mu? Değilse "Başaramadın." basıp NULL döner. */
static CHAR_DATA *portal_target( CHAR_DATA *ch, int level, bool nexus )
{
    CHAR_DATA *victim = get_char_world( ch, target_name );
    ROOM_INDEX_DATA *from = ch->in_room, *to;

    if ( victim == NULL
    ||   victim == ch
    ||   (to = victim->in_room) == NULL
    ||   !can_see_room( ch, to )
    ||   IS_SET(to->room_flags, ROOM_SAFE | ROOM_PRIVATE | ROOM_SOLITARY | ROOM_NOSUMMON)
    ||   (nexus ? (!can_see_room( ch, from ) || IS_SET(from->room_flags, ROOM_SAFE))
		: IS_SET(from->room_flags, ROOM_NOSUMMON | ROOM_NO_RECALL))
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && is_safe_nomessage( ch, victim )
	  && IS_SET(victim->imm_flags, IMM_SUMMON))
    ||   (IS_NPC(victim) && !IS_NPC(ch)
	  && victim->pIndexData->vnum == ch->pcdata->questmob)
    ||   (IS_NPC(victim) && saves_spell( level, victim, DAM_NONE )) )
    {
	send_to_char( "Başaramadın.\n\r", ch );
	return NULL;
    }
    return victim;
}

/* Elde tutulan warp taşını yakar; ölümsüz taşsız da yapabilir. */
static bool consume_warp_stone( CHAR_DATA *ch )
{
    OBJ_DATA *stone = get_hold_char( ch );

    if ( stone == NULL || stone->item_type != ITEM_WARP_STONE )
    {
	if ( IS_IMMORTAL( ch ) )
	    return TRUE;
	send_to_char( "Büyü için gereken bileşene sahip değilsin.\n\r", ch );
	return FALSE;
    }
    act( "$p içindeki güce yükleniyorsun.", ch, stone, NULL, TO_CHAR );
    act( "$p alev alıyor ve yokoluyor!", ch, stone, NULL, TO_CHAR );
    extract_obj( stone );
    return TRUE;
}

/* Odaya bir portal nesnesi koyar. */
static OBJ_DATA *place_portal( ROOM_INDEX_DATA *room, int to_vnum, int timer )
{
    OBJ_DATA *portal = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );

    portal->timer = timer;
    portal->value[3] = to_vnum;
    obj_to_room( portal, room );
    return portal;
}

/* Kişinin taşıdığı hüküm/yardımcı nişanlarını yok eder. */
static void remove_badges( CHAR_DATA *ch, CHAR_DATA *who,
			   const char *msg_char, const char *msg_room )
{
    OBJ_DATA *badge, *obj_next;

    for ( badge = who->carrying; badge != NULL; badge = obj_next )
    {
	obj_next = badge->next_content;
	if ( badge->pIndexData->vnum != OBJ_VNUM_DEPUTY_BADGE
	&&   badge->pIndexData->vnum != OBJ_VNUM_RULER_BADGE )
	    continue;
	act( msg_char, ch, badge, NULL, TO_CHAR );
	if ( msg_room != NULL )
	    act( msg_room, ch, badge, NULL, TO_ROOM );
	extract_obj( badge );
    }
}

/* Odanın en az bir çıkışı var mı? */
static bool room_has_any_exit( ROOM_INDEX_DATA *room )
{
    int door;

    for ( door = 0; door < 6; door++ )
	if ( room->exit[door] != NULL )
	    return TRUE;
    return FALSE;
}

/* Yaratılan silahın zar tipi: seviyeye göre. */
static int conjured_weapon_dice( int level )
{
    return level < 50 ? level / 10 : level / 6 - 3;
}

/*
 * Nefes/alan hasarı: büyücünün yp'sine ve seviyeye bağlı ROM formülü
 * (magic.c'deki *_breath büyüleriyle birleştirilebilir).
 */
static int breath_dam( CHAR_DATA *ch, int level, int dice_type, int div )
{
    int hpch = UMAX( 10, ch->hit );
    int hp_dam = number_range( hpch/9+1, hpch/5 );
    int dice_dam = dice( level, dice_type );

    return UMAX( hp_dam + dice_dam / div, dice_dam + hp_dam / div );
}

void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal;

    if ( (victim = portal_target( ch, level, FALSE )) == NULL )
	return;
    if ( !consume_warp_stone( ch ) )
	return;

    portal = place_portal( ch->in_room, victim->in_room->vnum, 2 + level / 25 );

    act("$p havaya yükseliyor.",ch,portal,NULL,TO_ROOM);
    act("$p önünde havaya yükseliyor.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal;
    ROOM_INDEX_DATA *to_room, *from_room;

    if ( (victim = portal_target( ch, level, TRUE )) == NULL )
	return;
    if ( !consume_warp_stone( ch ) )
	return;

    from_room = ch->in_room;
    to_room = victim->in_room;

    /* portal one */
    portal = place_portal( from_room, to_room->vnum, 1 + level / 10 );
    act("$p havaya yükseliyor.",ch,portal,NULL,TO_ROOM);
    act("$p önünde havaya yükseliyor.",ch,portal,NULL,TO_CHAR);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
	return;

    /* portal two */
    portal = place_portal( to_room, from_room->vnum, 1 + level / 10 );
    if (to_room->people != NULL)
    {
      act("$p havaya yükseliyor.",to_room->people,portal,NULL,TO_ROOM);
      act("$p havaya yükseliyor.",to_room->people,portal,NULL,TO_CHAR);
    }
}


/*
 * Ani ölüm büyüsü iskeleti: kurtarma zarı (ya da %60) tutarsa yedek hasar;
 * tutmazsa mesajlar, olay günlüğü ve raw_kill. msg_* NULL olabilir.
 */
static void instant_death_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim,
				 const char *msg_vict, const char *msg_room,
				 const char *msg_char, const char *shout,
				 const char *gone )
{
    char eventbuf[MAX_INPUT_LENGTH];

    if ( saves_spell( level, victim, DAM_MENTAL ) || number_percent() < 60 )
    {
	damage( ch, victim, dice( level, 24 ), sn, DAM_MENTAL, TRUE );
	return;
    }

    if ( msg_vict != NULL )
	act_color( msg_vict, victim, NULL, ch, TO_CHAR, POS_RESTING, CLR_RED );
    if ( msg_room != NULL )
	act_color( msg_room, ch, NULL, victim, TO_NOTVICT, POS_RESTING, CLR_RED );
    if ( msg_char != NULL )
	act_color( msg_char, ch, NULL, victim, TO_CHAR, POS_RESTING, CLR_RED );
    send_to_char( shout, victim );

    act( gone, ch, NULL, victim, TO_CHAR );
    act( gone, ch, NULL, victim, TO_ROOM );

    if ( !IS_NPC( victim ) )
    {
	snprintf( eventbuf, sizeof(eventbuf), "%s, %s tarafından öldürüldü.",
		  victim->name, ch->name );
	write_event_log( eventbuf );
    }

    raw_kill( victim );
}

void spell_disintegrate( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  instant_death_spell( sn, level, ch, (CHAR_DATA *) vo,
    "$S ince ışık ışını seni $C### PARÇALIYOR ###$c!",
    "$s ince ışık ışını $M $C### PARÇALIYOR ###$c!",
    "İnce ışık ışının $M $C### PARÇALIYOR ###$c!",
    "Ö L D Ü R Ü L D Ü N!\n\r",
    "$N artık yok!" );
}

void spell_arz_yutagi( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  instant_death_spell( sn, level, ch, (CHAR_DATA *) vo,
    "$S büyüsü seni $C### YERİN İÇİNE ÇEKİYOR ###$c!",
    "$s büyüsü $M $C### YERİN İÇİNE ÇEKİYOR ###$c!",
    "Büyün $M $C### YERİN İÇİNE ÇEKİYOR ###$c!",
    "Y U T U L D U N!\n\r",
    "$N arz tarafından yutuldu!" );
}

void spell_bark_skin( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	    send_to_char("Derin zaten kabuklarla kaplanmış.\n\r",ch);
	else
	    act("$N olabildiğince sert zaten.",ch,NULL,victim,TO_CHAR);
	return;
    }
    char_affect( victim, TO_AFFECTS, sn, level, level, APPLY_AC, -(level * 3 / 2), 0 );
    act( "$s derisi ağaç kabuklarıyla kaplanıyor.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Derin ağaç kabuklarıyla kaplanıyor.\n\r", victim );
}

void spell_ranger_staff( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  OBJ_DATA *staff;

  staff = create_object( get_obj_index(OBJ_VNUM_RANGER_STAFF),level);
  send_to_char("Bir korucu asası yaratıyorsun!\n\r",ch);
  act("$n bir korucu asası yaratıyor!",ch,NULL,NULL,TO_ROOM);

  staff->value[2] = conjured_weapon_dice( ch->level );
  staff->level = ch->level;

  obj_affect( staff, sn, ch->level, APPLY_HITROLL, 2 + level/5 );
  obj_affect( staff, sn, ch->level, APPLY_DAMROLL, 2 + level/5 );

  staff->timer = level;

  obj_to_char(staff,ch);
}

void spell_transform( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;

  if (is_affected(ch,sn) || ch->hit > ch->max_hit)
    {
      send_to_char("Zaten heryerinden sağlık fışkırıyor.\n\r",ch);
      return;
    }

  ch->hit += UMIN(30000 - ch->max_hit, ch->max_hit);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.location           = APPLY_HIT;
  af.modifier           = UMIN(30000 - ch->max_hit, ch->max_hit);
  af.bitvector          = 0;
  affect_to_char(ch,&af);


  send_to_char( "Sağlığın arttıkça aklın karışıyor.\n\r",ch);
}

void spell_mental_knife( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (ch->level < 40)
	dam = dice(level,8);
  else if (ch->level < 65)
	dam = dice(level,11);
  else dam = dice(level,14);

  if (saves_spell(level,victim, DAM_MENTAL))
	      dam /= 2;
  damage(ch,victim,dam,sn,DAM_MENTAL, TRUE);

  if (!is_affected(victim,sn) && !saves_spell(level, victim, DAM_MENTAL))
    {
      af.where		    = TO_AFFECTS;
      af.type               = sn;
      af.level              = level;
      af.duration           = level;
      af.location           = APPLY_INT;
      af.modifier           = -7;
      af.bitvector          = 0;
      affect_to_char(victim,&af);

      af.location = APPLY_WIS;
      affect_to_char(victim,&af);
      act( "Zihin bıçağın $S aklını kavuruyor!",ch,NULL,victim,TO_CHAR);
      act("$s zihin bıçağı senin aklını kavuruyor!",ch,NULL,victim,TO_VICT);
      act("$s zihin bıçağı $S aklını kavuruyor!",ch,NULL,victim,TO_NOTVICT);
    }
}

void spell_demon_summon( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *demon;
  AFFECT_DATA af;
  int i;

  if (is_affected(ch,sn))
    {
      send_to_char("Başka bir iblis çağırmaya gücün kalmadı.\n\r",ch);

      return;
    }

    send_to_char("Bir iblis çağırmayı deniyorsun.\n\r",ch);
    act("$n bir iblis çağırmayı deniyor.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  gch->pIndexData->vnum == MOB_VNUM_DEMON)
	{
    send_to_char( "İki iblisi kontrol edemezsin!\n\r",ch);
	  return;
	}
    }

  if (count_charmed(ch)) return;

  demon = create_mobile( get_mob_index(MOB_VNUM_DEMON) , NULL);

  for (i=0;i < MAX_STATS; i++)
    {
      demon->perm_stat[i] = ch->perm_stat[i];
    }

  demon->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: URANGE(ch->pcdata->perm_hit,ch->hit,30000);
  demon->hit = demon->max_hit;
  demon->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  demon->mana = demon->max_mana;
  demon->level = ch->level;
  for (i=0; i < 3; i++)
    demon->armor[i] = interpolate(demon->level,100,-100);
  demon->armor[3] = interpolate(demon->level,100,0);
  demon->silver = 0;
  demon->timer = 0;
  demon->damage[DICE_NUMBER] = number_range(level/15, level/12);
  demon->damage[DICE_TYPE] = number_range(level/3, level/2);
  demon->damage[DICE_BONUS] = number_range(level/10, level/8);

  char_to_room(demon,ch->in_room);
  send_to_char("Yerin çok çok altından bir iblis geliyor!\n\r",ch);
  act("Yerin çok çok altından bir iblis geliyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  if (number_percent() < 40)
    {
      if ( can_see( demon, ch ) )
        do_say(demon, "Beni rahatsız etmeye nasıl cüret edersin?!!!");
      else
        do_say(demon, "Beni rahatsız etmeye cüret eden kim?!!!");
      do_murder(demon, ch->name);
    }
  else {
    SET_BIT(demon->affected_by, AFF_CHARM);
    demon->master = demon->leader = ch;
  }

}

void spell_scourge( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *tmp_vict;
  CHAR_DATA *tmp_next;
  int dam;

  if (ch->level < 40)
	dam = dice(level,6);
  else if (ch->level < 65)
	dam = dice(level,9);
  else dam = dice(level,12);

  for (tmp_vict = ch->in_room->people;tmp_vict != NULL;
       tmp_vict = tmp_next)
    {
      tmp_next = tmp_vict->next_in_room;

      if ( !is_safe_spell(ch,tmp_vict,TRUE))
	{
	  if (!IS_NPC(ch) && tmp_vict != ch &&
	      ch->fighting != tmp_vict && tmp_vict->fighting != ch &&
	      (IS_SET(tmp_vict->affected_by,AFF_CHARM) || !IS_NPC(tmp_vict)))
	    {
	      victim_yell( tmp_vict, ch, "İmdat biri bana saldırıyor!",
	      	     "Geber %s, seni büyücü köpek!" );
	    }

	  if (!is_affected(tmp_vict,sn)) {


	    if (number_percent() < level)
	      spell_poison(gsn_poison, level, ch, tmp_vict, TARGET_CHAR);

	    if (number_percent() < level)
	      spell_blindness(gsn_blindness,level,ch,tmp_vict, TARGET_CHAR);

	    if (number_percent() < level)
	      spell_weaken(gsn_weaken, level, ch, tmp_vict, TARGET_CHAR);

            if (saves_spell(level,tmp_vict, DAM_FIRE))
	      dam /= 2;
	    damage( ch, tmp_vict, dam, sn, DAM_FIRE, TRUE );
	  }

	}
    }
}

void spell_doppelganger( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( (ch == victim) ||
      (is_affected(ch, sn) && (ch->doppel == victim)) )
    {
      act("Zaten onun gibi görünüyorsun.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if (IS_NPC(victim))
    {
      act("$N taklit edemeyeceğin kadar farklı.",ch,NULL,victim,TO_CHAR);
     return;
   }

  if (IS_IMMORTAL(victim))
    {
      send_to_char("Akıllı ol.\n\r",ch);
      return;
    }

  if (saves_spell(level,victim, DAM_CHARM))
   {
    send_to_char("Başaramadın.\n\r",ch);
    return;
   }

   act("$E benzemek için formunu değiştiriyorsun.",ch,NULL,victim,TO_CHAR);
   act("$n SANA benzemek için formunu değiştiriyor!",ch,NULL,victim,TO_VICT);
   act("$n $E benzemek için formunu değiştiriyor!",ch,NULL,victim,TO_NOTVICT);

  af.where 		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = (2 * level)/3;
  af.location           = APPLY_NONE;
  af.modifier           = 0;
  af.bitvector          = 0;

  affect_to_char(ch,&af);
  ch->doppel = victim;

}

void spell_manacles( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (!IS_SET(victim->act, PLR_WANTED))
    {
      act("Fakat $N aranmıyor.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if (!is_affected(victim, sn) && !saves_spell(ch->level, victim,DAM_CHARM))
    {
      AFFECT_DATA af;

      af.where		    = TO_AFFECTS;
      af.type               = sn;
      af.level              = level;
      af.duration           = 5 + level/5;
      af.bitvector          = 0;

      af.modifier           = 0 - (get_curr_stat(victim, STAT_DEX)-4);
      af.location           = APPLY_DEX;
      affect_to_char(victim, &af);

      af.modifier           = -5;
      af.location           = APPLY_HITROLL;
      affect_to_char(victim, &af);


      af.modifier           = -10;
      af.location           = APPLY_DAMROLL;
      affect_to_char(victim, &af);

      spell_charm_person(gsn_charm_person, level, ch, vo, TARGET_CHAR);
    }
}

/* Hüküm kalkanı: seviye eşiklerine göre vnum. */
static int ruler_shield_vnum( int level )
{
    if ( level >= 71 ) return OBJ_VNUM_RULER_SHIELD4;
    if ( level >= 51 ) return OBJ_VNUM_RULER_SHIELD3;
    if ( level >= 31 ) return OBJ_VNUM_RULER_SHIELD2;
    return OBJ_VNUM_RULER_SHIELD1;
}

/* Büyüyle yaratılan kalkan (hüküm / aslan): ortak istatistikler. */
static void conjure_shield( CHAR_DATA *ch, int sn, int level, int vnum, int ac_mod )
{
  OBJ_DATA *shield;

  shield = create_object( get_obj_index(vnum), level );
  shield->timer = level;
  shield->level = ch->level;
  shield->cost  = 0;
  obj_to_char(shield, ch);

  obj_affect( shield, sn, level, APPLY_HITROLL, level / 8 );
  obj_affect( shield, sn, level, APPLY_DAMROLL, level / 8 );
  obj_affect( shield, sn, level, APPLY_AC, ac_mod );
  obj_affect( shield, sn, level, APPLY_SAVING_SPELL, -level/9 );
  obj_affect( shield, sn, level, APPLY_CHA, UMAX(1,level / 30) );

  act("$p yaratıyorsun!",ch,shield,NULL,TO_CHAR);
  act("$n $p yaratıyor!",ch,shield,NULL,TO_ROOM);
}

void spell_shield_ruler( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  conjure_shield( ch, sn, level, ruler_shield_vnum( level ), -level/2 );
}

void spell_guard_call( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *guard;
  CHAR_DATA *guard2;
  CHAR_DATA *guard3;
  AFFECT_DATA af;
  char buf[] = "Muhafızlar!";
  int i;

  if (is_affected(ch,sn))
    {
      send_to_char( "Üç muhafız daha çağırmaya gücün kalmadı.\n\r",ch);
      return;
    }

  do_yell(ch, buf);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch
	  && gch->pIndexData->vnum == MOB_VNUM_SPECIAL_GUARD)
	{
	  do_say(gch, "Nasıl yani? Ben yeterince iyi değil miyim?");
	  return;
	}
    }

  if (count_charmed(ch)) return;

  guard = create_mobile( get_mob_index(MOB_VNUM_SPECIAL_GUARD), NULL );

  for (i=0;i < MAX_STATS; i++)
    guard->perm_stat[i] = ch->perm_stat[i];

  guard->max_hit = 2*ch->max_hit;
  guard->hit = guard->max_hit;
  guard->max_mana = ch->max_mana;
  guard->mana = guard->max_mana;
  guard->alignment = ch->alignment;
  guard->level = ch->level;
  for (i=0; i < 3; i++)
    guard->armor[i] = interpolate(guard->level,100,-200);
  guard->armor[3] = interpolate(guard->level,100,-100);
  guard->sex = ch->sex;
  guard->silver = 0;
  guard->timer = 0;

  guard->damage[DICE_NUMBER] = number_range(level/16, level/12);
  guard->damage[DICE_TYPE] = number_range(level/3, level/2);
  guard->damage[DICE_BONUS] = number_range(level/9, level/6);

  guard->detection = (A|C|D|E|F|G|H|ee);
  SET_BIT(guard->affected_by, AFF_CHARM);
  SET_BIT(guard->affected_by, AFF_SANCTUARY);

  guard2 = create_mobile(guard->pIndexData, NULL);
  clone_mobile(guard,guard2);

  guard3 = create_mobile(guard->pIndexData, NULL);
  clone_mobile(guard,guard3);

  guard->master = guard2->master = guard3->master = ch;
  guard->leader = guard2->leader = guard3->leader = ch;

  char_to_room(guard,ch->in_room);
  char_to_room(guard2,ch->in_room);
  char_to_room(guard3,ch->in_room);
  send_to_char( "Yardımına üç muhafız geliyor!\n\r",ch);
  act("$s yardımına üç muhafız geliyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 6;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

}

void spell_nightwalker( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *walker;
  AFFECT_DATA af;
  char buf[100];
  int i;

  if (is_affected(ch,sn))
    {
      send_to_char("Bir solucan çağıracak gücü bulamıyorsun.\n\r",ch);

      return;
    }

    send_to_char("Bir solucan çağırmayı denedin.\n\r",ch);
    act("$n bir solucan çağırmayı denedi.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  gch->pIndexData->vnum == MOB_VNUM_NIGHTWALKER)
	{
    send_to_char( "İki solucanı kontrol edemezsin!\n\r",ch);
	  return;
	}
    }

  if (count_charmed(ch)) return;

  walker = create_mobile( get_mob_index(MOB_VNUM_NIGHTWALKER) , NULL);

  for (i=0;i < MAX_STATS; i++)
    {
      walker->perm_stat[i] = ch->perm_stat[i];
    }

  walker->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
                : URANGE(ch->pcdata->perm_hit,1*ch->pcdata->perm_hit,30000);
  walker->hit = walker->max_hit;
  walker->max_mana = ch->max_mana;
  walker->mana = walker->max_mana;
  walker->level = ch->level;
  for (i=0; i < 3; i++)
    walker->armor[i] = interpolate(walker->level,100,-100);
  walker->armor[3] = interpolate(walker->level,100,0);
  walker->silver = 0;
  walker->timer = 0;
  walker->damage[DICE_NUMBER] = number_range(level/15, level/10);
  walker->damage[DICE_TYPE]   = number_range(level/3, level/2);
  walker->damage[DICE_BONUS]  = 0;

  char_to_room(walker,ch->in_room);
  send_to_char("Gölgelerin içinden bir solucan yükseliyor!\n\r",ch);
  act("Gölgelerin içinden bir solucan yükseliyor!",ch,NULL,NULL,TO_ROOM);
  snprintf(buf, sizeof(buf), "Bir solucan önünde eğiliyor.");
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Bir solucan %s önünde eğiliyor!", ch->name );
  act( buf, ch, NULL, NULL, TO_ROOM );

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  SET_BIT(walker->affected_by, AFF_CHARM);
  walker->master = walker->leader = ch;

}

void spell_eyes( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;

    if ( (victim = get_char_world(ch, target_name)) == NULL)
      {
        send_to_char("Casus ağın öyle birine rastlamıyor.\n\r",ch);
	return;
      }

    if ((victim->level > ch->level + 7)
	|| saves_spell((ch->level + 9), victim, DAM_NONE))
      {
        send_to_char("Casus ağın öyle birini bulamıyor.\n\r",ch);
	return;
      }

    scry_room( ch, victim->in_room );
}

void spell_shadow_cloak( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (ch->cabal != victim->cabal)
      {
        send_to_char("Bu büyüyü ancak kabal dostların üzerinde kullanabilirsin.\n\r",ch);
	return;
      }

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
  send_to_char("Zaten bir gölge peleriniyle korunuyorsun.\n\r",ch);
else
  act("$N zaten bir gölge peleriniyle korunuyor.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.modifier  = -level;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char("Gölgelerin seni koruduğunu hissediyorsun.\n\r", victim );
    if ( ch != victim )
	act("Bir gölge pelerini $M koruyor.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_nightfall( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *vch;
  OBJ_DATA  *light;

  if ( spell_busy( ch, sn, "Işıkları kontrol edebilecek gücün yok.\n\r" ) )
    return;

  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    for (light = vch->carrying; light != NULL; light = light->next_content)
      {
	if (light->item_type == ITEM_LIGHT && light->value[2] != 0
		&& !is_same_group(ch, vch)) {
	  if ( saves_spell(level, vch, DAM_ENERGY))
	  {
	    act("$p titreyerek sönüyor!",ch,light,NULL,TO_CHAR);
	    act("$p titreyerek sönüyor!",ch,light,NULL,TO_ROOM);
	    light->value[2] = 0;
	    if (get_light_char(ch) == NULL) ch->in_room->light--;
	  }
	}
      }

  for (light = ch->in_room->contents;light != NULL; light=light->next_content)
    if (light->item_type == ITEM_LIGHT && light->value[2] != 0) {
      act("$p titreyerek sönüyor!",ch,light,NULL,TO_CHAR);
      act("$p titreyerek sönüyor!",ch,light,NULL,TO_ROOM);
      light->value[2] = 0;
    }

  spell_set_cooldown( ch, sn, level, 2 );
}

void spell_mirror( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af = {0};
  int mirrors,new_mirrors;
  CHAR_DATA *gch;
  CHAR_DATA *tmp_vict;
  char long_buf[MAX_STRING_LENGTH];
  char short_buf[MAX_INPUT_LENGTH];
  int order;

  if (IS_NPC(victim)) {
    send_to_char("Yalnız oyuncular aynalanabilir.\n\r",ch);
    return;
  }

  for (mirrors = 0, gch = char_list; gch != NULL; gch = gch->next)
    if (IS_NPC(gch) && is_affected(gch,gsn_mirror)
	&& is_affected(gch,gsn_doppelganger) && gch->doppel == victim)
      mirrors++;

  if (mirrors >= level/5) {
    if (ch==victim)
      send_to_char("Daha fazla aynalanamazsın.\n\r",ch);
    else
      act("$N daha fazla aynalanamaz.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_AFFECTS;
  af.level     = level;
  af.modifier  = 0;
  af.location  = 0;
  af.bitvector = 0;

  for (tmp_vict = victim; is_affected(tmp_vict,gsn_doppelganger)
       && tmp_vict->doppel != NULL; tmp_vict = tmp_vict->doppel)
    ;

  snprintf(long_buf, sizeof(long_buf), "%s%s burada.\n\r", tmp_vict->name,
	   IS_NPC(tmp_vict) ? "" : tmp_vict->pcdata->title);
  snprintf(short_buf, sizeof(short_buf), "%s", tmp_vict->name);

  order = number_range(0,level/5 - mirrors);

  for (new_mirrors=0; mirrors + new_mirrors < level/5;new_mirrors++)
  {
    gch = create_mobile( get_mob_index(MOB_VNUM_MIRROR_IMAGE), NULL );
    free_string(gch->name);
    free_string(gch->short_descr);
    free_string(gch->long_descr);
    free_string(gch->description);
    gch->name = str_dup(tmp_vict->name);
    gch->short_descr = str_dup(short_buf);
    gch->long_descr = str_dup(long_buf);
    gch->description = (tmp_vict->description == NULL) ?
                       NULL : str_dup(tmp_vict->description);
    gch->sex = tmp_vict->sex;

    af.type = gsn_doppelganger;
    af.duration = level;
    affect_to_char(gch,&af);
    af.type = gsn_mirror;
    af.duration = -1;
    affect_to_char(gch,&af);

    gch->max_hit = gch->hit = 1;
    gch->level = 1;
    gch->doppel = victim;
    gch->master = victim;
    char_to_room(gch,victim->in_room);

    /* kurbanı oda listesinde rastgele bir yere kaydır: görüntülerden ayırt edilmesin */
    if (number_percent() < 20 || new_mirrors == order)
    {
	ROOM_INDEX_DATA *ori_room = victim->in_room;

	char_from_room(victim);
	char_to_room(victim, ori_room);
    }

    if (ch==victim) {
      send_to_char("Sana ait bir yalancı görüntü önünde beliriyor!\n\r",ch);
      act("$s önünde kendisine ait bir yalancı görüntü beliriyor!",ch,NULL,victim,TO_ROOM);
    }
    else {
      act("$S önünde kendisine ait bir yalancı görüntü beliriyor!",ch,NULL,victim,TO_CHAR);
      act("$S önünde kendisine ait bir yalancı görüntü beliriyor!",ch,NULL,victim,TO_NOTVICT);
      send_to_char("Sana ait bir yalancı görüntü önünde beliriyor!\n\r",victim);
    }

  }
}

void spell_garble( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (ch == victim)
  {
    send_to_char("Kimin konuşmasını sürçeceksin?\n\r",ch);
    return;
  }

  if (is_affected(victim,sn))
  {
    act("$S zaten sürç-i lisan ediyor.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (is_safe_nomessage(ch,victim)) {
    send_to_char("Onun konuşmasını sürçemezsin.\n\r",ch);
      return;
  }

  if ((victim->level > ch->level + 7)
        || saves_spell((ch->level + 9), victim, DAM_MENTAL))
    return;

  af.where	= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 10;
  af.modifier  = 0;
  af.location  = 0;
  af.bitvector = 0;
  affect_to_char(victim,&af);

  act("$S konuşmasını sürçüyorsun!",ch,NULL,victim,TO_CHAR);
  send_to_char("Sürç-i lisan ediyorsun.\n\r",victim);
}

void spell_confuse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *rch, *pick = NULL;
  int count = 0;

  if (is_affected(victim,gsn_confuse)) {
    act("$N zaten şaşırmış.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (saves_spell(level,victim, DAM_MENTAL))
    return;

  char_affect( victim, TO_AFFECTS, sn, level, 10, 0, 0, 0 );

  /* odadaki görünür kişilerden rastgele biri (büyücü ve kurban dışında) */
  for ( rch = victim->in_room->people; rch != NULL; rch = rch->next_in_room )
  {
    if ( rch == ch || rch == victim
    ||   !can_see( victim, rch )
    ||   get_trust( victim ) < rch->invis_level )
	continue;
    if ( number_range( 1, ++count ) == 1 )
	pick = rch;
  }

  if (pick != NULL)   do_murder(victim,pick->name);
  do_murder(victim, ch->name);
}

void spell_terangreal( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_NPC(victim))
	return;

    af.where		= TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
      send_to_char( "Bir uyuşukluk dalgası tarafından ele geçiriliyorsun.\n\r",victim );
    	act( "$n derin bir uykuya daldı.", victim, NULL, NULL, TO_ROOM );
    	victim->position = POS_SLEEPING;
    }

    return;
}

void spell_kassandra( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    if ( spell_busy( ch, sn, "Kassandra bu iş için yeni kullanıldı.\n\r" ) )
	return;
    spell_set_cooldown( ch, sn, level, 5 );
    ch->hit = UMIN( ch->hit + 150, ch->max_hit );
    update_pos( ch );
    send_to_char( "Sıcak bir dalga vücudunu sarıyor.\n\r", ch );
    act("$n daha iyi görünüyor.", ch, NULL, NULL, TO_ROOM);
}


void spell_sebat( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  if ( spell_busy( ch, sn, "Sebat bu iş için yeni kullanıldı.\n\r" ) )
    return;
  char_affect( ch, TO_AFFECTS, sn, level, level, APPLY_AC, -30, 0 );
  act("$n mistik bir kalkanla çevrelendi.",ch, NULL,NULL,TO_ROOM );
  send_to_char( "Mistik bir kalkanla çevrelendin.\n\r", ch );
}


void spell_matandra( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( spell_busy( ch, sn, "Matandra bu iş için yeni kullanıldı.\n\r" ) )
    return;
  spell_set_cooldown( ch, sn, level, 5 );
  damage(ch,victim,dice(level, 7),sn,DAM_HOLY, TRUE);
}

void spell_amnesia( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  int i;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (IS_NPC(victim))
    return;

  for (i = 0; i < MAX_SKILL; i++)
    victim->pcdata->learned[i] /= 2;

  act("Anılarının akıp gittiğini hissediyorsun.",victim,NULL,NULL,TO_CHAR);
  act("$n boş gözlerle etrafa bakıyor.",victim,NULL,NULL,TO_ROOM);
}


void spell_chaos_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  /* seviye onluğuna göre zar tipi: 1-10 → 2, 11-20 → 3, ... 81+ → 12 */
  static const int chaos_blade_dice[] = { 2, 3, 4, 5, 6, 7, 9, 11, 12 };
  OBJ_DATA *blade;

  blade = create_object( get_obj_index(OBJ_VNUM_CHAOS_BLADE),level);
  send_to_char("Bir yatağan yaratıyorsun!\n\r",ch);
  act("$n bir yatağan yaratıyor!",ch,NULL,NULL,TO_ROOM);

  blade->timer = level * 2;
  blade->level = ch->level;
  blade->value[2] = chaos_blade_dice[UMIN( UMAX(ch->level - 1, 0) / 10, 8 )];

  obj_affect( blade, sn, level, APPLY_HITROLL, level / 6 );
  obj_affect( blade, sn, level, APPLY_DAMROLL, level / 6 );

  obj_to_char(blade,ch);
}

void spell_tattoo( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  OBJ_DATA *tattoo;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int i;

  if (IS_NPC(victim))
    {
      act("$N sana tapamayacak kadar aptal!", ch, NULL, victim, TO_CHAR);
      return;
    }

  for (i = 0; i < MAX_RELIGION; i++)
    {
      if (!str_cmp(ch->name, religion_table[i].name))
	{
  	  tattoo = get_eq_char(victim, WEAR_TATTOO);
   	  if (tattoo != NULL)
    	    {
            act("$N zaten dövmelenmiş!  Önce diğerini çıkarmalısın.",
		ch, NULL, victim, TO_CHAR);
      	      act("$n sana ikinci bir dövme vermeye çalıştı, ama başaramadı.",
		ch, NULL, victim, TO_VICT);
              act("$n $E ikinci bir dövme vermeye çalıştı, ama başaramadı.",
		ch, NULL, victim, TO_NOTVICT);
      	      return;
    	    }
  	  else
    	    {
      	      tattoo = create_object(get_obj_index(religion_table[i].vnum),60);
              act("$E $p dövmesi kazıyorsun!",ch, tattoo, victim, TO_CHAR);
      	      act("$n $E $p dövmesi kazıyor!",ch,tattoo,victim,TO_NOTVICT);
      	      act("$n sana $p dövmesi kazıyor!",ch,tattoo,victim,TO_VICT);

      	      obj_to_char(tattoo,victim);
      	      equip_char(victim, tattoo, WEAR_TATTOO);
	      return;
	    }
	}
    }
    send_to_char("Dini bir dövmen yok.\n\r", ch);
  return;
}

void spell_remove_tattoo( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  OBJ_DATA *tattoo;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  tattoo = get_eq_char(victim, WEAR_TATTOO);
  if (tattoo != NULL)
    {
      extract_obj(tattoo);
      act( "Acılı bir işlemle $n dövmeni kaldırıyor.",
	ch, NULL, victim, TO_VICT);
      act("$S dövmesini kaldırıyorsun.", ch, NULL, victim, TO_CHAR);
      act("$n $S dövmesini kaldırıyor.", ch, NULL, victim, TO_NOTVICT);
    }
  else
  act("$S hiç dövmesi yok.", ch, NULL, victim, TO_CHAR);
}


void spell_wrath( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  AFFECT_DATA af;

  if ( !IS_NPC(ch) && IS_EVIL(ch) )
    victim = ch;

  if ( IS_GOOD(victim) )
    {
      act( "Tanrılar $M koruyor.", ch, NULL, victim, TO_ROOM );
      return;
    }

  if ( IS_NEUTRAL(victim) )
    {
      act("$N etkilenmiş görünmüyor.", ch, NULL, victim, TO_CHAR );
      return;
    }

  dam = dice(level,12);

  if ( saves_spell( level, victim, DAM_HOLY ) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_HOLY, TRUE );

  if (IS_AFFECTED(victim, AFF_CURSE) || saves_spell( level, victim, DAM_HOLY ))
    return;

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 2*level;
  af.location  = APPLY_HITROLL;
  af.modifier  = -1 * (level / 8);
  af.bitvector = AFF_CURSE;
  affect_to_char( victim, &af );

  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = level / 8;
  affect_to_char( victim, &af );

  send_to_char( "Kirlenmiş hissediyorsun.\n\r", victim );
  if ( ch != victim )
    act("$N rahatsız görünüyor.",ch,NULL,victim,TO_CHAR);
   return;
}

void spell_stalker( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim;
  CHAR_DATA *stalker;
  int i;
  char log_buf[MAX_INPUT_LENGTH];

  if ( ( victim = get_char_world( ch, target_name ) ) == NULL
      ||   victim == ch || victim->in_room == NULL
      || IS_NPC(victim) || !IS_SET(victim->act, PLR_WANTED))
    {
      send_to_char("Başaramadın.\n\r", ch);
      return;
    }

  if ( spell_busy( ch, sn, "Bu güç yakın zamanda kullanıldı.\n\r" ) )
    return;

  if (!is_safe_nomessage(ch,victim) && !IS_SET(ch->act,PLR_CANINDUCT) )
    {
      send_to_char("Bu iş için özel muhafızları kullansan daha iyi sonuç verir.\n\r", ch);
      return;
    }

    send_to_char("Bir avcı çağırmaya çalışıyorsun.\n\r",ch);
    act("$n bir avcı çağırmaya çalışıyor.",ch,NULL,NULL,TO_ROOM);

  stalker = create_mobile( get_mob_index(MOB_VNUM_STALKER), NULL );

  spell_set_cooldown( ch, sn, level, 6 );

  for (i=0;i < MAX_STATS; i++)
    {
      stalker->perm_stat[i] = victim->perm_stat[i];
    }

  stalker->max_hit = UMIN(30000,2 * victim->max_hit);
  stalker->hit = stalker->max_hit;
  stalker->max_mana = victim->max_mana;
  stalker->mana = stalker->max_mana;
  stalker->level = victim->level;

  stalker->damage[DICE_NUMBER] =
		number_range(victim->level/8, victim->level/6);
  stalker->damage[DICE_TYPE] =
		number_range(victim->level/6, victim->level/5);
  stalker->damage[DICE_BONUS] =
		number_range(victim->level/10, victim->level/8);

  stalker->hitroll = victim->level;
  stalker->damroll = victim->level;
  stalker->size = victim->size;
  for (i=0; i < 3; i++)
    stalker->armor[i] = interpolate(stalker->level,100,-100);
  stalker->armor[3] = interpolate(stalker->level,100,0);
  stalker->silver = 0;
  stalker->invis_level = LEVEL_IMMORTAL;
  stalker->detection = DETECT_IMP_INVIS | DETECT_FADE | DETECT_EVIL | DETECT_INVIS
		     | DETECT_MAGIC | DETECT_HIDDEN | DETECT_GOOD | DETECT_SNEAK
		     | ACUTE_VISION;
  stalker->affected_by = AFF_SANCTUARY | AFF_INFRARED | AFF_PROTECT_EVIL
		       | AFF_PROTECT_GOOD | AFF_PASS_DOOR | AFF_HASTE
		       | AFF_BERSERK | AFF_REGENERATION;

  char_to_room(stalker,victim->in_room);
  stalker->last_fought = victim;
  send_to_char("Görünmez bir avcı seni avlamaya geldi!\n\r",victim);
  act("Görünmez bir avcı $m avlamaya geldi!",victim,NULL,NULL,TO_ROOM);
  send_to_char("Görünmez bir avcı gönderildi.\n\r", ch);

  snprintf(log_buf, sizeof(log_buf), "%s used stalker on %s", ch->name, victim->name);
  log_string(log_buf);
}


/* Zaman-mekân ışınlaması: kişiyi odaya taşı, mesaj, bak. */
static void gate_char_to( CHAR_DATA *wch, ROOM_INDEX_DATA *to )
{
    char_from_room( wch );
    char_to_room( wch, to );
    act( "$n aniden geliyor.", wch, NULL, NULL, TO_ROOM );
    if ( wch->in_room == NULL )
	bug( "Tesseract: char sent to NULL room", 0 );
    else
	do_look( wch, "auto" );
}

void spell_tesseract( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim;
  CHAR_DATA *wch;
  CHAR_DATA *wch_next;
  ROOM_INDEX_DATA *to;
  bool gate_pet;

  if( (victim = get_char_world(ch,target_name))==NULL
	|| victim == ch
	|| (to = victim->in_room) == NULL
	|| ch->in_room == NULL) {
	send_to_char("Başaramadın.\n\r",ch);
	return;
	}

  if ( !can_see_room(ch,to)
      ||   (is_safe(ch,victim) && IS_SET(victim->act,PLR_NOSUMMON))
      ||   room_is_private(to)
      ||   IS_SET(to->room_flags, ROOM_NO_RECALL | ROOM_NOSUMMON)
      ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL | ROOM_NOSUMMON)
      ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
      ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
      ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON) && is_safe_nomessage(ch,victim))
      ||   (IS_NPC(victim) && !IS_NPC(ch) && victim->pIndexData->vnum == ch->pcdata->questmob)
      ||   (saves_spell( level, victim, DAM_NONE ) ) )
    {
      send_to_char( "Başaramadın.\n\r", ch );
      return;
    }

  gate_pet = ch->pet != NULL && ch->in_room == ch->pet->in_room;

  for (wch = ch->in_room->people; wch != NULL; wch = wch_next)
    {
      wch_next = wch->next_in_room;
      if (is_same_group(wch, ch) && wch != ch)
	{
	  act( "$n garip sözcükler fısıldarken çevrende aksayan zamanı hissediyorsun.",ch, NULL, wch, TO_VICT);
	  gate_char_to( wch, to );
	}
    }

  act( "Bir ışık patlamasıyla $n ve arkadaşları beliriyor!",ch,NULL,NULL,TO_ROOM);
  send_to_char("Tuhaf sözcükleri fısıldadığın anda zaman ve mekan bulanıklaşarak duruluyor.\n\r",ch);
  gate_char_to( ch, to );

  if (gate_pet)
    {
      send_to_char( "Zaman ve mekanın kaydığını hissediyorsun.\n\r",ch->pet);
      gate_char_to( ch->pet, to );
    }
}

/* İksir mayalama: hammadde türüne göre iksir vnum'u ve büyü havuzu. */
struct brew_choice { const char *name; sh_int *gsn; int weight; };
static sh_int sn_fireball, sn_cure_disease, sn_word_of_recall, sn_frenzy,
	      sn_create_spring, sn_holy_word, sn_detect_magic, sn_detect_invis,
	      sn_acute_vision, sn_cure_blindness;

static const struct brew_choice brew_trash[] = {
    { "fireball",       &sn_fireball,       20 },
    { "cure poison",    &gsn_cure_poison,   20 },
    { "cure blindness", &sn_cure_blindness, 20 },
    { "cure disease",   &sn_cure_disease,   20 },
    { "word of recall", &sn_word_of_recall, 20 },
};
static const struct brew_choice brew_treasure[] = {
    { "cure critical",  &gsn_cure_critical, 1 },
    { "haste",          &gsn_haste,         1 },
    { "frenzy",         &sn_frenzy,         1 },
    { "create spring",  &sn_create_spring,  1 },
    { "holy word",      &sn_holy_word,      1 },
    { "invis",          &gsn_invis,         1 },
    { "cure light",     &gsn_cure_light,    1 },
    { "cure serious",   &gsn_cure_serious,  1 },
};
static const struct brew_choice brew_key[] = {
    { "detect magic",   &sn_detect_magic,   20 },
    { "detect invis",   &sn_detect_invis,   20 },
    { "pass door",      &gsn_pass_door,     25 },
    { "acute vision",   &sn_acute_vision,   35 },
};

/* Ağırlıklı rastgele seçim; büyü tabloda yoksa -1. */
static int brew_pick( const struct brew_choice *tab, int n )
{
    int total = 0, roll, i;

    for ( i = 0; i < n; i++ )
	total += tab[i].weight;
    roll = number_range( 0, total - 1 );
    for ( i = 0; i < n; i++ )
    {
	if ( roll < tab[i].weight )
	    return *tab[i].gsn > 0 ? *tab[i].gsn : cached_sn( tab[i].gsn, tab[i].name );
	roll -= tab[i].weight;
    }
    return -1;
}

void spell_brew( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    OBJ_DATA *potion;
    OBJ_DATA *vial;
    int spell, vnum;

    if (obj->item_type != ITEM_TRASH && obj->item_type != ITEM_TREASURE
	&& obj->item_type != ITEM_KEY)
      {
        send_to_char("O bir iksire dönüştürülemez.\n\r",ch);
	return;
      }

    if (obj->wear_loc != -1)
      {
        send_to_char("Mayalanacak şey taşınmalı.\n\r",ch);
	return;
      }

    for( vial=ch->carrying; vial != NULL; vial=vial->next_content )
      if ( vial->pIndexData->vnum == OBJ_VNUM_POTION_VIAL )
        break;
    if (  vial == NULL )  {
      send_to_char( "İksiri içine mayalayabileceğin bir iksir şişen yok.\n\r", ch );
	return;
    }

    if (number_percent() < 50)
      {
        send_to_char("Başaramadın ve onu yokettin.\n\r", ch);
	extract_obj(obj);
	return;
      }

    if (obj->item_type == ITEM_TRASH)
    {
	vnum = OBJ_VNUM_POTION_SILVER;
	spell = brew_pick( brew_trash, (int) (sizeof(brew_trash)/sizeof(brew_trash[0])) );
    }
    else if (obj->item_type == ITEM_TREASURE)
    {
	vnum = OBJ_VNUM_POTION_GOLDEN;
	spell = brew_pick( brew_treasure, (int) (sizeof(brew_treasure)/sizeof(brew_treasure[0])) );
    }
    else
    {
	vnum = OBJ_VNUM_POTION_SWIRLING;
	spell = brew_pick( brew_key, (int) (sizeof(brew_key)/sizeof(brew_key[0])) );
    }

    if ( spell < 0 )
    {
	bug( "spell_brew: iksir büyüsü beceri tablosunda yok", 0 );
	send_to_char( "Başaramadın.\n\r", ch );
	return;
    }

    potion = create_object( get_obj_index(vnum), level);
    potion->value[0] = level;
    potion->value[1] = spell;
    extract_obj(obj);
    act("Elindekilerle $p mayalıyorsun!", ch, potion, NULL, TO_CHAR);
    act("$n elindekilerle $p mayalıyor!", ch, potion, NULL, TO_ROOM);

    obj_to_char(potion, ch);
    extract_obj( vial );
}


void spell_shadowlife( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *shadow;
  AFFECT_DATA af;
  int i;

  if (IS_NPC(victim))
    {
      send_to_char( "Bunu neden yapmak istiyorsun?!?\n\r", ch);
      return;
    }

  if (is_affected(ch,sn))
    {
      send_to_char( "Gölgeye can verecek gücün yok.\n\r",ch);
      return;
    }

    act("$S gölgesine yaşam veriyorsun!",ch, NULL, victim, TO_CHAR);
    act("$n $S gölgesine yaşam veriyor!",ch,NULL,victim,TO_NOTVICT);
    act("$n gölgene yaşam veriyor!", ch, NULL, victim, TO_VICT);

  shadow = create_mobile( get_mob_index(MOB_VNUM_SHADOW), NULL );

  for (i=0;i < MAX_STATS; i++)
    {
      shadow->perm_stat[i] = ch->perm_stat[i];
    }

  shadow->max_hit = (3 * ch->max_hit) / 4;
  shadow->hit = shadow->max_hit;
  shadow->max_mana = (3 * ch->max_mana) / 4;
  shadow->mana = shadow->max_mana;
  shadow->alignment = ch->alignment;
  shadow->level = ch->level;
  for (i=0; i < 3; i++)
    shadow->armor[i] = interpolate(shadow->level,100,-100);
  shadow->armor[3] = interpolate(shadow->level,100,0);
  shadow->sex = victim->sex;
  shadow->silver = 0;

  personalize_mob( shadow, victim->name );

  char_to_room(shadow,ch->in_room);

  do_murder(shadow, victim->name);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

}

void spell_ruler_badge( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  OBJ_DATA *badge;

  if (count_worn(ch, WEAR_NECK) >= max_can_wear(ch, WEAR_NECK) )
  {
    send_to_char("Fakat sen başka birşey giyiyorsun.\n\r", ch );
    return;
  }

  remove_badges( ch, ch, "Sendeki $p yokoluyor.", NULL );

  badge = create_object( get_obj_index(OBJ_VNUM_RULER_BADGE),level);

  obj_affect( badge, sn, level, APPLY_HIT, 100 + level / 2 );
  obj_affect( badge, sn, level, APPLY_MANA, 100 + level / 2 );
  obj_affect( badge, sn, level, APPLY_HITROLL, level / 8 );
  obj_affect( badge, sn, level, APPLY_DAMROLL, level / 8 );

  badge->timer = 200;
  act("Hüküm nişanını giyiyorsun!",ch, NULL, NULL, TO_CHAR);
  act("$n hüküm nişanını giyiyor!", ch, NULL, NULL, TO_ROOM);

  obj_to_char(badge,ch);
  equip_char( ch, badge, WEAR_NECK );
  ch->hit = UMIN( (ch->hit + 100 + level/2), ch->max_hit);
  ch->mana = UMIN( (ch->mana + 100 + level/2), ch->max_mana);
}

void spell_remove_badge( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  remove_badges( ch, victim, "$p yokoluyor.", "$s $p yokoluyor." );
}

void spell_dragon_strength(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

  if (is_affected(ch, sn))
    {
      send_to_char("Zaten ejderhanın gücüyle dolusun.\n\r",ch);
      return;
    }

  af.where		= TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 3;
  af.bitvector = 0;

  af.modifier = 2;
  af.location = APPLY_HITROLL;
  affect_to_char(ch, &af);

  af.modifier = 2;
  af.location = APPLY_DAMROLL;
  affect_to_char(ch, &af);

  af.modifier = 10;
  af.location = APPLY_AC;
  affect_to_char(ch, &af);

  af.modifier = 2;
  af.location = APPLY_STR;
  affect_to_char(ch, &af);

  af.modifier = -2;
  af.location = APPLY_DEX;
  affect_to_char(ch, &af);

  send_to_char( "Ejderhanın gücü ruhuna katılıyor.\n\r", ch);
  act("$n biraz daha acımasız görünüyor.", ch, NULL, NULL, TO_ROOM);
}

void spell_dragon_breath(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(level , 6);
  if (!is_safe_spell(ch, victim, TRUE))
    {
      if (saves_spell(level, victim, DAM_FIRE))
	dam /= 2;
      damage(ch, victim, dam, sn, DAM_FIRE, TRUE);
    }
}

void spell_golden_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *vch;

  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (!is_same_group(vch, ch))
	continue;

      if ( is_affected( vch, sn ) || IS_AFFECTED(vch, AFF_PROTECT_EVIL))
	{
	  if (vch == ch)
	    send_to_char("Zaten altın bir aurayla korunuyorsun.\n\r",ch);
	  else
	    act( "$N zaten altın bir aurayla korunuyor.",ch,NULL,vch,TO_CHAR);
	  continue;
	}

      char_affect( vch, TO_AFFECTS, sn, level, 6 + level, APPLY_NONE, 0, AFF_PROTECT_EVIL );
      char_affect( vch, TO_AFFECTS, sn, level, 6 + level, APPLY_HITROLL, level/8, 0 );
      char_affect( vch, TO_AFFECTS, sn, level, 6 + level, APPLY_SAVING_SPELL, 0 - level/8, 0 );

      send_to_char( "Etrafını saran altın aylayı hissediyorsun.\n\r", vch );
      if ( ch != vch )
	act("Altın bir ayla $M çevreliyor.",ch,NULL,vch,TO_CHAR);
    }
}

void spell_dragonplate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  OBJ_DATA *plate;

  plate = create_object( get_obj_index(OBJ_VNUM_PLATE), level + 5);
  plate->timer = 2 * level;
  plate->cost  = 0;
  plate->level  = ch->level;

  obj_affect( plate, sn, level, APPLY_HITROLL, level / 8 );
  obj_affect( plate, sn, level, APPLY_DAMROLL, level / 8 );

  obj_to_char(plate, ch);

  act("$p yarattın!",ch,plate,NULL,TO_CHAR);
  act("$n $p yarattı!",ch,plate,NULL,TO_ROOM);
}

void spell_squire( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *squire;
  AFFECT_DATA af;
  int i;

  if (is_affected(ch,sn))
    {
      send_to_char( "Şu an başka bir ağayı yönetemezsin.\n\r",ch);
      return;
    }

    send_to_char("Bir ağa çağırmayı denedin.\n\r",ch);
    act("$n bir ağa çağırmayı denedi.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  gch->pIndexData->vnum == MOB_VNUM_SQUIRE)
	{
    send_to_char("İki ağa ihtiyaç duyduğundan daha fazla değil mi?!\n\r",ch);
	  return;
	}
    }

  if (count_charmed(ch)) return;

  squire = create_mobile( get_mob_index(MOB_VNUM_SQUIRE), NULL );

  for (i=0;i < MAX_STATS; i++)
    {
      squire->perm_stat[i] = ch->perm_stat[i];
    }

  squire->max_hit = ch->max_hit;
  squire->hit = squire->max_hit;
  squire->max_mana = ch->max_mana;
  squire->mana = squire->max_mana;
  squire->level = ch->level;
  for (i=0; i < 3; i++)
    squire->armor[i] = interpolate(squire->level,100,-100);
  squire->armor[3] = interpolate(squire->level,100,0);
  squire->silver = 0;

  personalize_mob( squire, ch->name );

  squire->damage[DICE_NUMBER] = number_range(level/15, level/12);
  squire->damage[DICE_TYPE] = number_range(level/3, level/2);
  squire->damage[DICE_BONUS] = number_range(level/8, level/6);

  char_to_room(squire,ch->in_room);
  send_to_char("Bir ağa hiçlikten geliyor!\n\r",ch);
  act("Bir ağa hiçlikten geliyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  SET_BIT(squire->affected_by, AFF_CHARM);
  squire->master = squire->leader = ch;

}


void spell_dragonsword( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  int sword_vnum;
  OBJ_DATA *sword;
  char arg[MAX_INPUT_LENGTH];

  target_name = one_argument(target_name, arg);
  sword_vnum = 0;

  if (!str_cmp(arg, "kılıç"))
    sword_vnum = OBJ_VNUM_DRAGONSWORD;
  else if (!str_cmp(arg, "topuz"))
    sword_vnum = OBJ_VNUM_DRAGONMACE;
  else if (!str_cmp(arg, "hançer"))
    sword_vnum = OBJ_VNUM_DRAGONDAGGER;
  else if (!str_cmp(arg, "kargı"))
    sword_vnum = OBJ_VNUM_DRAGONLANCE;
  else
    {
      send_to_char( "Ne tür bir silah yapmak istiyorsun? Kılıç, topuz, hançer, kargı?\n\r", ch);
      return;
    }

  sword = create_object( get_obj_index(sword_vnum), level);
  sword->timer = level * 2;
  sword->cost  = 0;
  sword->value[2] = conjured_weapon_dice( ch->level );
  sword->level = ch->level;

  obj_affect( sword, sn, level, APPLY_HITROLL, level / 5 );
  obj_affect( sword, sn, level, APPLY_DAMROLL, level / 5 );

  if (IS_GOOD(ch))
	 SET_BIT(sword->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL));
  else if (IS_NEUTRAL(ch))
	 SET_BIT(sword->extra_flags,(ITEM_ANTI_GOOD | ITEM_ANTI_EVIL));
  else if (IS_EVIL(ch))
	 SET_BIT(sword->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD));
  obj_to_char(sword, ch);

  act("$p yarattın!",ch,sword,NULL,TO_CHAR);
  act("$n $p yarattı!",ch,sword,NULL,TO_ROOM);
}

void spell_entangle( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (ch->in_room->sector_type == SECT_INSIDE ||
      ch->in_room->sector_type == SECT_CITY ||
      ch->in_room->sector_type == SECT_DESERT ||
      ch->in_room->sector_type == SECT_AIR)
    {
      send_to_char("Burada bitki büyüyemez.\n\r", ch);
      return;
    }

  damage(ch,victim,ch->level,gsn_entangle,DAM_PIERCE, TRUE);

  act("Yerden fırlayan sarmaşıklar $s bacaklarına dolanıyor!", victim,
      NULL, NULL, TO_ROOM);
  act("Yerden fırlayan sarmaşıklar ayaklarına dolanıyor!", victim,
      NULL, NULL, TO_CHAR);

  victim->move -= dice(level, 6);
  victim->move = UMAX(0, victim->move);

  if (!is_affected(victim,gsn_entangle))
    char_affect( victim, TO_AFFECTS, gsn_entangle, level, level / 10,
		 APPLY_DEX, - (level / 10), 0 );
}

void spell_holy_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  if ( spell_busy( ch, sn, "Zaten zarara karşı korunuyorsun.\n\r" ) )
    return;

  char_affect( ch, TO_AFFECTS, sn, level, level, APPLY_AC,
	       - UMAX(10, 10 * (level / 5) ), 0 );
  act("$n zarardan korunmaya başladı.",ch,NULL,NULL,TO_ROOM );
  send_to_char("Zarara karşı korunmaya başladın.\n\r", ch );
}

void spell_love_potion( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 50;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  send_to_char("Çevredekileri izlemeyi seviyorsun.\n\r", ch);
}

void spell_protective_shield( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    {
      if (victim == ch)
        send_to_char( "Zaten koruyucu bir kalkanla çevrilisin.\n\r",ch);
      else
        act("$N zaten koruyucu bir kalkanla çevrili.",ch,NULL,
	    victim,TO_CHAR);
      return;
    }
  af.where	= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 30 ) + 3;
  af.location  = APPLY_AC;
  af.modifier  = 20;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act( "$n koruyucu bir kalkanla çevrelendi.",victim,NULL,NULL,TO_ROOM );
  send_to_char("Koruyucu bir kalkanla çevrelendin.\n\r", victim );
  return;
}

void spell_deafen( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (ch == victim) {
    send_to_char("Kimi sağır edeceksin?\n\r",ch);
    return;
  }

  if (is_affected(victim,sn)) {
    act("$N zaten sağır.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (is_safe_nomessage(ch,victim)) {
    send_to_char("Onu sağır edemezsin.\n\r",ch);
      return;
  }

  if (saves_spell(level,victim, DAM_NONE))
    return;

  af.where		= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 10;
  af.modifier  = 0;
  af.location  = 0;
  af.bitvector = 0;
  affect_to_char(victim,&af);

  act("$M sağır ettin!",ch,NULL,victim,TO_CHAR);
  send_to_char( "Gürültülü bir çınlama kulaklarını dolduruyor...artık duyamıyorsun!\n\r",victim);
}

/* Dağıtma için rastgele uygun oda; sınırlı deneme, bulunamazsa NULL. */
static ROOM_INDEX_DATA *disperse_room( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *room;
    int tries;

    for ( tries = 0; tries < 5000; tries++ )
    {
	room = get_room_index( number_range( 0, 65535 ) );
	if ( room != NULL
	&&   can_see_room( ch, room )
	&&   !room_is_private( room )
	&&   !IS_SET(room->room_flags, ROOM_NO_RECALL) )
	    return room;
    }
    return NULL;
}

void spell_disperse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  ROOM_INDEX_DATA *pRoomIndex;

  if ( spell_busy( ch, sn, "Yeterli dağıtma gücün yok.\n\r" ) )
    return;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;

      if ( vch->in_room != NULL
      &&   !IS_SET(vch->in_room->room_flags, ROOM_NO_RECALL)
      &&   !IS_IMMORTAL(vch)
      && (( IS_NPC(vch) && !IS_SET(vch->act, ACT_AGGRESSIVE) ) ||
      (!IS_NPC(vch) && vch->level >= KIDEMLI_OYUNCU_SEVIYESI && (
	!is_safe_nomessage(ch, vch)))) && vch != ch
      && !IS_SET(vch->imm_flags, IMM_SUMMON))
	{
	  if ( (pRoomIndex = disperse_room( ch )) == NULL )
	    continue;

	  send_to_char("Dünya çevrende dönüyor!\n\r",vch);
	  act( "$n yokoluyor!", vch, NULL, NULL, TO_ROOM );
	  char_from_room( vch );
	  char_to_room( vch, pRoomIndex );
	  act( "$n yavaşça beliriyor.", vch, NULL, NULL, TO_ROOM );
	  do_look( vch, "auto" );
	}
    }
  spell_set_cooldown( ch, sn, level, 10 );
}


void spell_acute_vision( int sn, int level, CHAR_DATA *ch, void *vo, int target )
 {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( CAN_DETECT(victim, ACUTE_VISION) )
    {
        if (victim == ch)
            send_to_char("Görüşün yeterince güçlü. \n\r",ch);
        else
            act("$S görüşü zaten güçlü.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where		= TO_DETECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ACUTE_VISION;
    affect_to_char( victim, &af );
    send_to_char("Görüşün keskinleşiyor.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Tamam.\n\r", ch );
    return;
}

void spell_dragons_breath(int sn,int level,CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;

    act("Ejderha lordunu yardıma çağırıyorsun.",ch,NULL,NULL,TO_CHAR);
    act("$n bir ejderha gibi solumaya başlıyor.",ch,NULL,victim,TO_NOTVICT);
    act("$s solumasından rahatsız oluyorsun!",ch,NULL,victim,TO_VICT);
    act("Ejderha lordunun nefesini çekiyorsun.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,30);

    dam = UMAX(hp_dam + dice_dam / 5, dice_dam + hp_dam / 5);

switch( dice(1,5) )
 {
	 case 1:
    fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch)
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;
	if ( is_safe(ch, vch) )
          continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_FIRE,TRUE);
	    }
	}
	else /* partial damage */
	{
	    if (saves_spell(level - 2,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	}
    }
    break;
    case 2:
    if (saves_spell(level,victim,DAM_ACID))
    {
	acid_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage(ch,victim,dam/2,sn,DAM_ACID,TRUE);
    }
    else
    {
	acid_effect(victim,level,dam,TARGET_CHAR);
	damage(ch,victim,dam,sn,DAM_ACID,TRUE);
    }
    break;
    case 3:
    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch)
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;
	if ( is_safe(ch, vch) )
          continue;


	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_COLD))
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
	else
	{
	    if (saves_spell(level - 2,vch,DAM_COLD))
	    {
		cold_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	}
    }
    break;
    case 4:
    poison_effect(ch->in_room,level,dam,TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(ch) && IS_NPC(vch)
	&&   (ch->fighting == vch || vch->fighting == ch)))
	    continue;
	if ( is_safe(ch, vch) )
          continue;
        if (!IS_NPC(ch) && vch != ch &&
            ch->fighting != vch && vch->fighting != ch &&
            (IS_SET(vch->affected_by,AFF_CHARM) || !IS_NPC(vch)))
          {
          victim_yell( vch, ch, "İmdat biri bana saldırıyor!",
          	     "Geber %s, seni büyücü köpek!" );
        }

	if (saves_spell(level,vch,DAM_POISON))
	{
	    poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	    damage(ch,vch,dam/2,sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch,level,dam,TARGET_CHAR);
	    damage(ch,vch,dam,sn,DAM_POISON,TRUE);
	}
    }
    break;
    case 5:
    if (saves_spell(level,victim,DAM_LIGHTNING))
    {
	shock_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage(ch,victim,dam/2,sn,DAM_LIGHTNING,TRUE);
    }
    else
    {
	shock_effect(victim,level,dam,TARGET_CHAR);
	damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE);
    }
    break;
  }
}

void spell_sand_storm( int sn, int level, CHAR_DATA *ch, void *vo,int target )
 {
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;

    if ( (ch->in_room->sector_type == SECT_AIR)
	|| (ch->in_room->sector_type == SECT_WATER_SWIM)
	|| (ch->in_room->sector_type == SECT_WATER_NOSWIM) )
	{
    send_to_char( "Hiç kum bulamıyorsun.\n\r",ch);
	 ch->wait = 0;
	 return;
	}

  act("$n yerdeki kumla fırtına yaratıyor.",ch,NULL,NULL,TO_ROOM);
  act("Şey yaratıyorsun...kum fırtınası.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
    sand_effect(ch->in_room,level,dam/2,TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch)
	&&   (ch->fighting != vch /*|| vch->fighting != ch*/)))
	    continue;
	if ( is_safe(ch, vch) )
          continue;

	    if (saves_spell(level,vch,DAM_COLD))
	    {
		sand_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		sand_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
    }
}

void spell_scream( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch, *vch_next;
    int dam;

    act("$n rahatsız edici bir sesle ÇIĞLIK atıyor!.",ch,NULL,NULL,TO_ROOM);
    act("Güçlü bir sesle çığlık atıyorsun.",ch,NULL,NULL,TO_CHAR);

    dam = breath_dam( ch, level, 20, 10 );

    scream_effect(ch->in_room,level,dam/2,TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) || is_safe(ch, vch))
	    continue;

	if (saves_spell(level,vch,DAM_ENERGY))
	{
	    WAIT_STATE(vch, PULSE_VIOLENCE );
	    scream_effect(vch,level/2,dam/4,TARGET_CHAR);
	}
	else
	{
	    WAIT_STATE(vch, ( skill_table[sn].beats + PULSE_VIOLENCE ));
	    scream_effect(vch,level,dam,TARGET_CHAR);
	    if (vch->fighting)  stop_fighting( vch , TRUE );
	}
    }
}

void spell_attract_other( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if  ( ch->sex == victim->sex )
    {
      send_to_char("Belki de şansını karşı cinste denemelisin!\n\r", ch );
	return;
    }
    spell_charm_person(sn,level,ch,vo,target);
    return;
}






void spell_animate_dead(int sn,int level, CHAR_DATA *ch, void *vo,int target )
 {
    CHAR_DATA *victim;
    CHAR_DATA *undead;
    OBJ_DATA *obj,*obj2,*next;
    AFFECT_DATA af;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char *argument,*arg;
    int i;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

      if (!(obj->item_type == ITEM_CORPSE_NPC ||
          obj->item_type == ITEM_CORPSE_PC))
    {
      send_to_char("Yalnız cesetleri canlandırabilirsin!\n\r",ch);
	return;
    }
/*
     if (obj->item_type == ITEM_CORPSE_PC)
	{
	send_to_char("The magic fails abruptly!\n\r",ch);
	return;
	}
*/
  if (is_affected(ch, sn))
    {
      send_to_char("Daha fazla hortlak bedenini kontrol edecek gücü bulamazsın.\n\r", ch);
      return;
    }

  if (count_charmed(ch)) return;

  if ( ch->in_room != NULL && IS_SET(ch->in_room->room_flags, ROOM_NO_MOB) )
  {
    send_to_char("Ölüleri burada canlandıramazsın.\n\r", ch );
     return;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)      ||
       IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)   ||
       IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)  )
  {
    send_to_char( "Burada canlandıramazsın.\n\r", ch );
    return;
  }

  undead = create_mobile( get_mob_index(MOB_VNUM_UNDEAD), NULL );
  char_to_room(undead,ch->in_room);
  for (i=0;i < MAX_STATS; i++)
    {
      undead->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);
    }

  undead->max_hit = IS_NPC(ch)? ch->max_hit : ch->pcdata->perm_hit;
  undead->hit = undead->max_hit;
  undead->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  undead->mana = undead->max_mana;
  undead->alignment = ch->alignment;
  undead->level = UMIN(100,(ch->level-2));

  for (i=0; i < 3; i++)
    undead->armor[i] = interpolate(undead->level,100,-100);
  undead->armor[3] = interpolate(undead->level,50,-200);
  undead->damage[DICE_NUMBER] = number_range(level/20,level/15);
  undead->damage[DICE_TYPE] = number_range(level/6, level/3);
  undead->damage[DICE_BONUS] = number_range(level/12, level/10);
  undead->sex = ch->sex;
  undead->silver = 0;

  SET_BIT(undead->act, ACT_UNDEAD);
  SET_BIT(undead->affected_by, AFF_CHARM);
  undead->master = ch;
  undead->leader = ch;

  snprintf(buf, sizeof(buf), "%s beden hortlak", obj->name);
  undead->name = str_dup(buf);
  snprintf(buf2, sizeof(buf2),"%s",obj->short_descr);
  argument = (char*)alloc_perm ( MAX_STRING_LENGTH );
  arg = (char*)alloc_perm ( MAX_STRING_LENGTH );
  argument = buf2;
  buf3[0] = '\0';
  while (argument[0] != '\0' )
	{
  argument = one_argument(argument, arg);
  if (!(!str_cmp(arg,"hortlak") || !str_cmp(arg,"beden") ||
	!str_cmp(arg,"ceset") ))
	 {
	  if (buf3[0] == '\0')   strcat(buf3,arg);
	  else  {
		 strcat(buf3," ");
		 strcat(buf3,arg);
		}
	 }
	}
  snprintf(buf, sizeof(buf), "%s'in hortlak bedeni", buf3);
  undead->short_descr = str_dup(buf);
  snprintf(buf, sizeof(buf), "%s'in hortlak bedeni yürürken sendeliyor.\n\r", buf3);
  undead->long_descr = str_dup(buf);

  for(obj2 = obj->contains;obj2;obj2=next)
    {
	next = obj2->next_content;
	obj_from_obj(obj2);
	obj_to_char(obj2, undead);
    }
    interpret( undead,"giy tümü", TRUE);

  af.where	 = TO_AFFECTS;
  af.type      = sn;
  af.level	 = ch->level;
  af.duration  = (ch->level / 10);
  af.modifier  = 0;
  af.bitvector = 0;
  af.location  = APPLY_NONE;
  affect_to_char( ch, &af );

  send_to_char("Mistik bir güçle onu canlandırıyorsun!\n\r",ch);
  snprintf(buf, sizeof(buf),"Mistik bir güçle %s %s'i canlandırıyor!",ch->name,obj->name);
  act(buf,ch,NULL,NULL,TO_ROOM);
  snprintf(buf, sizeof(buf),"%s sana bakarken onu rahatsız etmenin bedelini ödetmeyi planlıyor!",obj->short_descr);
  act(buf,ch,NULL,NULL,TO_CHAR);
  extract_obj (obj);
	return;
    }

    victim = (CHAR_DATA *) vo;

    if ( victim == ch )
    {
      send_to_char( "Fakat sen ölü değilsin!!\n\r", ch );
    	return;
        }

        send_to_char("Fakat o ölü değil!!\n\r", ch );
    return;
}



void spell_enhanced_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
  send_to_char("Zaten gelişmiş zırhla kaplısın.\n\r",ch);
else
  act("$N zaten gelişmiş zırhla kaplı.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.modifier  = -60;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char("Her tür saldırıya karşı daha iyi korunuyorsun.\n\r", victim );
    if ( ch != victim )
	act("$N senin büyünle korunuyor.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_meld_into_stone( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    {
      if (victim == ch)
        send_to_char("Zaten olabildiğince taşa dönüşmüşsün.\n\r",ch);
      else
        act("$S zaten olabildiğince taşa dönüşmüş.",ch,NULL,
	    victim,TO_CHAR);
      return;
    }
  af.where	= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 10;
  af.location  = APPLY_AC;
  af.modifier  = -100;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act( "$s taşa dönüşüyor.",victim,NULL,NULL,TO_ROOM );
  send_to_char( "Taşa dönüşüyorsun.\n\r", victim );
  return;
}

void spell_web( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

     if (saves_spell (level, victim,DAM_OTHER) ) return;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
  send_to_char("Zaten ağlısın.\n\r",ch);
else
  act("$N zaten ağlı.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = 1;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * ( level / 6);
    af.where	 = TO_DETECTS;
    af.bitvector = ADET_WEB;
    affect_to_char( victim, &af );

    af.location  = APPLY_DEX;
    af.modifier  = -2;
    affect_to_char( victim, &af );

    af.location  = APPLY_DAMROLL;
    af.modifier  = -1 * ( level / 6);
    affect_to_char( victim, &af );
    send_to_char( "Kalın ağlarla tutulmuşsun!\n\r", victim );
    if ( ch != victim )
	act("$M kalın ağlarla tutuyorsun!",ch,NULL,victim,TO_CHAR);
    return;
}


/*
 * Grup buff'ı: odadaki grup üyelerine tek etki. Zaten etkili olana
 * dup mesajı, uygulanana self/other mesajı.
 */
static void group_buff( CHAR_DATA *ch, int sn, int level, int duration,
			int location, int modifier, int bits, bool check_bit,
			const char *dup_self, const char *dup_other,
			const char *msg_self, const char *msg_other )
{
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) )
	    continue;
	if ( check_bit ? IS_AFFECTED( gch, bits ) : is_affected( gch, sn ) )
	{
	    if ( gch == ch )
		send_to_char( dup_self, ch );
	    else
		act( dup_other, ch, NULL, gch, TO_CHAR );
	    continue;
	}
	char_affect( gch, TO_AFFECTS, sn, level, duration, location, modifier, bits );
	send_to_char( msg_self, gch );
	if ( ch != gch )
	    act( msg_other, ch, NULL, gch, TO_CHAR );
    }
}

void spell_group_defense( int sn, int level, CHAR_DATA *ch, void *vo ,int target)
{
    static sh_int sn_armor;
    int armor_sn = cached_sn( &sn_armor, "armor" );

    group_buff( ch, armor_sn, level, level, APPLY_AC, -20, 0, FALSE,
		"Zaten zırhlısın.\n\r", "$N zaten zırhlı.",
		"Birinin seni koruduğunu hissediyorsun.\n\r",
		"$N senin büyünle korunuyor." );
    group_buff( ch, gsn_shield, level, level, APPLY_AC, -20, 0, FALSE,
		"Zaten kalkanlısın.\n\r", "$N zaten kalkanlı.",
		"Bir güç kalkanıyla çevrelendin.\n\r",
		"$N bir güç kalkanıyla çevrelendi." );
}


void spell_inspire( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *gch;

    for( gch=ch->in_room->people; gch != NULL; gch=gch->next_in_room )
    {
	if( !is_same_group( gch, ch) )
	    continue;
	if ( is_affected(gch, gsn_bless ) )
	{
	    if(gch == ch)
		send_to_char("Zaten ilhamlısın.\n\r",ch);
	    else
		act("$N zaten ilhamlı.",ch, NULL, gch, TO_CHAR);
	    continue;
	}
	char_affect( gch, TO_AFFECTS, gsn_bless, level, 6 + level, APPLY_HITROLL, level/12, 0 );
	char_affect( gch, TO_AFFECTS, gsn_bless, level, 6 + level, APPLY_SAVING_SPELL, 0 - level/12, 0 );

	send_to_char( "Ruhuna işleyen ilhamı hissediyorsun!\n\r", gch );
	if( ch != gch )
	    act( "$M yaratıcının gücüyle uyandırıyorsun!", ch, NULL, gch, TO_CHAR );
    }
}


void spell_mass_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    group_buff( ch, gsn_sanctuary, level, number_fuzzy( level/6 ), APPLY_NONE, 0,
		AFF_SANCTUARY, TRUE,
		"Zaten takdislisin.\n\r", "$N zaten takdisli.",
		"Beyaz bir aurayla çevrelendin.\n\r",
		"$N beyaz bir aurayla çevrelendi." );
}

void spell_mend( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int result,skill;

    if ( obj->condition > 99 )
    {
      send_to_char("Onu onarmaya gerek yok.\n\r",ch);
    return;
    }

    if (obj->wear_loc != -1)
    {
      send_to_char("Onarılacak şey taşınıyor olmalı.\n\r",ch);
	return;
    }

    skill = get_skill(ch,gsn_mend) / 2;
    result = number_percent ( ) + skill;

    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	  result -= 5;
    if (IS_OBJ_STAT(obj,ITEM_MAGIC))
	  result += 5;

    if (result >= 50)
    {
      act("$p parlamaya başlıyor.  İyi işti!",ch,obj,NULL,TO_CHAR);
    	act("$p parlamaya başlıyor.",ch,obj,NULL,TO_ROOM);
	obj->condition += result;
	obj->condition = UMIN( obj->condition , 100 );
	return;
    }

    else if ( result >=10)
    {
      send_to_char("Değişen bir şey olmadı.\n\r",ch);
	return;
    }

    else
    {
      act("$p alev alarak buharlaşıyor!",ch,obj,NULL,TO_CHAR);
    	act("$p alev alarak buharlaşıyor!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }
}

void spell_shielding( int sn, int level, CHAR_DATA *ch, void *vo ,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af = {0};

    if ( saves_spell( level, victim ,DAM_NONE) )
	{
	act("$N bir an için titriyor.", ch, NULL, victim, TO_CHAR );
	send_to_char("Bir an için titriyorsun.\n\r",victim);
	return;
	}

    if (is_affected(victim, sn) )
	{
	char_affect( victim, TO_AFFECTS, sn, level, level / 20, APPLY_NONE, 0, 0 );
	act("$M ruh akıntılarıyla örtüyorsun.", ch, NULL, victim, TO_CHAR );
	send_to_char("Kalkanlamanın arttığını hissediyorsun.\n\r",victim);
	return;
	}

    af.where    = TO_AFFECTS;
    af.type	= sn;
    af.level    = level;
    af.duration = level / 15;
    af.location = APPLY_NONE;
    af.modifier	= 0;
    af.bitvector = 0;
    affect_join( victim, &af );

    send_to_char( "Birşeylerle bağlantını yitirdiğini hissediyorsun.\n\r",victim );
    act("$M Gerçek Kaynak'tan ayırıyorsun.", ch, NULL, victim, TO_CHAR);
}


void spell_link ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int tmpmana;

    if ( victim == ch )
    {
	send_to_char( "Kendine bağlanamazsın.\n\r", ch );
	return;
    }

    tmpmana = ch->mana / 2;
    ch->mana = 0;
    ch->endur /= 2;
    tmpmana = ( tmpmana + number_percent() ) / 2;
    victim->mana = UMIN( victim->max_mana, victim->mana + tmpmana );
}

void spell_power_kill ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  act_color( "Parmağından çıkan karanlık $M sarıyor.",
  ch, NULL, victim, TO_CHAR, POS_RESTING, CLR_RED );
  act_color( "$s parmağından çıkan karanlık $M sarıyor.",
  ch, NULL, victim, TO_NOTVICT, POS_RESTING, CLR_RED );
  act_color( "$S parmağından çıkan karanlık seni sarıyor.",
		victim, NULL, ch, TO_CHAR, POS_RESTING, CLR_RED );

  instant_death_spell( sn, level, ch, victim, NULL, NULL, NULL,
    "Ö L D Ü R Ü L D Ü N!\n\r",
    "$N öldürüldü!" );
}

void spell_lion_help ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
  CHAR_DATA *lion;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int i;

  target_name = one_argument(target_name, arg);
  if (arg[0] == '\0')
	{
    send_to_char("Kimin öldürülmesini istiyorsun?\n\r",ch);
	 return;
	}

  if ( (victim = get_char_area(ch,arg)) == NULL)
	{
    send_to_char("Etrafta bu isimde biri yok.\n\r",ch);
	 return;
	}
  if (is_safe_nomessage(ch,victim))
	{
    send_to_char("Tanrılar kurbanını koruyor.\n\r",ch);
	 return;
	}

  send_to_char("Bir avcı aslan çağırıyorsun.\n\r",ch);
  act("$n bir avcı aslan çağırıyor.",ch,NULL,NULL,TO_ROOM);

  if ( spell_busy( ch, sn, "Daha fazla aslanı kontrol edecek gücün yok.\n\r" ) )
    return;

  if ( IS_SET(ch->in_room->room_flags, ROOM_NO_MOB) )
  {
    send_to_char( "Hiçbir aslan seni dinlemiyor.\n\r", ch );
     return;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_PRIVATE | ROOM_SOLITARY)
  ||   !room_has_any_exit( ch->in_room )
  ||   ( ch->in_room->sector_type != SECT_FIELD &&
         ch->in_room->sector_type != SECT_FOREST &&
         ch->in_room->sector_type != SECT_MOUNTAIN &&
         ch->in_room->sector_type != SECT_HILLS ) )
  {
    send_to_char("Hiçbir avcı aslan sana gelemez.\n\r", ch );
    return;
  }

  lion = create_mobile( get_mob_index(MOB_VNUM_HUNTER), NULL );

  for (i=0;i < MAX_STATS; i++)
    {
      lion->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);
    }

  lion->max_hit =  UMIN(30000, ch->max_hit * 6 / 5);
  lion->hit = lion->max_hit;
  lion->max_mana = ch->max_mana;
  lion->mana = lion->max_mana;
  lion->alignment = ch->alignment;
  lion->level = UMIN(100,ch->level);
  for (i=0; i < 3; i++)
    lion->armor[i] = interpolate(lion->level,100,-100);
  lion->armor[3] = interpolate(lion->level,100,0);
  lion->sex = ch->sex;
  lion->silver = 0;
  lion->damage[DICE_NUMBER] = number_range(level/15, level/10);
  lion->damage[DICE_TYPE] = number_range(level/3, level/2);
  lion->damage[DICE_BONUS] = number_range(level/8, level/6);
  lion->detection = DETECT_IMP_INVIS | DETECT_FADE | DETECT_EVIL | DETECT_INVIS
		  | DETECT_MAGIC | DETECT_HIDDEN | DETECT_GOOD | DETECT_SNEAK
		  | ACUTE_VISION;

  char_to_room(lion,ch->in_room);

  send_to_char("Bir avcı aslan kurbanını öldürmeye geldi!\n\r",ch);
  act("Bir avcı aslan $s kurbanını öldürmeye geldi!",ch,NULL,NULL,TO_ROOM);

  spell_set_cooldown( ch, sn, ch->level, 24 );
  SET_BIT(lion->act,ACT_HUNTER);
  lion->hunting = victim;
  hunt_victim(lion);
}


void spell_magic_jar ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *vial;
    OBJ_DATA *jar;

    if (victim == ch)
	{
    send_to_char("Kendini daha fazla seviyorsun.\n\r",ch);
	return;
	}

    if (IS_NPC(victim))
	{
	send_to_char("Kurbanın bir oyuncu! Buna gerek yok.\n\r",ch);
	return;
	}

    if (saves_spell(level ,victim,DAM_MENTAL))
       {
        send_to_char("Başaramadın.\n\r",ch);
        return;
       }

    for( vial=ch->carrying; vial != NULL; vial=vial->next_content )
      if ( vial->pIndexData->vnum == OBJ_VNUM_POTION_VIAL )
        break;

    if (  vial == NULL )  {
      send_to_char( "İçine kurbanının ruhunu koyabileceğin bir şişen yok.\n\r", ch );
	return;
    }
    extract_obj(vial);

    jar = create_owned_obj( ch, OBJ_VNUM_MAGIC_JAR );
    personalize_str( &jar->name, victim->name );
    personalize_str( &jar->short_descr, victim->name );
    personalize_str( &jar->description, victim->name );
    personalize_extra( jar, victim->name );
    jar->timer = ch->level;
    obj_to_char( jar, ch );

    SET_BIT(victim->act,PLR_NO_EXP);
    printf_to_char( ch, "%s'in ruhunu yakalayıp şişenin içine koyuyorsun.\n\r", victim->name );
}

/* Ölümden sonra bu kadar saniye içinde kaçırma uygulanmaz. */
#define TURN_DEATH_GRACE_SECS 10

void turn_spell ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, align;

    if (victim != ch)
    {
	act("$n elini kaldırıyor ve kör edici bir ışık huzmesi fışkırıyor!",
    	    ch,NULL,NULL,TO_ROOM);
	send_to_char("Elini kaldırıyorsun ve kör edici bir ışık huzmesi fışkırıyor!\n\r",ch);
    }

    if (IS_GOOD(victim) || IS_NEUTRAL(victim))
    {
	act("$n ışıktan etkilenmemiş görünüyor.",victim,NULL,victim,TO_ROOM);
	send_to_char("Işık seni etkileyemiyor.\n\r",victim);
	return;
    }

    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;

    align = victim->alignment;
    align -= 350;

    if (align < -1000)
	align = -1000 + (align + 1000) / 3;

    dam = (dam * align * align) / 1000000;

    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);

    /* cabal guardians */
    if (IS_NPC(victim) && victim->cabal != CABAL_NONE)
	return;

    /* kaçış: do_flee (fight.c) mantığının kopyası; orada ortak yardımcı olursa buraya bağlanır */
    if ( (IS_NPC(victim) && victim->position != POS_DEAD) ||
	(!IS_NPC(victim)
	 && (long long)(current_time - victim->last_death_time) > TURN_DEATH_GRACE_SECS))
    {
     ROOM_INDEX_DATA *was_in;
     ROOM_INDEX_DATA *now_in;
     int door;

     was_in = victim->in_room;
     for ( door = 0; door < 6; door++ )
     {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( victim, door, FALSE );
	if ( ( now_in = victim->in_room ) == was_in )
	    continue;

	victim->in_room = was_in;
	act( "$n kaçtı!", victim, NULL, NULL, TO_ROOM );
	victim->in_room = now_in;

	if (IS_NPC(victim))  victim->last_fought = NULL;

	stop_fighting( victim, TRUE );
	return;
     }
    }
}

void spell_turn ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if ( spell_busy( ch, sn, "Bu güç yakın zamanda kullanıldı.\n\r" ) )
	return;
    spell_set_cooldown( ch, sn, level, 5 );

    if (IS_EVIL(ch) )
    {
      send_to_char("Enerji içinde patlıyor!\n\r",ch);
	turn_spell(sn,ch->level,ch,ch,target);
	return;
    }

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) || is_safe(ch, vch))
	    continue;
	turn_spell(sn,ch->level,ch,vch,target);
    }
}


void spell_fear ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ((victim->iclass == CLASS_SAMURAI) && (victim->level >=10) )
	{
    send_to_char( "Kurbanın bu gücün ötesinde.\n\r", ch );
	 return;
	}

    if (is_affected(victim,gsn_fear ) || saves_spell( level, victim,DAM_OTHER) )
	return;

    char_affect( victim, TO_DETECTS, gsn_fear, level, level / 10, 0, 0, ADET_FEAR );
    send_to_char( "Bir tavşan kadar korkuyorsun.\n\r", victim );
    act("$n korkan gözlerle bakıyor.",victim,NULL,NULL,TO_ROOM);
}

/*
 * Sıcak/soğuk koruması: üç geçici koruma (sıcak, soğuk, ateş kalkanı)
 * birbirini dışlar; hangisi varsa ona göre mesaj verir.
 */
static bool has_temp_protection( CHAR_DATA *ch, CHAR_DATA *victim )
{
    static const struct { sh_int *gsn; const char *self; const char *other; } prot[] = {
	{ &gsn_protection_heat, "Zaten sıcaktan korunuyorsun.\n\r",     "$N zaten sıcaktan korunuyor." },
	{ &gsn_protection_cold, "Zaten soğuktan korunuyorsun.\n\r",     "$N zaten soğuktan korunuyor." },
	{ &gsn_fire_shield,     "Zaten ateş kalkanı kullanıyorsun.\n\r", "$N zaten ateş kalkanı kullanıyor." },
    };
    int i;

    for ( i = 0; i < 3; i++ )
    {
	if ( !is_affected( victim, *prot[i].gsn ) )
	    continue;
	if ( victim == ch )
	    send_to_char( prot[i].self, ch );
	else
	    act( prot[i].other, ch, NULL, victim, TO_CHAR );
	return TRUE;
    }
    return FALSE;
}

static void protection_spell( CHAR_DATA *ch, CHAR_DATA *victim, int gsn, int level,
			      const char *msg_self, const char *msg_other )
{
    if ( has_temp_protection( ch, victim ) )
	return;
    char_affect( victim, TO_AFFECTS, gsn, level, 24, APPLY_SAVING_SPELL, -1, 0 );
    send_to_char( msg_self, victim );
    if ( ch != victim )
	act( msg_other, ch, NULL, victim, TO_CHAR );
}

void spell_protection_heat ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    protection_spell( ch, (CHAR_DATA *) vo, gsn_protection_heat, level,
		      "Sıcağa karşı güçlendiğini hissediyorsun.\n\r",
		      "$N sıcağa karşı korunmaya başladı." );
}

void spell_protection_cold ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    protection_spell( ch, (CHAR_DATA *) vo, gsn_protection_cold, level,
		      "Soğuğa karşı güçlendiğini hissediyorsun.\n\r",
		      "$N soğuğa karşı korunmaya başladı." );
}

void spell_fire_shield ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *fire;

    target_name = one_argument(target_name, arg);
    if (!(!str_cmp(arg,"buz") || !str_cmp(arg,"ateş")))
	{
	send_to_char("Kalkanın türünü belirtmelisin.\n\r",ch);
	return;
	}

    fire = create_owned_obj( ch, OBJ_VNUM_FIRE_SHIELD );
    personalize_str( &fire->short_descr, arg );
    personalize_str( &fire->description, arg );
    personalize_extra( fire, arg );
    fire->timer = 5 * ch->level ;
    if (IS_GOOD(ch))
	 SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL));
    else if (IS_NEUTRAL(ch))
	 SET_BIT(fire->extra_flags,(ITEM_ANTI_GOOD | ITEM_ANTI_EVIL));
    else if (IS_EVIL(ch))
	 SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD));
    obj_to_char( fire, ch);
    send_to_char("Ateş kalkanı yaratıyorsun.\n\r",ch);
}

void spell_witch_curse ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (is_affected(victim,gsn_witch_curse))
    {
      send_to_char("Zaten yaşamı akıp gidiyor.\n\r",ch);
      return;
    }

  if (saves_spell((level+5),victim,DAM_MENTAL) || number_range(0,1) == 0)
  {
      send_to_char("Başaramadın!\n\r",ch);
      return;
  }

  ch->hit -=(2 * level);
  ch->hit = UMAX(ch->hit, 1);

  af.where		= TO_AFFECTS;
  af.type               = gsn_witch_curse;
  af.level              = level;
  af.duration           = 24;
  af.location           = APPLY_HIT;
  af.modifier           = - level;
  af.bitvector          = 0;
  affect_to_char(victim,&af);


  send_to_char("Şimdi ölüm yoluna girdi.\n\r",ch);
}


void spell_knock ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    char arg[MAX_INPUT_LENGTH];
    int chance=0;
    int door;

    target_name = one_argument(target_name,arg);

    if (arg[0] == '\0')
    {
      send_to_char("Hangi yönü ya da kapıyı çalacaksın.\n\r",ch);
    return;
    }

    if (ch->fighting)
    {
      send_to_char("Dövüş bitene kadar bekle.\n\r",ch);
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	  { send_to_char("Zaten açık.\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	  { send_to_char("Sadece açmayı dene.\n\r",     ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_NOPASS) )
	  { send_to_char( "Mistik bir güç çıkışı koruyor.\n\r",ch );
	      return; }
    chance = ch->level / 5 + get_curr_stat(ch,STAT_INT) + get_skill(ch,sn) / 5;

    act("$d'yi çalıyor ve açmaya çalışıyorsun!",
		ch,NULL,pexit->keyword,TO_CHAR);
    act("$n $d'yi çalıyor ve açmaya çalışıyor!",
		ch,NULL,pexit->keyword,TO_ROOM);

    if (room_dark(ch->in_room))
		chance /= 2;

    /* now the attack */
    if (number_percent() < chance )
     {
	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
  act("$n $d'yi çalıyor ve kapı açılıyor.", ch, NULL,
		pexit->keyword, TO_ROOM );
	send_to_char( "Kapıyı açmayı başardın.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
      act( "$d açılıyor.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
     }
    else
     {
       act("$d'yi açamadın!",
     	    ch,NULL,pexit->keyword,TO_CHAR);
     	act("$n $d'yi açamadı.",
	    ch,NULL,pexit->keyword,TO_ROOM);
     }
    return;
    }

    send_to_char("Onu göremiyorsun.\n\r",ch);
}


/*
 * Kendine bağışıklık/direnç: yoksa where/bits etkisi + msg_on, varsa msg_dup.
 */
static void self_flag_buff( CHAR_DATA *ch, int sn, int level, int where, int bits,
			    int duration, const char *msg_on, const char *msg_dup )
{
    if ( is_affected( ch, sn ) )
    {
	send_to_char( msg_dup, ch );
	return;
    }
    send_to_char( msg_on, ch );
    char_affect( ch, where, sn, ch->level, duration, 0, 0, bits );
}

void spell_magic_resistance ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    self_flag_buff( ch, sn, level, TO_RESIST, RES_MAGIC, level / 10,
		    "Artık büyüye dirençlisin.\n\r", "Büyüye zaten dirençlisin.\n\r" );
}

void spell_hallucination ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
  send_to_char( "Bu büyü henüz hazır değil.\n\r",ch);
}

void spell_wolf( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *demon;
  AFFECT_DATA af;
  int i;

  if (is_affected(ch,sn))
    {
      send_to_char("Başka bir kurt çağıracak gücün yok.\n\r",ch);
      return;
    }

    send_to_char("Bir kurt çağırmayı deniyorsun.\n\r",ch);
    act("$n bir kurt çağırmayı deniyor.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  gch->pIndexData->vnum == MOB_VNUM_WOLF)
	{
    send_to_char("İki kurt kontrol edebileceğinden fazla!\n\r",ch);
	  return;
	}
    }

  if (count_charmed(ch)) return;

  demon = create_mobile( get_mob_index(MOB_VNUM_WOLF), NULL );

  for (i=0;i < MAX_STATS; i++)
    {
      demon->perm_stat[i] = ch->perm_stat[i];
    }

  demon->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: URANGE(ch->pcdata->perm_hit,ch->hit,30000);
  demon->hit = demon->max_hit;
  demon->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  demon->mana = demon->max_mana;
  demon->level = ch->level;
  for (i=0; i < 3; i++)
    demon->armor[i] = interpolate(demon->level,100,-100);
  demon->armor[3] = interpolate(demon->level,100,0);
  demon->silver = 0;
  demon->timer = 0;
  demon->damage[DICE_NUMBER] = number_range(level/15, level/10);
  demon->damage[DICE_TYPE] = number_range(level/3, level/2);
  demon->damage[DICE_BONUS] = number_range(level/6, level/5);

  char_to_room(demon,ch->in_room);
  send_to_char("Kurt geliyor ve önünde eğiliyor!\n\r",ch);
  act("Biryerlerden bir kurt gelerek selam veriyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  SET_BIT(demon->affected_by, AFF_CHARM);
  demon->master = demon->leader = ch;
}

void spell_vam_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 12);
    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;
    damage( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
}

void spell_dragon_skin( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    {
      if (victim == ch)
      send_to_char("Derin bir kaya akdar sert zaten.\n\r",ch);

      else
      act("$S derisi bir kaya kadar sert zaten.",ch,NULL,
	    victim,TO_CHAR);
      return;
    }
  af.where	= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = - (2 * level);
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act( "$s derisi artık bir kaya kadar sert.",victim,NULL,NULL,TO_ROOM );
  send_to_char("Derin artık bir kaya kadar sert.\n\r", victim );
  return;
}


void spell_mind_light(int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af,af2;

    if ( is_affected_room( ch->in_room, sn ))
    {
      send_to_char("Bu odanın mana hızlandırıcısı var zaten.\n\r",ch);
	return;
    }

    af.where     = TO_ROOM_CONST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 30;
    af.location  = APPLY_ROOM_MANA;
    af.modifier  = level;
    af.bitvector = 0;
    affect_to_room( ch->in_room, &af );

    af2.where     = TO_AFFECTS;
    af2.type      = sn;
    af2.level	 = level;
    af2.duration  = level / 10;
    af2.modifier  = 0;
    af2.location  = APPLY_NONE;
    af2.bitvector = 0;
    affect_to_char( ch, &af2 );
    send_to_char( "Oda zihin ışığıyla dolmaya başlıyor.\n\r", ch );
    act("Oda $s zihin ışığıyla dolmaya başlıyor.",ch,NULL,NULL,TO_ROOM);
    return;
}

void spell_insanity ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_NPC(ch))
	{
    send_to_char("Bu büyü oyunculara yapılabilir.\n\r",ch);
	 return;
	}

    if (IS_AFFECTED(victim,AFF_BLOODTHIRST ) || saves_spell( level, victim,DAM_OTHER) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 10;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_BLOODTHIRST;
    affect_to_char( victim, &af );
    send_to_char( "Bir Öfke Kabalı üyesi kadar saldırgansın.\n\r", victim );
    act("$n kızıl gözlerle bakıyor.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_power_stun ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;


    if (is_affected(victim,sn ) || saves_spell( level, victim,DAM_OTHER) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 90;
    af.location  = APPLY_DEX;
    af.modifier  = - 3;
    af.bitvector = AFF_STUN;
    affect_to_char( victim, &af );
    send_to_char( "Sersemledin.\n\r", victim );
    act_color("$n sersemledi.",victim,NULL,NULL,TO_ROOM,POS_SLEEPING,CLR_RED);
    return;
}



void spell_improved_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_IMP_INVIS) )
	return;

  act( "$n ortadan kayboluyor.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 10 ;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_IMP_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Görünmez oluyorsun.\n\r", victim );
    return;
}



void spell_improved_detection( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( CAN_DETECT(victim, DETECT_IMP_INVIS) )
    {
	if (victim == ch)
  send_to_char("Gelişmiş görünmezliği zaten saptayabiliyorsun.\n\r",ch);
else
  act("$N zaten gelişmiş görünmezliği saptayabiliyor.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_DETECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 3;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = DETECT_IMP_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Gözlerin çok yanıyor.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Tamam.\n\r", ch );
    return;
}

void spell_severity_force( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    int dam;

    snprintf(buf, sizeof(buf),"Arzı %s'e doğru kırıyorsun.\n\r",victim->name);
    send_to_char(buf, ch);
    act("$n arzı sana doğru kırıyor!", ch, NULL, victim, TO_VICT );
    if (IS_AFFECTED(victim,AFF_FLYING))
	dam = 0;
    else dam = dice( level , 12 );
    damage(ch,victim,dam,sn,DAM_BASH,TRUE);
    return;
}

void spell_randomizer(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    AFFECT_DATA af,af2;

    if ( is_affected( ch, sn ) )
    {
      send_to_char
      ("Rasgele gücün tükenmiş durumda.\n\r",ch);
      return;
    }

  if (IS_SET(ch->in_room->room_flags, ROOM_LAW))
    {
      send_to_char(
	      "Bu oda seninkinin ötesinde bir güçle korunuyor.\n\r",ch);
      return;
    }
    if ( is_affected_room( ch->in_room, sn ))
    {
      send_to_char("Bu odaya zaten rasgele uygulanmış.\n\r",ch);
	return;
    }

  if (number_range(0,1) == 0)
    {
      send_to_char("Evren karmaşaya direniyor.\n\r",ch);
      af2.where     = TO_AFFECTS;
      af2.type      = sn;
      af2.level	    = ch->level;
      af2.duration  = level / 10;
      af2.modifier  = 0;
      af2.location  = APPLY_NONE;
      af2.bitvector = 0;
      affect_to_char( ch, &af2 );
      return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->level;
    af.duration  = level / 15;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_RANDOMIZER;
    affect_to_room( ch->in_room, &af );

    af2.where     = TO_AFFECTS;
    af2.type      = sn;
    af2.level	  = ch->level;
    af2.duration  = level / 5;
    af2.modifier  = 0;
    af2.location  = APPLY_NONE;
    af2.bitvector = 0;
    affect_to_char( ch, &af2 );
    send_to_char("Oda rasgele ile şekillendirildi!\n\r", ch);
    send_to_char("Kendini tükenmiş hissediyorsun.\n\r", ch);
    ch->hit -= UMIN(200, ch->hit/2);
    act("Odanın çıkışları karışıyor.",ch,NULL,NULL,TO_ROOM);
    return;
}

void spell_bless_weapon(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af = {0};

    if (obj->item_type != ITEM_WEAPON)
    {
      send_to_char("O bir silah değil.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
      send_to_char("Taşıdığın şeyi kutsayabilirsin.\n\r",ch);
	return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_HOLY))
    {
      act("$p zaten kutsanmış.",ch,obj,NULL,TO_CHAR);
	return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
    ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
    ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
    ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
    ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
    ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
    ||  IS_OBJ_STAT(obj,ITEM_BLESS)
    ||  IS_OBJ_STAT(obj,ITEM_BURN_PROOF) )
    {
      act("$p kutsanmamış görünüyor.",ch,obj,NULL,TO_CHAR);
	return;
    }

    af.where	 = TO_WEAPON;
    af.type	 = sn;
    af.level	 = level / 2;
    af.duration	 = level/8;
    af.location	 = 0;
    af.modifier	 = 0;
    af.bitvector = WEAPON_HOLY;
    affect_to_obj(obj,&af);

    act("$p kutsal saldırılara hazırlanıyor.",ch,obj,NULL,TO_ALL);
}

void spell_resilience( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    self_flag_buff( ch, sn, level, TO_RESIST, RES_ENERGY, level / 10,
		    "Emici saldırılara karşı esnekleştin.\n\r",
		    "Emici saldırılara karşı ZATEN esneksin.\n\r" );
}

void spell_super_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    heal_victim( ch, (CHAR_DATA *) vo, 170 + level + dice(1,20) );
}

void spell_master_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    heal_victim( ch, (CHAR_DATA *) vo, 500 + level + dice(1,40) );
}

void spell_group_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    static sh_int sn_master_healing, sn_refresh;
    CHAR_DATA *gch;
    int heal_num, refresh_num;

    heal_num = cached_sn( &sn_master_healing, "master healing" );
    refresh_num = cached_sn( &sn_refresh, "refresh" );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) )
	    continue;
	spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
	spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);
    }
}


void spell_restoring_light( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    static sh_int sn_cure_blindness_l, sn_cure_disease_l;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int mana_add;

    if (IS_AFFECTED(victim,AFF_BLIND))
	spell_cure_blindness(cached_sn( &sn_cure_blindness_l, "cure blindness" ),
			     level,ch,(void *)victim,TARGET_CHAR);
    if (IS_AFFECTED(victim,AFF_CURSE))
	spell_remove_curse(gsn_remove_curse,level,ch,(void *)victim,TARGET_CHAR);
    if (IS_AFFECTED(victim,AFF_POISON))
	spell_cure_poison(gsn_cure_poison,level,ch,(void *)victim,TARGET_CHAR);
    if (IS_AFFECTED(victim,AFF_PLAGUE))
	spell_cure_disease(cached_sn( &sn_cure_disease_l, "cure disease" ),
			   level,ch,(void *)victim,TARGET_CHAR);

    if (victim->hit != victim->max_hit)
	{
    	 mana_add = UMIN( (victim->max_hit - victim->hit), ch->mana );
    	 victim->hit = UMIN( victim->hit + mana_add, victim->max_hit );
    	 ch->mana -= mana_add;
	}
    update_pos( victim );
    send_to_char( "Sıcak bir duygu vücudunu sarıyor.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Tamam.\n\r", ch );
}


void spell_lesser_golem( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i=0;

  if (is_affected(ch,sn))
    {
      send_to_char("Başka bir golem yaratacak gücün yok.\n\r",ch);

      return;
    }

    send_to_char("Küçük bir golem yaratmayı deniyorsun.\n\r",ch);
    act( "$n küçük bir golem yaratmayı deniyor.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  ( gch->pIndexData->vnum == MOB_VNUM_LESSER_GOLEM ) )
	{
	  i++;
	  if (i > 2)
           {
             send_to_char("Daha fazla golemi kontrol altından tutamazsın!\n\r",ch);
	    return;
	   }
	}
    }

  if (count_charmed(ch)) return;

  golem = create_mobile( get_mob_index(MOB_VNUM_LESSER_GOLEM), NULL );


  for (i = 0; i < MAX_STATS; i ++)
       golem->perm_stat[i] = UMIN(25,15 + ch->level/10);

  golem->perm_stat[STAT_STR] += 3;
  golem->perm_stat[STAT_INT] -= 1;
  golem->perm_stat[STAT_CON] += 2;

  golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN( (2 * ch->pcdata->perm_hit) + 400,30000);
  golem->hit = golem->max_hit;
  golem->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  golem->mana = golem->max_mana;
  golem->level = ch->level;
  for (i=0; i < 3; i++)
    golem->armor[i] = interpolate(golem->level,100,-100);
  golem->armor[3] = interpolate(golem->level,100,0);
  golem->silver = 0;
  golem->timer = 0;
  golem->damage[DICE_NUMBER] = 3;
  golem->damage[DICE_TYPE] = 10;
  golem->damage[DICE_BONUS] = ch->level / 2;

  char_to_room(golem,ch->in_room);
  send_to_char("Küçük bir golem yaratıyorsun!\n\r",ch);
  act("$n küçük bir golem yaratıyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  SET_BIT(golem->affected_by, AFF_CHARM);
  golem->master = golem->leader = ch;

}


void spell_stone_golem( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i=0;

  if (is_affected(ch,sn))
    {
      send_to_char(   "Başka bir golem yaratacak gücün yok.\n\r",ch);
      return;
    }

    send_to_char("Bir taş golem yaratmayı deniyorsun.\n\r",ch);
    act("$n bir taş golem yaratmayı deniyor.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  ( gch->pIndexData->vnum == MOB_VNUM_STONE_GOLEM ) )
	{
	  i++;
	  if (i > 2)
           {
             send_to_char(	"Daha fazla golemi kontrol altından tutamazsın!\n\r",ch);
	    return;
	   }
	}
    }

  if (count_charmed(ch)) return;

  golem = create_mobile( get_mob_index(MOB_VNUM_STONE_GOLEM), NULL );


  for (i = 0; i < MAX_STATS; i ++)
       golem->perm_stat[i] = UMIN(25,15 + ch->level/10);

  golem->perm_stat[STAT_STR] += 3;
  golem->perm_stat[STAT_INT] -= 1;
  golem->perm_stat[STAT_CON] += 2;

  golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN( (5 * ch->pcdata->perm_hit) + 2000, 30000);
  golem->hit = golem->max_hit;
  golem->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  golem->mana = golem->max_mana;
  golem->level = ch->level;
  for (i=0; i < 3; i++)
    golem->armor[i] = interpolate(golem->level,100,-100);
  golem->armor[3] = interpolate(golem->level,100,0);
  golem->silver = 0;
  golem->timer = 0;
  golem->damage[DICE_NUMBER] = 8;
  golem->damage[DICE_TYPE] = 4;
  golem->damage[DICE_BONUS] = ch->level / 2;

  char_to_room(golem,ch->in_room);
  send_to_char("Bir taş golem yaratıyorsun!\n\r",ch);
  act("$n bir taş golem yaratıyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  SET_BIT(golem->affected_by, AFF_CHARM);
  golem->master = golem->leader = ch;

}


void spell_iron_golem( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i = 0;

  if (is_affected(ch,sn))
    {
      send_to_char( "Başka bir golem yaratacak gücün yok.\n\r",ch);
      return;
    }

    send_to_char( "Bir demir golem yaratmayı deniyorsun.\n\r",ch);
    act("$n bir demir golem yaratmayı deniyor.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  ( gch->pIndexData->vnum == MOB_VNUM_IRON_GOLEM ) )
	{
    send_to_char("Daha fazla golemi kontrol altından tutamazsın!\n\r",ch);
	    return;
	}
    }

  if (count_charmed(ch)) return;

  golem = create_mobile( get_mob_index(MOB_VNUM_IRON_GOLEM), NULL );


  for (i = 0; i < MAX_STATS; i ++)
       golem->perm_stat[i] = UMIN(25,15 + ch->level/10);

  golem->perm_stat[STAT_STR] += 3;
  golem->perm_stat[STAT_INT] -= 1;
  golem->perm_stat[STAT_CON] += 2;

  golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN( (10 * ch->pcdata->perm_hit) + 1000, 30000);
  golem->hit = golem->max_hit;
  golem->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  golem->mana = golem->max_mana;
  golem->level = ch->level;
  for (i=0; i < 3; i++)
    golem->armor[i] = interpolate(golem->level,100,-100);
  golem->armor[3] = interpolate(golem->level,100,0);
  golem->silver = 0;
  golem->timer = 0;
  golem->damage[DICE_NUMBER] = 11;
  golem->damage[DICE_TYPE] = 5;
  golem->damage[DICE_BONUS] = ch->level / 2 + 10;

  char_to_room(golem,ch->in_room);
  send_to_char("Bir demir golem yaratıyorsun!\n\r",ch);
  act("$n bir demir golem yaratıyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  SET_BIT(golem->affected_by, AFF_CHARM);
  golem->master = golem->leader = ch;

}


void spell_adamantite_golem( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i = 0;

  if (is_affected(ch,sn))
    {
      send_to_char( "Başka bir golem yaratacak gücün yok.\n\r",ch);
      return;
    }

    send_to_char("Bir serttaşı golem yaratmayı deniyorsun.\n\r",ch);
    act("$n bir serttaşı golem yaratmayı deniyor.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  ( gch->pIndexData->vnum == MOB_VNUM_ADAMANTITE_GOLEM ) )
	{
    send_to_char("Daha fazla golemi kontrol altından tutamazsın!\n\r",ch);
	    return;
	}
    }

  if (count_charmed(ch)) return;

  golem = create_mobile( get_mob_index(MOB_VNUM_ADAMANTITE_GOLEM) , NULL);


  for (i = 0; i < MAX_STATS; i ++)
       golem->perm_stat[i] = UMIN(25,15 + ch->level/10);

  golem->perm_stat[STAT_STR] += 3;
  golem->perm_stat[STAT_INT] -= 1;
  golem->perm_stat[STAT_CON] += 2;

  golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN( (10 * ch->pcdata->perm_hit) + 4000, 30000);
  golem->hit = golem->max_hit;
  golem->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  golem->mana = golem->max_mana;
  golem->level = ch->level;
  for (i=0; i < 3; i++)
    golem->armor[i] = interpolate(golem->level,100,-100);
  golem->armor[3] = interpolate(golem->level,100,0);
  golem->silver = 0;
  golem->timer = 0;
  golem->damage[DICE_NUMBER] = 13;
  golem->damage[DICE_TYPE] = 9;
  golem->damage[DICE_BONUS] = ch->level / 2 + 10;

  char_to_room(golem,ch->in_room);
  send_to_char( "Bir serttaşı golem yarattın!\n\r",ch);
  act("$n bir serttaşı golem yarattı!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  SET_BIT(golem->affected_by, AFF_CHARM);
  golem->master = golem->leader = ch;

}



/*
 * Oda lanetleri: ROOM_LAW → zaten var → TO_ROOM_AFFECTS etkisi + mesajlar.
 * sanctify_lands aynı tabloyu tersten uygular.
 */
struct room_curse
{
    sh_int *gsn;
    int bit;
    const char *dup_msg;      /* zaten etkiliyse */
    const char *cast_msg;     /* büyücü ve oda */
    const char *cleanse_msg;  /* sanctify: büyücü ve oda */
};

static const struct room_curse room_curses[] =
{
    { &gsn_deadly_venom,     AFF_ROOM_POISON, "Oda zaten ölüm zehriyle dolu.\n\r",
      "Oda zehirle dolmaya başlıyor.",           "Bölge daha sağlıklı görünüyor." },
    { &gsn_cursed_lands,     AFF_ROOM_CURSE,  "Oda zaten lanetli.\n\r",
      "Tanrılar odayı başıboş bıraktılar.",      "Bölgedeki lanet yokoluyor." },
    { &gsn_lethargic_mist,   AFF_ROOM_SLOW,   "Oda zaten uyuşukluk sisiyle dolu.\n\r",
      "Odanın havası seni yavaşlatıyor.",        "Uyuşukluk sisi yokoluyor." },
    { &gsn_black_death,      AFF_ROOM_PLAGUE, "Bu oda zaten hastalıkla dolu.\n\r",
      "Oda hastalıkla dolmaya başlıyor.",        "Bölgenin hastalığı yokoldu." },
    { &gsn_mysterious_dream, AFF_ROOM_SLEEP,  "Bu oda zaten uyku gazının etkisinde.\n\r",
      "Uyumak için güzel bir yer.",              "Bölge gizemli uykusundan uyanıyor." },
};
#define ROOM_CURSE_COUNT ((int) (sizeof(room_curses) / sizeof(room_curses[0])))

static void room_curse_spell( CHAR_DATA *ch, int level, const struct room_curse *rc )
{
    AFFECT_DATA af = {0};

    if ( IS_SET(ch->in_room->room_flags, ROOM_LAW) )
    {
	send_to_char( "Oda tanrılarca korunuyor.\n\r", ch );
	return;
    }
    if ( is_affected_room( ch->in_room, *rc->gsn ) )
    {
	send_to_char( rc->dup_msg, ch );
	return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = *rc->gsn;
    af.level     = ch->level;
    af.duration  = level / 15;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = rc->bit;
    affect_to_room( ch->in_room, &af );

    send_to_char( rc->cast_msg, ch );
    send_to_char( "\n\r", ch );
    act( rc->cast_msg, ch, NULL, NULL, TO_ROOM );
}

void spell_sanctify_lands(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  int i;

  if (number_range(0,1) == 0)
    {
      send_to_char("Başaramadın.\n\r",ch);
      return;
    }

  for ( i = 0; i < ROOM_CURSE_COUNT; i++ )
  {
      if ( !IS_RAFFECTED( ch->in_room, room_curses[i].bit ) )
	  continue;
      affect_strip_room( ch->in_room, *room_curses[i].gsn );
      send_to_char( room_curses[i].cleanse_msg, ch );
      send_to_char( "\n\r", ch );
      act( room_curses[i].cleanse_msg, ch, NULL, NULL, TO_ROOM );
  }
}


void spell_deadly_venom(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    room_curse_spell( ch, level, &room_curses[0] );
}

void spell_cursed_lands(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    room_curse_spell( ch, level, &room_curses[1] );
}

void spell_lethargic_mist(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    room_curse_spell( ch, level, &room_curses[2] );
}

void spell_black_death(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    room_curse_spell( ch, level, &room_curses[3] );
}

void spell_mysterious_dream(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    room_curse_spell( ch, level, &room_curses[4] );
}

void spell_polymorph( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    int race;

    if ( spell_busy( ch, sn, "Zaten şeklin değişmiş.\n\r" ) )
	return;

    if (target_name == NULL || target_name[0] == '\0')
    {
      send_to_char( "Kullanım: büyü 'binbir şekil' <ırk>.\n\r",ch);
     return;
    }

    race = race_lookup( target_name );

    if (race == 0 || !race_table[race].pc_race)
    {
      send_to_char("O dönüşebileceğin birşey değil.\n\r",ch);
     return;
    }

    char_affect( ch, TO_RACE, sn, level, level/10, APPLY_NONE, race, 0 );

    act("$n kendini $t yapıyor.",
		ch, race_table[race].name[1], NULL,TO_ROOM );
    act( "Kendini $t yapıyorsun.\n\r",
		ch, race_table[race].name[1], NULL,TO_CHAR );
}



void spell_blade_barrier(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    static const int barrier_dice[] = { 7, 5, 4, 2 };
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int i, dam;

    act("$s çevresinde bıçaklar oluşuyor ve $E çarpıyor.",
	ch,NULL,victim,TO_ROOM);
    act("Çevrende bıçaklar oluşuyor ve $E çarpıyor.",
	ch,NULL,victim,TO_CHAR);
    act("$s çevresinde bıçaklar oluşuyor ve sana çarpıyor!",
	ch,NULL,victim,TO_VICT);

    for ( i = 0; i < 4; i++ )
    {
	if ( i > 0 )
	    act("Bıçak duvarı $e çarpıyor!",victim,NULL,NULL,TO_ROOM);
	dam = dice(level,barrier_dice[i]);
	if (saves_spell(level,victim,DAM_PIERCE))
	    dam /= 2;
	damage(ch,victim,dam,sn,DAM_PIERCE,TRUE);
	if ( !still_in_room( ch, victim ) )   /* öldü ya da odadan gitti */
	    return;

	if ( i == 0 )
	{
	    if (!IS_NPC(ch) && victim != ch &&
		ch->fighting != victim && victim->fighting != ch &&
		(IS_SET(victim->affected_by,AFF_CHARM) || !IS_NPC(victim)))
		victim_yell( victim, ch, "İmdat biri bana saldırıyor!",
			     "Geber %s, seni büyücü köpek!" );
	}
	else
	{
	    act("Bıçak duvarı sana çarpıyor!",victim,NULL,NULL,TO_CHAR);
	    if ( i < 3 && number_percent() < 50 )
		return;
	}
    }
}


void spell_protection_negative ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    self_flag_buff( ch, sn, level, TO_IMMUNE, IMM_NEGATIVE, level / 4,
		    "Negatif saldırılara bağışıklı olduğunu hissediyorsun.\n\r",
		    "Zaten negatif saldırılara bağışıklısın.\n\r" );
}


void spell_ruler_aura( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    self_flag_buff( ch, sn, level, TO_IMMUNE, IMM_CHARM, level / 4,
		    "Kendine ve ideallerine olan inancın güçleniyor.\n\r",
		    "Kendinden eminsin zaten.\n\r" );
}


void spell_evil_spirit( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
 AREA_DATA *pArea = ch->in_room->area;
 ROOM_INDEX_DATA *room;
 AFFECT_DATA af,af2;
 int i;

 if (IS_RAFFECTED(ch->in_room, AFF_ROOM_ESPIRIT)
	|| is_affected_room(ch->in_room,sn) )
  {
    send_to_char( "Bölge zaten şerle dolu.\n\r",ch);
   return;
  }

 if ( is_affected( ch, sn ) )
    {
      send_to_char(  "Yeterince gücün yok.\n\r",ch);
      return;
    }

  if (IS_SET(ch->in_room->room_flags, ROOM_LAW)
	|| IS_SET(ch->in_room->area->area_flag,AREA_HOMETOWN) )
    {
      send_to_char(  "Odadaki kutsal aura seni engelliyor.\n\r",ch);
      return;
    }

    af2.where     = TO_AFFECTS;
    af2.type      = sn;
    af2.level	  = ch->level;
    af2.duration  = level / 5;
    af2.modifier  = 0;
    af2.location  = APPLY_NONE;
    af2.bitvector = 0;
    affect_to_char( ch, &af2 );

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->level;
    af.duration  = level / 25;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_ESPIRIT;

    for (i=pArea->min_vnum; i<pArea->max_vnum; i++)
    {
     if ((room = get_room_index(i)) == NULL) continue;
     affect_to_room( room, &af );
     if (room->people)
     act( "Oda şer ruhların bilinciyle dolmaya başlıyor.",room->people,NULL,NULL,TO_ALL);
    }

}


void spell_disgrace( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (!is_affected(victim,sn) && !saves_spell(level, victim, DAM_MENTAL))
    {
      af.where		    = TO_AFFECTS;
      af.type               = sn;
      af.level              = level;
      af.duration           = level;
      af.location           = APPLY_CHA;
      af.modifier           = - (5 + level / 5);
      af.bitvector          = 0;
      affect_to_char(victim,&af);

      act("$N kendinden emin değil!",ch,NULL,victim,TO_ALL);
    }
  else send_to_char("Başaramadın.\n\r",ch);
}


void spell_control_undead( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if  ( !IS_NPC(victim) || !IS_SET(victim->act,ACT_UNDEAD) )
    {
      act("$N bir hortlağa benzemiyor.",ch,NULL,victim,TO_CHAR);
	return;
    }
    spell_charm_person(sn,level,ch,vo,target);
    return;
}



/* Yardım/destek: kısa bekleme + yp artışı (max ile sınırlı). */
static void assist_spell( CHAR_DATA *ch, CHAR_DATA *victim, int sn, int level,
			  int duration, int amount )
{
    if ( spell_busy( ch, sn, "Bu güç yakın zamanda kullanıldı.\n\r" ) )
	return;
    spell_set_cooldown( ch, sn, level, duration );

    victim->hit = UMIN( victim->hit + amount, victim->max_hit );
    update_pos( victim );
    send_to_char("Sıcak bir his vücudunu sarıyor.\n\r", victim );
    act("$n daha iyi görünüyor.", victim, NULL, NULL, TO_ROOM);
    if (ch != victim) send_to_char("Tamam.\n\r",ch);
}

void spell_assist( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    assist_spell( ch, (CHAR_DATA *) vo, sn, level, 1 + level / 50, 100 + level * 5 );
}



void spell_aid( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    assist_spell( ch, (CHAR_DATA *) vo, sn, level, level / 50, level * 5 );
}



void spell_summon_shadow( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *shadow;
  AFFECT_DATA af;
  int i;

  if (is_affected(ch,sn))
    {
      send_to_char(  "Başka bir gölge çağırmak için yeterli gücün yok.\n\r",ch);
      return;
    }

    send_to_char("Bir gölge çağırmayı deniyorsun.\n\r",ch);
    act("$n bir gölge çağırmayı deniyor.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  gch->pIndexData->vnum == MOB_VNUM_SUM_SHADOW)
	{
    send_to_char("İki gölge kontrol edebileceğinden fazla!\n\r",ch);
	  return;
	}
    }

  if (count_charmed(ch)) return;

  shadow = create_mobile( get_mob_index(MOB_VNUM_SUM_SHADOW), NULL );

  for (i=0;i < MAX_STATS; i++)
    {
      shadow->perm_stat[i] = ch->perm_stat[i];
    }

  shadow->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: URANGE(ch->pcdata->perm_hit,ch->hit,30000);
  shadow->hit = shadow->max_hit;
  shadow->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  shadow->mana = shadow->max_mana;
  shadow->level = ch->level;
  for (i=0; i < 3; i++)
    shadow->armor[i] = interpolate(shadow->level,100,-100);
  shadow->armor[3] = interpolate(shadow->level,100,0);
  shadow->silver = 0;
  shadow->timer = 0;
  shadow->damage[DICE_NUMBER] = number_range(level/15, level/10);
  shadow->damage[DICE_TYPE] = number_range(level/3, level/2);
  shadow->damage[DICE_BONUS] = number_range(level/8, level/6);

  char_to_room(shadow,ch->in_room);
  act("Bir gölge oluşuyor!",ch,NULL,NULL,TO_ALL);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  SET_BIT(shadow->affected_by, AFF_CHARM);
  shadow->master = shadow->leader = ch;

}


void spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    ROOM_INDEX_DATA *room;

    if ( (room = check_place(ch,target_name)) == NULL)
      {
        send_to_char("O uzaklığı göremezsin.\n\r",ch);
	return;
      }

    scry_room( ch, room );
}


void spell_remove_fear( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (check_dispel(level,victim,gsn_fear))
    {
      send_to_char("Daha cesur hissediyorsun.\n\r",victim);
    	act("$n daha bilinçli görünüyor.",victim,NULL,NULL,TO_ROOM);
    }
    else send_to_char("Başaramadın.\n\r",ch);
}

void spell_desert_fist( int sn, int level, CHAR_DATA *ch, void *vo,int target )
 {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( (ch->in_room->sector_type != SECT_HILLS)
	&& (ch->in_room->sector_type != SECT_MOUNTAIN)
	&& (ch->in_room->sector_type != SECT_DESERT) )
	{
    send_to_char("Yumruk yaratmak için gerekli kumu bulamıyorsun.\n\r",ch);
	 ch->wait = 0;
	 return;
	}

  act("Büyük bir kum kütlesi yükselerek bir yumruk oluşturuyor, ve $e vuruyor.",
  victim,NULL,NULL,TO_ROOM);
  act("Büyük bir kum kütlesi yükselerek bir yumruk oluşturuyor, ve sana vuruyor.",
		victim,NULL,NULL,TO_CHAR);
    dam = dice( level , 16 );
    damage(ch,victim,dam,sn,DAM_OTHER,TRUE);
    sand_effect(victim,level,dam,TARGET_CHAR);
}

void spell_holy_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
        send_to_char("Zaten kutsal bir aylan var.\n\r",ch);
       else
         act("$S zaten kutsal bir aylası var.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( !IS_GOOD(victim) )
    {
      send_to_char("O kutsal auraya layık değil!\n\r", ch);
	return;
    }

    char_affect( victim, TO_AFFECTS, sn, level, 7 + level / 6, APPLY_AC, - (20 + level / 4), 0 );
    char_affect( victim, TO_RESIST,  sn, level, 7 + level / 6, 0, 0, RES_NEGATIVE );

    send_to_char( "Kadim güçlerin seni koruduğunu hissediyorsun.\n\r", victim );
    if ( ch != victim )
        act("$N kadim kutsal güçlerce korunmaya başlıyor.",ch,NULL,victim,TO_CHAR);
}

void spell_holy_fury(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    static sh_int sn_calm;
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
        if (victim == ch)
            send_to_char("Zaten hiddetlisin.\n\r",ch);
        else
            act("$N zaten hiddetli.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (is_affected(victim,cached_sn( &sn_calm, "calm" )))
    {
        if (victim == ch)
            send_to_char("Neden bir an için rahatlamıyorsun?\n\r",ch);
        else
            act("$N kavgaya meraklıymış gibi durmuyor.",
              ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( !IS_GOOD(victim) )
    {
      act("$N kutsal hiddette layık değil.",ch,NULL,victim,TO_CHAR);
        return;
    }

    char_affect( victim, TO_AFFECTS, sn, level, level / 3, APPLY_HITROLL, level / 6, 0 );
    char_affect( victim, TO_AFFECTS, sn, level, level / 3, APPLY_DAMROLL, level / 6, 0 );
    char_affect( victim, TO_AFFECTS, sn, level, level / 3, APPLY_AC, 10 * (level / 12), 0 );

    send_to_char("Kutsal hiddetle doluyorsun!\n\r",victim);
    act("$s gözlerine hiddet doluyor!",victim,NULL,NULL,TO_ROOM);
}

void spell_light_arrow( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 12 );
    if ( saves_spell( level, victim, DAM_HOLY ) )
        dam /= 2;
    damage( ch, victim, dam, sn,DAM_HOLY,TRUE);
    return;
}


void spell_hydroblast( int sn, int level, CHAR_DATA *ch, void *vo,int target )
 {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( (ch->in_room->sector_type != SECT_WATER_SWIM)
	&& (ch->in_room->sector_type != SECT_WATER_NOSWIM)
	&& (weather_info.sky != SKY_RAINING || !IS_OUTSIDE(ch)) )
	{
    send_to_char( "Su kaynağına erişemiyorsun.\n\r",ch);
	 ch->wait = 0;
	 return;
	}

  act("$s çevresindeki su molekülleri birleşerek bir yumruk oluşturuyor.",
  ch,NULL,NULL,TO_ROOM);
  act("Çevrendeki su molekülleri birleşerek bir yumruk oluşturuyor.",
		ch,NULL,NULL,TO_CHAR);
    dam = dice( level , 14 );
    damage(ch,victim,dam,sn,DAM_BASH,TRUE);
}

void spell_wolf_spirit( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    int dur = 3 + level / 30;

    if ( spell_busy( ch, sn, "Damarlarındaki kan zaten olabildiğince hızlı akıyor!\n\r" ) )
	return;

    /* haste, damroll, infravision */
    char_affect( ch, TO_AFFECTS, sn, level, dur, APPLY_DEX,
		 1 + (level > 40) + (level > 60), AFF_HASTE );
    char_affect( ch, TO_AFFECTS, sn, level, dur, APPLY_DAMROLL, level / 2, AFF_BERSERK );
    char_affect( ch, TO_AFFECTS, sn, level, dur, APPLY_NONE, 0, AFF_INFRARED );

    send_to_char( "Damarlarındaki kan hızlı akmaya başlıyor.\n\r", ch );
    act("$s gözleri KIZILLAŞIYOR!",ch,NULL,NULL,TO_ROOM);
}


void spell_sword_of_justice( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( (IS_GOOD(ch) && IS_GOOD(victim))
	|| (IS_EVIL(ch) && IS_EVIL(victim))
	|| IS_NEUTRAL(ch) )
  {
    if (IS_NPC(victim) || !IS_SET(victim->act, PLR_WANTED))
    {
	send_to_char("Başaramadın!\n\r", ch);
	return;
    }
  }

  if (!IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED))
	dam = dice(level, 20);
  else	dam = dice(level, 14);

  if (saves_spell(level,victim, DAM_MENTAL))
	      dam /= 2;

  do_yell(ch, "Adalet Kılıcı!");
  act("Adalet kılıcı ortaya çıkarak $E vuruyor!",ch,NULL,victim,TO_ALL);

  damage(ch,victim,dam,sn,DAM_MENTAL, TRUE);
}

void spell_guard_dogs( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *dog;
  CHAR_DATA *dog2;
  AFFECT_DATA af;
  int i;

  if (is_affected(ch,sn))
    {
      send_to_char("Başka bir köpeği çağıracak gücün yok.\n\r",ch);
      return;
    }

    send_to_char("Bir köpek çağırmayı deniyorsun.\n\r",ch);
    act("$n bir köpek çağırmayı deniyor.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  gch->pIndexData->vnum == MOB_VNUM_DOG)
	{
    send_to_char( "İki köpek kontrol edebileceğinden fazla!\n\r",ch);
	  return;
	}
    }

  if (count_charmed(ch)) return;

  dog = create_mobile( get_mob_index(MOB_VNUM_DOG) , NULL);

  for (i=0;i < MAX_STATS; i++)
    {
      dog->perm_stat[i] = ch->perm_stat[i];
    }

  dog->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: URANGE(ch->pcdata->perm_hit,ch->hit,30000);
  dog->hit = dog->max_hit;
  dog->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  dog->mana = dog->max_mana;
  dog->level = ch->level;
  for (i=0; i < 3; i++)
    dog->armor[i] = interpolate(dog->level,100,-100);
  dog->armor[3] = interpolate(dog->level,100,0);
  dog->silver = 0;
  dog->timer = 0;
  dog->damage[DICE_NUMBER] = number_range(level/15, level/12);
  dog->damage[DICE_TYPE] = number_range(level/3, level/2);
  dog->damage[DICE_BONUS] = number_range(level/10, level/8);

  dog2 = create_mobile(dog->pIndexData, NULL);
  clone_mobile(dog,dog2);

  SET_BIT(dog->affected_by, AFF_CHARM);
  SET_BIT(dog2->affected_by, AFF_CHARM);
  dog->master = dog2->master = ch;
  dog->leader = dog2->leader = ch;

  char_to_room(dog,ch->in_room);
  char_to_room(dog2,ch->in_room);
  send_to_char(  "İki köpek geliyor ve önünde eğiliyor!\n\r",ch);
  act("İki köpek geliyor ve selam veriyor!",ch,NULL,NULL,TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

}

void spell_eyes_of_tiger( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;

    if ( (victim = get_char_world(ch, target_name)) == NULL)
      {
        send_to_char(	"Kaplan gözlerin öyle birini görmüyor.\n\r",ch);
	return;
      }

    if ( IS_NPC(victim) || victim->cabal != CABAL_HUNTER)
    {
      send_to_char("Kaplan gözleri yalnız avcıları görür!\n\r",ch);
	return;
    }

    if ((victim->level > ch->level + 7)
	|| saves_spell((ch->level + 9), victim, DAM_NONE))
      {
        send_to_char("Kaplan gözlerin onu görmüyor.\n\r",ch);
	return;
      }

    scry_room( ch, victim->in_room );
}

void spell_lion_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  conjure_shield( ch, sn, level, OBJ_VNUM_LION_SHIELD, -(level * 2) / 3 );
}

void spell_evolve_lion( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  int dur = 3 + level / 30;
  int hp = IS_NPC(ch) ? ch->max_hit : ch->pcdata->perm_hit;

  if (is_affected(ch,sn) || ch->hit > ch->max_hit)
    {
      send_to_char( "Olabildiğince aslansın zaten.\n\r",ch);
      return;
    }

  ch->hit += hp;

  char_affect( ch, TO_AFFECTS, sn, level, dur, APPLY_HIT, hp, 0 );
  char_affect( ch, TO_AFFECTS, sn, level, dur, APPLY_DEX, -(1 + level/30), AFF_SLOW );
  char_affect( ch, TO_AFFECTS, sn, level, dur, APPLY_DAMROLL, level / 2, AFF_BERSERK );
  char_affect( ch, TO_DETECTS, sn, level, dur, APPLY_NONE, 0, ADET_LION );

  send_to_char("Kendini biraz sakar, fakat daha güçlü hissediyorsun.\n\r",ch);
  act("$s derisi grileşiyor!",ch,NULL,NULL,TO_ROOM);
}

void spell_prevent(int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af,af2;

    if ( is_affected_room( ch->in_room, sn ))
    {
      send_to_char("Bu odada zaten intikam alınamıyor!\n\r",ch);
	return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 30;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_PREVENT;
    affect_to_room( ch->in_room, &af );

    af2.where     = TO_AFFECTS;
    af2.type      = sn;
    af2.level	  = level;
    af2.duration  = level / 10;
    af2.modifier  = 0;
    af2.location  = APPLY_NONE;
    af2.bitvector = 0;
    affect_to_char( ch, &af2 );
    send_to_char("Bu odada intikam alınamayacak!\n\r", ch );
    act("Oda $s Avcıları engelleyici gücüyle dolmaya başlıyor",ch,NULL,NULL,TO_ROOM);
    return;
}


void spell_enlarge( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
  send_to_char("Daha fazla genişleyemezsin!\n\r",ch);
else
  act("$N olabildiğince genişlemiş zaten.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_SIZE;
    af.modifier  = 1 + (level >= 35) + (level >= 65);
    af.modifier  = UMIN((SIZE_GARGANTUAN - victim->size), af.modifier);
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char("Gittikçe irileştiğini hissediyorsun.\n\r", victim );
    act("$s vücudu genişlemeye başladı.",victim,NULL,NULL,TO_ROOM);
   return;
}

void spell_chromatic_orb( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level,14);
    if ( saves_spell( level, victim,DAM_LIGHT) )
        dam /= 2;

    if (number_percent() < get_skill(ch,sn) * 7 / 10 )
    {
	spell_blindness(gsn_blindness, (level-10),ch,(void *) victim,TARGET_CHAR);
	spell_slow(gsn_slow, (level-10),ch,(void *) victim,TARGET_CHAR);
    }

    damage( ch, victim, dam, sn, DAM_LIGHT,TRUE );
}

void spell_suffocate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (CAN_DETECT(victim,ADET_SUFFOCATE))
        {
          act("$N zaten nefes alamıyor.\n\r",ch,NULL,victim,TO_CHAR);
         return;
        }

    if (saves_spell(level,victim,DAM_NEGATIVE) ||
        (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
        if (ch == victim)
            send_to_char("Bir an için kendini hasta hissediyorsun, ama sonra geçiyor.\n\r",ch);
        else
            act("$N etkilenmiş görünmüyor.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_DETECTS;
    af.type      = sn;
    af.level     = level * 3/4;
    af.duration  = 3 + level / 30;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ADET_SUFFOCATE;
    affect_join(victim,&af);

    send_to_char ("Nefes alamıyorsun.\n\r",victim);
    act("$n nefes almaya çalışıyor, ama başaramıyor.", victim,NULL,NULL,TO_ROOM);

  return;
}

void spell_mummify( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    CHAR_DATA *undead;
    OBJ_DATA *obj,*obj2,*next;
    AFFECT_DATA af;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char *argument,*arg;
    int i;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

    if (!(obj->item_type == ITEM_CORPSE_NPC ||
          obj->item_type == ITEM_CORPSE_PC))
    {
      send_to_char("Yalnız cesetleri canlandırabilirsin!\n\r",ch);
	return;
    }

    if (obj->level > level + 10)
    {
      send_to_char("Ölü beden mumlayamayacağın kadar güçlü!\n\r",ch);
	return;
    }

    if (is_affected(ch, sn))
    {
      send_to_char( "Daha fazla cesedi mumyalayacak gücü bulamıyorsun.\n\r", ch);
      return;
    }

  if (count_charmed(ch)) return;

  if ( ch->in_room != NULL && IS_SET(ch->in_room->room_flags, ROOM_NO_MOB) )
  {
    send_to_char("Ölüleri burada mumyalayamazsın.\n\r", ch );
     return;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)      ||
       IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)   ||
       IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)  )
  {
    send_to_char( "Burada mumyalayamazsın.\n\r", ch );
    return;
  }

  undead = create_mobile( get_mob_index(MOB_VNUM_UNDEAD), NULL );
  char_to_room(undead,ch->in_room);
  for (i=0;i < MAX_STATS; i++)
    {
      undead->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);
    }

  undead->level = obj->level;
  undead->max_hit = (undead->level < 30) ? (undead->level * 30) :
			(undead->level < 60) ? (undead->level * 60) :
				(undead->level * 90);
  undead->hit = undead->max_hit;
  undead->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  undead->mana = undead->max_mana;
  undead->alignment = ch->alignment;

  for (i=0; i < 3; i++)
    undead->armor[i] = interpolate(undead->level,100,-100);
  undead->armor[3] = interpolate(undead->level,50,-200);
  undead->damage[DICE_NUMBER] = number_range(undead->level/20,undead->level/15);
  undead->damage[DICE_TYPE] = number_range(undead->level/6, undead->level/3);
  undead->damage[DICE_BONUS] = number_range(undead->level/12, undead->level/10);
  undead->sex = ch->sex;
  undead->silver = 0;

  SET_BIT(undead->act, ACT_UNDEAD);
  SET_BIT(undead->affected_by, AFF_CHARM);
  undead->master = ch;
  undead->leader = ch;

  snprintf(buf, sizeof(buf), "%s beden hortlak", obj->name);
  undead->name = str_dup(buf);
  snprintf(buf2, sizeof(buf2),"%s",obj->short_descr);
  argument = (char *)alloc_perm ( MAX_STRING_LENGTH );
  arg = (char *)alloc_perm ( MAX_STRING_LENGTH );
  argument = buf2;
  buf3[0] = '\0';
  while (argument[0] != '\0' )
	{
  argument = one_argument(argument, arg);
  if (!(!str_cmp(arg,"hortlak") || !str_cmp(arg,"beden") ||
	!str_cmp(arg,"ceset") ))
	 {
	  if (buf3[0] == '\0')   strcat(buf3,arg);
	  else  {
		 strcat(buf3," ");
		 strcat(buf3,arg);
		}
	 }
	}
  snprintf(buf, sizeof(buf), "%s'in mumyalanmış bedeni", buf3);
  undead->short_descr = str_dup(buf);
  snprintf(buf, sizeof(buf), "%s'in mumyalanmış bedeni aksayarak dolaşıyor.\n\r", buf3);
  undead->long_descr = str_dup(buf);

  for(obj2 = obj->contains;obj2;obj2=next)
    {
	next = obj2->next_content;
	obj_from_obj(obj2);
	obj_to_char(obj2, undead);
    }
    interpret( undead,"giy tümü", TRUE);

  af.where	 = TO_AFFECTS;
  af.type      = sn;
  af.level	 = ch->level;
  af.duration  = (ch->level / 10);
  af.modifier  = 0;
  af.bitvector = 0;
  af.location  = APPLY_NONE;
  affect_to_char( ch, &af );

  send_to_char("Mistik güçlerin yardımıyla ona yaşam veriyorsun!\n\r",ch);
  snprintf(buf, sizeof(buf),"Mistik güçlerin yardımıyla %s %s'i mumyalayarak ona yaşam veriyor!",
		ch->name,obj->name);
  act(buf,ch,NULL,NULL,TO_ROOM);
  snprintf(buf, sizeof(buf),"%s sana bakarak onu rahatsız etmenin bedelini ödetmenin planlarını yapıyor!",
	obj->short_descr);
  act(buf,ch,NULL,NULL,TO_CHAR);
  extract_obj (obj);
	return;
    }

  victim = (CHAR_DATA *) vo;

  if ( victim == ch )
  {
    send_to_char( "Fakat sen ölü değilsin!!\n\r", ch );
  	return;
    }

    send_to_char("O ölü değil!!\n\r", ch );
  return;
}

void spell_soul_bind( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (ch->pet != NULL)
  {
    send_to_char("Zaten ruhun birine bağlı.\n\r", ch);
	return;
  }

  if (!IS_NPC(victim)
	|| !(IS_AFFECTED(victim, AFF_CHARM) && victim->master == ch))
  {
    send_to_char("O ruhu kendine bağlayamazsın.\n\r", ch);
	return;
  }

  victim->leader = ch;
  ch->pet = victim;

  act("$S ruhunu kendine bağlıyorsun.", ch, NULL, victim, TO_CHAR);
  act("$n $S ruhunu kendine bağlıyor.", ch, NULL, victim, TO_ROOM);

  return;
}

/*
 * ADET_PROTECTOR etkisi (güç kafesi / demir beden / element küresi):
 * aynı bit paylaşıldığından biri varken diğeri atılamaz.
 */
static void protector_spell( CHAR_DATA *ch, CHAR_DATA *victim, int sn, int level,
			     const char *dup_self, const char *dup_other,
			     const char *msg_room, const char *msg_vict )
{
    if ( CAN_DETECT( victim, ADET_PROTECTOR ) )
    {
	if ( victim == ch )
	    send_to_char( dup_self, ch );
	else
	    act( dup_other, ch, NULL, victim, TO_CHAR );
	return;
    }
    char_affect( victim, TO_DETECTS, sn, level, level / 6, APPLY_NONE, 0, ADET_PROTECTOR );
    act( msg_room, victim, NULL, NULL, TO_ROOM );
    send_to_char( msg_vict, victim );
}

void spell_forcecage( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    protector_spell( ch, (CHAR_DATA *) vo, sn, level,
	"Zaten çevrende bir koruyucu güç var.\n\r",
	"$S çevresinde bir koruyucu güç zaten var.",
	"$n çevresinde bir kafes oluşturmak için unutulmuş güçleri çağırıyor.",
	"Çevrende bir kafes oluşturmak için unutulmuş güçleri çağırıyorsun.\n\r" );
}

void spell_iron_body( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    protector_spell( ch, (CHAR_DATA *) vo, sn, level,
	"Zaten çevrende bir koruyucu güç var.\n\r",
	"$S çevresinde bir koruyucu güç zaten var.",
	"$s derisi demir gibi sertleşti.",
	"Derin demir gibi sertleşti.\n\r" );
}

void spell_elemental_sphere( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    protector_spell( ch, (CHAR_DATA *) vo, sn, level,
	"Zaten çevrende bir koruyucu güç var.\n\r",
	"$S çevresinde bir koruyucu güç zaten var.",
	"$n doğayla ilgili tüm güçleri kendisine bir küre yapmak için kullanıyor.",
	"Doğayla ilgili tüm güçleri kendine bir küre yapmak için kullanıyorsun.\n\r" );
}


void spell_aura_of_chaos( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (ch->cabal != victim->cabal)
      {
        send_to_char("Bu büyüyü yalnız kabal dostların üzerinde kullanabilirsin.\n\r",ch);
	return;
      }

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
  send_to_char("Zaten kaos tanrılarınca korunuyorsun.\n\r",ch);
else
  act("$N zaten kaos tanrılarınca korunuyor.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where	 = TO_DETECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = ADET_AURA_CHAOS;
    affect_to_char( victim, &af );
    send_to_char(  "Kaos tanrılarınca korunduğunu hissediyorsun.\n\r", victim );
    if ( ch != victim )
    act("$S çevresinde bir aura oluşuyor.",ch,NULL,victim,TO_CHAR);
    return;
}
