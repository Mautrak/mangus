/*
 * Birim testleri: UTF-8 yardımcıları ve parola özeti.
 * Sunucuya bağımlılığı yoktur; CTest tarafından çalıştırılır.
 */
#include <stdio.h>
#include <string.h>

#include "password.h"
#include "utf8.h"

static int failures;

#define CHECK(cond)                                                           \
    do                                                                        \
    {                                                                         \
        if (!(cond))                                                          \
        {                                                                     \
            printf("BAŞARISIZ %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
            failures++;                                                       \
        }                                                                     \
    } while (0)

static void test_decode_encode(void)
{
    uint32_t cp;
    char buf[8];

    CHECK(utf8_decode("a", &cp) == 1 && cp == 'a');
    CHECK(utf8_decode("ş", &cp) == 2 && cp == 0x15F);
    CHECK(utf8_decode("İ", &cp) == 2 && cp == 0x130);
    CHECK(utf8_decode("€", &cp) == 3 && cp == 0x20AC);
    CHECK(utf8_decode("\xFD", &cp) == 1 && cp == UTF8_REPLACEMENT);
    CHECK(utf8_decode("", &cp) == 0);
    CHECK(utf8_encode(0x131, buf) == 2 && memcmp(buf, "ı", 2) == 0);
    CHECK(utf8_strlen("kılıç") == 5 && strlen("kılıç") == 8);
    CHECK(utf8_width("kılıç", 10) == 13);
    CHECK(utf8_width("abc", 10) == 10);
}

static void test_case_mapping(void)
{
    char buf[32];

    CHECK(utf8_tolower_cp('I') == 0x131);
    CHECK(utf8_tolower_cp(0x130) == 'i');
    CHECK(utf8_toupper_cp('i') == 0x130);
    CHECK(utf8_toupper_cp(0x131) == 'I');
    CHECK(utf8_toupper_cp(0x15F) == 0x15E);
    CHECK(utf8_toupper_cp(0x11F) == 0x11E);
    CHECK(utf8_tolower_cp(0xC7) == 0xE7);
    CHECK(utf8_fold_cp('I') == 'i' && utf8_fold_cp(0x131) == 'i' && utf8_fold_cp(0x130) == 'i');

    strcpy(buf, "işık");
    utf8_upper_first(buf, sizeof(buf));
    CHECK(strcmp(buf, "İşık") == 0);
    strcpy(buf, "ışık");
    utf8_upper_first(buf, sizeof(buf));
    CHECK(strcmp(buf, "Işık") == 0);
    strcpy(buf, "şato");
    utf8_upper_first(buf, sizeof(buf));
    CHECK(strcmp(buf, "Şato") == 0);

    utf8_lower_str(buf, sizeof(buf), "ÇİĞDEM IŞIK");
    CHECK(strcmp(buf, "çiğdem ışık") == 0);
}

static void test_compare(void)
{
    CHECK(!utf8_str_cmp("kılıç", "KILIÇ"));
    CHECK(!utf8_str_cmp("kim", "KIM"));
    CHECK(!utf8_str_cmp("İyi", "iyi"));
    CHECK(utf8_str_cmp("kılıç", "kılıc"));
    CHECK(!utf8_str_prefix("kıl", "kılıç"));
    CHECK(!utf8_str_prefix("KIL", "kılıç"));
    CHECK(utf8_str_prefix("kılıç", "kıl"));
    CHECK(utf8_str_prefix("x", ""));
    CHECK(!utf8_str_prefix("", "abc"));
}

static void test_latin5(void)
{
    char buf[64];

    strcpy(buf, "yard\xFDm \xE7\xF6\xFC \xD0\xDD\xDE");
    utf8_from_latin5(buf, sizeof(buf));
    CHECK(strcmp(buf, "yardım çöü ĞİŞ") == 0);

    strcpy(buf, "zaten utf-8 ışık");
    utf8_from_latin5(buf, sizeof(buf));
    CHECK(strcmp(buf, "zaten utf-8 ışık") == 0);

    strcpy(buf, "karışık ş\xFE");
    utf8_from_latin5(buf, sizeof(buf));
    CHECK(strcmp(buf, "karışık şş") == 0);

    strcpy(buf, "\xFD\xFD\xFD");
    utf8_from_latin5(buf, 5);
    CHECK(strcmp(buf, "ıı") == 0);

    strcpy(buf, "kılıç");
    utf8_truncate(buf, 4);
    CHECK(strcmp(buf, "kıl") == 0);
    CHECK(utf8_valid("kılıç") && !utf8_valid("k\xFDl"));
}

static void test_password(void)
{
    char hex[65];
    const char *h;
    char copy[PWD_MAX_LEN];

    sha256_hex("abc", 3, hex);
    CHECK(strcmp(hex, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0);
    sha256_hex("", 0, hex);
    CHECK(strcmp(hex, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855") == 0);
    sha256_hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56, hex);
    CHECK(strcmp(hex, "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1") == 0);

    h = pwd_hash("gizli123");
    strcpy(copy, h);
    CHECK(strncmp(copy, "$m1$", 4) == 0 && strlen(copy) == 4 + 16 + 1 + 64);
    CHECK(pwd_check(copy, "gizli123"));
    CHECK(!pwd_check(copy, "gizli124"));
    CHECK(!pwd_check(copy, ""));
    CHECK(!pwd_is_legacy(copy));
    CHECK(pwd_is_legacy("AlHVvwOVMBOs6"));
    CHECK(!pwd_check("$m1$bozuk", "x"));
    CHECK(strcmp(copy, pwd_hash("gizli123")) != 0); /* farklı tuz */
}

int main(void)
{
    test_decode_encode();
    test_case_mapping();
    test_compare();
    test_latin5();
    test_password();
    if (failures == 0)
        printf("Birim testleri: tümü geçti.\n");
    return failures == 0 ? 0 : 1;
}
