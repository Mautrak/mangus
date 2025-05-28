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

#include "turkish_morphology.h"
#include "merc.h" // For TRUE/FALSE, MAX_STRING_LENGTH, etc. if not fully standalone
#include <string.h> // For strlen, strcpy, strcat, strncmp, memcpy
#include <ctype.h>  // For tolower (for ASCII part of convert_to_turkish_lowercase_utf8)
#include <stdio.h>  // For snprintf

// If merc.h doesn't define these, provide local ones.
// Assuming bool, true, false are available via stdbool.h included in turkish_morphology.h
#ifndef MAX_STRING_LENGTH
#define MAX_STRING_LENGTH 4096
#endif
#ifndef MAX_INPUT_LENGTH // Often used in MUDs
#define MAX_INPUT_LENGTH 256
#endif

// =============================================================================
// GLOBAL DATA - From ek.c
// =============================================================================

// pers_suffix_table is defined here.
// Note: The string literals for suffixes like "in", "ın" might need UTF-8 review later
// if they are directly used, but generate_turkish_suffixed_word should handle actual suffix generation.
const struct pers_suffix_type pers_suffix_table[] =
{
	{(char*)"sS"	,(char*)"birisinin"	,(char*)"bir ölümsüzün"	,(char*)"n"	,(char*)"ın",(char*)"in",(char*)"un",(char*)"ün"	},
	{(char*)"mM"	,(char*)"birisini"	,(char*)"bir ölümsüzü"	,(char*)"y"	,(char*)"ı"	,(char*)"i"	,(char*)"u"	,(char*)"ü"	},
	{(char*)"eE"	,(char*)"birisine"	,(char*)"bir ölümsüze"	,(char*)"y"	,(char*)"a"	,(char*)"e"	,(char*)"a"	,(char*)"e"	},
	{(char*)"yY"	,(char*)"birisinde"	,(char*)"bir ölümsüzde"	,(char*)"d"	,(char*)"a"	,(char*)"e"	,(char*)"a"	,(char*)"e"	},
	{(char*)"zZ"	,(char*)"birisinden",(char*)"bir ölümsüzden",(char*)"d"	,(char*)"an",(char*)"en",(char*)"an",(char*)"en"	},
    {NULL	,NULL			,NULL				,NULL	,NULL	,NULL	,NULL	,NULL	}
};

// =============================================================================
// STATIC HELPER FUNCTIONS
// =============================================================================

// Function to get the length of a UTF-8 character.
// Returns 0 if s is NULL or points to a null terminator.
// Returns 1 for invalid UTF-8 sequence to allow skipping.
static int utf8_char_len(const unsigned char *s) {
    if (!s || *s == '\0') return 0;
    if (*s < 0x80) return 1;      // 0xxxxxxx ASCII
    if ((*s & 0xE0) == 0xC0) { // 110xxxxx
        if (*(s+1) && (*(s+1) & 0xC0) == 0x80) return 2;
    } else if ((*s & 0xF0) == 0xE0) { // 1110xxxx
        if (*(s+1) && (*(s+1) & 0xC0) == 0x80 &&
            *(s+2) && (*(s+2) & 0xC0) == 0x80) return 3;
    } else if ((*s & 0xF8) == 0xF0) { // 11110xxx
        if (*(s+1) && (*(s+1) & 0xC0) == 0x80 &&
            *(s+2) && (*(s+2) & 0xC0) == 0x80 &&
            *(s+3) && (*(s+3) & 0xC0) == 0x80) return 4;
    }
    return 1; // Invalid sequence, treat as 1 byte to skip.
}

// =============================================================================
// PUBLIC FUNCTIONS - Turkish Morphology Core Functions
// =============================================================================

