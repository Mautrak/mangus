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
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include "magic.h"
#include "utf8.h"

DECLARE_DO_FUN(	do_say	);

/* Şifacının hizmet türleri. */
enum heal_kind
{
    HS_SPELL,		/* skill/spell çifti ile büyü atılır */
    HS_MANA_SMALL,	/* 'mana': küçük mana yenileme */
    HS_MANA_BIG		/* 'takat': büyük mana yenileme */
};

struct heal_service
{
    const char     *key;	/* komut anahtarı (str_prefix) */
    const char     *alias;	/* ikinci anahtar, yoksa NULL */
    const char     *label;	/* listede görünen ad, NULL ise key */
    const char     *desc;	/* liste açıklaması */
    const char     *skill;	/* skill_lookup adı (HS_SPELL) */
    SPELL_FUN      *spell;
    const char     *words;	/* mırıldanılan sözler */
    int             cost;
    enum heal_kind  kind;
};

static const struct heal_service heal_table[] =
{
    { "hafif",    NULL,      NULL,          "hafif yara tedavisi",  "cure light",     spell_cure_light,     "judicandus dies",      100, HS_SPELL      },
    { "ciddi",    NULL,      NULL,          "ciddi yara tedavisi",  "cure serious",   spell_cure_serious,   "judicandus gzfuajg",   150, HS_SPELL      },
    { "kritik",   NULL,      NULL,          "kritik yara tedavisi", "cure critical",  spell_cure_critical,  "judicandus qfuhuqar",  250, HS_SPELL      },
    { "şifa",     NULL,      NULL,          "şifa büyüsü",          "heal",           spell_heal,           "pzar",                 500, HS_SPELL      },
    { "körlük",   NULL,      NULL,          "körlük tedavisi",      "cure blindness", spell_cure_blindness, "judicandus noselacri", 200, HS_SPELL      },
    { "hastalık", NULL,      NULL,          "hastalık tedavisi",    "cure disease",   spell_cure_disease,   "judicandus eugzagz",   150, HS_SPELL      },
    { "zehir",    NULL,      NULL,          "zehir tedavisi",       "cure poison",    spell_cure_poison,    "judicandus sausabru",  250, HS_SPELL      },
    { "lanet",    NULL,      NULL,          "lanet kaldırma",       "remove curse",   spell_remove_curse,   "candussido judifgz",   500, HS_SPELL      },
    { "yenileme", "hareket", NULL,          "yorgunluk azaltma",    "refresh",        spell_refresh,        "candusima",             50, HS_SPELL      },
    { "mana",     NULL,      NULL,          "mana yenileme",        NULL,             NULL,                 "candamira",            100, HS_MANA_SMALL },
    { "yüksek",   NULL,      "yüksek şifa", "yüksek tedavi",        "master healing", spell_master_heal,    "candastra nikazubra", 2000, HS_SPELL      },
    { "takat",    NULL,      NULL,          "ileri mana tedavi",    NULL,             NULL,                 "energizer",           2000, HS_MANA_BIG   },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, HS_SPELL }
};

/* Bu şifacı bu karaktere hizmet verir mi? Kabal şifacıları alan dosyasında
 * "kabal"/"cabal" + kabal kısa adı anahtar kelimeleriyle tanımlanır. */
static bool healer_serves( CHAR_DATA *mob, CHAR_DATA *ch )
{
    if ( !IS_NPC( mob ) || !IS_SET( mob->act, ACT_IS_HEALER ) )
	return FALSE;
    if ( ch->cabal && ( is_name( "cabal", mob->name ) || is_name( "kabal", mob->name ) ) )
	return is_name( (char *) cabal_table[ch->cabal].short_name, mob->name );
    return TRUE;
}

