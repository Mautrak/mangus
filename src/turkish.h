/***************************************************************************
 * turkish.h - Türkçe ad durumu ekleri (act() içindeki $s $m $e $y $z).    *
 ***************************************************************************/
#ifndef TURKISH_H
#define TURKISH_H

#include "merc.h"

/* Verilen sözcüğün son ünlüsüne göre ünlü uyumu sınıfı: 0 a/ı, 1 e/i,
 * 2 o/u, 3 ö/ü; ünlü yoksa -1. */
int         tr_vowel_class(const char *word);
/* Sözcük ünlüyle mi bitiyor? */
bool        tr_ends_with_vowel(const char *word);
/* Son sert ünsüzü yumuşatır (p/ç/t/k -> b/c/d/ğ, nk -> ng); tek heceliye dokunmaz. */
void        tr_soften(char *word);
/* act() için: 'to' görebiliyorsa 'ch' adına ek takılmış hali ("Ali'nin",
 * "goblinin"); göremiyorsa "birisinin" gibi genel biçim. Statik tampon. */
const char *ekler(CHAR_DATA *to, CHAR_DATA *ch, const char *format);

#endif