// Implementation of convert_to_turkish_lowercase_utf8
// (Consolidated from both files, keeping the more robust version from turkish_morphology.c)
void convert_to_turkish_lowercase_utf8(const char *input, char *output, size_t output_size) {
    const unsigned char *p = (const unsigned char *)input;
    unsigned char *q = (unsigned char *)output;
    size_t remaining_size = output_size - 1;

    if (!input || !output || output_size == 0) {
        if (output && output_size > 0) *output = '\0';
        return;
    }

    while (*p && remaining_size > 0) {
        int len = utf8_char_len(p);
        if (len == 0) break; // Should not happen if *p is true

        // Special Turkish uppercase to lowercase conversions
        if (len == 1) { // ASCII
            if (*p == 'I') { // ASCII 'I' to UTF-8 'ı' (0xC4 0xB1)
                if (remaining_size >= 2) {
                    *q++ = 0xC4; *q++ = 0xB1; remaining_size -= 2;
                } else { break; }
            } else {
                if (remaining_size >= 1) {
                    *q++ = tolower(*p); remaining_size--;
                } else { break; }
            }
        } else if (len == 2) {
            unsigned char c1 = *p;
            unsigned char c2 = *(p+1);
            if (c1 == 0xC4 && c2 == 0xB0) { // İ (0xC4 0xB0) -> i (ASCII 'i')
                if (remaining_size >= 1) {
                    *q++ = 'i'; remaining_size--;
                } else { break; }
            } else if (c1 == 0xC3 && c2 == 0x87) { // Ç (0xC3 0x87) -> ç (0xC3 0xA7)
                if (remaining_size >= 2) { *q++ = 0xC3; *q++ = 0xA7; remaining_size -= 2; } else { break; }
            } else if (c1 == 0xC4 && c2 == 0x9E) { // Ğ (0xC4 0x9E) -> ğ (0xC4 0x9F)
                if (remaining_size >= 2) { *q++ = 0xC4; *q++ = 0x9F; remaining_size -= 2; } else { break; }
            } else if (c1 == 0xC3 && c2 == 0x96) { // Ö (0xC3 0x96) -> ö (0xC3 0xB6)
                if (remaining_size >= 2) { *q++ = 0xC3; *q++ = 0xB6; remaining_size -= 2; } else { break; }
            } else if (c1 == 0xC5 && c2 == 0x9E) { // Ş (0xC5 0x9E) -> ş (0xC5 0x9F)
                if (remaining_size >= 2) { *q++ = 0xC5; *q++ = 0x9F; remaining_size -= 2; } else { break; }
            } else if (c1 == 0xC3 && c2 == 0x9C) { // Ü (0xC3 0x9C) -> ü (0xC3 0xBC)
                if (remaining_size >= 2) { *q++ = 0xC3; *q++ = 0xBC; remaining_size -= 2; } else { break; }
            } else { // Not a special Turkish char, copy as is
                if (remaining_size >= (size_t)len) {
                    memcpy(q, p, len); q += len; remaining_size -= len;
                } else { break; }
            }
        } else { // 3 or 4 byte chars, or invalid (len=1), copy as is
            if (remaining_size >= (size_t)len) {
                memcpy(q, p, len); q += len; remaining_size -= len;
            } else { break; }
        }
        p += len;
    }
    *q = '\0';
}

int get_last_char_utf8(const char *str, char *last_char_buffer, size_t buffer_size) {
    if (!str || *str == '\0' || !last_char_buffer || buffer_size == 0) return 0;

    const unsigned char *s = (const unsigned char *)str;
    const unsigned char *last_s_ptr = NULL;
    int current_char_len = 0;

    if (*s == '\0') return 0; // Empty string

    while (*s != '\0') {
        last_s_ptr = s;
        current_char_len = utf8_char_len(s);
        if (current_char_len == 0) return 0; // Should not happen
        s += current_char_len;
    }

    if (last_s_ptr) {
        if (buffer_size < (size_t)current_char_len + 1) return 0; // Not enough space for char + null
        memcpy(last_char_buffer, last_s_ptr, current_char_len);
        last_char_buffer[current_char_len] = '\0';
        return current_char_len;
    }
    return 0;
}

