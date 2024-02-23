#define LOG_MODULE "util-net"

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "core/log.h"

#include "util/mem.h"
#include "util/net.h"
#include "util/str.h"

#include <inttypes.h>

static bool net_parse_url(const char *str, struct net_addr *addr)
{
    char host[NET_MAX_URL_HOSTNAME_LEN + 1];
    char path[NET_MAX_URL_PATH_LEN + 1];
    uint32_t ipv4[4];
    uint32_t port;
    int toks;

    addr->type = NET_ADDR_TYPE_URL;

    memset(host, 0, sizeof(host));
    memset(path, 0, sizeof(path));

    if (!strncmp(str, "http://", strlen("http://"))) {
        addr->url.is_https = false;
        str += strlen("http://");
    } else if (!strncmp(str, "https://", strlen("https://"))) {
        addr->url.is_https = true;
        str += strlen("https://");
    } else {
        return false;
    }

    /* Try url with IPV4 address */

    if (sscanf(str, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3]) ==
        4) {
        addr->url.type = NET_ADDR_TYPE_IPV4;
        addr->url.ipv4.addr = (ipv4[0] & 0xFF) | ((ipv4[1] & 0xFF) << 8) |
            ((ipv4[2] & 0xFF) << 16) | ((ipv4[3] & 0xFF) << 24);
        addr->url.ipv4.port = NET_INVALID_PORT;
        memset(addr->url.path, 0, sizeof(addr->url.path));

        /* Port and trailing path are optional, check with and without */

        toks = sscanf(
            str,
            "%d.%d.%d.%d:%d/%s",
            &ipv4[0],
            &ipv4[1],
            &ipv4[2],
            &ipv4[3],
            &port,
            path);

        if (toks > 4) {
            if (toks >= 5) {
                addr->url.ipv4.port = (uint16_t) port;
            }

            if (toks == 6) {
                strcpy(addr->url.path, path);
            }
        } else {
            /* No port, but may have trailing path */
            toks = sscanf(
                str,
                "%d.%d.%d.%d/%s",
                &ipv4[0],
                &ipv4[1],
                &ipv4[2],
                &ipv4[3],
                path);

            if (toks > 4) {
                strcpy(addr->url.path, path);
            }
        }
    } else {
        /* URL with hostname */
        addr->url.type = NET_ADDR_TYPE_HOSTNAME;
        memset(addr->url.hostname.host, 0, sizeof(addr->url.hostname.host));
        addr->url.hostname.port = NET_INVALID_PORT;
        memset(addr->url.path, 0, sizeof(addr->url.path));

        toks = sscanf(str, "%[^:]:%d/%s", host, &port, path);

        if (toks == 0) {
            return false;
        }

        if (toks >= 2) {
            strcpy(addr->url.hostname.host, host);
            addr->url.hostname.port = port;

            if (toks == 3) {
                strcpy(addr->url.path, path);
            }
        } else {
            /* No port, but may have trailing path */
            toks = sscanf(str, "%[^/]/%s", host, path);
            strcpy(addr->url.hostname.host, host);

            if (toks > 1) {
                strcpy(addr->url.path, path);
            }
        }
    }

    return true;
}

