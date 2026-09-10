/***************************************************************************
 * utf8.c - UTF-8 yardımcıları ve Türkçe farkındalıklı harf dönüşümü.     *
 ***************************************************************************/
#include "utf8.h"

#include <stdlib.h>
#include <string.h>

int utf8_decode(const char *s, uint32_t *cp)
{
    const unsigned char *p = (const unsigned char *)s;

    if (p[0] < 0x80)
    {
        *cp = p[0];
        return p[0] ? 1 : 0;
    }
    if ((p[0] & 0xE0) == 0xC0 && (p[1] & 0xC0) == 0x80)
    {
        *cp = ((uint32_t)(p[0] & 0x1F) << 6) | (p[1] & 0x3F);
        if (*cp >= 0x80)
            return 2;
    }
    else if ((p[0] & 0xF0) == 0xE0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80)
    {
        *cp = ((uint32_t)(p[0] & 0x0F) << 12) | ((uint32_t)(p[1] & 0x3F) << 6) | (p[2] & 0x3F);
        if (*cp >= 0x800 && (*cp < 0xD800 || *cp > 0xDFFF))
            return 3;
    }
    else if ((p[0] & 0xF8) == 0xF0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80
             && (p[3] & 0xC0) == 0x80)
    {
        *cp = ((uint32_t)(p[0] & 0x07) << 18) | ((uint32_t)(p[1] & 0x3F) << 12)
              | ((uint32_t)(p[2] & 0x3F) << 6) | (p[3] & 0x3F);
        if (*cp >= 0x10000 && *cp <= 0x10FFFF)
            return 4;
    }
    *cp = UTF8_REPLACEMENT;
    return 1;
}