// Turkish vowels: a, e, ı, i, o, ö, u, ü
// Their UTF-8 representations (lowercase):
// a: 0x61
// e: 0x65
// ı: 0xC4 0xB1
// i: 0x69
// o: 0x6F
// ö: 0xC3 0xB6
// u: 0x75
// ü: 0xC3 0xBC
bool is_turkish_vowel(const char *utf8_char_ptr, int char_len) {
    if (!utf8_char_ptr || char_len <= 0) return false;
    unsigned char c1 = (unsigned char)utf8_char_ptr[0];
    unsigned char c2 = (char_len > 1) ? (unsigned char)utf8_char_ptr[1] : 0;

    if (char_len == 1) {
        return (c1 == 'a' || c1 == 'e' || c1 == 'i' || c1 == 'o' || c1 == 'u');
    } else if (char_len == 2) {
        if (c1 == 0xC4 && c2 == 0xB1) return true; // ı
        if (c1 == 0xC3 && c2 == 0xB6) return true; // ö
        if (c1 == 0xC3 && c2 == 0xBC) return true; // ü
    }
    return false;
}

int get_last_turkish_vowel_utf8(const char *str, char *last_vowel_buffer, size_t buffer_size) {
    if (!str || *str == '\0' || !last_vowel_buffer || buffer_size == 0) return 0;

    const unsigned char *s = (const unsigned char *)str;
    const unsigned char *last_vowel_ptr = NULL;
    int last_vowel_len = 0;
    
    char current_char_orig_case_buffer[5]; // Max 4 bytes UTF-8 char + null
    char current_char_lower_case_buffer[5];

    while (*s != '\0') {
        int current_char_len = utf8_char_len(s);
        if (current_char_len == 0) break;
        if (current_char_len > 4) { s += current_char_len; continue; } // Should not happen

        memcpy(current_char_orig_case_buffer, s, current_char_len);
        current_char_orig_case_buffer[current_char_len] = '\0';

        convert_to_turkish_lowercase_utf8(current_char_orig_case_buffer, current_char_lower_case_buffer, sizeof(current_char_lower_case_buffer));
        int lower_char_len = strlen(current_char_lower_case_buffer);

        if (is_turkish_vowel(current_char_lower_case_buffer, lower_char_len)) {
            // Store the original cased vowel from the input string
            last_vowel_ptr = s; 
            last_vowel_len = current_char_len;
        }
        s += current_char_len;
    }

    if (last_vowel_ptr) {
        if (buffer_size < (size_t)last_vowel_len + 1) return 0; // Not enough space
        memcpy(last_vowel_buffer, last_vowel_ptr, last_vowel_len);
        last_vowel_buffer[last_vowel_len] = '\0';
        return last_vowel_len;
    }
    *last_vowel_buffer = '\0';
    return 0;
}

bool ends_with_turkish_vowel_utf8(const char *str) {
    char last_char_buf[5];
    int last_char_len = get_last_char_utf8(str, last_char_buf, sizeof(last_char_buf));
    if (last_char_len == 0) return false;

    char last_char_lower_buf[5];
    convert_to_turkish_lowercase_utf8(last_char_buf, last_char_lower_buf, sizeof(last_char_lower_buf));
    int lower_len = strlen(last_char_lower_buf);

    return is_turkish_vowel(last_char_lower_buf, lower_len);
}

