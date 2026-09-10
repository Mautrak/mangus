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
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT		           *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Mandrake}	canpunar@rorqual.cc.metu.edu.tr    *
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
#include "interp.h"
#include "prog_util.h"

void one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool second);
bool cabal_area_check   (CHAR_DATA *ch);

DECLARE_OPROG_FUN_WEAR( wear_prog_excalibur 	);
DECLARE_OPROG_FUN_REMOVE( remove_prog_excalibur );
DECLARE_OPROG_FUN_DEATH( death_prog_excalibur 	);
DECLARE_OPROG_FUN_SPEECH( speech_prog_excalibur );
DECLARE_OPROG_FUN_SAC( sac_prog_excalibur 	);

DECLARE_OPROG_FUN_SAC( sac_prog_cabal_item 	);
DECLARE_OPROG_FUN_GET( get_prog_cabal_item 	);

DECLARE_OPROG_FUN_FIGHT( fight_prog_sub_weapon  );
DECLARE_OPROG_FUN_SPEECH( speech_prog_kassandra );

DECLARE_OPROG_FUN_FIGHT( fight_prog_chaos_blade );
DECLARE_OPROG_FUN_DEATH( death_prog_chaos_blade );

DECLARE_OPROG_FUN_FIGHT( fight_prog_tattoo_kame );
DECLARE_OPROG_FUN_FIGHT( fight_prog_tattoo_nir );
DECLARE_OPROG_FUN_FIGHT( fight_prog_tattoo_nyah );
DECLARE_OPROG_FUN_FIGHT( fight_prog_tattoo_sint );

DECLARE_OPROG_FUN_FIGHT( fight_prog_golden_weapon );
DECLARE_OPROG_FUN_DEATH( death_prog_golden_weapon );

DECLARE_OPROG_FUN_GET( get_prog_heart 		);

DECLARE_OPROG_FUN_WEAR( wear_prog_bracer 	);
DECLARE_OPROG_FUN_REMOVE( remove_prog_bracer 	);

DECLARE_OPROG_FUN_WEAR(wear_prog_ranger_staff	);
DECLARE_OPROG_FUN_FIGHT( fight_prog_ranger_staff );
DECLARE_OPROG_FUN_DEATH( death_prog_ranger_staff );

DECLARE_OPROG_FUN_WEAR(wear_prog_coconut	);
DECLARE_OPROG_FUN_ENTRY(entry_prog_coconut	);
DECLARE_OPROG_FUN_GREET(greet_prog_coconut	);
DECLARE_OPROG_FUN_GET(get_prog_coconut		);
DECLARE_OPROG_FUN_REMOVE(remove_prog_coconut	);

DECLARE_OPROG_FUN_FIGHT(fight_prog_firegauntlets);
DECLARE_OPROG_FUN_WEAR(wear_prog_firegauntlets	);
DECLARE_OPROG_FUN_REMOVE(remove_prog_firegauntlets);
/* ibrahim armbands */
DECLARE_OPROG_FUN_FIGHT(fight_prog_armbands	);
DECLARE_OPROG_FUN_WEAR(wear_prog_armbands	);
DECLARE_OPROG_FUN_REMOVE(remove_prog_armbands	);

DECLARE_OPROG_FUN_FIGHT(fight_prog_demonfireshield);
DECLARE_OPROG_FUN_WEAR(wear_prog_demonfireshield);
DECLARE_OPROG_FUN_REMOVE(remove_prog_demonfireshield);

DECLARE_OPROG_FUN_FIGHT(fight_prog_vorbalblade	);
DECLARE_OPROG_FUN_GET(get_prog_spec_weapon	);
DECLARE_OPROG_FUN_FIGHT(fight_prog_shockwave	);
DECLARE_OPROG_FUN_FIGHT(fight_prog_snake 	);
/* new ones by chronos */
DECLARE_OPROG_FUN_WEAR( wear_prog_wind_boots 	);
DECLARE_OPROG_FUN_REMOVE( remove_prog_wind_boots);

DECLARE_OPROG_FUN_WEAR( wear_prog_arm_hercules );
DECLARE_OPROG_FUN_REMOVE( remove_prog_arm_hercules );

DECLARE_OPROG_FUN_WEAR( wear_prog_girdle_giant );
DECLARE_OPROG_FUN_REMOVE( remove_prog_girdle_giant );

DECLARE_OPROG_FUN_WEAR( wear_prog_breastplate_strength );
DECLARE_OPROG_FUN_REMOVE( remove_prog_breastplate_strength );

DECLARE_OPROG_FUN_WEAR( wear_prog_boots_flying );
DECLARE_OPROG_FUN_REMOVE( remove_prog_boots_flying );

DECLARE_OPROG_FUN_FIGHT( fight_prog_rose_shield );
DECLARE_OPROG_FUN_FIGHT( fight_prog_lion_claw );

DECLARE_OPROG_FUN_SPEECH( speech_prog_ring_ra );
DECLARE_OPROG_FUN_WEAR( wear_prog_eyed_sword );
DECLARE_OPROG_FUN_WEAR( wear_prog_katana_sword );

DECLARE_OPROG_FUN_WEAR( wear_prog_snake 	);
DECLARE_OPROG_FUN_REMOVE( remove_prog_snake 	);
DECLARE_OPROG_FUN_GET( get_prog_snake 		);

DECLARE_OPROG_FUN_WEAR( wear_prog_fire_shield 	);
DECLARE_OPROG_FUN_REMOVE( remove_prog_fire_shield );
DECLARE_OPROG_FUN_WEAR( wear_prog_quest_weapon );
DECLARE_OPROG_FUN_GET(get_prog_quest_reward	);

DECLARE_OPROG_FUN_FIGHT(fight_prog_ancient_gloves);
DECLARE_OPROG_FUN_WEAR(wear_prog_ancient_gloves	);
DECLARE_OPROG_FUN_REMOVE(remove_prog_ancient_gloves);

DECLARE_OPROG_FUN_FIGHT(fight_prog_ancient_shield);
DECLARE_OPROG_FUN_WEAR(wear_prog_ancient_shield	);
DECLARE_OPROG_FUN_REMOVE(remove_prog_ancient_shield);

DECLARE_OPROG_FUN_WEAR( wear_prog_neckguard );
DECLARE_OPROG_FUN_REMOVE( remove_prog_neckguard );

DECLARE_OPROG_FUN_WEAR( wear_prog_headguard );
DECLARE_OPROG_FUN_REMOVE( remove_prog_headguard );

DECLARE_OPROG_FUN_WEAR( wear_prog_blackguard );
DECLARE_OPROG_FUN_REMOVE( remove_prog_blackguard );

void    raw_kill        ( CHAR_DATA *victim );
void    raw_kill_org    ( CHAR_DATA *victim, int part );

/*
 * Eşya prog tablosu: yeni prog eklemek için işlevi yazıp buraya bir satır ekle.
 * Ortak çekirdek (prog_lookup) mob_prog.c'dedir.
 */