static void heal_list( CHAR_DATA *ch, CHAR_DATA *mob )
{
    const struct heal_service *hs;

    act( "Şifacı şu hizmetleri veriyor.", ch, NULL, mob, TO_CHAR );
    for ( hs = heal_table; hs->key != NULL; hs++ )
    {
	const char *label = hs->label != NULL ? hs->label : hs->key;

	printf_to_char( ch, "  %-*s: %-*s%4d akçe\n\r",
	    utf8_width( label, 12 ), label,
	    utf8_width( hs->desc, 21 ), hs->desc, hs->cost );
    }
    send_to_char( " Hizmet almak için: iyileş <tip>\n\r", ch );
}

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    const struct heal_service *hs;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj = NULL;
    int sn = -1;

    if ( ch->in_room == NULL )
	return;

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	if ( healer_serves( mob, ch ) )
	    break;

    if ( mob == NULL )
    {
      send_to_char( "Burada yapamazsın.\n\r", ch );
        return;
    }

    if ( ch->cabal == CABAL_BATTLE )
    {
      send_to_char( "Sen Öfke Kabalı üyesisin, pis bir büyücü değil!\n\r",ch );
        return;
    }

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	heal_list( ch, mob );
	return;
    }

    for ( hs = heal_table; hs->key != NULL; hs++ )
	if ( !str_prefix( arg, hs->key )
	||   ( hs->alias != NULL && !str_prefix( arg, hs->alias ) ) )
	    break;

    if ( hs->key == NULL )
    {
      act("Şifacı bu hizmeti vermiyor. Hizmet listesi için 'iyileş' yazın.",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    if ( hs->kind == HS_SPELL )
    {
	if ( ( sn = skill_lookup( hs->skill ) ) < 0 )
	{
	    bugf( "do_heal: '%s' yeteneği bulunamadı.", hs->skill );
	    return;
	}

	if ( hs->spell == spell_remove_curse )
	{
	    one_argument( argument, arg2 );
	    if ( arg2[0] != '\0'
	    &&   ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
	    {
		send_to_char( "Sende öyle birşey yok.\n\r", ch );
		return;
	    }
	}
    }

    if ( hs->cost > ch->silver )
    {
      act("Yeterli akçen yok.",ch,NULL,mob,TO_CHAR);
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    deduct_cost(ch,hs->cost);
    mob->silver += hs->cost;

    act("$n mırıldanıyor, '$T'.",mob,NULL,hs->words,TO_ROOM);

    switch ( hs->kind )
    {
    case HS_MANA_BIG:
	ch->mana += 300;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	send_to_char("Vücudundan şifalı bir sıcaklık geçiyor.\n\r",ch);
	return;
    case HS_MANA_SMALL:
	ch->mana += dice(2,8) + mob->level / 3;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	send_to_char("Vücudundan şifalı bir sıcaklık geçiyor.\n\r",ch);
	return;
    case HS_SPELL:
	break;
    }

    if ( obj != NULL )
	(*hs->spell)( sn, mob->level, mob, obj, TARGET_OBJ );
    else
	(*hs->spell)( sn, mob->level, mob, ch, TARGET_CHAR );
}

/* Dövüşteki Öfke üyesine şifacının uyguladığı tedaviler. */
static const struct
{
    int         aff;
    const char *skill;
    SPELL_FUN  *spell;
} battle_cures[] =
{
    { AFF_BLIND,  "cure blindness", spell_cure_blindness },
    { AFF_PLAGUE, "cure disease",   spell_cure_disease   },
    { AFF_POISON, "cure poison",    spell_cure_poison    },
    { AFF_CURSE,  "remove curse",   spell_remove_curse   },
    { 0, NULL, NULL }
};

static void battle_cast( CHAR_DATA *mob, CHAR_DATA *ch, const char *skill, SPELL_FUN *spell )
{
    int sn = skill_lookup( skill );

    if ( sn < 0 )
    {
	bugf( "heal_battle: '%s' yeteneği bulunamadı.", skill );
	return;
    }
    (*spell)( sn, mob->level, mob, ch, TARGET_CHAR );
}

void heal_battle(CHAR_DATA *mob, CHAR_DATA *ch )
{
    int i;
    bool needs = FALSE;

    /* kendi kabalinin şifacısıysa sessizce hiçbir şey yapma */
    if (is_name((char*)cabal_table[ch->cabal].short_name,mob->name) )
	return;

    if (IS_NPC(ch) || ch->cabal != CABAL_BATTLE)
       {
         do_say(mob,"Sana yardım etmem.");
	return;
       }

    for ( i = 0; battle_cures[i].skill != NULL; i++ )
	if ( IS_AFFECTED( ch, battle_cures[i].aff ) )
	    needs = TRUE;

    if ( !needs )
       {
	/* etki yok; yine de lanetli eşyalar için 'remove curse' denenir */
	do_say(mob,"Benim yardımıma ihtiyacın yok evladım. Fakat...");
	battle_cast( mob, ch, "remove curse", spell_remove_curse );
	return;
       }

       act("$n sana yemen için otlar veriyor.",mob,NULL,ch,TO_VICT);
       act("Otları yiyorsun.",mob,NULL,ch,TO_VICT);
       act("Otları $E veriyorsun.",mob,NULL,ch,TO_CHAR);
       act("$N verdiğin otları yiyor.",mob,NULL,ch,TO_CHAR);
       act("$n $E ot veriyor.",mob,NULL,ch,TO_NOTVICT);
       act("$n $S verdiği otları yiyor.",mob,NULL,ch,TO_NOTVICT);

    WAIT_STATE(ch,PULSE_VIOLENCE);

    for ( i = 0; battle_cures[i].skill != NULL; i++ )
	if ( IS_AFFECTED( ch, battle_cures[i].aff ) )
	    battle_cast( mob, ch, battle_cures[i].skill, battle_cures[i].spell );
}