// Consonants that soften: p, ç, t, k (lowercase)
// p -> b
// ç (0xC3 0xA7) -> c (0x63)
// t -> d
// k -> ğ (0xC4 0x9F) or g (0x67) - using ğ as it's more common for suffixation.
bool is_consonant_that_softens_turkish(const char *utf8_char_ptr, int char_len) {
    if (!utf8_char_ptr || char_len <= 0) return false;
    // Ensure we are checking lowercase for these rules
    char lower_char_buf[5];
    char orig_char_buf[5];
    if(char_len > 4) return false;
    memcpy(orig_char_buf, utf8_char_ptr, char_len);
    orig_char_buf[char_len] = '\0';
    convert_to_turkish_lowercase_utf8(orig_char_buf, lower_char_buf, sizeof(lower_char_buf));
    int lower_len = strlen(lower_char_buf);

    unsigned char c1 = (unsigned char)lower_char_buf[0];
    unsigned char c2 = (lower_len > 1) ? (unsigned char)lower_char_buf[1] : 0;

    if (lower_len == 1) {
        return (c1 == 'p' || c1 == 't' || c1 == 'k');
    } else if (lower_len == 2) {
        return (c1 == 0xC3 && c2 == 0xA7); // ç
    }
    return false;
}

int get_softened_turkish_consonant(const char *utf8_char_ptr, int char_len, char *softened_char_buffer, size_t softened_buffer_size) {
    if (!utf8_char_ptr || char_len <= 0 || !softened_char_buffer || softened_buffer_size == 0) return 0;
    
    softened_char_buffer[0] = '\0';

    // Work with lowercase versions for rule matching
    char lower_char_buf[5];
    char orig_char_buf[5];
    if(char_len > 4) return 0;
    memcpy(orig_char_buf, utf8_char_ptr, char_len);
    orig_char_buf[char_len] = '\0';
    convert_to_turkish_lowercase_utf8(orig_char_buf, lower_char_buf, sizeof(lower_char_buf));
    int lower_len = strlen(lower_char_buf);

    unsigned char lc1 = (unsigned char)lower_char_buf[0];
    unsigned char lc2 = (lower_len > 1) ? (unsigned char)lower_char_buf[1] : 0;

    int softened_len = 0;

    if (lower_len == 1 && lc1 == 'p') { // p -> b
        if (softened_buffer_size >= 2) { softened_char_buffer[0] = 'b'; softened_char_buffer[1] = '\0'; softened_len = 1; }
    } else if (lower_len == 2 && lc1 == 0xC3 && lc2 == 0xA7) { // ç -> c
        if (softened_buffer_size >= 2) { softened_char_buffer[0] = 'c'; softened_char_buffer[1] = '\0'; softened_len = 1; }
    } else if (lower_len == 1 && lc1 == 't') { // t -> d
        if (softened_buffer_size >= 2) { softened_char_buffer[0] = 'd'; softened_char_buffer[1] = '\0'; softened_len = 1; }
    } else if (lower_len == 1 && lc1 == 'k') { // k -> ğ (0xC4 0x9F)
        if (softened_buffer_size >= 3) { softened_char_buffer[0] = 0xC4; softened_char_buffer[1] = 0x9F; softened_char_buffer[2] = '\0'; softened_len = 2; }
    } else { // Not a soften-able consonant, or already soft. Copy original.
        if (softened_buffer_size >= (size_t)char_len + 1) {
            memcpy(softened_char_buffer, utf8_char_ptr, char_len);
            softened_char_buffer[char_len] = '\0';
            softened_len = char_len;
        } else { return 0; /* Not enough buffer for original char */ }
    }
    return softened_len;
}

