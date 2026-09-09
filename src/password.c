/***************************************************************************
 * password.c - Taşınabilir parola özeti.                                  *
 *                                                                         *
 * Biçim: $m1$<16 onaltılık tuz>$<64 onaltılık SHA-256>                    *
 * Özet = SHA256(tuz || parola), ardından PWD_ROUNDS kez                   *
 *        özet = SHA256(özet || parola).                                    *
 ***************************************************************************/
#ifdef _WIN32
#define _CRT_RAND_S
#endif

#include "password.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(HAVE_CRYPT_H)
#include <crypt.h>
#elif defined(HAVE_CRYPT)
#include <unistd.h>
#endif

#define PWD_ROUNDS 10000
#define PWD_PREFIX "$m1$"

/* ---- SHA-256 ---------------------------------------------------------- */

typedef struct
{
    uint32_t      h[8];
    uint64_t      total;
    unsigned char buf[64];
    size_t        buflen;
} sha256_ctx;

static const uint32_t sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void sha256_block(sha256_ctx *c, const unsigned char *p)
{
    uint32_t w[64], a, b, cc, d, e, f, g, h, t1, t2;
    int i;

    for (i = 0; i < 16; i++)
        w[i] = ((uint32_t)p[4 * i] << 24) | ((uint32_t)p[4 * i + 1] << 16)
               | ((uint32_t)p[4 * i + 2] << 8) | (uint32_t)p[4 * i + 3];
    for (; i < 64; i++)
    {
        uint32_t s0 = ROR(w[i - 15], 7) ^ ROR(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = ROR(w[i - 2], 17) ^ ROR(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    a = c->h[0]; b = c->h[1]; cc = c->h[2]; d = c->h[3];
    e = c->h[4]; f = c->h[5]; g = c->h[6]; h = c->h[7];
    for (i = 0; i < 64; i++)
    {
        t1 = h + (ROR(e, 6) ^ ROR(e, 11) ^ ROR(e, 25)) + ((e & f) ^ (~e & g)) + sha256_k[i] + w[i];
        t2 = (ROR(a, 2) ^ ROR(a, 13) ^ ROR(a, 22)) + ((a & b) ^ (a & cc) ^ (b & cc));
        h = g; g = f; f = e; e = d + t1; d = cc; cc = b; b = a; a = t1 + t2;
    }
    c->h[0] += a; c->h[1] += b; c->h[2] += cc; c->h[3] += d;
    c->h[4] += e; c->h[5] += f; c->h[6] += g; c->h[7] += h;
}

static void sha256_init(sha256_ctx *c)
{
    static const uint32_t iv[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    memcpy(c->h, iv, sizeof(iv));
    c->total = 0;
    c->buflen = 0;
}

static void sha256_update(sha256_ctx *c, const void *data, size_t len)
{
    const unsigned char *p = (const unsigned char *)data;

    c->total += len;
    while (len > 0)
    {
        size_t take = 64 - c->buflen;
        if (take > len)
            take = len;
        memcpy(c->buf + c->buflen, p, take);
        c->buflen += take;
        p += take;
        len -= take;
        if (c->buflen == 64)
        {
            sha256_block(c, c->buf);
            c->buflen = 0;
        }
    }
}

static void sha256_final(sha256_ctx *c, unsigned char out[32])
{
    uint64_t bits = c->total * 8;
    unsigned char pad = 0x80;
    unsigned char lenbuf[8];
    int i;

    sha256_update(c, &pad, 1);
    pad = 0;
    while (c->buflen != 56)
        sha256_update(c, &pad, 1);
    for (i = 0; i < 8; i++)
        lenbuf[i] = (unsigned char)(bits >> (56 - 8 * i));
    sha256_update(c, lenbuf, 8);
    for (i = 0; i < 8; i++)
    {
        out[4 * i]     = (unsigned char)(c->h[i] >> 24);
        out[4 * i + 1] = (unsigned char)(c->h[i] >> 16);
        out[4 * i + 2] = (unsigned char)(c->h[i] >> 8);
        out[4 * i + 3] = (unsigned char)(c->h[i]);
    }
}

static void to_hex(const unsigned char *in, size_t len, char *out)
{
    static const char digits[] = "0123456789abcdef";
    size_t i;

    for (i = 0; i < len; i++)
    {
        out[2 * i]     = digits[in[i] >> 4];
        out[2 * i + 1] = digits[in[i] & 0x0F];
    }
    out[2 * len] = '\0';
}

void sha256_hex(const void *data, size_t len, char out[65])
{
    sha256_ctx c;
    unsigned char digest[32];

    sha256_init(&c);
    sha256_update(&c, data, len);
    sha256_final(&c, digest);
    to_hex(digest, 32, out);
}

/* ---- Tuz -------------------------------------------------------------- */

static void random_bytes(unsigned char *out, size_t len)
{
    size_t i;
#ifdef _WIN32
    for (i = 0; i < len; i += 4)
    {
        unsigned int r = 0;
        rand_s(&r);
        memcpy(out + i, &r, len - i < 4 ? len - i : 4);
    }
#else
    FILE *fp = fopen("/dev/urandom", "rb");
    if (fp != NULL)
    {
        size_t got = fread(out, 1, len, fp);
        fclose(fp);
        if (got == len)
            return;
    }
    {
        /* Yedek: /dev/urandom yoksa (yalnızca tuz için) zaman + yığın adresi tohumu. */
        static uint64_t state;
        if (state == 0)
            state = (uint64_t)time(NULL) ^ ((uint64_t)(uintptr_t)out << 32) ^ (uint64_t)clock();
        for (i = 0; i < len; i++)
        {
            state ^= state << 13;
            state ^= state >> 7;
            state ^= state << 17;
            out[i] = (unsigned char)(state >> 24);
        }
    }
#endif
}

/* ---- Parola API ------------------------------------------------------- */

static void derive(const char *salt_hex, const char *plain, char out_hex[65])
{
    sha256_ctx c;
    unsigned char digest[32];
    size_t plen = strlen(plain);
    int i;

    sha256_init(&c);
    sha256_update(&c, salt_hex, strlen(salt_hex));
    sha256_update(&c, plain, plen);
    sha256_final(&c, digest);
    for (i = 0; i < PWD_ROUNDS; i++)
    {
        sha256_init(&c);
        sha256_update(&c, digest, sizeof(digest));
        sha256_update(&c, plain, plen);
        sha256_final(&c, digest);
    }
    to_hex(digest, 32, out_hex);
}

const char *pwd_hash(const char *plain)
{
    static char result[PWD_MAX_LEN];
    unsigned char salt[8];
    char salt_hex[17];
    char digest_hex[65];

    random_bytes(salt, sizeof(salt));
    to_hex(salt, sizeof(salt), salt_hex);
    derive(salt_hex, plain, digest_hex);
    snprintf(result, sizeof(result), "%s%s$%s", PWD_PREFIX, salt_hex, digest_hex);
    return result;
}

bool pwd_is_legacy(const char *stored)
{
    return stored == NULL || strncmp(stored, PWD_PREFIX, strlen(PWD_PREFIX)) != 0;
}

bool pwd_check(const char *stored, const char *plain)
{
    char salt_hex[17];
    char digest_hex[65];
    const char *dollar;

    if (stored == NULL || plain == NULL)
        return false;

    if (pwd_is_legacy(stored))
    {
#if defined(HAVE_CRYPT) || defined(HAVE_CRYPT_H)
        const char *r = crypt(plain, stored);
        return r != NULL && strcmp(r, stored) == 0;
#else
        return false;
#endif
    }

    stored += strlen(PWD_PREFIX);
    dollar = strchr(stored, '$');
    if (dollar == NULL || dollar - stored != 16)
        return false;
    memcpy(salt_hex, stored, 16);
    salt_hex[16] = '\0';
    derive(salt_hex, plain, digest_hex);
    return strcmp(digest_hex, dollar + 1) == 0;
}
