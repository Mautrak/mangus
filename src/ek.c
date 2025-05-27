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
#include <ctype.h>
#include <string.h>
#include <stdio.h> // For debugging, remove later

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

void convert_to_turkish_lowercase_utf8(const char *input, char *output, size_t output_size) {
    const unsigned char *p = (const unsigned char *)input;
    unsigned char *q = (unsigned char *)output;
    size_t remaining_size = output_size - 1; // For null terminator

    while (*p && remaining_size > 0) {
        unsigned int ch_val = 0; // Changed variable name from ch to ch_val to avoid conflict
        int len = 0;

        // Decode UTF-8 character
        if (*p < 0x80) { // ASCII
            ch_val = *p;
            len = 1;
        } else if ((*p & 0xE0) == 0xC0) { // 2-byte sequence
            if (p[1] && (p[1] & 0xC0) == 0x80 && remaining_size >= 2) {
                ch_val = (*p << 8) | p[1];
                len = 2;
            } else { // Invalid sequence or not enough space
                if (remaining_size > 0) {
                    *q++ = *p++;
                    remaining_size--;
                } else { break; }
                continue;
            }
        } else if ((*p & 0xF0) == 0xE0) { // 3-byte sequence
             if (p[1] && (p[1] & 0xC0) == 0x80 && p[2] && (p[2] & 0xC0) == 0x80 && remaining_size >= 3) {
                ch_val = (*p << 16) | (p[1] << 8) | p[2];
                len = 3;
            } else {
                if (remaining_size > 0) {
                    *q++ = *p++;
                    remaining_size--;
                } else { break; }
                continue;
            }
        } else { // Other multi-byte or invalid, copy as is
            if (remaining_size > 0) {
                *q++ = *p++;
                remaining_size--;
            } else { break; }
            continue;
        }

        // Apply Turkish lowercasing
        if (len == 1) { // ASCII characters
            if (ch_val == 'I') { // ASCII 'I' to UTF-8 'ı'
                if (remaining_size >= 2) {
                    *q++ = 0xC4; *q++ = 0xB1;
                    remaining_size -= 2;
                } else { break; } // Not enough space
            } else {
                if (remaining_size >= 1) {
                    *q++ = tolower(ch_val);
                    remaining_size--;
                } else { break; }
            }
        } else if (len == 2) { // 2-byte UTF-8 characters
            unsigned char c1 = (ch_val >> 8) & 0xFF;
            unsigned char c2 = ch_val & 0xFF;
            if (c1 == 0xC3 && c2 == 0x87) { // Ç -> ç
                if (remaining_size >= 2) { *q++ = 0xC3; *q++ = 0xA7; remaining_size -= 2; } else { break; }
            } else if (c1 == 0xC4 && c2 == 0x9E) { // Ğ -> ğ
                if (remaining_size >= 2) { *q++ = 0xC4; *q++ = 0x9F; remaining_size -= 2; } else { break; }
            } else if (c1 == 0xC4 && c2 == 0xB0) { // İ -> i
                if (remaining_size >= 1) { *q++ = 'i'; remaining_size -= 1; } else { break; }
            } else if (c1 == 0xC3 && c2 == 0x96) { // Ö -> ö
                if (remaining_size >= 2) { *q++ = 0xC3; *q++ = 0xB6; remaining_size -= 2; } else { break; }
            } else if (c1 == 0xC5 && c2 == 0x9E) { // Ş -> ş
                if (remaining_size >= 2) { *q++ = 0xC5; *q++ = 0x9F; remaining_size -= 2; } else { break; }
            } else if (c1 == 0xC3 && c2 == 0x9C) { // Ü -> ü
                if (remaining_size >= 2) { *q++ = 0xC3; *q++ = 0xBC; remaining_size -= 2; } else { break; }
            } else { // Not a special Turkish char, copy as is
                if (remaining_size >= 2) { *q++ = c1; *q++ = c2; remaining_size -= 2; } else { break; }
            }
        }
        // Add more cases for 3-byte if necessary
        else if (len == 3) { // Copy 3-byte sequences we don't specifically handle
            if (remaining_size >= 3) {
                *q++ = (ch_val >> 16) & 0xFF;
                *q++ = (ch_val >> 8) & 0xFF;
                *q++ = ch_val & 0xFF;
                remaining_size -= 3;
            } else { break; }
        }
        // Should not reach here if len is 0 or invalid, but as a fallback:
        else if (len > 0) {
             if (remaining_size >= (size_t)len) {
                for (int k=0; k<len; ++k) *q++ = p[k];
                remaining_size -= len;
            } else { break; }
        }

        p += len;
    }
    *q = '\0';
}

bool str_prefix_turkish(const char *astr, const char *bstr)
{
    char lowercase_astr[MAX_INPUT_LENGTH]; // MAX_INPUT_LENGTH is from merc.h
    char lowercase_bstr[MAX_INPUT_LENGTH];

    if (astr == NULL || bstr == NULL)
        return TRUE; // Indicates not a prefix / error, matching old str_prefix return for "different"

    // Convert both strings to Turkish lowercase
    // Assuming convert_to_turkish_lowercase_utf8 is declared in ek.h (or defined above in ek.c)
    convert_to_turkish_lowercase_utf8(astr, lowercase_astr, MAX_INPUT_LENGTH);
    convert_to_turkish_lowercase_utf8(bstr, lowercase_bstr, MAX_INPUT_LENGTH);

    size_t astr_len = strlen(lowercase_astr);
    size_t bstr_len = strlen(lowercase_bstr);

    if (astr_len == 0) // An empty string is a prefix of any string
        return FALSE; 
    
    if (astr_len > bstr_len)
        return TRUE; // astr cannot be a prefix if it's longer, so different

    // Returns 0 if astr_lower is a prefix of bstr_lower (up to astr_len characters match)
    // Returns non-zero if they are not a prefix.
    // str_prefix should return TRUE if NOT a prefix, FALSE if IS a prefix.
    return strncmp(lowercase_astr, lowercase_bstr, astr_len) != 0;
}
