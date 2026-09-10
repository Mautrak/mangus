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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

/* Dosya yolları area/ dizinine göredir (sunucu orada çalışır). */
#define UD_DATA_FILE	"../data/ud_data"
#define KANAL_LOG_DIR	"../log/kanal/"
#define EVENT_LOG_FILE	"../log/events/events"

extern int max_on;
extern int max_on_so_far;
extern int ikikat_tp;
extern int ikikat_gp;

/* "YYYY/AA/GG SS:DD:ss" damgası. */
static void fmt_timestamp(char *buf, size_t size, const struct tm *tm)
{
	snprintf(buf, size, "%02d/%02d/%02d %02d:%02d:%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void ud_data_write(void)
{
	FILE *data;

	cevrimici_oyuncu_sayisi();
	data = fopen(UD_DATA_FILE, "w");
	if ( data == NULL )
		return;
	fprintf(data,"* Cevrimici oyuncu rekoru\n");
	fprintf(data,"Encokcevrimici %d\n",max_on_so_far);
	fprintf(data,"IkikatTP %d\n",ikikat_tp);
	fprintf(data,"IkikatGP %d\n",ikikat_gp);
	fprintf(data,"End\n");
	fclose(data);
}

void ud_data_read(void)
{
	FILE *fp;
	char *word;

	max_on = 0;
	max_on_so_far  = 0;
	fp = fopen(UD_DATA_FILE, "r");
	if ( fp == NULL )
		return;

	for ( ; ; )
	{
		word   = feof( fp ) ? "End" : fread_word( fp );
		if (!str_cmp( word, "Encokcevrimici"))
		{
			max_on_so_far = fread_number( fp );
		}
		else if (!str_cmp( word, "IkikatTP"))
		{
			ikikat_tp = fread_number( fp );
		}
		else if (!str_cmp( word, "IkikatGP"))
		{
			ikikat_gp = fread_number( fp );
		}
		else if(!str_cmp( word, "*"))
		{
			fread_to_eol(fp);
		}
		else if(!str_cmp( word, "End"))
		{
			fclose(fp);
			return;
		}
	}
}

/* Kanal günlüğü dosya adı ön ekleri (KANAL_* sırasıyla). */
static const char *const kanal_prefix[] =
{
	[KANAL_SOYLE]	= "soyle",
	[KANAL_KD]	= "kd",
	[KANAL_ACEMI]	= "acemi",
	[KANAL_HAYKIR]	= "haykir",
	[KANAL_IMM]	= "imm",
	[KANAL_GSOYLE]	= "gsoyle",
	[KANAL_DUYGU]	= "duygu",
};

void write_channel_log(CHAR_DATA *ch, CHAR_DATA *vc, int kanal, char *argument)
{
	FILE *data;
	char filename[MAX_STRING_LENGTH];
	char stamp[64];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	if ( argument[0] == '\0' )
		return;

	if(IS_NPC(ch))
		return;

	if ( kanal < 0 || kanal >= (int)(sizeof(kanal_prefix) / sizeof(kanal_prefix[0]))
	||   kanal_prefix[kanal] == NULL )
	{
		bugf("write_channel_log: hatali kanal %d", kanal);
		return;
	}

	snprintf(filename, sizeof(filename), KANAL_LOG_DIR "%s_%d_%02d_%02d",
		kanal_prefix[kanal], tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	data = fopen(filename,"a");
	if ( data == NULL )
		return;
	fmt_timestamp(stamp, sizeof(stamp), &tm);
	fprintf(data,"%s, Oda:%6d, Char: %10s, Victim: %10s, Log: %s\n",
		stamp, ch->in_room->vnum, ch->name,
		(vc != NULL) ? vc->name : "None", argument);
	fclose(data);
}

extern char * const month_name[];

void write_event_log(char *argument)
{
	FILE *data;
	char stamp[64];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	if ( argument[0] == '\0' )
		return;

	data = fopen(EVENT_LOG_FILE, "a");
	if ( data == NULL )
		return;

	fmt_timestamp(stamp, sizeof(stamp), &tm);
	fprintf(data,"%s|%ld|%s|%ld|%ld|%s\n", stamp,
		time_info.year, month_name[time_info.month-1],
		time_info.day, time_info.hour, argument);
	fclose(data);
}