static const struct prog_type oprog_types[] =
{
    { "wear_prog",   OPROG_WEAR   },
    { "remove_prog", OPROG_REMOVE },
    { "get_prog",    OPROG_GET    },
    { "drop_prog",   OPROG_DROP   },
    { "sac_prog",    OPROG_SAC    },	/* TRUE dönerse eşya yok olmaz */
    { "entry_prog",  OPROG_ENTRY  },
    { "give_prog",   OPROG_GIVE   },
    { "greet_prog",  OPROG_GREET  },
    { "fight_prog",  OPROG_FIGHT  },
    { "death_prog",  OPROG_DEATH  },	/* TRUE dönerse ölümü engeller */
    { "speech_prog", OPROG_SPEECH },
    { "area_prog",   OPROG_AREA   },
    { NULL, 0 }
};

#define OPROG(bit, fun)	{ #fun, bit, (void (*)(void)) fun }

static const struct prog_entry oprog_table[] =
{
    OPROG(OPROG_WEAR,   wear_prog_excalibur),
    OPROG(OPROG_WEAR,   wear_prog_bracer),
    OPROG(OPROG_WEAR,   wear_prog_coconut),
    OPROG(OPROG_WEAR,   wear_prog_firegauntlets),
    OPROG(OPROG_WEAR,   wear_prog_armbands),
    OPROG(OPROG_WEAR,   wear_prog_demonfireshield),
    OPROG(OPROG_WEAR,   wear_prog_ranger_staff),
    OPROG(OPROG_WEAR,   wear_prog_wind_boots),
    OPROG(OPROG_WEAR,   wear_prog_boots_flying),
    OPROG(OPROG_WEAR,   wear_prog_arm_hercules),
    OPROG(OPROG_WEAR,   wear_prog_girdle_giant),
    OPROG(OPROG_WEAR,   wear_prog_breastplate_strength),
    OPROG(OPROG_WEAR,   wear_prog_katana_sword),
    OPROG(OPROG_WEAR,   wear_prog_eyed_sword),
    OPROG(OPROG_WEAR,   wear_prog_snake),
    OPROG(OPROG_WEAR,   wear_prog_fire_shield),
    OPROG(OPROG_WEAR,   wear_prog_quest_weapon),
    OPROG(OPROG_WEAR,   wear_prog_ancient_gloves),
    OPROG(OPROG_WEAR,   wear_prog_ancient_shield),
    OPROG(OPROG_WEAR,   wear_prog_neckguard),
    OPROG(OPROG_WEAR,   wear_prog_headguard),
    OPROG(OPROG_WEAR,   wear_prog_blackguard),

    OPROG(OPROG_REMOVE, remove_prog_excalibur),
    OPROG(OPROG_REMOVE, remove_prog_bracer),
    OPROG(OPROG_REMOVE, remove_prog_coconut),
    OPROG(OPROG_REMOVE, remove_prog_firegauntlets),
    OPROG(OPROG_REMOVE, remove_prog_armbands),
    OPROG(OPROG_REMOVE, remove_prog_demonfireshield),
    OPROG(OPROG_REMOVE, remove_prog_wind_boots),
    OPROG(OPROG_REMOVE, remove_prog_boots_flying),
    OPROG(OPROG_REMOVE, remove_prog_arm_hercules),
    OPROG(OPROG_REMOVE, remove_prog_girdle_giant),
    OPROG(OPROG_REMOVE, remove_prog_breastplate_strength),
    OPROG(OPROG_REMOVE, remove_prog_snake),
    OPROG(OPROG_REMOVE, remove_prog_fire_shield),
    OPROG(OPROG_REMOVE, remove_prog_ancient_gloves),
    OPROG(OPROG_REMOVE, remove_prog_ancient_shield),
    OPROG(OPROG_REMOVE, remove_prog_neckguard),
    OPROG(OPROG_REMOVE, remove_prog_headguard),
    OPROG(OPROG_REMOVE, remove_prog_blackguard),

    OPROG(OPROG_GET,    get_prog_cabal_item),
    OPROG(OPROG_GET,    get_prog_heart),
    OPROG(OPROG_GET,    get_prog_coconut),
    OPROG(OPROG_GET,    get_prog_spec_weapon),
    OPROG(OPROG_GET,    get_prog_snake),
    OPROG(OPROG_GET,    get_prog_quest_reward),

    OPROG(OPROG_SAC,    sac_prog_excalibur),
    OPROG(OPROG_SAC,    sac_prog_cabal_item),

    OPROG(OPROG_ENTRY,  entry_prog_coconut),
    OPROG(OPROG_GREET,  greet_prog_coconut),

    OPROG(OPROG_FIGHT,  fight_prog_ranger_staff),
    OPROG(OPROG_FIGHT,  fight_prog_sub_weapon),
    OPROG(OPROG_FIGHT,  fight_prog_chaos_blade),
    OPROG(OPROG_FIGHT,  fight_prog_tattoo_kame),
    OPROG(OPROG_FIGHT,  fight_prog_tattoo_nir),
    OPROG(OPROG_FIGHT,  fight_prog_tattoo_nyah),
    OPROG(OPROG_FIGHT,  fight_prog_tattoo_sint),
    OPROG(OPROG_FIGHT,  fight_prog_golden_weapon),
    OPROG(OPROG_FIGHT,  fight_prog_snake),
    OPROG(OPROG_FIGHT,  fight_prog_shockwave),
    OPROG(OPROG_FIGHT,  fight_prog_firegauntlets),
    OPROG(OPROG_FIGHT,  fight_prog_armbands),
    OPROG(OPROG_FIGHT,  fight_prog_demonfireshield),
    OPROG(OPROG_FIGHT,  fight_prog_vorbalblade),
    OPROG(OPROG_FIGHT,  fight_prog_rose_shield),
    OPROG(OPROG_FIGHT,  fight_prog_lion_claw),
    OPROG(OPROG_FIGHT,  fight_prog_ancient_gloves),
    OPROG(OPROG_FIGHT,  fight_prog_ancient_shield),

    OPROG(OPROG_DEATH,  death_prog_excalibur),
    OPROG(OPROG_DEATH,  death_prog_ranger_staff),
    OPROG(OPROG_DEATH,  death_prog_chaos_blade),
    OPROG(OPROG_DEATH,  death_prog_golden_weapon),

    OPROG(OPROG_SPEECH, speech_prog_excalibur),
    OPROG(OPROG_SPEECH, speech_prog_kassandra),
    OPROG(OPROG_SPEECH, speech_prog_ring_ra),

    { NULL, 0, NULL }
};

