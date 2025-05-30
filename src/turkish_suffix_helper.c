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
#include "merc.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef MAX_STRING_LENGTH
#define MAX_STRING_LENGTH 4096
#endif

// Static buffers for suffix functions - rotating to allow multiple calls in single statement
#define NUM_SUFFIX_BUFFERS 8
static char suffix_buffers[NUM_SUFFIX_BUFFERS][SUFFIX_BUF_SIZE];
static int current_buffer = 0;

// Get next buffer in rotation
static char *get_next_suffix_buffer(void) {
    char *buffer = suffix_buffers[current_buffer];
    current_buffer = (current_buffer + 1) % NUM_SUFFIX_BUFFERS;
    return buffer;
}

// Helper function to check if word is likely proper noun
static bool is_proper_noun_auto(const char *word) {
    if (!word || *word == '\0') return false;
    
    // Simple heuristic: starts with uppercase letter
    unsigned char first_byte = (unsigned char)word[0];
    
    // Check for ASCII uppercase
    if (first_byte >= 'A' && first_byte <= 'Z') return true;
    
    // Check for Turkish uppercase characters
    if (strlen(word) >= 2) {
        unsigned char second_byte = (unsigned char)word[1];
        // İ (0xC4 0xB0), Ğ (0xC4 0x9E), Ş (0xC5 0x9E), Ü (0xC3 0x9C), Ö (0xC3 0x96), Ç (0xC3 0x87)
        if ((first_byte == 0xC4 && (second_byte == 0xB0 || second_byte == 0x9E)) ||
            (first_byte == 0xC5 && second_byte == 0x9E) ||
            (first_byte == 0xC3 && (second_byte == 0x9C || second_byte == 0x96 || second_byte == 0x87))) {
            return true;
        }
    }
    
    return false;
}

char *tr_genitive(const char *word, bool is_proper) {
    char *buffer = get_next_suffix_buffer();
    if (!word || *word == '\0') {
        buffer[0] = '\0';
        return buffer;
    }
    
    int result = generate_turkish_suffixed_word(word, SUFFIX_GENITIVE, is_proper, buffer, SUFFIX_BUF_SIZE);
    if (result != 0) {
        // Fallback to original word if suffix generation fails
        strncpy(buffer, word, SUFFIX_BUF_SIZE - 1);
        buffer[SUFFIX_BUF_SIZE - 1] = '\0';
    }
    
    return buffer;
}

char *tr_accusative(const char *word, bool is_proper) {
    char *buffer = get_next_suffix_buffer();
    if (!word || *word == '\0') {
        buffer[0] = '\0';
        return buffer;
    }
    
    int result = generate_turkish_suffixed_word(word, SUFFIX_ACCUSATIVE, is_proper, buffer, SUFFIX_BUF_SIZE);
    if (result != 0) {
        strncpy(buffer, word, SUFFIX_BUF_SIZE - 1);
        buffer[SUFFIX_BUF_SIZE - 1] = '\0';
    }
    
    return buffer;
}

char *tr_dative(const char *word, bool is_proper) {
    char *buffer = get_next_suffix_buffer();
    if (!word || *word == '\0') {
        buffer[0] = '\0';
        return buffer;
    }
    
    int result = generate_turkish_suffixed_word(word, SUFFIX_DATIVE, is_proper, buffer, SUFFIX_BUF_SIZE);
    if (result != 0) {
        strncpy(buffer, word, SUFFIX_BUF_SIZE - 1);
        buffer[SUFFIX_BUF_SIZE - 1] = '\0';
    }
    
    return buffer;
}

char *tr_locative(const char *word, bool is_proper) {
    char *buffer = get_next_suffix_buffer();
    if (!word || *word == '\0') {
        buffer[0] = '\0';
        return buffer;
    }
    
    int result = generate_turkish_suffixed_word(word, SUFFIX_LOCATIVE, is_proper, buffer, SUFFIX_BUF_SIZE);
    if (result != 0) {
        strncpy(buffer, word, SUFFIX_BUF_SIZE - 1);
        buffer[SUFFIX_BUF_SIZE - 1] = '\0';
    }
    
    return buffer;
}

char *tr_ablative(const char *word, bool is_proper) {
    char *buffer = get_next_suffix_buffer();
    if (!word || *word == '\0') {
        buffer[0] = '\0';
        return buffer;
    }
    
    int result = generate_turkish_suffixed_word(word, SUFFIX_ABLATIVE, is_proper, buffer, SUFFIX_BUF_SIZE);
    if (result != 0) {
        strncpy(buffer, word, SUFFIX_BUF_SIZE - 1);
        buffer[SUFFIX_BUF_SIZE - 1] = '\0';
    }
    
    return buffer;
}

// Auto-detect versions
char *tr_genitive_auto(const char *word) {
    return tr_genitive(word, is_proper_noun_auto(word));
}

char *tr_accusative_auto(const char *word) {
    return tr_accusative(word, is_proper_noun_auto(word));
}

char *tr_dative_auto(const char *word) {
    return tr_dative(word, is_proper_noun_auto(word));
}

char *tr_locative_auto(const char *word) {
    return tr_locative(word, is_proper_noun_auto(word));
}

char *tr_ablative_auto(const char *word) {
    return tr_ablative(word, is_proper_noun_auto(word));
}

// Safe sprintf replacement
int tr_sprintf(char *buffer, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}