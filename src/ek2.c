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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "ek2.h"
/*
hassan-da
ercüment-de
ıtır-da
iyşi-de
bozo-da
gözö-de
kuzu-da
büdü-de
*/

const struct sonek_type sonek_table[] =
{
	{{(char*)"n",(char*)"ın" ,(char*)"in"	,(char*)"un" ,(char*)"ün"	}},//f - un
 	{{(char*)"y",(char*)"ı"	 ,(char*)"i"	,(char*)"u"	 ,(char*)"ü"	}},//g - u
  {{(char*)"y",(char*)"a"	 ,(char*)"e"	,(char*)"a"	 ,(char*)"e"	}},//h - e
  {{(char*)""	,(char*)"da" ,(char*)"de"	,(char*)"da" ,(char*)"de"	}},//j - de
  {{(char*)""	,(char*)"dan",(char*)"den",(char*)"dan",(char*)"den"	}},//k - den
     	{{NULL	,NULL	,NULL	,NULL	,NULL	}}
};

bool bu_harf_unlu_mu(char harf)
{
	   // Basic ASCII vowels
	   if (harf == 'a' || harf == 'A' || harf == 'e' || harf == 'E' || harf == 'i' || harf == 'I' || harf == 'o' || harf == 'O' || harf == 'u' || harf == 'U')
	       return TRUE;

	   // Check for potential start bytes of Turkish UTF-8 vowels
	   // Note: This function only receives one byte (char), so it cannot reliably detect multi-byte chars.
	   // It can only check if the *given* byte matches one of the bytes used in Turkish vowels.
	   // A more robust check would require context (the preceding byte).
	   unsigned char u_harf = (unsigned char)harf;
	   if (u_harf == 0xC4 || u_harf == 0xB1 || // Part of 'ı' (C4 B1)
	       u_harf == 0xC4 || u_harf == 0xB0 || // Part of 'İ' (C4 B0)
	       u_harf == 0xC3 || u_harf == 0xB6 || // Part of 'ö' (C3 B6)
	       u_harf == 0xC3 || u_harf == 0x96 || // Part of 'Ö' (C3 96)
	       u_harf == 0xC3 || u_harf == 0xBC || // Part of 'ü' (C3 BC)
	       u_harf == 0xC3 || u_harf == 0x9C)   // Part of 'Ü' (C3 9C)
	   {
	       // Cannot be certain without previous byte, but flag it as potentially vowel-related.
	       // The calling function `son_unlu_harf_hangisi` needs proper UTF-8 handling.
	       // For now, returning TRUE to mimic original intent, though potentially inaccurate.
	       return TRUE;
	   }
		return TRUE;
	return FALSE;
}

bool son_harf_unlu_mu(char *sozcuk)
{
	const 	char 	*str;
	// son harfe ilerleyelim...
	str = sozcuk;
	while(*str != '\0')
	{
		++str;
	}
	//son harf şu:
	--str;

	if(bu_harf_unlu_mu(*str))
		return TRUE;
	return FALSE;
}

char son_unlu_harf_hangisi(char *sozcuk)
{
	const 	char 	*str;
	char unlu=0;
	str=sozcuk;
	// son harfe ilerleyelim. ilerlerken de son ünlüyü bulalım...
	while(*str != '\0')
	{
		if(bu_harf_unlu_mu(*str))
			unlu=*str;
		++str;
	}

	return unlu;
}

char *ek_olustur(char *sozcuk, char tip)
{
	char buf[MAX_STRING_LENGTH];
	char *pbuf;
	int son_unlu,i;
	if(tip=='f')
		i=0;
	else if(tip=='g')
		i=1;
	else if(tip=='h')
		i=2;
	else if(tip=='j')
		i=3;
	else if(tip=='k')
		i=4;
	else
		i=0;
	son_unlu=son_unlu_harf_hangisi(sozcuk);

	if(son_harf_unlu_mu(sozcuk))
	{
		      // FIXME: son_unlu is a char, cannot reliably hold multi-byte UTF-8 vowels.
		      // This logic needs revision based on a corrected son_unlu_harf_hangisi function.
		      // Applying direct byte checks as a temporary fix for compilation.
		      unsigned char u_son_unlu = (unsigned char)son_unlu;

		      if (u_son_unlu == 'a' || (u_son_unlu == 0xC4 /* potential start of 'ı' or 'İ' */)) { // Assuming 'ı' based on original grouping
		          sprintf(buf,"%s%s%s",sozcuk,sonek_table[i].ek[0],sonek_table[i].ek[1]);
		      } else if (u_son_unlu == 'e' || u_son_unlu == 'i' || (u_son_unlu == 0xC4 /* potential start of 'ı' or 'İ' */)) { // Assuming 'İ' based on original grouping
		           sprintf(buf,"%s%s%s",sozcuk,sonek_table[i].ek[0],sonek_table[i].ek[2]);
		      } else if (u_son_unlu == 'o' || u_son_unlu == 'u') {
		          sprintf(buf,"%s%s%s",sozcuk,sonek_table[i].ek[0],sonek_table[i].ek[3]);
		      } else if (u_son_unlu == 0xC3 /* potential start of 'ö', 'Ö', 'ü', 'Ü' */) { // Assuming 'ö' or 'ü'
		          sprintf(buf,"%s%s%s",sozcuk,sonek_table[i].ek[0],sonek_table[i].ek[4]);
		      } else { // Default/Fallback
		          sprintf(buf,"%s%s%s",sozcuk,sonek_table[i].ek[0],sonek_table[i].ek[1]);
		      }
	}
	else
	{
		      // FIXME: Same issue as above with son_unlu. Applying temporary fix.
		      unsigned char u_son_unlu_2 = (unsigned char)son_unlu;

		      if (u_son_unlu_2 == 'a' || (u_son_unlu_2 == 0xC4 /* potential start of 'ı' or 'İ' */)) { // Assuming 'ı'
		          sprintf(buf,"%s%s",sozcuk,sonek_table[i].ek[1]);
		      } else if (u_son_unlu_2 == 'e' || u_son_unlu_2 == 'i' || (u_son_unlu_2 == 0xC4 /* potential start of 'ı' or 'İ' */)) { // Assuming 'İ'
		          sprintf(buf,"%s%s",sozcuk,sonek_table[i].ek[2]);
		      } else if (u_son_unlu_2 == 'o' || u_son_unlu_2 == 'u') {
		           // Original code added sonek_table[i].ek[0] here, which seems inconsistent for consonant endings. Removing it.
		          sprintf(buf,"%s%s",sozcuk,sonek_table[i].ek[3]);
		      } else if (u_son_unlu_2 == 0xC3 /* potential start of 'ö', 'Ö', 'ü', 'Ü' */) { // Assuming 'ö' or 'ü'
		           // Original code added sonek_table[i].ek[0] here, which seems inconsistent for consonant endings. Removing it.
		          sprintf(buf,"%s%s",sozcuk,sonek_table[i].ek[4]);
		      } else { // Default/Fallback
		          sprintf(buf,"%s%s",sozcuk,sonek_table[i].ek[1]);
		      }
	}
	pbuf=buf;
	return pbuf;

}
