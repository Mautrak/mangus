/***************************************************************************
 * utf8.h - UTF-8 yardımcıları ve Türkçe farkındalıklı büyük/küçük harf   *
 * dönüşümü. Oyun metinleri, alan dosyaları ve ağ trafiği UTF-8'dir.      *
 ***************************************************************************/
#ifndef UTF8_H
#define UTF8_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UTF8_REPLACEMENT 0xFFFDu

/* Bir kod noktası çözer; tüketilen bayt sayısını döndürür (dizgi sonunda 0).
 * Geçersiz baytlar tek tek tüketilir ve *cp = UTF8_REPLACEMENT olur. */
int      utf8_decode(const char *s, uint32_t *cp);
/* Kod noktasını yazar (en fazla 4 bayt, NUL eklemez); yazılan bayt sayısı. */
int      utf8_encode(uint32_t cp, char *out);
/* Kod noktası sayısı. */
size_t   utf8_strlen(const char *s);
/* printf'in %-*s hizalaması için: istenen görsel genişlik + fazladan baytlar. */
int      utf8_width(const char *s, int width);
/* Türkçe kurallı harf dönüşümleri (I<->ı, İ<->i, Ç Ğ Ö Ş Ü). */
uint32_t utf8_tolower_cp(uint32_t cp);
uint32_t utf8_toupper_cp(uint32_t cp);
/* Büyük/küçük harf duyarsız karşılaştırma anahtarı: küçük harf + ı->i. */
uint32_t utf8_fold_cp(uint32_t cp);
bool     utf8_is_alpha_cp(uint32_t cp);
bool     utf8_is_upper_cp(uint32_t cp);
/* Dizginin ilk harfini büyütür (gerekirse genişler; cap tampon boyutu). */
void     utf8_upper_first(char *s, size_t cap);
/* Tüm dizgiyi küçük harfe çevirir (dst en az cap bayt). */
void     utf8_lower_str(char *dst, size_t cap, const char *src);
/* Dizgi geçerli UTF-8 mi? */
bool     utf8_valid(const char *s);
/* Geçersiz UTF-8 baytlarını ISO-8859-9 (Latin-5) kabul edip UTF-8'e çevirir.
 * Eski istemci girdileri ve eski oyuncu dosyaları için. */
void     utf8_from_latin5(char *buf, size_t cap);
/* Dizgiyi kod noktası sınırında en fazla maxbytes bayta kısaltır. */
void     utf8_truncate(char *s, size_t maxbytes);
/* printf'in %-N.Ns kalıbının UTF-8 karşılığı: s'yi width görsel sütuna kod
   noktası sınırında kesip boşlukla doldurarak out'a yazar (cap tampon boyutu). */
void     utf8_fit(const char *s, int width, char *out, size_t cap);
/* Büyük/küçük harf duyarsız karşılaştırmalar; farklıysa true (ROM geleneği). */
bool     utf8_str_cmp(const char *a, const char *b);
bool     utf8_str_prefix(const char *a, const char *b);
/* İki dizginin ilk harfi (büyük/küçük duyarsız) aynı mı? */
bool     utf8_first_eq(const char *a, const char *b);

#endif