void oprog_set(OBJ_INDEX_DATA *objindex, const char *progtype, const char *name)
{
    const struct prog_entry *e =
	prog_lookup('O', objindex->vnum, oprog_types, oprog_table, progtype, name);
    OPROG_DATA *p = objindex->oprogs;

    switch (e->bit)
    {
    case OPROG_WEAR:   p->wear_prog   = (OPROG_FUN_WEAR *)   e->fun; break;
    case OPROG_REMOVE: p->remove_prog = (OPROG_FUN_REMOVE *) e->fun; break;
    case OPROG_GET:    p->get_prog    = (OPROG_FUN_GET *)    e->fun; break;
    case OPROG_DROP:   p->drop_prog   = (OPROG_FUN_DROP *)   e->fun; break;
    case OPROG_SAC:    p->sac_prog    = (OPROG_FUN_SAC *)    e->fun; break;
    case OPROG_ENTRY:  p->entry_prog  = (OPROG_FUN_ENTRY *)  e->fun; break;
    case OPROG_GIVE:   p->give_prog   = (OPROG_FUN_GIVE *)   e->fun; break;
    case OPROG_GREET:  p->greet_prog  = (OPROG_FUN_GREET *)  e->fun; break;
    case OPROG_FIGHT:  p->fight_prog  = (OPROG_FUN_FIGHT *)  e->fun; break;
    case OPROG_DEATH:  p->death_prog  = (OPROG_FUN_DEATH *)  e->fun; break;
    case OPROG_SPEECH: p->speech_prog = (OPROG_FUN_SPEECH *) e->fun; break;
    case OPROG_AREA:   p->area_prog   = (OPROG_FUN_AREA *)   e->fun; break;
    }
    SET_BIT(objindex->progtypes, e->bit);
}

/*
 * Yerel yardımcılar
 */

/* Seviyeye göre kalıcı nitelik artışı: 1 (+18) +30 +45 */
static int level_mod(CHAR_DATA *ch)
{
    return 1 + (ch->level >= 18) + (ch->level >= 30) + (ch->level >= 45);
}

/* Eşyaya bağlı kalıcı (duration -2) efekt; zaten varsa FALSE döner. */
static bool oprog_affect_on(CHAR_DATA *ch, int sn, int where, int bitvector, int location, int modifier)
{
    AFFECT_DATA af;

    if (is_affected(ch, sn))
	return FALSE;

    af.where = where;
    af.type = sn;
    af.duration = -2;
    af.level = ch->level;
    af.bitvector = bitvector;
    af.location = location;
    af.modifier = modifier;
    affect_to_char(ch, &af);
    return TRUE;
}

static bool oprog_affect_off(CHAR_DATA *ch, int sn)
{
    if (!is_affected(ch, sn))
	return FALSE;
    affect_strip(ch, sn);
    return TRUE;
}

/* Eşyanın (yoksa şablonunun) ilk ek açıklaması; hiç yoksa "" */
static const char *obj_extra_text(OBJ_DATA *obj)
{
    if (obj->extra_descr != NULL && obj->extra_descr->description != NULL)
	return obj->extra_descr->description;
    if (obj->pIndexData->extra_descr != NULL && obj->pIndexData->extra_descr->description != NULL)
	return obj->pIndexData->extra_descr->description;
    return "";
}

/* Giyildiğinde silah zarı seviyeye göre ayarlanır */
static void set_weapon_dice(OBJ_DATA *obj, CHAR_DATA *ch)
{
    obj->value[2] = prog_weapon_dice(ch->level);
}

/* Sahibi olmayanın elinden düşen eşya */
static void drop_unowned(OBJ_DATA *obj, CHAR_DATA *ch)
{
    act( "Sen $p tarafından çarpıldın ve onu düşürdün.", ch, obj, NULL, TO_CHAR );
    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
}

/* Din dövmesi parlaması: saldırıda kızıl, şifada mavi (renk kodu verilen) */
static void tattoo_glow(CHAR_DATA *ch, bool attack, const char *color)
{
    act_color(attack ? "$COmzundaki dövme kızıl renkte parlıyor.$c"
		     : "$COmzundaki dövme mavi renkte parlıyor.$c",
	      ch, NULL, NULL, TO_CHAR, POS_DEAD, color);
}

/* Kalkanların/eldivenlerin alev vuruşu (ateş etkisi + hasar) */
static void fire_strike(CHAR_DATA *ch, int dam, int fire_level, int sn)
{
    fire_effect( ch->fighting, fire_level, dam, TARGET_CHAR );
    damage( ch, ch->fighting, dam, sn, DAM_FIRE, TRUE);
}

/*
 * Prog işlevleri
 */

void wear_prog_excalibur(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act("$p parlak beyaz renkte parlamaya başladı.",ch,obj,NULL,TO_CHAR);
  act("$p parlak beyaz renkte parlamaya başladı.",ch,obj,NULL,TO_ROOM);
  set_weapon_dice(obj, ch);
}

void wear_prog_bracer(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (oprog_affect_on(ch, gsn_haste, TO_AFFECTS, AFF_HASTE, APPLY_DEX, level_mod(ch)))
    {
      send_to_char( "Taktığın bileklikler seninle bütünleşiyor.\n\r", ch);
      send_to_char("Ellerin ve kolların inanılmaz derecede hafifledi.\n\r", ch);
    }
}

void remove_prog_bracer(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (oprog_affect_off(ch, gsn_haste))
      send_to_char("Ellerin ve kolların ağırlaştı.\n\r", ch);
}

void remove_prog_excalibur(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act("$p artık parlamıyor.",ch,obj,NULL,TO_CHAR);
  act("$p artık parlamıyor.",ch,obj,NULL,TO_ROOM);
}

bool death_prog_excalibur(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act_new("$p mavi bir ayla ile parlıyor.",ch,obj,NULL,TO_CHAR,POS_DEAD);
  act("$p mavi bir ayla ile parlıyor,",ch,obj,NULL,TO_ROOM);
  ch->hit = ch->max_hit;
  send_to_char("Kendini daha iyi hissediyorsun.\n\r",ch);
  act("$n daha iyi görünüyor.",ch,NULL,NULL,TO_ROOM);
  return TRUE;
}

void speech_prog_excalibur(OBJ_DATA *obj, CHAR_DATA *ch, char *speech)
{
  if (!str_cmp(speech, "asit")
      && (ch->fighting) && is_wielded_char(ch,obj)  )
    {
      send_to_char("Excalibur'un bıçağından asit fışkırıyor.\n\r",ch);
      act("Excalibur'un bıçağından asit fışkırıyor.",ch,NULL,NULL,TO_ROOM);
      obj_cast_spell(gsn_acid_blast,ch->level,ch,ch->fighting,NULL);
      WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    }
}

bool sac_prog_excalibur(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act("Ölümsüzler çileden çıktı!",ch,NULL,NULL,TO_CHAR);
  act("Ölümsüzler çileden çıktı!",ch,NULL,NULL,TO_ROOM);
  damage(ch,ch,
	 (ch->hit - 1) > 1000? 1000 : (ch->hit - 1),
	 TYPE_HIT,DAM_HOLY, TRUE);
  ch->silver = 0;
  return TRUE;
}

