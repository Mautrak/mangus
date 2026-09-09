/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefiting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                                                  *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "merc.h"
#include "utf8.h"

extern ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH]; /* db.c */

/* ROOM_INDEX_DATA.exit[] boyu (merc.h'de ayrı bir MAX_DIR yok). */
#define EXIT_COUNT ((int)(sizeof ((ROOM_INDEX_DATA *)0)->exit / sizeof ((ROOM_INDEX_DATA *)0)->exit[0]))

/* Oda adı ve alan adı sütun genişlikleri (görsel). */
#define ROOM_COL	26
#define AREA_COL	8

/* get the 'short' name of an area (e.g. MIDGAARD, MIRROR etc. */
/* assumes that the filename saved in the AREA_DATA struct is something like midgaard.are */
static const char *area_name (AREA_DATA *pArea)
{
	static char buffer[64]; /* short filename */
	char  *period;

	assert (pArea != NULL);

	snprintf (buffer, sizeof buffer, "%s", pArea->file_name);
	period = strchr (buffer, '.'); /* find the period (midgaard.are) */
	if (period) /* if there was one */
		*period = '\0'; /* terminate the string there (midgaard) */

	return buffer;
}

typedef enum {exit_from, exit_to, exit_both} exit_status;

/* depending on status print > or < or <> between the 2 rooms */
static void room_pair (ROOM_INDEX_DATA* left, ROOM_INDEX_DATA* right, exit_status ex,
		       char *buffer, size_t cap)
{
	const char *sExit;
	char lname[ROOM_COL * 4 + 1], rname[ROOM_COL * 4 + 1], aname[AREA_COL * 4 + 1];

	switch (ex)
	{
		default:
			sExit = "??"; break; /* invalid usage */
		case exit_from:
			sExit = "< "; break;
		case exit_to:
			sExit = " >"; break;
		case exit_both:
			sExit = "<>"; break;
	}

	utf8_fit (left->name,  ROOM_COL, lname, sizeof lname);
	utf8_fit (right->name, ROOM_COL, rname, sizeof rname);
	utf8_fit (area_name (right->area), AREA_COL, aname, sizeof aname);
	snprintf (buffer, cap, "%5d %s %s%5d %s(%s)\n\r",
		  left->vnum, lname, sExit, right->vnum, rname, aname);
}

/* for every exit in 'room' which leads to or from pArea but NOT both, print it */
static void checkexits (ROOM_INDEX_DATA *room, AREA_DATA *pArea, char* buffer, size_t cap)
{
	char buf[MAX_STRING_LENGTH];
	int i;
	EXIT_DATA *exit;
	ROOM_INDEX_DATA *to_room;
	size_t len = 0;

	buffer[0] = '\0';
	for (i = 0; i < EXIT_COUNT; i++)
	{
		bool two_way;

		exit = room->exit[i];
		if (!exit || (to_room = exit->u1.to_room) == NULL)
			continue;

		two_way = to_room->exit[rev_dir[i]] != NULL
		       && to_room->exit[rev_dir[i]]->u1.to_room == room;

		if (room->area == pArea && to_room->area != pArea)
			/* an exit from our area to another area */
			room_pair (room, to_room, two_way ? exit_both : exit_to, buf, sizeof buf);
		else if (room->area != pArea && to_room->area == pArea && !two_way)
			/* an exit from another area to our area; two-way exits are
			   handled in the other branch */
			room_pair (to_room, room, exit_from, buf, sizeof buf);
		else
			continue;

		len += (size_t)snprintf (buffer + len, cap - len, "%s", buf);
		if (len >= cap)
			break;
	}
}

/* for now, no arguments, just list the current area */
void do_exlist (CHAR_DATA *ch, char * argument)
{
	AREA_DATA* pArea;
	ROOM_INDEX_DATA* room;
	int i;
	char buffer[MAX_STRING_LENGTH];

	pArea = ch->in_room->area; /* this is the area we want info on */
	for (i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
	for (room = room_index_hash[i]; room != NULL; room = room->next)
	/* run through all the rooms on the MUD */
	{
		checkexits (room, pArea, buffer, sizeof buffer);
		if (buffer[0] != '\0')
			send_to_char (buffer, ch);
	}
}

/* To have VLIST show more than vnum 0 - 9900, change the number below: */

#define MAX_SHOW_VNUM   499 /* show only 1 - 100*100 */
#define COLUMNS 		5   /* number of columns */
#define MAX_ROW 		((MAX_SHOW_VNUM / COLUMNS)+1) /* rows */

void do_vlist (CHAR_DATA *ch, char *argument)
{
	int i,j,vnum;
	ROOM_INDEX_DATA *room;
	char buffer[COLUMNS * 100]; /* should be plenty */
	char buf2 [100];
	char aname[AREA_COL * 4 + 1];
	size_t len;

	for (i = 0; i < MAX_ROW; i++)
	{
		buffer[0] = '\0'; /* clear the buffer for this row */
		len = 0;

		for (j = 0; j < COLUMNS; j++) /* for each column */
		{
			vnum = ((j*MAX_ROW) + i); /* find a vnum whih should be there */
			if (vnum < MAX_SHOW_VNUM)
			{
				room = get_room_index (vnum * 100 + 1); /* each zone has to have a XXX01 room */
				utf8_fit (room ? area_name(room->area) : "-", AREA_COL, aname, sizeof aname);
				snprintf(buf2, sizeof(buf2), "%3d %s  ", vnum, aname);
						 /* something there or unused ? */
				len += (size_t)snprintf (buffer + len, sizeof buffer - len, "%s", buf2);
			}
		} /* for columns */

		send_to_char (buffer,ch);
		send_to_char ("\n\r",ch);
	} /* for rows */
}