// --- Main Suffix Generation Logic ---
int generate_turkish_suffixed_word(
    const char *base_word,
    enum TurkishSuffixType type,
    bool is_proper_noun,
    char *output_buffer,
    size_t buffer_size) {
    if (!base_word || !output_buffer || buffer_size == 0) {
        if (output_buffer && buffer_size > 0) output_buffer[0] = '\0';
        return -2;
    }
    output_buffer[0] = '\0';
    size_t base_word_len = strlen(base_word);
    if (base_word_len == 0) return 0;
    char last_vowel_in_word[5]; // Increased size for safety with UTF-8
    get_last_turkish_vowel_utf8(base_word, last_vowel_in_word, sizeof(last_vowel_in_word));
    char last_vowel_lower[5]; // Increased size
    convert_to_turkish_lowercase_utf8(last_vowel_in_word, last_vowel_lower, sizeof(last_vowel_lower));
    bool word_ends_v = ends_with_turkish_vowel_utf8(base_word);
    char current_base_word[MAX_STRING_LENGTH];
    strncpy(current_base_word, base_word, MAX_STRING_LENGTH -1);
    current_base_word[MAX_STRING_LENGTH-1] = '\0';
    size_t current_base_word_len = base_word_len;
    char last_char_of_base[5]; // Increased size
    int last_char_len = get_last_char_utf8(current_base_word, last_char_of_base, sizeof(last_char_of_base));
    bool suffix_starts_with_vowel = false;
    switch (type) {
        case SUFFIX_GENITIVE:
        case SUFFIX_ACCUSATIVE:
        case SUFFIX_DATIVE:
            suffix_starts_with_vowel = true;
            break;
        default:
            suffix_starts_with_vowel = false;
            break;
    }
    if (!word_ends_v && last_char_len > 0 && is_consonant_that_softens_turkish(last_char_of_base, last_char_len)) {
        if (!is_proper_noun && suffix_starts_with_vowel) {
            char softened_cons[5]; // Increased size
            int softened_len = get_softened_turkish_consonant(last_char_of_base, last_char_len, softened_cons, sizeof(softened_cons));
            if (softened_len > 0 && strncmp(last_char_of_base, softened_cons, softened_len) != 0) { // Compare actual strings
                current_base_word_len = current_base_word_len - last_char_len + softened_len;
                if (current_base_word_len < MAX_STRING_LENGTH) {
                     current_base_word[base_word_len - last_char_len] = '\0'; // Truncate
                     strcat(current_base_word, softened_cons); // Append softened
                } else { /* Handle error: buffer too small */ return -1; }
            }
        }
    }
    const char *suffix_form = "";
    char buffer_consonant[2] = ""; // "y" or "n" or ""
    // Vowel Harmony Rules:
    // Last vowel: a, ı -> suffix vowel: a, ı (e.g., -ın, -ı, -a, -da, -dan)
    // Last vowel: e, i -> suffix vowel: e, i (e.g., -in, -i, -e, -de, -den)
    // Last vowel: o, u -> suffix vowel: u (e.g., -un, -u, -a (dative exception), -da, -dan)
    // Last vowel: ö, ü -> suffix vowel: ü (e.g., -ün, -ü, -e (dative exception), -de, -den)
    unsigned char u_lv = (unsigned char)last_vowel_lower[0]; // First byte of last vowel (lowercase)
    unsigned char u_lv2 = (strlen(last_vowel_lower) > 1) ? (unsigned char)last_vowel_lower[1] : 0;
    // Determine suffix based on type and vowel harmony
    switch (type) {
        case SUFFIX_GENITIVE: // -in, -ın, -un, -ün. Buffer 'n' if word_ends_vowel.
            if (word_ends_v) strcpy(buffer_consonant, "n");
            if (u_lv == 'a' || (u_lv == 0xC4 && u_lv2 == 0xB1)) suffix_form = "ın"; // a, ı
            else if (u_lv == 'e' || u_lv == 'i') suffix_form = "in";                   // e, i
            else if (u_lv == 'o' || u_lv == 'u') suffix_form = "un";                   // o, u
            else if ((u_lv == 0xC3 && u_lv2 == 0xB6) || (u_lv == 0xC3 && u_lv2 == 0xBC)) suffix_form = "ün"; // ö, ü
            else suffix_form = "in"; // Default or error case for vowel harmony
            break;
        case SUFFIX_ACCUSATIVE: // -i, -ı, -u, -ü. Buffer 'y' if word_ends_vowel.
            if (word_ends_v) strcpy(buffer_consonant, "y");
            if (u_lv == 'a' || (u_lv == 0xC4 && u_lv2 == 0xB1)) suffix_form = "ı";
            else if (u_lv == 'e' || u_lv == 'i') suffix_form = "i";
            else if (u_lv == 'o' || u_lv == 'u') suffix_form = "u";
            else if ((u_lv == 0xC3 && u_lv2 == 0xB6) || (u_lv == 0xC3 && u_lv2 == 0xBC)) suffix_form = "ü";
            else suffix_form = "i";
            break;
        case SUFFIX_DATIVE: // -e, -a. Buffer 'y' if word_ends_vowel.
            if (word_ends_v) strcpy(buffer_consonant, "y");
            if (u_lv == 'a' || (u_lv == 0xC4 && u_lv2 == 0xB1) || u_lv == 'o' || u_lv == 'u') suffix_form = "a"; // a, ı, o, u
            else if (u_lv == 'e' || u_lv == 'i' || (u_lv == 0xC3 && u_lv2 == 0xB6) || (u_lv == 0xC3 && u_lv2 == 0xBC)) suffix_form = "e"; // e, i, ö, ü
            else suffix_form = "e";
            break;
        case SUFFIX_LOCATIVE: // -de, -da, -te, -ta
        case SUFFIX_ABLATIVE: { // -den, -dan, -ten, -tan
            // For LOCATIVE and ABLATIVE, check for consonant hardening (fıstıkçı şahap)
            bool use_hard_consonant_suffix = false;
            if (!word_ends_v && last_char_len > 0) {
                // Check if the last character is one of f, s, t, k, ç, ş, h, p
                char last_char_lower_buf[5];
                convert_to_turkish_lowercase_utf8(last_char_of_base, last_char_lower_buf, sizeof(last_char_lower_buf));
                unsigned char lcl_c1 = (unsigned char)last_char_lower_buf[0];
                // Simplified check, assuming single-byte consonants for f,s,t,k,h,p
                // and specific UTF-8 for ç, ş
                if ( (strlen(last_char_lower_buf) == 1 &&
                      (lcl_c1 == 'f' || lcl_c1 == 's' || lcl_c1 == 't' || lcl_c1 == 'k' || lcl_c1 == 'h' || lcl_c1 == 'p')) ||
                     ((strlen(last_char_lower_buf) == 2) && // Check for ç (0xC3 0xA7) and ş (0xC5 0x9F)
                      (((unsigned char)last_char_lower_buf[0] == 0xC3 && (unsigned char)last_char_lower_buf[1] == 0xA7) || // ç
                       ((unsigned char)last_char_lower_buf[0] == 0xC5 && (unsigned char)last_char_lower_buf[1] == 0x9F)))    // ş
                   ) {
                    use_hard_consonant_suffix = true;
                }
            }
            if (type == SUFFIX_LOCATIVE) {
                if (u_lv == 'a' || (u_lv == 0xC4 && u_lv2 == 0xB1) || u_lv == 'o' || u_lv == 'u') suffix_form = use_hard_consonant_suffix ? "ta" : "da";
                else if (u_lv == 'e' || u_lv == 'i' || (u_lv == 0xC3 && u_lv2 == 0xB6) || (u_lv == 0xC3 && u_lv2 == 0xBC)) suffix_form = use_hard_consonant_suffix ? "te" : "de";
                else suffix_form = use_hard_consonant_suffix ? "te" : "de";
            } else { // SUFFIX_ABLATIVE
                if (u_lv == 'a' || (u_lv == 0xC4 && u_lv2 == 0xB1) || u_lv == 'o' || u_lv == 'u') suffix_form = use_hard_consonant_suffix ? "tan" : "dan";
                else if (u_lv == 'e' || u_lv == 'i' || (u_lv == 0xC3 && u_lv2 == 0xB6) || (u_lv == 0xC3 && u_lv2 == 0xBC)) suffix_form = use_hard_consonant_suffix ? "ten" : "den";
                else suffix_form = use_hard_consonant_suffix ? "ten" : "den";
            }
            break;
        }
        default:
            return -2; // Should not happen
    }
    // Construct the final word
    if (is_proper_noun) {
        snprintf(output_buffer, buffer_size, "%s'%s%s", current_base_word, buffer_consonant, suffix_form);
    } else {
        snprintf(output_buffer, buffer_size, "%s%s%s", current_base_word, buffer_consonant, suffix_form);
    }
    // Check for buffer overflow
    size_t required_len = strlen(current_base_word) + strlen(buffer_consonant) + strlen(suffix_form) + (is_proper_noun ? 1 : 0);
    if (required_len >= buffer_size) {
         // Truncate and indicate error, or handle as preferred
         output_buffer[buffer_size-1] = '\0';
         return -1; // Error: buffer too small
    }
    return 0; // Success
}

