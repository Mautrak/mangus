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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "tables.h"

int flag_lookup ( const char *name, const struct flag_type *flag_table);

/* Karakter üzerinde 'flag' komutuyla değiştirilebilen bayrak alanları. */
struct flag_field
{
    const char *name;
    size_t offset;			/* CHAR_DATA içindeki long alan */
    const struct flag_type *table;
    int who;				/* 1 yalnız yaratık, -1 yalnız oyuncu, 0 ikisi */
    const char *wrong;			/* yanlış tür için ileti */
};

static const struct flag_field flag_fields[] =
{
    { "act",	  offsetof(CHAR_DATA, act),	    act_flags,     1, "Oyuncular için plr kullan.\n\r" },
    { "plr",	  offsetof(CHAR_DATA, act),	    plr_flags,    -1, "Yaratıklar için act kullan.\n\r" },
    { "aff",	  offsetof(CHAR_DATA, affected_by), affect_flags,  0, NULL },
    { "immunity", offsetof(CHAR_DATA, imm_flags),   imm_flags,     0, NULL },
    { "resist",	  offsetof(CHAR_DATA, res_flags),   imm_flags,     0, NULL },
    { "vuln",	  offsetof(CHAR_DATA, vuln_flags),  imm_flags,     0, NULL },
    { "form",	  offsetof(CHAR_DATA, form),	    form_flags,    1, "Oyuncuya form verilemez.\n\r" },
    { "parts",	  offsetof(CHAR_DATA, parts),	    part_flags,    1, "Oyuncuya parts verilemez.\n\r" },
    { "comm",	  offsetof(CHAR_DATA, comm),	    comm_flags,   -1, "Yaratığa comm verilemez.\n\r" },
    { NULL, 0, NULL, 0, NULL }
};

void do_flag(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH],arg3[MAX_INPUT_LENGTH];
    char word[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    const struct flag_field *f;
    const struct flag_type *flag_table;
    long *flag, old, inew = 0, marked = 0, bit;
    int pos;
    char type;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);
    argument = one_argument(argument,arg3);

    type = argument[0];

    if (type == '=' || type == '-' || type == '+')
        argument = one_argument(argument,word);

    if (arg1[0] == '\0')
    {
	send_to_char("Yazım:\n\r",ch);
	send_to_char("  flag mob  <isim> <alan> <bayraklar>\n\r",ch);
	send_to_char("  flag char <isim> <alan> <bayraklar>\n\r",ch);
	send_to_char("  yaratık alanları: act,aff,imm,res,vuln,form,parts\n\r",ch);
	send_to_char("  oyuncu alanları : plr,comm,aff,imm,res,vuln\n\r",ch);
	send_to_char("  +: bayrak ekler, -: kaldırır, =: yalnız verilenleri bırakır,\n\r",ch);
	send_to_char("  işaretsiz: sayılan bayrakları tersine çevirir.\n\r",ch);
	return;
    }

    if (str_prefix(arg1,"mob") && str_prefix(arg1,"char"))
    {
	send_to_char("Yalnız mob ya da char üzerinde bayrak değiştirilebilir.\n\r",ch);
	return;
    }

    if (arg2[0] == '\0')
    {
	send_to_char("Kimin bayraklarını değiştireceksin?\n\r",ch);
	return;
    }

    if (arg3[0] == '\0')
    {
	send_to_char("Hangi alanı değiştireceğini belirtmelisin.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Hangi bayrakları değiştirmek istiyorsun?\n\r",ch);
	return;
    }

    victim = get_char_world(ch,arg2);
    if (victim == NULL)
    {
	send_to_char("Onu bulamıyorsun.\n\r",ch);
	return;
    }

    for (f = flag_fields; f->name != NULL; f++)
	if (!str_prefix(arg3, f->name))
	    break;

    if (f->name == NULL)
    {
	send_to_char("Böyle bir bayrak alanı yok.\n\r",ch);
	return;
    }

    if ((f->who > 0 && !IS_NPC(victim)) || (f->who < 0 && IS_NPC(victim)))
    {
	send_to_char(f->wrong,ch);
	return;
    }

    flag = (long *)((char *)victim + f->offset);
    flag_table = f->table;

    old = *flag;
    /* Bayrakları elle değiştirilen yaratık bölgesine bağlı kalmaz (mset gibi). */
    victim->zone = NULL;

    if (type != '=')
	inew = old;

    /* verilen sözcükleri işaretle */
    for (; ;)
    {
	argument = one_argument(argument,word);

	if (word[0] == '\0')
	    break;

	bit = flag_lookup(word,flag_table);
	if (bit == 0)
	{
	    send_to_char("Böyle bir bayrak yok!\n\r",ch);
	    return;
	}
	SET_BIT(marked,bit);
    }

    for (pos = 0; flag_table[pos].name != NULL; pos++)
    {
	bit = flag_table[pos].bit;

	if (!flag_table[pos].settable && IS_SET(old,bit))
	{
	    SET_BIT(inew,bit);
	    continue;
	}

	if (!IS_SET(marked,bit))
	    continue;

	switch(type)
	{
	    case '=':
	    case '+':
		SET_BIT(inew,bit);
		break;
	    case '-':
		REMOVE_BIT(inew,bit);
		break;
	    default:
		if (IS_SET(inew,bit))
		    REMOVE_BIT(inew,bit);
		else
		    SET_BIT(inew,bit);
	}
    }
    *flag = inew;
}
