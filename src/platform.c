/***************************************************************************
 * platform.c - Soket ve işletim sistemi katmanı.                          *
 ***************************************************************************/
#ifdef _WIN32
#define FD_SETSIZE 1024
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#include <process.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

static fd_set poll_read_set;
static fd_set poll_write_set;

void net_perror(const char *prefix)
{
#ifdef _WIN32
    char msg[256];
    DWORD err = WSAGetLastError();

    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, err, 0, msg, sizeof(msg), NULL) == 0)
        snprintf(msg, sizeof(msg), "Winsock hatası %lu", (unsigned long)err);
    fprintf(stderr, "%s: %s\n", prefix, msg);
#else
    perror(prefix);
#endif
}

/* POSIX'te fd_set fd NUMARASIYLA indekslenir: FD_SETSIZE ve üstü select()
 * ile yoklanamaz, FD_SET tampon dışına yazar. Windows'ta sınır soket sayısıdır. */
static bool fd_pollable(int fd)
{
#ifdef _WIN32
    return fd >= 0;
#else
    return fd >= 0 && fd < FD_SETSIZE;
#endif
}

void net_startup(void)
{
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "WSAStartup başarısız.\n");
        exit(1);
    }
#else
    signal(SIGPIPE, SIG_IGN);
#endif
    FD_ZERO(&poll_read_set);
    FD_ZERO(&poll_write_set);
}

void net_shutdown(void)
{
#ifdef _WIN32
    WSACleanup();
#endif
}

static bool set_nonblocking(int fd)
{
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket((SOCKET)fd, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    return flags != -1 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
#endif
}

int net_listen(int port)
{
    struct sockaddr_in sa;
    int fd;
    int x = 1;

    fd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        net_perror("net_listen: socket");
        return -1;
    }
#ifdef _WIN32
    if (setsockopt((SOCKET)fd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char *)&x, sizeof(x)) < 0)
    {
        net_perror("net_listen: SO_EXCLUSIVEADDRUSE");
        net_close(fd);
        return -1;
    }
#else
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&x, sizeof(x)) < 0)
    {
        net_perror("net_listen: SO_REUSEADDR");
        net_close(fd);
        return -1;
    }
#endif
    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port        = htons((unsigned short)port);
    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        net_perror("net_listen: bind");
        net_close(fd);
        return -1;
    }
    if (listen(fd, 8) < 0)
    {
        net_perror("net_listen: listen");
        net_close(fd);
        return -1;
    }
    return fd;
}

int net_accept(int listen_fd, char *host, size_t hostlen)
{
    struct sockaddr_in sock;
#ifdef _WIN32
    int size = sizeof(sock);
#else
    socklen_t size = sizeof(sock);
#endif
    int desc;

    desc = (int)accept(listen_fd, (struct sockaddr *)&sock, &size);
    if (desc < 0)
    {
        net_perror("net_accept: accept");
        return -1;
    }
    if (!fd_pollable(desc))
    {
        fprintf(stderr, "net_accept: fd %d yoklanamaz (FD_SETSIZE), bağlantı reddedildi.\n", desc);
        net_close(desc);
        return -1;
    }
    if (!set_nonblocking(desc))
    {
        net_perror("net_accept: nonblocking");
        net_close(desc);
        return -1;
    }
    if (inet_ntop(AF_INET, &sock.sin_addr, host, (socklen_t)hostlen) == NULL)
        snprintf(host, hostlen, "(bilinmiyor)");
    return desc;
}

int net_read(int fd, char *buf, size_t len)
{
#ifdef _WIN32
    int n = recv((SOCKET)fd, buf, (int)len, 0);
    if (n == SOCKET_ERROR)
        return WSAGetLastError() == WSAEWOULDBLOCK ? NET_WOULDBLOCK : NET_ERROR;
    return n;
#else
    ssize_t n = read(fd, buf, len);
    if (n < 0)
        return (errno == EWOULDBLOCK || errno == EAGAIN) ? NET_WOULDBLOCK : NET_ERROR;
    return (int)n;
#endif
}