// =============================================================================
// PUBLIC FUNCTIONS - From ek.c
// =============================================================================

// Refactored ekler function
char *ekler(CHAR_DATA *to, CHAR_DATA *ch, char *format_char_ptr) {
    // Using one static buffer. Be cautious if ekler can be called in a nested fashion
    // within a single statement, as this buffer would be overwritten.
    static char result_buffer[MAX_STRING_LENGTH]; 
    result_buffer[0] = '\0';

    if (!ch || !format_char_ptr || *format_char_ptr == '\0') {
        return result_buffer; // Return empty string for invalid input
    }

    char format_char = *format_char_ptr;
    enum TurkishSuffixType suffix_type = SUFFIX_GENITIVE; // Default suffix type
    bool mapping_found = FALSE;
    int table_idx_for_invisible = 0; // To pick the correct "birisi..." string if ch is not visible

    // Map the legacy format_char to TurkishSuffixType
    for (int i = 0; pers_suffix_table[i].belirtec != NULL; ++i) {
        if (strchr(pers_suffix_table[i].belirtec, format_char)) {
            table_idx_for_invisible = i; // Save this index
            mapping_found = TRUE;
            // Determine suffix_type based on the format character or table row
            if (strchr("sS", format_char)) { suffix_type = SUFFIX_GENITIVE; break; }
            if (strchr("mM", format_char)) { suffix_type = SUFFIX_ACCUSATIVE; break; }
            if (strchr("eE", format_char)) { suffix_type = SUFFIX_DATIVE; break; }
            if (strchr("yY", format_char)) { suffix_type = SUFFIX_LOCATIVE; break; }
            if (strchr("zZ", format_char)) { suffix_type = SUFFIX_ABLATIVE; break; }
            
            // Fallback: if a belirtec matched but not a specific char, use table order
            // This part assumes a correspondence between table order and suffix types.
            switch(i) {
                case 0: suffix_type = SUFFIX_GENITIVE; break;
                case 1: suffix_type = SUFFIX_ACCUSATIVE; break;
                case 2: suffix_type = SUFFIX_DATIVE; break;
                case 3: suffix_type = SUFFIX_LOCATIVE; break;
                case 4: suffix_type = SUFFIX_ABLATIVE; break;
                default: suffix_type = SUFFIX_GENITIVE; // Should ideally not be reached
            }
            break; 
        }
    }

    if (!mapping_found) {
        // If format_char is unknown and ch is not visible, return a generic default.
        // If ch is visible, will use the default suffix_type (SUFFIX_GENITIVE).
        if (!can_see(to, ch)) {
            if (pers_suffix_table[0].birisi) // Default to the first "birisi..." string
                snprintf(result_buffer, MAX_STRING_LENGTH, "%s", pers_suffix_table[0].birisi);
            else
                snprintf(result_buffer, MAX_STRING_LENGTH, "%s", "biri"); // Absolute fallback
            return result_buffer;
        }
    }

    if (can_see(to, ch)) {
        const char *base_name = IS_NPC(ch) ? ch->short_descr : ch->name;
        bool is_proper = !IS_NPC(ch); // Player names are proper nouns

        if (!base_name || *base_name == '\0') {
            return result_buffer; // Empty base name, return empty string
        }

        // Call the new function from turkish_morphology module
        int result = generate_turkish_suffixed_word(base_name, suffix_type, is_proper, result_buffer, MAX_STRING_LENGTH);
        
        if (result != 0) { 
            // Error in suffix generation (e.g., buffer too small)
            // Return base_name as a fallback if result_buffer is empty or indicates error
            if (result_buffer[0] == '\0' || result == -1 /* buffer too small */) {
                snprintf(result_buffer, MAX_STRING_LENGTH, "%s", base_name);
            }
            // result_buffer might contain truncated output or base_name
        }
        return result_buffer;

    } else {
        // Cannot see 'ch', use the generic placeholder from pers_suffix_table
        const char* invisible_placeholder = NULL;

        // Select the correct "birisi..." string based on the original format_char
        // This uses the direct mapping as intended by the table structure for these cases.
        if (strchr("sS", format_char) && pers_suffix_table[0].birisi) invisible_placeholder = pers_suffix_table[0].birisi;
        else if (strchr("mM", format_char) && pers_suffix_table[1].birisi) invisible_placeholder = pers_suffix_table[1].birisi;
        else if (strchr("eE", format_char) && pers_suffix_table[2].birisi) invisible_placeholder = pers_suffix_table[2].birisi;
        else if (strchr("yY", format_char) && pers_suffix_table[3].birisi) invisible_placeholder = pers_suffix_table[3].birisi;
        else if (strchr("zZ", format_char) && pers_suffix_table[4].birisi) invisible_placeholder = pers_suffix_table[4].birisi;
        else if (mapping_found && pers_suffix_table[table_idx_for_invisible].birisi) {
            // Fallback to the matched table row's "birisi" if specific char not found above
            invisible_placeholder = pers_suffix_table[table_idx_for_invisible].birisi;
        } else if (pers_suffix_table[0].birisi) { // Further fallback
            invisible_placeholder = pers_suffix_table[0].birisi;
        } else {
            invisible_placeholder = "biri"; // Absolute fallback
        }
        
        snprintf(result_buffer, MAX_STRING_LENGTH, "%s", invisible_placeholder);
        return result_buffer;
    }
}

