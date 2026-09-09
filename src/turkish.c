/***************************************************************************
 *                                                                         *
 * Uzak Diyarlar açık kaynak Türkçe Mud projesidir.                        *
 * Oyun geliştirmesi Jai ve Maru tarafından yönetilmektedir.               *
 * Unutulmaması gerekenler: Nir, Kame, Randalin, Nyah, Sint                *
 *                                                                         *
 * Github  : https://github.com/yelbuke/UzakDiyarlar                       *
 * Web     : http://www.uzakdiyarlar.net                                   *
 * Discord : https://discord.gg/kXyZzv                                     *
 *                                                                         *
 ***************************************************************************/

/*
 * turkish.c - Türkçe ad durumu ekleri.
 *
 * act() biçim dizgilerinde $s/$m/$e/$y/$z (ve büyük harfli karşılıkları
 * kurban için) ilgili karakterin adına ünlü uyumuna uygun tamlayan,
 * belirtme, yönelme, bulunma ve ayrılma eki takar.
 */
#include "turkish.h"

#include <stdio.h>
#include <string.h>

#include "utf8.h"

struct suffix_type
{
    char        belirtec[3];   /* biçim harfleri: küçük ch, büyük vch      */
    const char *birisi;        /* görünmeyen karakter için genel biçim     */
    const char *kaynastirma;   /* ünlüyle biten sözcükte araya giren harf  */
    const char *ek[4];         /* a/ı, e/i, o/u, ö/ü sınıfları için ek     */
};

static const struct suffix_type suffix_table[] =
{
    { "sS", "birisinin",  "n", { "ın", "in", "un", "ün" } },
    { "mM", "birisini",   "y", { "ı",  "i",  "u",  "ü"  } },
    { "eE", "birisine",   "y", { "a",  "e",  "a",  "e"  } },
    { "yY", "birisinde",  "",  { "da", "de", "da", "de" } },
    { "zZ", "birisinden", "",  { "dan","den","dan","den"} },
};

#define SUFFIX_COUNT ((int)(sizeof(suffix_table) / sizeof(suffix_table[0])))
#define EKLER_BUFS   8

static int vowel_class_cp(uint32_t cp)
{
    switch (utf8_tolower_cp(cp))
    {
    case 'a': case 0x131: case 0xE2: return 0;   /* a ı â */
    case 'e': case 'i':   case 0xEE: return 1;   /* e i î */
    case 'o': case 'u':   case 0xFB: return 2;   /* o u û */
    case 0xF6: case 0xFC:            return 3;   /* ö ü   */
    default:                         return -1;
    }
}

int tr_vowel_class(const char *word)
{
    int result = -1, cls, len;
    uint32_t cp;

    while ((len = utf8_decode(word, &cp)) > 0)
    {
        cls = vowel_class_cp(cp);
        if (cls >= 0)
            result = cls;
        word += len;
    }
    return result;
}

bool tr_ends_with_vowel(const char *word)
{
    int last = -1, len;
    uint32_t cp;

    while ((len = utf8_decode(word, &cp)) > 0)
    {
        last = vowel_class_cp(cp);
        word += len;
    }
    return last >= 0;
}

static int vowel_count(const char *word)
{
    int n = 0, len;
    uint32_t cp;

    while ((len = utf8_decode(word, &cp)) > 0)
    {
        if (vowel_class_cp(cp) >= 0)
            n++;
        word += len;
    }
    return n;
}

/*
 * Sözcük (ya da öbeğin son sözcüğü) sonundaki sert ünsüzü ünlüyle başlayan
 * ek önünde yumuşatır: p/ç/t/k -> b/c/d/ğ, "nk" -> "ng" (renk -> rengi).
 * Tek heceli sözcükler yumuşamaz (at -> atı, top -> topu, saç -> saçı).
 */
void tr_soften(char *word)
{
    size_t len = strlen(word);
    const char *last;

    if (len == 0)
        return;

    if (len >= 2 && !strcmp(word + len - 2, "nk"))
    {
        word[len - 1] = 'g';
        return;
    }

    last = strrchr(word, ' ');
    last = last != NULL ? last + 1 : word;
    if (vowel_count(last) < 2)
        return;

    if (len >= 2 && !strcmp(word + len - 2, "ç"))
        strcpy(word + len - 2, "c");
    else if (word[len - 1] == 'p')
        word[len - 1] = 'b';
    else if (word[len - 1] == 't')
        word[len - 1] = 'd';
    else if (word[len - 1] == 'k')
        strcpy(word + len - 1, "ğ");
}

const char *ekler(CHAR_DATA *to, CHAR_DATA *ch, const char *format)
{
    /* act() tek biçimde en çok EKLER_BUFS ek isteyebilir; sonuçlar sırayla
       bu döngüsel tamponlarda tutulur ve act'in kendi tamponuna kopyalanır. */
    static char bufs[EKLER_BUFS][MAX_STRING_LENGTH];
    static int  which;
    char *buf = bufs[which = (which + 1) % EKLER_BUFS];
    const struct suffix_type *s = &suffix_table[1];
    const char *name;
    int i, cls;

    for (i = 0; i < SUFFIX_COUNT; i++)
    {
        if (suffix_table[i].belirtec[0] == *format || suffix_table[i].belirtec[1] == *format)
        {
            s = &suffix_table[i];
            break;
        }
    }

    if (!can_see(to, ch))
    {
        snprintf(buf, MAX_STRING_LENGTH, "%s", s->birisi);
        return buf;
    }

    name = IS_NPC(ch) ? ch->short_descr : ch->name;
    cls  = tr_vowel_class(name);
    snprintf(buf, MAX_STRING_LENGTH, "%s%s", name, IS_NPC(ch) ? "" : "'");

    if (IS_NPC(ch) && s->kaynastirma[0] != '\0' && !tr_ends_with_vowel(name))
        tr_soften(buf);

    if (cls < 0)
        cls = 0;
    if (tr_ends_with_vowel(name) && s->kaynastirma[0] != '\0')
        strncat(buf, s->kaynastirma, MAX_STRING_LENGTH - strlen(buf) - 1);
    strncat(buf, s->ek[cls], MAX_STRING_LENGTH - strlen(buf) - 1);
    return buf;
}
