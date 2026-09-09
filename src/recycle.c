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
#include "bot.h"
#include "recycle.h"

/*
 * Serbest liste kalıbı tek yerde: listeden al ya da kalıcı bellekten ayır,
 * her durumda yapıyı sıfırla; serbest bırakırken listenin başına ekle.
 */
#define FREELIST_POP(head, p, T)				\
    do							\
    {							\
	if ((head) == NULL)				\
	    (p) = (T *) alloc_perm(sizeof(*(p)));	\
	else						\
	{						\
	    (p) = (head);				\
	    (head) = (head)->next;			\
	}						\
	memset((p), 0, sizeof(*(p)));			\
    } while (0)

#define FREELIST_PUSH(head, p)				\
    do							\
    {							\
	(p)->next = (head);				\
	(head) = (p);					\
    } while (0)

/* stuff for recycling ban structures */
BAN_DATA *ban_free;

BAN_DATA *new_ban(void)
{
    BAN_DATA *ban;

    FREELIST_POP(ban_free, ban, BAN_DATA);
    VALIDATE(ban);
    ban->name = &str_empty[0];
    return ban;
}

void free_ban(BAN_DATA *ban)
{
    if (!IS_VALID(ban))
	return;

    free_string(ban->name);
    INVALIDATE(ban);
    FREELIST_PUSH(ban_free, ban);
}

/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor(void)
{
    DESCRIPTOR_DATA *d;

    FREELIST_POP(descriptor_free, d, DESCRIPTOR_DATA);
    VALIDATE(d);
    return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
    if (!IS_VALID(d))
	return;

    free_string( d->host );
    free_mem( d->outbuf, d->outsize );
    INVALIDATE(d);
    FREELIST_PUSH(descriptor_free, d);
}

/* stuff for recycling extended descs */
EXTRA_DESCR_DATA *extra_descr_free;

EXTRA_DESCR_DATA *new_extra_descr(void)
{
    EXTRA_DESCR_DATA *ed;

    FREELIST_POP(extra_descr_free, ed, EXTRA_DESCR_DATA);
    ed->keyword = &str_empty[0];
    ed->description = &str_empty[0];
    VALIDATE(ed);
    return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
    if (!IS_VALID(ed))
	return;

    free_string(ed->keyword);
    free_string(ed->description);
    INVALIDATE(ed);
    FREELIST_PUSH(extra_descr_free, ed);
}


/* stuff for recycling affects */
AFFECT_DATA *affect_free;

AFFECT_DATA *new_affect(void)
{
    AFFECT_DATA *af;

    FREELIST_POP(affect_free, af, AFFECT_DATA);
    VALIDATE(af);
    return af;
}

void free_affect(AFFECT_DATA *af)
{
    if (!IS_VALID(af))
	return;

    INVALIDATE(af);
    FREELIST_PUSH(affect_free, af);
}

/* stuff for recycling objects */
OBJ_DATA *obj_free;

OBJ_DATA *new_obj(void)
{
    OBJ_DATA *obj;

    FREELIST_POP(obj_free, obj, OBJ_DATA);
    VALIDATE(obj);
    return obj;
}

void free_obj(OBJ_DATA *obj)
{
    AFFECT_DATA *paf, *paf_next;
    EXTRA_DESCR_DATA *ed, *ed_next;

    if (!IS_VALID(obj))
	return;

    for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
	paf_next = paf->next;
	free_affect(paf);
    }
    obj->affected = NULL;

    for (ed = obj->extra_descr; ed != NULL; ed = ed_next )
    {
	ed_next = ed->next;
	free_extra_descr(ed);
    }
    obj->extra_descr = NULL;

    free_string( obj->name        );
    free_string( obj->description );
    free_string( obj->short_descr );
    free_string( obj->owner     );
    INVALIDATE(obj);
    FREELIST_PUSH(obj_free, obj);
}


/* stuff for recyling characters */
CHAR_DATA *char_free;

/* Yeni karakterin başlangıç değerleri. */
#define NEW_CHAR_ARMOR	100
#define NEW_CHAR_HIT	 20
#define NEW_CHAR_MANA	100
#define NEW_CHAR_MOVE	100
#define NEW_CHAR_STAT	 13

CHAR_DATA *new_char (void)
{
    CHAR_DATA *ch;
    size_t i;

    FREELIST_POP(char_free, ch, CHAR_DATA);
    VALIDATE(ch);
    ch->name                    = &str_empty[0];
    ch->short_descr             = &str_empty[0];
    ch->long_descr              = &str_empty[0];
    ch->description             = &str_empty[0];
    ch->prompt                  = &str_empty[0];
    ch->prefix			= &str_empty[0];
    ch->logon                   = current_time;
    ch->lines                   = PAGELEN;
    for (i = 0; i < sizeof(ch->armor) / sizeof(ch->armor[0]); i++)
        ch->armor[i]            = NEW_CHAR_ARMOR;
    ch->position                = POS_STANDING;
    ch->hit                     = NEW_CHAR_HIT;
    ch->max_hit                 = NEW_CHAR_HIT;
    ch->mana                    = NEW_CHAR_MANA;
    ch->max_mana                = NEW_CHAR_MANA;
    ch->move                    = NEW_CHAR_MOVE;
    ch->max_move                = NEW_CHAR_MOVE;
    ch->language		= LANG_COMMON;

    for (i = 0; i < MAX_STATS; i ++)
        ch->perm_stat[i] = NEW_CHAR_STAT;

    return ch;
}


