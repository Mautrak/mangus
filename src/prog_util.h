/***************************************************************************
 * prog_util.h - mob_prog.c / obj_prog.c / quest.c ortak saf yardımcıları. *
 * Sunucuya (merc.h) bağımlı değildir; birim testinde de derlenir.         *
 ***************************************************************************/
#ifndef PROG_UTIL_H
#define PROG_UTIL_H

#include <stdbool.h>
#include <stddef.h>

/* Alan dosyasından gelen şablondaki her "%s" yerine sırasıyla a1, a2 yazılır;
 * başka '%' dizileri ve fazla "%s"ler olduğu gibi kopyalanır (biçim dizgisi
 * olarak asla yorumlanmaz). Sonuç her zaman sonlandırılır. */
void prog_subst(char *buf, size_t size, const char *tmpl, const char *a1, const char *a2);

/* name, text içinde harf/rakama bitişik olmadan tam sözcük olarak geçiyor mu?
 * ("Ali", "Ali'nin kılıcı" -> TRUE; "Ali", "Alican'ın kılıcı" -> FALSE) */
bool prog_name_in_text(const char *text, const char *name);

/* Giyildiğinde seviyeye göre silah zar sayısı (value[2]): 2,3,4,5,6,7,9,11,12 */
int prog_weapon_dice(int level);

#endif