void fight_prog_ranger_staff(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if ( is_wielded_char(ch,obj) && number_percent() < 10)
    {
      send_to_char("Korucu asan mavi renkte parlamaya başlıyor!\n\r",ch);
      act("$s korucu asası mavi renkte parlamaya başlıyor!",ch,NULL,NULL,TO_ROOM);

      obj_cast_spell(gsn_cure_critical,ch->level,ch,ch,obj);
    }
}

void fight_prog_sub_weapon(OBJ_DATA *obj, CHAR_DATA *ch)
{
  float ratio;

  if (!is_wielded_char(ch,obj) || number_percent() >= 30)
    return;

  ratio = ((float) ch->hit) / ((float) UMAX(1, ch->max_hit));
  if (ratio > 0.9)
    send_to_char("Silahın fısıldıyor '{yİyi iş çıkarıyorsun!{x'.\n\r", ch);
  else if (ratio > 0.6)
    send_to_char("Silahın fısıldıyor '{yBöyle devam et!{x'.\n\r", ch);
  else if (ratio > 0.4)
    send_to_char("Silahın fısıldıyor '{yBaşarabilirsin!{x'.\n\r", ch);
  else
    send_to_char("Silahın fısıldıyor '{yKaç! Kaç!{x'.\n\r", ch);
}

bool death_prog_ranger_staff(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Korucu asan yokoluyor.\n\r",ch);
  act("$s korucu asası yokoluyor.",ch,NULL,NULL,TO_ROOM);
  extract_obj(obj);
  return FALSE;
}

void get_prog_spec_weapon(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if ( prog_obj_owned_by(obj, ch) || IS_IMMORTAL(ch))
  {
    if ( IS_AFFECTED( ch, AFF_POISON ) && (dice(1,5)==1) )  {
      send_to_char( "Silahın mavi renkte parlıyor.\n\r", ch );
      act( "$s silahı mavi renkte parlıyor.", ch, NULL, NULL, TO_ROOM );
      spell_cure_poison( gsn_cure_poison, 30, ch, ch, TARGET_CHAR );
      return;
    }
    if ( IS_AFFECTED( ch, AFF_CURSE ) && (dice(1,5)==1) )  {
      send_to_char( "Silahın mavi renkte parlıyor.\n\r", ch );
      act( "$s silahı mavi renkte parlıyor.", ch, NULL, NULL, TO_ROOM );
      spell_remove_curse( gsn_remove_curse, 30, ch, ch, TARGET_CHAR );
      return;
    }
    send_to_char( "Silahın daha güçlü vızıldamaya başladı.\n\r", ch );
    return;
  }

  drop_unowned(obj, ch);

  switch( dice(1, 10) )  {
  case 1:
    spell_curse( gsn_curse, ch->level < 10? 1 : ch->level-9, ch, ch, TARGET_CHAR );
    break;
  case 2:
    spell_poison( gsn_poison, ch->level < 10? 1 : ch->level-9, ch, ch, TARGET_CHAR );
    break;
  }
}

void get_prog_quest_reward(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if ( prog_obj_owned_by(obj, ch) )
  {
    act_color("$C$p parlamaya başladı.\n\r$c",
		ch,obj,NULL,TO_CHAR,POS_SLEEPING,CLR_BLUE);
    return;
  }

  drop_unowned(obj, ch);
}

