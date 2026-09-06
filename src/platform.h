/***************************************************************************
 * platform.h - İşletim sistemine bağımlı tek nokta: soketler, zaman,      *
 * uyku, dizin oluşturma. POSIX (Linux, macOS) ve Windows (Winsock2).       *
 ***************************************************************************/
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#define NULL_FILE "NUL"
#else
#define NULL_FILE "/dev/null"
#endif

/* Aynı anda izlenebilecek en fazla soket sayısı. */
#define NET_MAX_POLL 1024

/* net_read dönüş değerleri: >0 okunan bayt, 0 bağlantı kapandı, <0 hata. */
#define NET_EOF        0
#define NET_ERROR     (-1)
#define NET_WOULDBLOCK (-2)

void net_startup(void);
void net_shutdown(void);
int  net_listen(int port);
int  net_accept(int listen_fd, char *host, size_t hostlen);
int  net_read(int fd, char *buf, size_t len);
int  net_write(int fd, const char *buf, size_t len);
void net_close(int fd);
/* fds içindeki soketleri okuma/yazma için en fazla timeout_us bekleyerek
 * yoklar; hazır soket sayısını (hata için <0) döndürür. Sonuçlar
 * net_can_read / net_can_write ile sorgulanır. */
int  net_poll(const int *fds, int count, long timeout_us);
bool net_can_read(int fd);
bool net_can_write(int fd);

long long platform_now_us(void);
void      platform_sleep_us(long usec);
bool      platform_mkdir(const char *path);
int       platform_pid(void);

#endif
