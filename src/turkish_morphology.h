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

#ifndef TURKISH_MORPHOLOGY_H
#define TURKISH_MORPHOLOGY_H

#include <stddef.h> // For size_t
#include <stdbool.h> // For bool

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h" // For CHAR_DATA definition

// =============================================================================
// ENUMS AND STRUCTS
// =============================================================================

// Represents the type of Turkish suffix to apply.
// Values should cover common cases like genitive, accusative, dative, locative, ablative.
// More can be added as needed.
enum TurkishSuffixType {
    SUFFIX_GENITIVE,    // -in, -ın, -un, -ün (of/s)
    SUFFIX_ACCUSATIVE,  // -i, -ı, -u, -ü (direct object)
    SUFFIX_DATIVE,      // -e, -a (to/for)
    SUFFIX_LOCATIVE,    // -de, -da, -te, -ta (in/on/at)
    SUFFIX_ABLATIVE,    // -den, -dan, -ten, -tan (from)
    SUFFIX_INSTRUMENTAL,// -le, -la, -yle, -yla (with/by means of)
    SUFFIX_PLURAL,      // -ler, -lar (plural)
    // New inflected plural types (plural + case suffix)
    SUFFIX_GENITIVE_PLURAL,     // -ların, -lerin (of plurals)
    SUFFIX_DATIVE_PLURAL,       // -lara, -lere (to plurals)
    SUFFIX_ACCUSATIVE_PLURAL,   // -ları, -leri (direct object plurals)
    SUFFIX_ABLATIVE_PLURAL,     // -lardan, -lerden (from plurals)
    SUFFIX_INSTRUMENTAL_PLURAL, // -larla, -lerle (with plurals)
    SUFFIX_LOCATIVE_PLURAL      // -larda, -lerde (in/on/at plurals)
};

// From ek.h
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

// =============================================================================
// EXTERNAL DATA
// =============================================================================

extern const struct pers_suffix_type pers_suffix_table[];

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

/**
 * Generates a Turkish word with the specified suffix, applying vowel harmony,
 * consonant softening, and buffer consonants.
 *
 * @param base_word The base word to append the suffix to (UTF-8 encoded).
 * @param type The type of suffix to apply (e.g., SUFFIX_GENITIVE).
 * @param is_proper_noun If true, an apostrophe will be used before the suffix if the word ends in a consonant,
 *                       or directly after the vowel if it ends in a vowel (e.g. Ankara'ya, Elif'in).
 *                       If false, no apostrophe is used (e.g. masaya, kedinin).
 * @param output_buffer Buffer to store the resulting word with the suffix.
 * @param buffer_size Size of the output_buffer.
 * @return 0 on success, -1 if buffer_size is too small, -2 for other errors.
 */
int generate_turkish_suffixed_word(
    const char *base_word,
    enum TurkishSuffixType type,
    bool is_proper_noun,
    char *output_buffer,
    size_t buffer_size);

/**
 * Converts a given UTF-8 string to Turkish lowercase.
 * Handles special Turkish characters: I->ı, İ->i, Ş->ş, Ğ->ğ, Ü->ü, Ö->ö, Ç->ç.
 *
 * @param input The input UTF-8 string.
 * @param output The buffer to store the lowercased string.
 * @param output_size The size of the output buffer.
 */
void convert_to_turkish_lowercase_utf8(const char *input, char *output, size_t output_size);

/**
 * Extracts the last UTF-8 character from a string.
 * @param str The input UTF-8 string.
 * @param last_char_buffer Buffer to store the last UTF-8 character. Must be at least 5 bytes.
 * @param buffer_size Size of the last_char_buffer.
 * @return Length of the last UTF-8 character (1-4 bytes), or 0 if string is empty or error.
 */
int get_last_char_utf8(const char *str, char *last_char_buffer, size_t buffer_size);

/**
 * Extracts the last Turkish vowel from a UTF-8 string.
 * @param str The input UTF-8 string.
 * @param last_vowel_buffer Buffer to store the last UTF-8 vowel. Must be at least 3 bytes for Turkish vowels.
 * @param buffer_size Size of the last_vowel_buffer.
 * @return Length of the last UTF-8 vowel (1-2 bytes for Turkish), or 0 if no vowel found or error.
 */
int get_last_turkish_vowel_utf8(const char *str, char *last_vowel_buffer, size_t buffer_size);

/**
 * Checks if a given UTF-8 character sequence is a Turkish vowel.
 * @param utf8_char Pointer to the start of the UTF-8 character.
 * @param char_len Length of the UTF-8 character in bytes.
 * @return True if it's a Turkish vowel, false otherwise.
 */
bool is_turkish_vowel(const char *utf8_char, int char_len);

/**
 * Checks if the given UTF-8 string ends with a Turkish vowel.
 * @param str The input UTF-8 string.
 * @return True if the string ends with a Turkish vowel, false otherwise.
 */
bool ends_with_turkish_vowel_utf8(const char *str);

/**
 * Checks if a given UTF-8 character is one of the Turkish consonants that softens (p, ç, t, k).
 * @param utf8_char Pointer to the start of the UTF-8 character.
 * @param char_len Length of the UTF-8 character.
 * @return True if it's a consonant that softens, false otherwise.
 */
bool is_consonant_that_softens_turkish(const char *utf8_char, int char_len);

/**
 * Gets the softened form of a Turkish consonant (p->b, ç->c, t->d, k->ğ).
 * Assumes utf8_char is one of these.
 * @param utf8_char Pointer to the start of the consonant UTF-8 character.
 * @param char_len Length of the consonant character.
 * @param softened_char_buffer Buffer to store the softened UTF-8 character. Must be at least 3 bytes.
 * @param softened_buffer_size Size of the softened_char_buffer.
 * @return Length of the softened character, or 0 on error.
 */
int get_softened_turkish_consonant(const char *utf8_char, int char_len, char *softened_char_buffer, size_t softened_buffer_size);

/**
 * Main function for Turkish suffix handling in the MUD context.
 * Declaration for ekler - ensure CHAR_DATA is known (via merc.h)
 * @param to The character who will see the result
 * @param ch The character whose name will be suffixed
 * @param format The format character indicating which suffix to apply
 * @return Pointer to the suffixed name string
 */
char *ekler(CHAR_DATA *to, CHAR_DATA *ch, char *format);

/**
 * Turkish-aware string prefix comparison function.
 * @param astr The prefix string to check
 * @param bstr The string to check against
 * @return True if astr is NOT a prefix of bstr, false if it IS a prefix
 */
bool str_prefix_turkish(const char *astr, const char *bstr);

#endif // TURKISH_MORPHOLOGY_H