void get_prog_cabal_item(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (IS_NPC(ch))
  {
    act("Taşımayı haketmediğin $p elinden düşüyor.", ch, obj, NULL, TO_CHAR);
    act("$s taşımayı haketmediği $p elinden düşüyor.", ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    return;
  }

  if (obj->timer < 1)
  {
    obj->timer = 30;
    act("$p saydamlaştı.", ch, obj, NULL, TO_CHAR);
    act("$p saydamlaştı.", ch, obj, NULL, TO_ROOM);
  }
}

/* Kabal eşyasının yeniden oluştuğu sunak/kaide (0: Shalafi sunağı) */
static const int cabal_altar_vnum[MAX_CABAL] =
{
    [CABAL_RULER]   = OBJ_VNUM_RULER_STAND,
    [CABAL_INVADER] = OBJ_VNUM_INVADER_SKULL,
    [CABAL_BATTLE]  = OBJ_VNUM_BATTLE_THRONE,
    [CABAL_KNIGHT]  = OBJ_VNUM_KNIGHT_ALTAR,
    [CABAL_CHAOS]   = OBJ_VNUM_CHAOS_ALTAR,
    [CABAL_LIONS]   = OBJ_VNUM_LIONS_ALTAR,
    [CABAL_HUNTER]  = OBJ_VNUM_HUNTER_ALTAR,
};

bool sac_prog_cabal_item(OBJ_DATA *obj, CHAR_DATA *ch)
{
  OBJ_DATA *container;
  ROOM_INDEX_DATA *room;
  char buf[MAX_INPUT_LENGTH];
  int i, altar;

  act("Ölümsüzler çileden çıktı!",ch,NULL,NULL,TO_CHAR);
  act("Ölümsüzler çileden çıktı!",ch,NULL,NULL,TO_ROOM);
  damage(ch,ch,(int)(ch->hit/10),TYPE_HIT,DAM_HOLY, TRUE);
  ch->silver = 0;

  obj_from_room(obj);
  for(i=0;i<MAX_CABAL;i++)
    if (cabal_table[i].obj_ptr == obj) break;

  if ( i >= MAX_CABAL )
  {
    extract_obj(obj);
    bug( "oprog: Sac_cabal_item: Was not the cabal's item.", 0);
    return FALSE;
  }

  altar = cabal_altar_vnum[i] != 0 ? cabal_altar_vnum[i] : OBJ_VNUM_SHALAFI_ALTAR;
  room = get_room_index(cabal_table[i].room_vnum);
  if (room == NULL || (container = prog_create_object(altar, 100)) == NULL)
  {
    extract_obj(obj);
    bug("oprog: Sac_cabal_item: cabal %d için sunak ya da oda yok.", i);
    return FALSE;
  }

  obj_to_obj( obj, container );
  obj_to_room( container, room );
  snprintf(buf, sizeof(buf), "%s yavaşça tekrar oluşuyor.\n\r", container->short_descr );
  if ( room->people != NULL )
  {
    act( buf, room->people,NULL,NULL, TO_CHAR);
    act( buf, room->people,NULL,NULL, TO_ROOM);
  }
  return TRUE;
}

void speech_prog_kassandra(OBJ_DATA *obj, CHAR_DATA *ch, char *speech)
{
  if (IS_NPC(ch) || get_hold_char(ch) != obj)
    return;

  if (!str_cmp(speech, "kassandra"))
    obj_cast_spell(gsn_kassandra,ch->level,ch,ch,NULL);
  else if (!str_cmp(speech, "sebat"))
    obj_cast_spell(gsn_sebat,ch->level,ch,ch,NULL);
  else if (!str_cmp(speech, "matandra") && ch->fighting)
    {
      act("Bir enerji topu elinden $E fırlıyor!",
	  ch,NULL,ch->fighting,TO_CHAR);
      act("Bir enerji topu $s elinden sana fırlıyor!",
	  ch,NULL,ch->fighting,TO_VICT);
      act("Bir enerji topu $s elinden $E fırlıyor!",
	  ch,NULL,ch->fighting,TO_NOTVICT);
      obj_cast_spell(gsn_matandra,ch->level,ch,ch->fighting,NULL);
    }
}

void fight_prog_chaos_blade(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if ( !is_wielded_char(ch,obj) )
    return;

  switch(number_range(0,127)) {
  case 0:
    act("Yatağan titriyor!", ch, NULL, NULL, TO_ROOM);
    send_to_char("Yatağanın titriyor!\n\r", ch);
    obj_cast_spell(gsn_mirror,ch->level,ch,ch,obj);
    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    break;

  case 1:
    act("Yatağan biraz sallanıyor.", ch, NULL, NULL, TO_ROOM);
    send_to_char("Yatağanın biraz sallanıyor.\n\r", ch);
    obj_cast_spell(gsn_garble,ch->level,ch,ch->fighting,obj);
    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    break;

  case 2:
    act("Yatağan şiddetle titriyor!",ch,NULL,NULL,TO_ROOM);
    send_to_char("Yatağanın şiddetle titriyor!\n\r",ch);
    obj_cast_spell(gsn_confuse, ch->level,ch,ch->fighting,obj);
    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    break;
  }
}

bool death_prog_chaos_blade(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Yatağanın yokoluyor.\n\r",ch);
  act("$s yatağanı yokoluyor.",ch,NULL,NULL,TO_ROOM);
  extract_obj(obj);
  return FALSE;
}

void fight_prog_tattoo_kame(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (get_eq_char(ch, WEAR_TATTOO) != obj)
    return;

  switch(number_range(0,63)) {
  case 0:
  case 1:
    tattoo_glow(ch, FALSE, CLR_CYAN);
    obj_cast_spell(gsn_cure_serious, ch->level, ch, ch, obj);
    break;
  case 2:
    tattoo_glow(ch, TRUE, CLR_RED);
    do_yell( ch, "Kutsal güçle dans etme....");
    spell_holy_word(skill_lookup("holy word"),ch->level,ch,NULL,TARGET_CHAR);
    break;
  }
}

void fight_prog_tattoo_nir(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (get_eq_char(ch, WEAR_TATTOO) != obj)
    return;

  switch(number_range(0,63)) {
  case 0:
  case 1:
  case 2:
    tattoo_glow(ch, FALSE, CLR_CYAN);
    obj_cast_spell(gsn_cure_critical, ch->level, ch, ch, obj);
    break;
  case 3:
    tattoo_glow(ch, FALSE, CLR_CYAN);
    if (IS_AFFECTED(ch,AFF_PLAGUE))
      spell_cure_disease(skill_lookup("cure disease"),100,ch,ch,TARGET_CHAR);
    if (IS_AFFECTED(ch,AFF_POISON))
      spell_cure_poison(gsn_cure_poison,100,ch,ch,TARGET_CHAR);
    break;
  }
}

void fight_prog_tattoo_nyah(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (get_eq_char(ch, WEAR_TATTOO) != obj)
    return;

  switch(number_range(0,63)) {
  case 0:
    tattoo_glow(ch, FALSE, CLR_CYAN);
    obj_cast_spell(gsn_cure_serious, ch->level, ch,ch, obj);
    break;
  case 1:
    tattoo_glow(ch, TRUE, CLR_RED);
    spell_bluefire(gsn_bluefire, ch->level, ch, ch->fighting, TARGET_CHAR);
    break;
  }
}

void fight_prog_tattoo_sint(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (get_eq_char(ch, WEAR_TATTOO) != obj)
    return;

  switch(number_range(0,63)) {
  case 0:
    tattoo_glow(ch, FALSE, CLR_BLUE);
    obj_cast_spell(gsn_cure_serious, ch->level, ch, ch, obj);
    break;
  case 1:
    tattoo_glow(ch, TRUE, CLR_RED);
    obj_cast_spell(gsn_demonfire, ch->level, ch, ch->fighting, obj);
    break;
  }
}

bool death_prog_golden_weapon(OBJ_DATA *obj, CHAR_DATA *ch)
{
  ROOM_INDEX_DATA *room;

  send_to_char("Altın silahın yokoluyor.\n\r",ch);
  act("$s altın silahı yok oluyor.",ch,NULL,NULL,TO_ROOM);
  extract_obj(obj);
  ch->hit = 1;
  while ( ch->affected )
    affect_remove( ch, ch->affected );
  ch->last_fight_time = -1;
  ch->last_death_time = current_time;
  if (cabal_area_check(ch)
  &&  (room = get_room_index( cabal_table[CABAL_KNIGHT].room_vnum )) != NULL)
  {
    act("$n yokoluyor.",ch,NULL,NULL,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, room);
    act("$n odada beliriyor.",ch,NULL,NULL,TO_ROOM);
  }
  return TRUE;
}

void fight_prog_golden_weapon(OBJ_DATA *obj, CHAR_DATA *ch)
{
  int sn;

  if ( !is_wielded_char(ch,obj) )
    return;

  if (number_percent() < 4)
    sn = gsn_cure_critical;
  else if (number_percent() > 92)
    sn = gsn_cure_serious;
  else
    return;

  act("$p parlak mavi renkte parlıyor!\n\r",ch, obj, NULL, TO_CHAR);
  act("$s $p parlak mavi renkte parlıyor!",ch,obj,NULL,TO_ROOM);
  obj_cast_spell(sn,ch->level,ch,ch,obj);
}

void get_prog_heart(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (obj->timer == 0)
    obj->timer = 24;
}

void fight_prog_snake(OBJ_DATA *obj, CHAR_DATA *ch)
{
  int sn;

  if ( !is_wielded_char(ch,obj) )
    return;

  switch(number_range(0,127)) {
  case 0:  sn = gsn_poison; break;
  case 1:  sn = gsn_weaken; break;
  default: return;
  }

  act("Kırbacındaki yılanlardan biri $M ısırıyor!", ch, NULL, ch->fighting, TO_CHAR);
  act("$s kırbacındaki yılanlardan biri seni ısırıyor!", ch, NULL, ch->fighting, TO_VICT);
  act("$s kırbacındaki yılanlardan biri $E saldırıyor!", ch, NULL, ch->fighting, TO_NOTVICT);
  obj_cast_spell(sn, ch->level, ch, ch->fighting, obj);
}

void fight_prog_shockwave(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if ( !is_wielded_char(ch,obj) || number_range(0,31) != 0 )
    return;

  act("Silahından fırlayan bir yıldırım $M vuruyor!", ch, NULL, ch->fighting, TO_CHAR);
  act("$s silahından fırlayan bir yıldırım sana atlıyor!", ch, NULL, ch->fighting, TO_VICT);
  act("$s silahından fırlayan bir yıldırım $E atlıyor!", ch, NULL, ch->fighting, TO_NOTVICT);
  obj_cast_spell(gsn_lightning_bolt, ch->level, ch, ch->fighting, NULL);
}

void wear_prog_ranger_staff(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if ( ch->iclass != CLASS_RANGER )
  {
    send_to_char("Onu nasıl kullanacağını bilmiyorsun.\n\r", ch );
    unequip_char( ch, obj );
    send_to_char("Korucu asası elinden kayıp düşüyor.\n\r", ch );
    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
  }
}

void wear_prog_coconut(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act("Hindistancevizi kabuklarını birbirine vurmaya başlıyorsun.",ch,NULL,NULL,TO_CHAR);
  act("Dörtnala gelen atların sesini duyunca bineğine atlıyorsun.",
	ch, NULL, NULL, TO_CHAR);
  act("$n görünmez bir atı sürüyormuş rolü yapıyor.",
	ch,NULL,NULL,TO_ROOM);
}

void entry_prog_coconut(OBJ_DATA *obj)
{
  if (obj->carried_by != NULL && get_hold_char(obj->carried_by) == obj)
    act("$n iki hindistancevizini birbirine vurup görünmez bineğiyle dörtnala atılıyor.",
	obj->carried_by, NULL, NULL, TO_ROOM);
}

void greet_prog_coconut(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (obj->carried_by != NULL)
    {
      if (get_hold_char(obj->carried_by) == obj && obj->carried_by != ch)
	act("Dörtnala giden atların sesini duyuyorsun.", ch, NULL, NULL, TO_CHAR);
    }
  else
    act("$p dörtnala giden atların zayıf sesiyle birini çağırıyor.", ch, obj, NULL, TO_CHAR);
}

void get_prog_coconut(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char(
"Hindistancevizini kulağına tutunca dörtnala giden atların\n\rzayıf kükreyişini duyuyorsun.\n\r", ch);
  act("$n bir hindistancevizini kulağına tutuyor.", ch, NULL, NULL, TO_ROOM);
}

void remove_prog_coconut(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Atların sesi yokoluyor.\n\r", ch);
  act("$n bir attan iniyormuş numarası yapıyor.", ch, NULL, NULL, TO_ROOM);
}

void fight_prog_firegauntlets( OBJ_DATA *obj, CHAR_DATA *ch )
{
  int dam;

  if (get_wield_char(ch,FALSE) != NULL || get_eq_char( ch, WEAR_HANDS ) != obj)
	return;

  if ( number_percent() < 50 )  {
	dam = dice( ch->level, 8) + number_percent() / 2;
	act( "Eldivenlerin $S yüzünü yakıyor!", ch, NULL, ch->fighting, TO_CHAR);
	act("$s eldivenleri $S yüzünü yakıyor!", ch, NULL, ch->fighting, TO_NOTVICT);
	act( "$S eldivenleri yüzünü yakıyor!", ch->fighting, NULL, ch, TO_CHAR);
	damage( ch, ch->fighting, dam/2, gsn_burning_hands, DAM_FIRE, TRUE);
	if ( ch->fighting == NULL )
	  return;
	fire_effect( ch->fighting, obj->level/2, dam/2, TARGET_CHAR );
  }
}

void wear_prog_firegauntlets(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Eldivenler ellerini ısıtıyor.\n\r", ch );
}

void remove_prog_firegauntlets(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Ellerin soğuyor.\n\r", ch );
}

void fight_prog_armbands( OBJ_DATA *obj, CHAR_DATA *ch )
{
  int dam;

  if ( get_eq_char( ch, WEAR_ARMS ) != obj || IS_NPC(ch) )
	return;

  if ( number_percent() < 20 )  {
	dam = number_percent()/2 + 30 + 5*ch->level;
	act( "Kolbantların $S yüzünü yakıyor!", ch, NULL, ch->fighting, TO_CHAR);
	act( "$s kolbantları $S yüzünü yakıyor!", ch, NULL, ch->fighting, TO_NOTVICT);
	act( "$S kolbantları senin yüzünü yakıyor!", ch->fighting, NULL, ch, TO_CHAR);
	damage( ch, ch->fighting, dam, gsn_burning_hands, DAM_FIRE, TRUE);
	if ( ch->fighting == NULL )
	  return;
	fire_effect( ch->fighting, obj->level/2, dam, TARGET_CHAR );
  }
}

void wear_prog_armbands(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char( "Kolbantların kollarını ısıtıyor.\n\r", ch );
}

void remove_prog_armbands(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Kolların soğuyor.\n\r", ch );
}

void fight_prog_demonfireshield( OBJ_DATA *obj, CHAR_DATA *ch )
{
  int dam;

  if ( get_shield_char( ch ) != obj )
	return;

  if ( number_percent() < 15 )  {
	dam = number_percent()/2 + 5 * ch->level;
	act("Kalkanının önünde gizemli bir delik açılıyor!", ch, NULL, ch->fighting, TO_CHAR);
	act("Kalkanın $S yüzünü yakıyor!", ch, NULL, ch->fighting, TO_CHAR);
	act( "$s kalkanı $S yüzünü yakıyor!", ch, NULL, ch->fighting, TO_NOTVICT);
	act( "$S kalkanı yüzünü yakıyor!", ch->fighting, NULL, ch, TO_CHAR);
	fire_strike(ch,dam, obj->level, gsn_demonfire);
  }
}

void wear_prog_demonfireshield(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Kalkanın ellerini ısıtıyor.\n\r", ch );
}

void remove_prog_demonfireshield(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Ellerin soğuyor.\n\r", ch );
}

void fight_prog_vorbalblade( OBJ_DATA *obj, CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  char eventbuf[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) || !is_wielded_char( ch, obj ) )
    return;

  victim = ch->fighting;
  if (victim == NULL || !IS_EVIL(victim))
    return;

  if ( number_percent() >= 10 )
    return;

  send_to_char("Silahın aniden rakibinin boynuna atılıyor!\n\r", ch);
  if ( number_percent() >= 20 )
    return;

  act( "Silahın havada bir yay çizerek $S kafasını koparıyor!", ch, NULL, victim, TO_CHAR);
  act("$S silahı havada ıslık çalarak senin kafanı koparıyor!", ch, NULL, victim, TO_NOTVICT);
  act( "$s silahı havada ıslık çalarak $S kafasını koparıyor!", ch, NULL, victim, TO_ROOM);
  act( "$n ÖLDÜ!!", victim, NULL, NULL, TO_ROOM );
  act( "$n ÖLDÜ!!", victim, NULL, NULL, TO_CHAR );
  raw_kill_org( victim, 3 );
  send_to_char( "Ö L D Ü R Ü L D Ü N!!\n\r", victim );

  /* event */
  if (!IS_NPC(victim))
  {
    snprintf(eventbuf, sizeof(eventbuf),"%s, %s tarafından öldürüldü.",victim->name, ch->name);
    write_event_log(eventbuf);
  }
}

