/***************************************************************************
 * prog_util.c - mob_prog.c / obj_prog.c / quest.c ortak saf yardımcıları. *
 ***************************************************************************/
#include <string.h>

#include "prog_util.h"

void prog_subst(char *buf, size_t size, const char *tmpl, const char *a1, const char *a2)
{
    const char *args[2] = { a1, a2 };
    size_t nargs = 0;
    size_t len = 0;

    if (size == 0)
        return;
    if (tmpl == NULL)
        tmpl = "";

    while (*tmpl != '\0' && len + 1 < size)
    {
        if (tmpl[0] == '%' && tmpl[1] == 's' && nargs < 2)
        {
            const char *arg = args[nargs++];
            size_t alen;

            if (arg == NULL)
                arg = "";
            alen = strlen(arg);
            if (alen > size - 1 - len)
                alen = size - 1 - len;
            memcpy(buf + len, arg, alen);
            len += alen;
            tmpl += 2;
            continue;
        }
        buf[len++] = *tmpl++;
    }
    buf[len] = '\0';
}

static bool is_word_byte(unsigned char c)
{
    /* ASCII harf/rakam ya da çok baytlı (UTF-8) harf */
    return c >= 0x80 || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool prog_name_in_text(const char *text, const char *name)
{
    size_t nlen;
    const char *p;

    if (text == NULL || name == NULL || name[0] == '\0')
        return false;
    nlen = strlen(name);
    for (p = strstr(text, name); p != NULL; p = strstr(p + 1, name))
    {
        bool start_ok = (p == text) || !is_word_byte((unsigned char) p[-1]);
        bool end_ok = !is_word_byte((unsigned char) p[nlen]);

        if (start_ok && end_ok)
            return true;
    }
    return false;
}

int prog_weapon_dice(int level)
{
    static const struct { int max_level; int dice; } steps[] = {
        { 10, 2 }, { 20, 3 }, { 30, 4 }, { 40, 5 }, { 50, 6 },
        { 60, 7 }, { 70, 9 }, { 80, 11 }
    };
    size_t i;

    for (i = 0; i < sizeof(steps) / sizeof(steps[0]); i++)
        if (level <= steps[i].max_level)
            return steps[i].dice;
    return 12;
}
