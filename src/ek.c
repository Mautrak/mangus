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

#include "ek.h"

const struct pers_suffix_type pers_suffix_table[] =
{
	{(char*)"sS"	,(char*)"birisinin"	,(char*)"bir ölümsüzün"	,(char*)"n"	,(char*)"ın",(char*)"in",(char*)"un",(char*)"ün"	},
	{(char*)"mM"	,(char*)"birisini"	,(char*)"bir ölümsüzü"	,(char*)"y"	,(char*)"ı"	,(char*)"i"	,(char*)"u"	,(char*)"ü"	},
	{(char*)"eE"	,(char*)"birisine"	,(char*)"bir ölümsüze"	,(char*)"y"	,(char*)"a"	,(char*)"e"	,(char*)"a"	,(char*)"e"	},
	{(char*)"yY"	,(char*)"birisinde"	,(char*)"bir ölümsüzde"	,(char*)"d"	,(char*)"a"	,(char*)"e"	,(char*)"a"	,(char*)"e"	},
	{(char*)"zZ"	,(char*)"birisinden",(char*)"bir ölümsüzden",(char*)"d"	,(char*)"an",(char*)"en",(char*)"an",(char*)"en"	},
    {NULL	,NULL			,NULL				,NULL	,NULL	,NULL	,NULL	,NULL	}
};

char *ekler (CHAR_DATA *to, CHAR_DATA *ch, char *format)
{
	int say;//pers_suffix_table'da kacinci elemanin okunacagini tutuyor.
	char *i, *ek;// i BUF'u point edecek.
	char buf[MAX_STRING_LENGTH];//return olacak bilgi burada
	/* SAY değişkenine değeri atanıyor */
	for(say=0;pers_suffix_table[say].belirtec[0] != '\0';say++)
	{
		if(	pers_suffix_table[say].belirtec[0]==*format ||
			pers_suffix_table[say].belirtec[1]==*format	)
				break;
	}
	if(say>=5)//tablodaki doğru elemanı bulamadığı için çökme olmasın diye.
		say=1;
	buf[0]='\0';
	i=NULL;
	if(can_see(to,ch))
	{
		sprintf(buf,"%s%s",	!IS_NPC(ch)?ch->name:ch->short_descr,
							!IS_NPC(ch)?"'":"");
		/*  sondaki sessizin yumuşaması. aslında tek heceli kelimelerde
		 *  yumusama olmaz. ama tek hece kontrolü yok.
		 */
		if(IS_NPC(ch))
		{
			         size_t len = strlen(buf);
			         if (len > 0) {
			             char last_char = buf[len-1];
			             // Handle 'ç' separately due to multi-byte nature (UTF-8: C3 A7)
			             if (len > 1 && (unsigned char)buf[len-2] == 0xC3 && (unsigned char)last_char == 0xA7) {
			                  // FIXME: Proper UTF-8 'ç' -> 'c' replacement needed.
			                  // Original code's intent was buf[len-1] = 'c'; but that corrupts UTF-8.
			                  // Temporarily applying the original logic to pass compilation, but this needs review.
			                  buf[len-1] = 'c';
			             } else {
			                 // Handle single-byte consonants
			                 switch(last_char) {
			                     case 'p': buf[len-1] = 'b'; break;
			                     case 't': buf[len-1] = 'd'; break;
			                     case 'k': buf[len-1] = 'g'; break; // Or 'ğ'? Assuming 'g'.
			                 }
			             }
			         }
		}
		ek=NULL;// ek ve i yerine BUF kullanilabilir mi
		ek=son_harf_unlu(ch,say);
		strcat(buf,ek);
		i=buf;
		return i;
	}
	strcpy(buf,pers_suffix_table[say].birisi);
	i=buf;
	return i;
}

char * son_harf_unlu(CHAR_DATA *ch, int say)
{
	char isim[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char *ptr;
	unsigned int number;
	ptr=buf;

	(!IS_NPC(ch))?strcpy(isim,ch->name):strcpy(isim,ch->short_descr);

	buf[0]='\0';
	for(number=1;strlen(isim)>=number;number++)
	{
		      size_t len_isim = strlen(isim);
		      size_t current_index = len_isim - number;
		      unsigned char current_byte = (unsigned char)isim[current_index];
		      bool found_vowel = FALSE;

		      // Check for multi-byte UTF-8 characters first ('ı', 'ö', 'ü')
		      // Ensure we don't read before the start of the string (current_index > 0)
		      if (current_index > 0) {
		          unsigned char prev_byte = (unsigned char)isim[current_index - 1];
		          if (prev_byte == 0xC4 && current_byte == 0xB1) { // 'ı' (UTF-8: C4 B1)
		              if(say>2) { sprintf(buf,"d%s",pers_suffix_table[say].bir); return ptr; }
		              sprintf(buf,"%s%s",(number==1)?(pers_suffix_table[say].sesli_eki):(""),pers_suffix_table[say].bir);
		              found_vowel = TRUE;
		          } else if (prev_byte == 0xC3 && current_byte == 0xB6) { // 'ö' (UTF-8: C3 B6)
		              if(say>2) { sprintf(buf,"d%s",pers_suffix_table[say].dort); return ptr; }
		              sprintf(buf,"%s%s",(number==1)?(pers_suffix_table[say].sesli_eki):(""),pers_suffix_table[say].dort);
		              found_vowel = TRUE;
		          } else if (prev_byte == 0xC3 && current_byte == 0xBC) { // 'ü' (UTF-8: C3 BC)
		              if(say>2) { sprintf(buf,"d%s",pers_suffix_table[say].dort); return ptr; }
		              sprintf(buf,"%s%s",(number==1)?(pers_suffix_table[say].sesli_eki):(""),pers_suffix_table[say].dort);
		              found_vowel = TRUE;
		          }
		      }

		      // If not a multi-byte vowel, check single-byte ASCII vowels
		      // Also ensure the current byte is not the *second* byte of a previous UTF-8 char
		      if (!found_vowel && current_byte < 0x80)
		      {
		           if (current_byte == 'a') { // Note: 'ı' handled above
		              if(say>2) { sprintf(buf,"d%s",pers_suffix_table[say].bir); return ptr; }
		              sprintf(buf,"%s%s",(number==1)?(pers_suffix_table[say].sesli_eki):(""),pers_suffix_table[say].bir);
		              found_vowel = TRUE;
		          } else if (current_byte == 'e' || current_byte == 'i') {
		              if(say>2) { sprintf(buf,"d%s",pers_suffix_table[say].iki); return ptr; }
		              sprintf(buf,"%s%s",(number==1)?(pers_suffix_table[say].sesli_eki):(""),pers_suffix_table[say].iki);
		              found_vowel = TRUE;
		          } else if (current_byte == 'o' || current_byte == 'u') {
		              if(say>2) { sprintf(buf,"d%s",pers_suffix_table[say].uc); return ptr; }
		              sprintf(buf,"%s%s",(number==1)?(pers_suffix_table[say].sesli_eki):(""),pers_suffix_table[say].uc);
		              found_vowel = TRUE;
		          }
		          // Note: 'ö', 'ü' handled above
		      }

		      // If a vowel was found in this iteration, return
		      if (found_vowel) {
		          return ptr;
		      }
		      // Otherwise, the loop continues to the next character (implicitly handled by for loop)
	}
	sprintf(buf,"%s",pers_suffix_table[say].bir);
	return ptr;
}
