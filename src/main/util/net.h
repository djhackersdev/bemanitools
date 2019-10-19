#ifndef UTIL_NET_H
#define UTIL_NET_H

#include <stdbool.h>
#include <stdint.h>

#define NET_INVALID_ADDR 0xFFFFFFFF
#define NET_INVALID_PORT 0xFFFF

#define NET_LOCALHOST_ADDR 0x0100007F
#define NET_LOCALHOST_NAME "localhost"

#define NET_MAX_URL_HOSTNAME_LEN 512
#define NET_MAX_URL_PATH_LEN 1024

/**
 * Types of net addresses supported.
 */
enum net_addr_type {
    NET_ADDR_TYPE_INVALID = 0,
    NET_ADDR_TYPE_IPV4 = 1,
    NET_ADDR_TYPE_HOSTNAME = 2,
    NET_ADDR_TYPE_URL = 3,
};

/**
 * Struct storing an IPV4 address + optional port.
 */
struct net_addr_ipv4 {
    /* IP address stored in little endian byte order */
    uint32_t addr;
    uint16_t port;
};

/**
 * Struct storing a hostname + optional port.
 */
struct net_addr_hostname {
    char host[NET_MAX_URL_HOSTNAME_LEN + 1];
    uint16_t port;
};

/**
 * Struct storing a full https URL.
 */
struct net_addr_url {
    bool is_https;
    char path[NET_MAX_URL_PATH_LEN + 1];
    enum net_addr_type type;
    union {
        struct net_addr_ipv4 ipv4;
        struct net_addr_hostname hostname;
    };
};

/**
 * Structure representing a "network address".
 */
struct net_addr {
    enum net_addr_type type;
    union {
        struct net_addr_ipv4 ipv4;
        struct net_addr_hostname hostname;
        struct net_addr_url url;
    };
};

/**
 * Parse a string to a net_addr sturct. The string can have one of the following
 * formats:
 * - Plain IPV4 address, e.g. 127.0.0.1
 * - Plain IPV4 address with port, e.g. 127.0.0.1:1234
 * - Plain hostname, e.g. localhost
 * - Plain hostname with port, e.g. localhost:1234
 * - HTTP(s) URL, e.g. http://www.myremote.com
 * - HTTP(s) URL with port, e.g. http://www.myremote.com:1234
 * - HTTP(s) URL with or without port and path:
 * http://www.myremote.com:1234/somewhere
 *
 * @param str String to parse.
 * @param addr Pointer to a net_addr struct to write the result to.
 * @return True if parsing is successful, false on parse error.
 */
bool net_str_parse(const char *str, struct net_addr *addr);

// port not printed if invalid
/**
 * net_addr struct to string function. Create a single c-string representation.
 *
 * @param addr Input net_addr.
 * @return c-string representation. Note: Invalid port values are omitted.
 */
char *net_addr_to_str(const struct net_addr *addr);

/**
 * Resolve a hostname to an IPV4 address.
 *
 * @param hostname Hostname to resolve, e.g. localhost.
 * @param res_addr Pointer to a net_addr_ipv4 struct to store the result to.
 * @return True if resolving was successful, false on error.
 */
bool net_resolve_hostname(const char *hostname, struct net_addr_ipv4 *res_addr);

/**
 * Resolve a net_addr struct to an IPV4 address. If the net_addr struct is
 * actually of IPV4 type, the IPV4 address and port are simply copied to the
 * result struct.
 *
 * @param addr net_addr struct to resolve.
 * @param res_addr Pointer to a net_addr_ipv4 struct to store the result to.
 * @return True if resolving was successful, false on error.
 */
bool net_resolve_hostname_net_addr(
    const struct net_addr *addr, struct net_addr_ipv4 *res_addr);

/**
 * Check if a remote server is reachable, i.e. it is possible to open a
 * connection to it.
 *
 * @param addr net_addr struct of target server to check.
 * @param timeout_ms Timeout for the check in ms.
 * @return True if target server is reachable/connectable, false otherwise.
 */
bool net_check_remote_connection(
    const struct net_addr *addr, uint32_t timeout_ms);

/**
 * Check if a remote server is reachable, i.e. it is possible to open a
 * connection to it.
 *
 * @param addr net_addr_ipv4 struct of target server to check.
 * @param timeout_ms Timeout for the check in ms.
 * @return True if target server is reachable/connectable, false otherwise.
 */
bool net_check_remote_connection_ipv4(
    const struct net_addr_ipv4 *addr, uint32_t timeout_ms);

#endif