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

#ifndef TURKISH_SUFFIX_HELPER_H
#define TURKISH_SUFFIX_HELPER_H

#include "turkish_morphology.h"
#include <stddef.h>

/**
 * Helper macros for quick suffix generation in sprintf/printf calls.
 * These replace hardcoded suffixes with proper morphological forms.
 */

// Buffer size for suffix operations
#ifndef SUFFIX_BUF_SIZE
#define SUFFIX_BUF_SIZE 512
#endif

/**
 * Generate a string with Turkish genitive suffix (replaces 'nin, 'ın, etc.)
 * Usage: TR_GEN(word, is_proper) 
 */
char *tr_genitive(const char *word, bool is_proper);

/**
 * Generate a string with Turkish accusative suffix (replaces 'yi, 'ı, etc.)
 * Usage: TR_ACC(word, is_proper)
 */
char *tr_accusative(const char *word, bool is_proper);

/**
 * Generate a string with Turkish dative suffix (replaces 'ye, 'e, etc.)
 * Usage: TR_DAT(word, is_proper)
 */
char *tr_dative(const char *word, bool is_proper);

/**
 * Generate a string with Turkish locative suffix (replaces 'de, 'da, etc.)
 * Usage: TR_LOC(word, is_proper)
 */
char *tr_locative(const char *word, bool is_proper);

/**
 * Generate a string with Turkish ablative suffix (replaces 'den, 'dan, etc.)
 * Usage: TR_ABL(word, is_proper)
 */
char *tr_ablative(const char *word, bool is_proper);

/**
 * Auto-detect proper noun and generate genitive
 */
char *tr_genitive_auto(const char *word);

/**
 * Auto-detect proper noun and generate accusative
 */
char *tr_accusative_auto(const char *word);

/**
 * Auto-detect proper noun and generate dative
 */
char *tr_dative_auto(const char *word);

/**
 * Auto-detect proper noun and generate locative
 */
char *tr_locative_auto(const char *word);

/**
 * Auto-detect proper noun and generate ablative
 */
char *tr_ablative_auto(const char *word);

// Convenience macros for cleaner code
#define TR_GEN(word) tr_genitive_auto(word)
#define TR_ACC(word) tr_accusative_auto(word)
#define TR_DAT(word) tr_dative_auto(word)
#define TR_LOC(word) tr_locative_auto(word)
#define TR_ABL(word) tr_ablative_auto(word)

/**
 * Safe sprintf replacement that handles Turkish suffixes
 * Usage: tr_sprintf(buf, size, "%s etiği", TR_GEN(name))
 */
int tr_sprintf(char *buffer, size_t size, const char *format, ...);

#endif // TURKISH_SUFFIX_HELPER_H