int utf8_encode(uint32_t cp, char *out)
{
    unsigned char *o = (unsigned char *)out;

    if (cp < 0x80)
    {
        o[0] = (unsigned char)cp;
        return 1;
    }
    if (cp < 0x800)
    {
        o[0] = (unsigned char)(0xC0 | (cp >> 6));
        o[1] = (unsigned char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000)
    {
        o[0] = (unsigned char)(0xE0 | (cp >> 12));
        o[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
        o[2] = (unsigned char)(0x80 | (cp & 0x3F));
        return 3;
    }
    o[0] = (unsigned char)(0xF0 | (cp >> 18));
    o[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3F));
    o[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
    o[3] = (unsigned char)(0x80 | (cp & 0x3F));
    return 4;
}

size_t utf8_strlen(const char *s)
{
    size_t n = 0;
    uint32_t cp;
    int len;

    while ((len = utf8_decode(s, &cp)) > 0)
    {
        s += len;
        n++;
    }
    return n;
}

int utf8_width(const char *s, int width)
{
    size_t bytes = strlen(s);
    size_t chars = utf8_strlen(s);

    return width + (int)(bytes - chars);
}

uint32_t utf8_tolower_cp(uint32_t c)
{
    if (c >= 'A' && c <= 'Z')
        return c == 'I' ? 0x131 : c + 32;
    if (c == 0x130)
        return 'i';
    if (c == 0x11E || c == 0x15E)
        return c + 1;
    if (c >= 0xC0 && c <= 0xDE && c != 0xD7)
        return c + 32;
    return c;
}

uint32_t utf8_toupper_cp(uint32_t c)
{
    if (c >= 'a' && c <= 'z')
        return c == 'i' ? 0x130 : c - 32;
    if (c == 0x131)
        return 'I';
    if (c == 0x11F || c == 0x15F)
        return c - 1;
    if (c >= 0xE0 && c <= 0xFE && c != 0xF7)
        return c - 32;
    return c;
}

uint32_t utf8_fold_cp(uint32_t c)
{
    c = utf8_tolower_cp(c);
    return c == 0x131 ? 'i' : c;
}

bool utf8_is_alpha_cp(uint32_t c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return true;
    if (c >= 0xC0 && c <= 0xFF && c != 0xD7 && c != 0xF7)
        return true;
    return c >= 0x100 && c <= 0x17F;   /* Latin Extended-A: Ğ ğ İ ı Ş ş Ł ő ... */
}

bool utf8_is_upper_cp(uint32_t c)
{
    return utf8_is_alpha_cp(c) && utf8_tolower_cp(c) != c;
}

void utf8_upper_first(char *s, size_t cap)
{
    uint32_t cp;
    int oldlen, newlen;
    char enc[4];
    size_t rest;

    oldlen = utf8_decode(s, &cp);
    if (oldlen <= 0 || cp == UTF8_REPLACEMENT)
        return;
    newlen = utf8_encode(utf8_toupper_cp(cp), enc);
    rest = strlen(s + oldlen);
    if ((size_t)newlen + rest + 1 > cap)
        return;
    if (newlen != oldlen)
        memmove(s + newlen, s + oldlen, rest + 1);
    memcpy(s, enc, (size_t)newlen);
}

void utf8_lower_str(char *dst, size_t cap, const char *src)
{
    size_t o = 0;
    uint32_t cp;
    int len;
    char enc[4];
    int n;

    while ((len = utf8_decode(src, &cp)) > 0)
    {
        src += len;
        n = utf8_encode(cp == UTF8_REPLACEMENT ? cp : utf8_tolower_cp(cp), enc);
        if (o + (size_t)n + 1 > cap)
            break;
        memcpy(dst + o, enc, (size_t)n);
        o += (size_t)n;
    }
    dst[o] = '\0';
}

bool utf8_valid(const char *s)
{
    uint32_t cp;
    int len;

    while ((len = utf8_decode(s, &cp)) > 0)
    {
        if (cp == UTF8_REPLACEMENT)
            return false;
        s += len;
    }
    return true;
}

/* ISO-8859-9'un Latin-1'den farklı altı kod noktası. */
static uint32_t latin5_to_cp(unsigned char b)
{
    switch (b)
    {
    case 0xD0: return 0x11E; /* Ğ */
    case 0xDD: return 0x130; /* İ */
    case 0xDE: return 0x15E; /* Ş */
    case 0xF0: return 0x11F; /* ğ */
    case 0xFD: return 0x131; /* ı */
    case 0xFE: return 0x15F; /* ş */
    default:   return b;
    }
}

void utf8_from_latin5(char *buf, size_t cap)
{
    char stack[4096];
    char *tmp;
    const char *p;
    size_t o = 0;
    uint32_t cp;
    int len, n;
    char enc[4];

    if (utf8_valid(buf))
        return;

    tmp = cap <= sizeof stack ? stack : (char *)malloc(cap);
    if (tmp == NULL)
        return;

    for (p = buf; (len = utf8_decode(p, &cp)) > 0; p += len)
    {
        if (cp == UTF8_REPLACEMENT)
            cp = latin5_to_cp((unsigned char)*p);
        n = utf8_encode(cp, enc);
        if (o + (size_t)n + 1 > cap)
            break;
        memcpy(tmp + o, enc, (size_t)n);
        o += (size_t)n;
    }
    tmp[o] = '\0';
    memcpy(buf, tmp, o + 1);
    if (tmp != stack)
        free(tmp);
}

void utf8_truncate(char *s, size_t maxbytes)
{
    size_t i = 0;
    uint32_t cp;
    int len;

    while ((len = utf8_decode(s + i, &cp)) > 0)
    {
        if (i + (size_t)len > maxbytes)
            break;
        i += (size_t)len;
    }
    s[i] = '\0';
}

void utf8_fit(const char *s, int width, char *out, size_t cap)
{
    size_t o = 0;
    int cols = 0, len;
    uint32_t cp;

    if (cap == 0)
        return;

    while (cols < width && (len = utf8_decode(s, &cp)) > 0)
    {
        if (o + (size_t)len + 1 > cap)
            break;
        memcpy(out + o, s, (size_t)len);
        o += (size_t)len;
        s += len;
        cols++;
    }
    while (cols < width && o + 2 <= cap)
    {
        out[o++] = ' ';
        cols++;
    }
    out[o] = '\0';
}

bool utf8_str_cmp(const char *a, const char *b)
{
    uint32_t ca, cb;
    int la, lb;

    for (;;)
    {
        la = utf8_decode(a, &ca);
        lb = utf8_decode(b, &cb);
        if (la == 0 && lb == 0)
            return false;
        if (utf8_fold_cp(ca) != utf8_fold_cp(cb))
            return true;
        a += la;
        b += lb;
    }
}

bool utf8_str_prefix(const char *a, const char *b)
{
    uint32_t ca, cb;
    int la, lb;

    for (;;)
    {
        la = utf8_decode(a, &ca);
        if (la == 0)
            return false;
        lb = utf8_decode(b, &cb);
        if (lb == 0 || utf8_fold_cp(ca) != utf8_fold_cp(cb))
            return true;
        a += la;
        b += lb;
    }
}

bool utf8_first_eq(const char *a, const char *b)
{
    uint32_t ca, cb;

    utf8_decode(a, &ca);
    utf8_decode(b, &cb);
    return utf8_fold_cp(ca) == utf8_fold_cp(cb);
}
