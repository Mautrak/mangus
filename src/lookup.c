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
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "lookup.h"
#include "utf8.h"

/*
 * Ad alanı tablolarında ön ek araması. 'first' ilk kaydın ad alanını,
 * 'stride' kayıt boyutunu gösterir; tablo NULL adla biter. Bulunan kaydın
 * sırası, bulunamazsa -1 döner. Boş ad her şeyin ön eki sayılacağından
 * baştan elenir (ROM'un ilk harf hızlandırmasının tek işlevi buydu).
 */
int name_table_lookup(const char *name, const char *const *first, size_t stride)
{
    const char *p = (const char *)first;
    int i;

    if (name[0] == '\0')
        return -1;

    for (i = 0; *(const char *const *)p != NULL; i++, p += stride)
    {
        if (!str_prefix(name, *(const char *const *)p))
            return i;
    }

    return -1;
}

int flag_lookup (const char *name, const struct flag_type *flag_table)
{
    int i = name_table_lookup(name, &flag_table[0].name, sizeof flag_table[0]);

    return i < 0 ? 0 : flag_table[i].bit;
}

int position_lookup (const char *name)
{
    return name_table_lookup(name, &position_table[0].name, sizeof position_table[0]);
}

int sex_lookup (const char *name)
{
    return name_table_lookup(name, &sex_table[0].name, sizeof sex_table[0]);
}

int size_lookup (const char *name)
{
    return name_table_lookup(name, &size_table[0].name, sizeof size_table[0]);
}

/* LANG_* sabitini döndürür; "analisan" (ırk dili) için MAX_LANGUAGE, bulunamazsa -1. */
int lang_lookup (const char *name)
{
    int lang;

    if (name[0] == '\0')
        return -1;

    if (!str_prefix(name, "analisan"))
        return MAX_LANGUAGE;

    for (lang = 0; lang < MAX_LANGUAGE; lang++)
    {
        if (utf8_first_eq(name, language_table[lang].name)
        &&  !str_prefix(name, language_table[lang].name))
            return language_table[lang].vnum;
    }

    return -1;
}
