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
 
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

char * son_harf_unlu(CHAR_DATA *ch, int say);

struct pers_suffix_type
{
    char *belirtec;
    char *birisi;
	char *olumsuz;
	char *sesli_eki;
	char *bir;//a,ı
	char *iki;//e,i
	char *uc;//o,u
	char *dort;//ö,ü
};

extern	const	struct	pers_suffix_type	pers_suffix_table[];

void convert_to_turkish_lowercase_utf8(const char *input, char *output, size_t output_size);
bool str_prefix_turkish(const char *astr, const char *bstr);