void wear_prog_wind_boots(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (oprog_affect_on(ch, gsn_fly, TO_AFFECTS, AFF_FLYING, 0, 0))
    {
      send_to_char("Rüzgar çizmelerini ayağına giydiğin anda uçmaya başlıyorsun.\n\r", ch);
      send_to_char("Uçmaya başladın.\n\r", ch);
    }
}

void remove_prog_wind_boots(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (oprog_affect_off(ch, gsn_fly))
    {
      send_to_char("Yere düşüyorsun.\n\r", ch);
      send_to_char("Ahhh!.\n\r", ch);
    }
}

void wear_prog_boots_flying(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (oprog_affect_on(ch, gsn_fly, TO_AFFECTS, AFF_FLYING, 0, 0))
    {
      send_to_char("Uçuş botlarını ayaklarına giydiğin gibi havaya yükseliyorsun.\n\r", ch);
      send_to_char("Uçmaya başladın.\n\r", ch);
    }
}

void remove_prog_boots_flying(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (oprog_affect_off(ch, gsn_fly))
    {
      send_to_char("Yere düştün.\n\r", ch);
      send_to_char("Uçmak yerine yürümeye başladın!.\n\r", ch);
    }
}

/* Dev gücü veren üç eşya (kolluk, kemer, göğüslük) yalnızca ilk iletide ayrılır. */
static void wear_giant_strength(CHAR_DATA *ch, const char *msg)
{
  if (oprog_affect_on(ch, gsn_giant_strength, TO_AFFECTS, 0, APPLY_STR, level_mod(ch)))
    {
      send_to_char(msg, ch);
      send_to_char("Kaslarının irileştiğini hissediyorsun.\n\r", ch);
    }
}