static bool net_parse_ipv4_or_hostname(const char *str, struct net_addr *addr)
{
    char host[NET_MAX_URL_HOSTNAME_LEN + 1];
    uint32_t ipv4[4];
    uint32_t port;
    int toks;

    memset(host, 0, sizeof(host));

    toks = sscanf(
        str, "%d.%d.%d.%d:%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3], &port);

    if (toks >= 4) {
        addr->type = NET_ADDR_TYPE_IPV4;
        addr->ipv4.addr = (ipv4[0] & 0xFF) | ((ipv4[1] & 0xFF) << 8) |
            ((ipv4[2] & 0xFF) << 16) | ((ipv4[3] & 0xFF) << 24);
        addr->ipv4.port = NET_INVALID_PORT;

        if (toks == 5) {
            addr->ipv4.port = port;
        }
    } else if (toks == 0) {
        addr->type = NET_ADDR_TYPE_HOSTNAME;
        addr->hostname.port = NET_INVALID_PORT;

        toks = sscanf(str, "%[^:]:%d", host, &port);

        if (toks == 0) {
            return false;
        }

        strcpy(addr->hostname.host, host);

        if (toks > 1) {
            addr->hostname.port = port;
        }
    } else {
        return false;
    }

    return true;
}

static char *net_ipv4_to_str(const struct net_addr_ipv4 *addr)
{
    size_t len;
    uint8_t *addr_ipv4;
    char *str;

    addr_ipv4 = (uint8_t *) &addr->addr;

    if (addr->port != NET_INVALID_PORT) {
        len = snprintf(
            NULL,
            0,
            "%d.%d.%d.%d:%d",
            addr_ipv4[0],
            addr_ipv4[1],
            addr_ipv4[2],
            addr_ipv4[3],
            addr->port);
        str = xmalloc(len + 1);

        sprintf(
            str,
            "%d.%d.%d.%d:%d",
            addr_ipv4[0],
            addr_ipv4[1],
            addr_ipv4[2],
            addr_ipv4[3],
            addr->port);
    } else {
        len = snprintf(
            NULL,
            0,
            "%d.%d.%d.%d",
            addr_ipv4[0],
            addr_ipv4[1],
            addr_ipv4[2],
            addr_ipv4[3]);
        str = xmalloc(len + 1);

        sprintf(
            str,
            "%d.%d.%d.%d",
            addr_ipv4[0],
            addr_ipv4[1],
            addr_ipv4[2],
            addr_ipv4[3]);
    }

    return str;
}

static char *net_hostname_to_str(const struct net_addr_hostname *addr)
{
    size_t len;
    char *str;

    if (addr->port != NET_INVALID_PORT) {
        len = snprintf(NULL, 0, "%s:%d", addr->host, addr->port);
        str = xmalloc(len + 1);

        sprintf(str, "%s:%d", addr->host, addr->port);
    } else {
        str = str_dup(addr->host);
    }

    return str;
}

static char *net_url_to_str(const struct net_addr_url *addr)
{
    size_t len;
    size_t str_len;
    char *tmp;
    char *str;

    switch (addr->type) {
        case NET_ADDR_TYPE_IPV4:
            tmp = net_ipv4_to_str(&addr->ipv4);
            break;

        case NET_ADDR_TYPE_HOSTNAME:
            tmp = net_hostname_to_str(&addr->hostname);
            break;

        case NET_ADDR_TYPE_INVALID:
        case NET_ADDR_TYPE_URL:
            /* URL is not a valid type here */
        default:
            log_assert(false);
            return NULL;
    }

    str_len = strlen(addr->path);

    len = snprintf(
        NULL,
        0,
        "http%s://%s%s%s",
        addr->is_https ? "s" : "",
        tmp,
        str_len > 0 ? "/" : "",
        str_len > 0 ? addr->path : "");
    str = xmalloc(len + 1);

    sprintf(
        str,
        "http%s://%s%s%s",
        addr->is_https ? "s" : "",
        tmp,
        str_len > 0 ? "/" : "",
        str_len > 0 ? addr->path : "");

    free(tmp);

    return str;
}

static bool net_resolve_hostname_str(const char *host, uint32_t *ipv4)
{
    struct WSAData data;
    struct addrinfo hints;
    struct addrinfo *res;
    int err;

    log_assert(host);
    log_assert(ipv4);

    /* Resolve and cache */
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    /* Using gethostbyname seems to crash on certain setups...use
        getaddrinfo (which is not deprecated) */
    WSAStartup(MAKEWORD(2, 0), &data);

    err = getaddrinfo(host, NULL, &hints, &res);

    if (err != 0) {
        *ipv4 = NET_INVALID_ADDR;
        return false;
    }

    *ipv4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr.S_un.S_addr;

    freeaddrinfo(res);

    WSACleanup();

    return true;
}

static bool net_resolve_hostname_ipv4(
    const struct net_addr_ipv4 *addr, struct net_addr_ipv4 *res_addr)
{
    res_addr->addr = addr->addr;
    res_addr->port = addr->port;

    return true;
}

static bool net_resolve_hostname_host(
    const struct net_addr_hostname *addr, struct net_addr_ipv4 *res_addr)
{
    res_addr->port = addr->port;
    return net_resolve_hostname_str(addr->host, &res_addr->addr);
}

static bool net_resolve_hostname_url(
    const struct net_addr_url *addr, struct net_addr_ipv4 *res_addr)
{
    switch (addr->type) {
        case NET_ADDR_TYPE_IPV4:
            return net_resolve_hostname_ipv4(&addr->ipv4, res_addr);

        case NET_ADDR_TYPE_HOSTNAME:
            return net_resolve_hostname_host(&addr->hostname, res_addr);

        case NET_ADDR_TYPE_INVALID:
        case NET_ADDR_TYPE_URL:
            /* Not a valid type here */
        default:
            log_assert(false);
            return false;
    }
}

bool net_str_parse(const char *str, struct net_addr *addr)
{
    log_assert(str);
    log_assert(addr);

    if (!strncmp(str, "http", strlen("http"))) {
        return net_parse_url(str, addr);
    } else {
        return net_parse_ipv4_or_hostname(str, addr);
    }
}

char *net_addr_to_str(const struct net_addr *addr)
{
    log_assert(addr);

    switch (addr->type) {
        case NET_ADDR_TYPE_IPV4:
            return net_ipv4_to_str(&addr->ipv4);

        case NET_ADDR_TYPE_HOSTNAME:
            return net_hostname_to_str(&addr->hostname);

        case NET_ADDR_TYPE_URL:
            return net_url_to_str(&addr->url);

        case NET_ADDR_TYPE_INVALID:
        default:
            log_assert(false);
            return NULL;
    }
}

bool net_resolve_hostname(const char *hostname, struct net_addr_ipv4 *res_addr)
{
    return net_resolve_hostname_str(hostname, &res_addr->addr);
}

bool net_resolve_hostname_net_addr(
    const struct net_addr *addr, struct net_addr_ipv4 *res_addr)
{
    switch (addr->type) {
        case NET_ADDR_TYPE_IPV4:
            return net_resolve_hostname_ipv4(&addr->ipv4, res_addr);

        case NET_ADDR_TYPE_HOSTNAME:
            return net_resolve_hostname_host(&addr->hostname, res_addr);

        case NET_ADDR_TYPE_URL:
            return net_resolve_hostname_url(&addr->url, res_addr);

        case NET_ADDR_TYPE_INVALID:
        default:
            log_assert(false);
            return false;
    }
}

bool net_check_remote_connection(
    const struct net_addr *addr, uint32_t timeout_ms)
{
    struct net_addr_ipv4 target;

    if (!net_resolve_hostname_net_addr(addr, &target)) {
        return false;
    }

    return net_check_remote_connection_ipv4(&target, timeout_ms);
}

bool net_check_remote_connection_ipv4(
    const struct net_addr_ipv4 *addr, uint32_t timeout_ms)
{
    struct WSAData data;
    SOCKET s;
    u_long block;
    fd_set set_w;
    fd_set set_e;
    struct timeval timeout;
    int err;
    bool success;
    struct sockaddr_in sockaddr;

    WSAStartup(MAKEWORD(2, 0), &data);

    err = 0;
    success = true;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == INVALID_SOCKET) {
        log_assert(false);
        return false;
    }

    /* Need socket in non-blocking mode to timeout */
    block = 1;

    if (ioctlsocket(s, FIONBIO, &block) == SOCKET_ERROR) {
        log_assert(false);
        closesocket(s);
        return false;
    }

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = addr->addr;
    sockaddr.sin_port = htons(addr->port);

    if (connect(s, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) ==
        SOCKET_ERROR) {
        if (WSAGetLastError() != WSAEWOULDBLOCK) {
            /* connection failed */;
            success = false;
        } else {
            FD_ZERO(&set_w);
            FD_SET(s, &set_w);
            FD_ZERO(&set_e);
            FD_SET(s, &set_e);

            timeout.tv_sec = timeout_ms / 1000;
            timeout.tv_usec = (timeout_ms % 1000) * 1000;

            err = select(0, NULL, &set_w, &set_e, &timeout);

            if (err <= 0) {
                /* select failed or timeout */
                success = false;
            } else {
                if (FD_ISSET(s, &set_e)) {
                    /* Connection failed */
                    success = false;
                }
            }
        }
    } else {
        success = false;
    }

    closesocket(s);
    WSACleanup();

    return success;
}