int net_write(int fd, const char *buf, size_t len)
{
#ifdef _WIN32
    int n = send((SOCKET)fd, buf, (int)len, 0);
    if (n == SOCKET_ERROR)
        return WSAGetLastError() == WSAEWOULDBLOCK ? NET_WOULDBLOCK : NET_ERROR;
    return n;
#else
    ssize_t n = write(fd, buf, len);
    if (n < 0)
        return (errno == EWOULDBLOCK || errno == EAGAIN) ? NET_WOULDBLOCK : NET_ERROR;
    return (int)n;
#endif
}

void net_close(int fd)
{
#ifdef _WIN32
    closesocket((SOCKET)fd);
#else
    close(fd);
#endif
}

int net_poll(const int *fds, int count, long timeout_us)
{
    struct timeval tv;
    int i, maxfd = -1, n;

    FD_ZERO(&poll_read_set);
    FD_ZERO(&poll_write_set);
    for (i = 0; i < count; i++)
    {
        if (!fd_pollable(fds[i]))
            continue;
        FD_SET((unsigned)fds[i], &poll_read_set);
        FD_SET((unsigned)fds[i], &poll_write_set);
        if (fds[i] > maxfd)
            maxfd = fds[i];
    }
    tv.tv_sec  = timeout_us / 1000000;
    tv.tv_usec = timeout_us % 1000000;
    n = select(maxfd + 1, &poll_read_set, &poll_write_set, NULL, &tv);
    if (n < 0)
    {
        FD_ZERO(&poll_read_set);
        FD_ZERO(&poll_write_set);
    }
    return n;
}

bool net_can_read(int fd)
{
    return fd_pollable(fd) && FD_ISSET((unsigned)fd, &poll_read_set) != 0;
}

bool net_can_write(int fd)
{
    return fd_pollable(fd) && FD_ISSET((unsigned)fd, &poll_write_set) != 0;
}

long long platform_now_us(void)
{
#ifdef _WIN32
    FILETIME ft;
    ULARGE_INTEGER u;
    GetSystemTimeAsFileTime(&ft);
    u.LowPart  = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    return (long long)((u.QuadPart - 116444736000000000ULL) / 10);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
#endif
}

void platform_sleep_us(long long usec)
{
    if (usec <= 0)
        return;
#ifdef _WIN32
    Sleep((DWORD)((usec + 999) / 1000));
#else
    struct timeval tv;
    tv.tv_sec  = (time_t)(usec / 1000000);
    tv.tv_usec = (suseconds_t)(usec % 1000000);
    select(0, NULL, NULL, NULL, &tv);
#endif
}

bool platform_mkdir(const char *path)
{
#ifdef _WIN32
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0755) == 0 || errno == EEXIST;
#endif
}

int platform_pid(void)
{
#ifdef _WIN32
    return _getpid();
#else
    return (int)getpid();
#endif
}

bool platform_replace_file(const char *from, const char *to)
{
#ifdef _WIN32
    return MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) != 0;
#else
    return rename(from, to) == 0;
#endif
}

bool platform_remove_file(const char *path)
{
    return remove(path) == 0;
}

void platform_time_str(time_t t, char *buf, size_t cap)
{
    char tmp[32];
    size_t n;

    if (cap == 0)
        return;
#ifdef _WIN32
    if (ctime_s(tmp, sizeof(tmp), &t) != 0)
        tmp[0] = '\0';
#else
    if (ctime_r(&t, tmp) == NULL)
        tmp[0] = '\0';
#endif
    n = strlen(tmp);
    while (n > 0 && (tmp[n - 1] == '\n' || tmp[n - 1] == '\r'))
        tmp[--n] = '\0';
    snprintf(buf, cap, "%s", tmp);
}

bool platform_gunzip(const char *path)
{
#ifdef _WIN32
    (void)path;
    return false;
#else
    char cmd[1024];

    snprintf(cmd, sizeof(cmd), "gzip -dfq '%s.gz'", path);
    return system(cmd) == 0;
#endif
}