static void remove_giant_strength(CHAR_DATA *ch)
{
  if (oprog_affect_off(ch, gsn_giant_strength))
    send_to_char("Kasların doğal haline döndü.\n\r", ch);
}

void wear_prog_arm_hercules(OBJ_DATA *obj, CHAR_DATA *ch)
{
  wear_giant_strength(ch, "Kollukları giydiğin anda güçlendiğini hissediyorsun.\n\r");
}

void remove_prog_arm_hercules(OBJ_DATA *obj, CHAR_DATA *ch)
{
  remove_giant_strength(ch);
}

void wear_prog_girdle_giant(OBJ_DATA *obj, CHAR_DATA *ch)
{
  wear_giant_strength(ch, "Kemeri giydiğin anda güçlendiğini hissediyorsun.\n\r");
}

void remove_prog_girdle_giant(OBJ_DATA *obj, CHAR_DATA *ch)
{
  remove_giant_strength(ch);
}

void wear_prog_breastplate_strength(OBJ_DATA *obj, CHAR_DATA *ch)
{
  wear_giant_strength(ch, "Göğüslüğü giydiğin anda güçlendiğini hissediyorsun.\n\r");
}

void remove_prog_breastplate_strength(OBJ_DATA *obj, CHAR_DATA *ch)
{
  remove_giant_strength(ch);
}

void fight_prog_rose_shield(OBJ_DATA *obj, CHAR_DATA *ch)
{
  int sect;

  if (ch->in_room == NULL)
    return;
  sect = ch->in_room->sector_type;
  /* yalnızca doğada: çayır, orman, dağ, tepe */
  if (sect != SECT_FIELD && sect != SECT_FOREST && sect != SECT_MOUNTAIN && sect != SECT_HILLS)
    return;

  if (get_shield_char(ch) != obj )
    return;

  if ( number_percent() < 90 )  return;

  send_to_char("Kalkanındaki yapraklar irileşmeye başlıyor.\n\r",ch);
  send_to_char("Kalkanın yaprakları seni sarıyor!.\n\r",ch->fighting);
  act("$s gül kalkanı irileşiyor.",ch,NULL,NULL,TO_ROOM);
  obj_cast_spell(gsn_slow,ch->level,ch,ch->fighting,NULL);
}

void fight_prog_lion_claw(OBJ_DATA *obj, CHAR_DATA *ch)
{
  int i;

  if ( number_percent() < 90 || !is_wielded_char(ch,obj) )
    return;

  send_to_char("Parmakların ucundan pençenin tırnakları beliriyor.\n\r",ch);
  act_color("$s pençesinden bir an tırnaklar çıkıyor.",
	    ch,NULL,NULL,TO_ROOM,POS_DEAD,CLR_WHITE);
  for (i = 0; i < 4; i++)
  {
    if (ch->fighting == NULL)	/* kurban öldü */
      return;
    one_hit(ch,ch->fighting,TYPE_HIT,FALSE);
  }
  send_to_char("Pençenin tırnakları yokoluyor.\n\r",ch);
  act_color("$s pençesinin tırnakları yokoluyor.",
	    ch,NULL,NULL,TO_ROOM,POS_DEAD,CLR_WHITE);
}

void speech_prog_ring_ra(OBJ_DATA *obj, CHAR_DATA *ch, char *speech)
{
  if (!str_cmp(speech, "punish")
      && (ch->fighting) && is_equiped_char(ch,obj,WEAR_FINGER) )
    {
      send_to_char("Bir elektrik arkı yüzükten fırlıyor.\n\r",ch);
      act("Bir elektrik arkı yüzükten fırlıyor.",ch,NULL,NULL,TO_ROOM);
      obj_cast_spell(gsn_lightning_breath,ch->level,ch,ch->fighting,NULL);
      WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    }
}

void wear_prog_eyed_sword(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act("$p objesindeki göz açılıyor.",ch,obj,NULL,TO_CHAR);
  act("$p objesindeki göz açılıyor.",ch,obj,NULL,TO_ROOM);
  set_weapon_dice(obj, ch);
  obj->level = ch->level;
}

void wear_prog_katana_sword(OBJ_DATA *obj, CHAR_DATA *ch)
{
  obj->value[2] = 2;
  if ( obj->item_type == ITEM_WEAPON
	&& IS_WEAPON_STAT(obj,WEAPON_KATANA)
	&& prog_obj_owned_by(obj, ch))
  {
   set_weapon_dice(obj, ch);
   obj->level = ch->level;
   send_to_char("Katananın senin bir parçan olduğunu hissediyorsun!\n\r",ch);
  }
}

void wear_prog_snake(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act_color("$CKırbacın yılanları zehirli bir gaz salmaya başlıyor.$c",ch,obj,NULL,TO_CHAR,POS_DEAD,CLR_GREEN);
  act_color("$CKırbacın yılanları zehirli bir gaz salmaya başlıyor.$c",ch,obj,NULL,TO_ROOM,POS_DEAD,CLR_GREEN);
  set_weapon_dice(obj, ch);
}