bool str_prefix_turkish(const char *astr, const char *bstr)
{
    // MAX_INPUT_LENGTH should be defined in merc.h or similar
    // If not, this function might have issues. For now, assuming it's available.
    char lowercase_astr[MAX_INPUT_LENGTH]; 
    char lowercase_bstr[MAX_INPUT_LENGTH];

    if (astr == NULL || bstr == NULL) // Should this be TRUE or FALSE? Original was TRUE.
        return TRUE; // Original behavior: if one is NULL, they are "different" (not a prefix)

    if (*astr == '\0' && *bstr == '\0') return FALSE; // "" is prefix of ""
    if (*astr == '\0') return FALSE; // "" is prefix of anything
    if (*bstr == '\0' && *astr != '\0') return TRUE; // "abc" is not prefix of ""

    convert_to_turkish_lowercase_utf8(astr, lowercase_astr, MAX_INPUT_LENGTH);
    convert_to_turkish_lowercase_utf8(bstr, lowercase_bstr, MAX_INPUT_LENGTH);

    size_t astr_len = strlen(lowercase_astr);
    if (astr_len == 0) // Should have been caught by *astr == '\0'
        return FALSE; 
    
    // strncmp returns 0 if equal up to N characters.
    // We want to return FALSE if it IS a prefix (strncmp is 0)
    // and TRUE if it is NOT a prefix (strncmp is non-zero).
    return strncmp(lowercase_astr, lowercase_bstr, astr_len) != 0;
}