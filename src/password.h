/***************************************************************************
 * password.h - Taşınabilir parola özeti (tuzlu, yinelemeli SHA-256).      *
 * Eski crypt(3) özetleri POSIX'te doğrulanır ve girişte yükseltilir.      *
 ***************************************************************************/
#ifndef PASSWORD_H
#define PASSWORD_H

#include <stdbool.h>
#include <stddef.h>

#define PWD_MAX_LEN 128

/* Yeni biçimde ("$m1$tuz$özet") özet üretir; statik tampon döndürür. */
const char *pwd_hash(const char *plain);
/* Saklanan özet ile düz metni karşılaştırır. */
bool        pwd_check(const char *stored, const char *plain);
/* Saklanan özet eski biçimde mi (crypt(3))? */
bool        pwd_is_legacy(const char *stored);
/* SHA-256'nın onaltılık çıktısı (testler için). */
void        sha256_hex(const void *data, size_t len, char out[65]);

#endif