void remove_prog_snake(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act_color("$CKırbacın yılanları kabartmalara dönüşüyor.$c",ch,obj,NULL,TO_CHAR,POS_DEAD,CLR_RED);
  act_color("$CKırbacın yılanları kabartmalara dönüşüyor.$c",ch,obj,NULL,TO_ROOM,POS_DEAD,CLR_RED);
}

void get_prog_snake(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act("Kalkanın yılanlarının uykuya daldığını hissediyorsun.",ch,obj,NULL,TO_CHAR);
}

/* Ek açıklamasında "cold" geçen kalkan soğuğa, diğerleri ateşe karşı korur. */
static bool fire_shield_is_cold(OBJ_DATA *obj)
{
  return strstr( obj_extra_text(obj), "cold" ) != NULL;
}

void wear_prog_fire_shield(OBJ_DATA *obj, CHAR_DATA *ch)
{
  bool cold = fire_shield_is_cold(obj);

  if (oprog_affect_on(ch, gsn_fire_shield, TO_RESIST, cold ? RES_COLD : RES_FIRE, 0, 0))
    send_to_char(cold ? "Kalkanı kullanmaya başladığında soğuğa karşı dayanıklılaşıyorsun.\n\r"
		      : "Kalkanı kullanmaya başladığında sıcağa karşı dayanıklılaşıyorsun.\n\r", ch);
}

void remove_prog_fire_shield(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (oprog_affect_off(ch, gsn_fire_shield))
    send_to_char(fire_shield_is_cold(obj) ? "Soğuk saldırılarına karşı korunman azalıyor.\n\r"
					  : "Ateş saldırılarına karşı korunman azalıyor.\n\r", ch);
}

void wear_prog_quest_weapon(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if ( prog_obj_owned_by(obj, ch) )
  {
    send_ch_color("$CSilahın parlamaya başlıyor.$c",ch,POS_SLEEPING,CLR_BLUE);
    set_weapon_dice(obj, ch);
    obj->level = ch->level;
    return;
  }

  drop_unowned(obj, ch);
}

void fight_prog_ancient_gloves( OBJ_DATA *obj, CHAR_DATA *ch )
{
 int dam;

 if ( get_eq_char(ch, WEAR_HANDS) != obj
	|| get_wield_char(ch, FALSE) != NULL)
        return;

 if (number_percent() < 20)
 {
   dam = number_percent() + dice(ch->level, 14);
   act("$E dokunmanla elindeki alevin ona vurması bir oluyor!",
	ch, NULL, ch->fighting,TO_CHAR);
   act(  "$s $E dokunmasıyla $s elindeki alevin $M vurması bir oluyor!",
	ch, NULL, ch->fighting,TO_NOTVICT);
   act("$S sana dokunmasıyla elindeki alevin seni vurması bir oluyor!"
	, ch->fighting, NULL,ch,TO_CHAR);
   fire_strike(ch,dam, obj->level, gsn_burning_hands);
 }
}

void remove_prog_ancient_gloves(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Ellerindeki alev yokoluyor.\n\r", ch );
}

void wear_prog_ancient_gloves(OBJ_DATA *obj, CHAR_DATA *ch)
{
  send_to_char("Ellerinde bir alev dans etmeye başlıyor!\n\r", ch );
}

void fight_prog_ancient_shield(OBJ_DATA *obj, CHAR_DATA *ch)
{
 int chance;
 int dam;

 if ( get_shield_char( ch ) != obj )
        return;

 if ( (chance = number_percent()) < 5)
 {
   dam = dice(ch->level, 20);
   act("Kalkanın ışık saçmaya başladı!", ch, NULL,ch->fighting,TO_CHAR);
   act("$s kalkanı ışık saçmaya başladı!", ch, NULL,ch->fighting,TO_VICT);
   act("$s kalkanı ışık saçmaya başladı!", ch, NULL,ch->fighting,TO_NOTVICT);
   fire_strike(ch,dam, obj->level/2, gsn_fire_breath);
 }
 else if ( chance < 10 )
 {
   act("Kalkanın kızıl bir aurayla parlamaya başladı!",
	ch, NULL,ch->fighting,TO_CHAR);
   act("$s kalkanı kızıl bir aurayla parlamaya başladı!",
	 ch, NULL,ch->fighting,TO_VICT);
   act("$s kalkanı kızıl bir aurayla parlamaya başladı!",
	ch, NULL,ch->fighting,TO_NOTVICT);
   obj_cast_spell(gsn_blindness, ch->level+ 5, ch, ch->fighting,obj);
   if (ch->fighting != NULL)
     obj_cast_spell(gsn_slow, ch->level + 5, ch, ch->fighting, obj);
 }
}

void remove_prog_ancient_shield(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act_color(  "$CKalkanın eski haline dönüyor.$c",ch,obj,NULL,TO_CHAR,POS_DEAD,CLR_RED);
  act_color(  "$C$s kalkanı eski haline dönüyor.$c",ch,obj,NULL,TO_ROOM,POS_DEAD,CLR_RED);
}

void wear_prog_ancient_shield(OBJ_DATA *obj, CHAR_DATA *ch)
{
  act_color(
 "$CKalkanın şekil değiştirerek bir ejderhaya benzemeye başlıyor.\n\r"
 "Kalkanın üstünden yükselen bir ejderha başı ağzını açıyor!$c",
                ch,obj,NULL,TO_CHAR,POS_DEAD,CLR_RED);
  act_color(
 "$C$s kalkanı şekil değiştirerek bir ejderhaya benzemeye başlıyor.\n\r"
 "Kalkanın üstünden yükselen bir ejderha başı ağzını açıyor!$c",
                ch,obj,NULL,TO_ROOM,POS_DEAD,CLR_RED);
}

/* Boyunluk/başlık/karalık: iletisiz kalıcı işaret efektleri */
void wear_prog_neckguard(OBJ_DATA *obj, CHAR_DATA *ch)
{
  oprog_affect_on(ch, gsn_neckguard, TO_AFFECTS, 0, APPLY_NONE, 0);
}

void remove_prog_neckguard(OBJ_DATA *obj, CHAR_DATA *ch)
{
  oprog_affect_off(ch, gsn_neckguard);
}

void wear_prog_headguard(OBJ_DATA *obj, CHAR_DATA *ch)
{
  oprog_affect_on(ch, gsn_headguard, TO_AFFECTS, 0, APPLY_NONE, 0);
}

void remove_prog_headguard(OBJ_DATA *obj, CHAR_DATA *ch)
{
  oprog_affect_off(ch, gsn_headguard);
}

void wear_prog_blackguard(OBJ_DATA *obj, CHAR_DATA *ch)
{
  oprog_affect_on(ch, gsn_blackguard, TO_AFFECTS, 0, APPLY_NONE, 0);
}

void remove_prog_blackguard(OBJ_DATA *obj, CHAR_DATA *ch)
{
  oprog_affect_off(ch, gsn_blackguard);
}