void free_char (CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    if (!IS_VALID(ch))
	return;

    if (IS_NPC(ch))
	mobile_count--;

    ch->extracted = TRUE;
    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	extract_obj_nocount(obj);
    }

    for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
	paf_next = paf->next;
	affect_remove(ch,paf);
    }

    free_string(ch->name);
    free_string(ch->short_descr);
    free_string(ch->long_descr);
    free_string(ch->description);
    free_string(ch->prompt);
    free_string(ch->prefix);
    free_string(ch->material);
    free_string(ch->in_mind);

    if (ch->pcdata != NULL)
    	free_pcdata(ch->pcdata);

    FREELIST_PUSH(char_free, ch);

    ch->extracted = FALSE;
    INVALIDATE(ch);
    return;
}

PC_DATA *pcdata_free;

PC_DATA *new_pcdata(void)
{
    PC_DATA *pcdata;

    FREELIST_POP(pcdata_free, pcdata, PC_DATA);
    pcdata->buffer = new_buf();

    VALIDATE(pcdata);
    return pcdata;
}


void free_pcdata(PC_DATA *pcdata)
{
    int alias;

    if (!IS_VALID(pcdata))
	return;

    bot_pcdata_freed(pcdata);

    free_string(pcdata->pwd);
    free_string(pcdata->bamfin);
    free_string(pcdata->bamfout);
    free_string(pcdata->title);
    free_string(pcdata->discord_id);
    free_buf(pcdata->buffer);

    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
	free_string(pcdata->alias[alias]);
	free_string(pcdata->alias_sub[alias]);
    }
    INVALIDATE(pcdata);
    FREELIST_PUSH(pcdata_free, pcdata);

    return;
}




/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
    long val;

    val = (long) current_time;
    if (val <= last_pc_id)
	val = last_pc_id + 1;
    last_pc_id = val;
    return val;
}

long get_mob_id(void)
{
    last_mob_id++;
    return last_mob_id;
}


/* procedures and constants needed for buffering */

BUFFER *buf_free;



/* buffer sizes */
static const int buf_size[MAX_BUF_LIST] =
{
    16,32,64,128,256,1024,2048,4096,8192,16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
static int buf_size_for (int val)
{
    int i;

    for (i = 0; i < MAX_BUF_LIST; i++)
	if (buf_size[i] >= val)
	{
	    return buf_size[i];
	}

    return -1;
}

BUFFER *new_buf()
{
    BUFFER *buffer;

    FREELIST_POP(buf_free, buffer, BUFFER);
    buffer->state	= BUFFER_SAFE;
    buffer->size	= buf_size_for(BASE_BUF);

    buffer->string	= (char *)alloc_mem(buffer->size);
    buffer->string[0]	= '\0';
    VALIDATE(buffer);

    return buffer;
}


void free_buf(BUFFER *buffer)
{
    if (!IS_VALID(buffer))
	return;

    free_mem(buffer->string,buffer->size);
    buffer->string = NULL;
    buffer->size   = 0;
    buffer->state  = BUFFER_FREED;
    INVALIDATE(buffer);
    FREELIST_PUSH(buf_free, buffer);
}


bool add_buf(BUFFER *buffer, char *string)
{
    size_t cur, add;
    int newsize;
    char *newstr;

    if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
	return FALSE;

    if (string == NULL)
    {
	bug("Add_buf: NULL string.", 0);
	return FALSE;
    }

    cur = strlen(buffer->string);
    add = strlen(string);

    if (cur + add + 1 > (size_t) buffer->size) /* increase the buffer size */
    {
	newsize = buf_size_for((int) (cur + add + 1));
	if (newsize < 0) /* overflow */
	{
	    buffer->state = BUFFER_OVERFLOW;
	    bug("Add_buf: buffer overflow past size %d.", buffer->size);
	    return FALSE;
	}

	newstr = (char *)alloc_mem(newsize);
	memcpy(newstr, buffer->string, cur + 1);
	free_mem(buffer->string, buffer->size);
	buffer->string = newstr;
	buffer->size   = newsize;
    }

    memcpy(buffer->string + cur, string, add + 1);
    return TRUE;
}


void clear_buf(BUFFER *buffer)
{
    buffer->string[0] = '\0';
    buffer->state     = BUFFER_SAFE;
}


char *buf_string(BUFFER *buffer)
{
    return buffer->string;
}
