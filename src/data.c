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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

extern int max_on;
extern int max_on_so_far;
extern int ikikat_tp;
extern int ikikat_gp;

void ud_data_write(void)
{
	FILE *data;
	/* int suan; */ /* Fix L29: Removed unused variable */

	cevrimici_oyuncu_sayisi();
	system("rm -f ../data/ud_data");
	data=fopen("../data/ud_data","a");
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
	fp=fopen("../data/ud_data","r");

	for ( ; ; )
	{
		word   = feof( fp ) ? (char*)"End" : fread_word( fp );
		if (!str_cmp( word, (char*)"Encokcevrimici"))
		{
			max_on_so_far = fread_number( fp );
		}
		else if (!str_cmp( word, (char*)"IkikatTP"))
		{
			ikikat_tp = fread_number( fp );
		}
		else if (!str_cmp( word, (char*)"IkikatGP"))
		{
			ikikat_gp = fread_number( fp );
		}
		else if(!str_cmp( word, (char*)"*"))
		{
			fread_to_eol(fp);
		}
		else if(!str_cmp( word, (char*)"End"))
		{
			fclose(fp);
			return;
		}
	}
	fclose(fp);
	return;
}

void write_channel_log(CHAR_DATA *ch, CHAR_DATA *vc, int kanal, char *argument)
{

	if ( argument[0] == '\0' )
		return;

	if(IS_NPC(ch))
		return;

	FILE *data;
	char filename[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH+100];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	if( kanal<0 || kanal>6 )
	{
		sprintf( buf, "write_channel_log: hatali kanal %d", kanal );
		bug(buf,0);
		return;
	}

	switch(kanal)
	{
		case KANAL_SOYLE:
			sprintf(filename, "../log/kanal/soyle_%d_%02d_%02d",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday);break;
		case KANAL_KD:
			sprintf(filename, "../log/kanal/kd_%d_%02d_%02d",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday);break;
		case KANAL_ACEMI:
			sprintf(filename, "../log/kanal/acemi_%d_%02d_%02d",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday);break;
		case KANAL_HAYKIR:
			sprintf(filename, "../log/kanal/haykir_%d_%02d_%02d",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday);break;
		case KANAL_IMM:
			sprintf(filename, "../log/kanal/imm_%d_%02d_%02d",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday);break;
		case KANAL_GSOYLE:
			sprintf(filename, "../log/kanal/gsoyle_%d_%02d_%02d",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday);break;
		case KANAL_DUYGU:
			sprintf(filename, "../log/kanal/duygu_%d_%02d_%02d",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday);break;
	}

	if (vc == NULL)
	{

	}

	data=fopen(filename,"a");
	sprintf(buf,"%02d/%02d/%02d %02d:%02d:%02d, Oda:%6d, Char: %10s, Victim: %10s, Log: %s\n",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,ch->in_room->vnum,ch->name,(vc != NULL)?vc->name:"None",argument);
	fprintf(data, "%s", buf); /* Fix L107: Use format string */
	fclose(data);
	return;
}

char *	const	month_name	[] =
{
	(char*)"Albars",
	(char*)"Kadimler",
    (char*)"Büyük Acı",
	(char*)"Zeytin",
	(char*)"Yılan",
	(char*)"Yelbüke",
	(char*)"Pusu",
	(char*)"Savaş",
	(char*)"Albastı",
	(char*)"Gölge",
	(char*)"Kara Ölüm",
	(char*)"Alacakaranlık",
};

void write_event_log(char *argument)
{

	if ( argument[0] == '\0' )
	{
		return;
	}

	FILE *data;
	char buf[MAX_STRING_LENGTH+100];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	data=fopen("../log/events/events","a");

	sprintf(buf,"%02d/%02d/%02d %02d:%02d:%02d|%ld|%s|%ld|%ld|%s\n",\
				tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, \
				time_info.year,month_name[time_info.month-1],time_info.day, time_info.hour, argument);
	fprintf(data, "%s", buf); /* Fix L166: Use format string */
	fclose(data);
	return;

}
