#include "test/check.h"
#include "test/test.h"

#include "util/net.h"

/* Note: Don't care about cleaning up of returned strings to avoid code
 * clutter 
 */

static void test_parse_invalid()
{
    struct net_addr addr;

    check_bool_false(net_str_parse("127.0.0", &addr));
}

static void test_parse_ipv4()
{
    struct net_addr addr;

    check_bool_true(net_str_parse("127.0.0.1", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.ipv4.port, NET_INVALID_PORT);

    check_bool_true(net_str_parse("127.0.0.1:1234", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.ipv4.port, 1234);
}

static void test_parse_hostname()
{
    struct net_addr addr;

    check_bool_true(net_str_parse(NET_LOCALHOST_NAME, &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.hostname.host, NET_LOCALHOST_NAME);
    check_int_eq(addr.hostname.port, NET_INVALID_PORT);

    check_bool_true(net_str_parse(NET_LOCALHOST_NAME ":1234", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.hostname.host, NET_LOCALHOST_NAME);
    check_int_eq(addr.hostname.port, 1234);
}

static void test_parse_url_http_ipv4()
{
    struct net_addr addr;

    check_bool_true(net_str_parse("http://127.0.0.1", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_false(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.url.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.url.ipv4.port, NET_INVALID_PORT);
    check_str_eq(addr.url.path, "");

    check_bool_true(net_str_parse("http://127.0.0.1:1234", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_false(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.url.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.url.ipv4.port, 1234);
    check_str_eq(addr.url.path, "");

    check_bool_true(net_str_parse("http://127.0.0.1/somewhere", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_false(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.url.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.url.ipv4.port, NET_INVALID_PORT);
    check_str_eq(addr.url.path, "somewhere");

    check_bool_true(net_str_parse("http://127.0.0.1:22/somewhere", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_false(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.url.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.url.ipv4.port, 22);
    check_str_eq(addr.url.path, "somewhere");
}

static void test_parse_url_https_ipv4()
{
    struct net_addr addr;

    check_bool_true(net_str_parse("https://127.0.0.1", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_true(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.url.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.url.ipv4.port, NET_INVALID_PORT);
    check_str_eq(addr.url.path, "");

    check_bool_true(net_str_parse("https://127.0.0.1:1234", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_true(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.url.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.url.ipv4.port, 1234);
    check_str_eq(addr.url.path, "");

    check_bool_true(net_str_parse("https://127.0.0.1/somewhere", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_true(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.url.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.url.ipv4.port, NET_INVALID_PORT);
    check_str_eq(addr.url.path, "somewhere");

    check_bool_true(net_str_parse("https://127.0.0.1:22/somewhere", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_true(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_IPV4);
    check_int_eq(addr.url.ipv4.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.url.ipv4.port, 22);
    check_str_eq(addr.url.path, "somewhere");
}

static void test_parse_url_http_hostname()
{
    struct net_addr addr;

    check_bool_true(net_str_parse("http://www.google.com", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_false(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.url.hostname.host, "www.google.com");
    check_int_eq(addr.url.hostname.port, NET_INVALID_PORT);
    check_str_eq(addr.url.path, "");

    check_bool_true(net_str_parse("http://www.google.com:1234", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_false(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.url.hostname.host, "www.google.com");
    check_int_eq(addr.url.hostname.port, 1234);
    check_str_eq(addr.url.path, "");

    check_bool_true(net_str_parse("http://www.google.com/somewhere", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_false(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.url.hostname.host, "www.google.com");
    check_int_eq(addr.url.hostname.port, NET_INVALID_PORT);
    check_str_eq(addr.url.path, "somewhere");

    check_bool_true(net_str_parse("http://www.google.com:22/somewhere", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_false(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.url.hostname.host, "www.google.com");
    check_int_eq(addr.url.hostname.port, 22);
    check_str_eq(addr.url.path, "somewhere");
}

static void test_parse_url_https_hostname()
{
    struct net_addr addr;

    check_bool_true(net_str_parse("https://www.google.com", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_true(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.url.hostname.host, "www.google.com");
    check_int_eq(addr.url.hostname.port, NET_INVALID_PORT);
    check_str_eq(addr.url.path, "");

    check_bool_true(net_str_parse("https://www.google.com:1234", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_true(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.url.hostname.host, "www.google.com");
    check_int_eq(addr.url.hostname.port, 1234);
    check_str_eq(addr.url.path, "");

    check_bool_true(net_str_parse("https://www.google.com/somewhere", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_true(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.url.hostname.host, "www.google.com");
    check_int_eq(addr.url.hostname.port, NET_INVALID_PORT);
    check_str_eq(addr.url.path, "somewhere");

    check_bool_true(net_str_parse("https://www.google.com:22/somewhere", &addr));
    check_int_eq(addr.type, NET_ADDR_TYPE_URL);
    check_bool_true(addr.url.is_https);
    check_int_eq(addr.url.type, NET_ADDR_TYPE_HOSTNAME);
    check_str_eq(addr.url.hostname.host, "www.google.com");
    check_int_eq(addr.url.hostname.port, 22);
    check_str_eq(addr.url.path, "somewhere");
}

static void test_net_addr_to_str_ipv4()
{
    struct net_addr addr;

    addr.type = NET_ADDR_TYPE_IPV4;
    addr.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.ipv4.port = NET_INVALID_PORT;

    check_str_eq(net_addr_to_str(&addr), "127.0.0.1");

    addr.type = NET_ADDR_TYPE_IPV4;
    addr.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.ipv4.port = 1234;

    check_str_eq(net_addr_to_str(&addr), "127.0.0.1:1234");
}

static void test_net_addr_to_str_hostname()
{
    struct net_addr addr;

    addr.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.hostname.host, NET_LOCALHOST_NAME);
    addr.hostname.port = NET_INVALID_PORT;

    check_str_eq(net_addr_to_str(&addr), "localhost");

    addr.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.hostname.host, NET_LOCALHOST_NAME);
    addr.hostname.port = 1234;

    check_str_eq(net_addr_to_str(&addr), "localhost:1234");
}

static void test_net_addr_to_str_url_http_ipv4()
{
    struct net_addr addr;

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = false;
    addr.url.type = NET_ADDR_TYPE_IPV4;
    addr.url.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.url.ipv4.port = NET_INVALID_PORT;
    memset(addr.url.path, 0, sizeof(addr.url.path));

    check_str_eq(net_addr_to_str(&addr), "http://127.0.0.1");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = false;
    addr.url.type = NET_ADDR_TYPE_IPV4;
    addr.url.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.url.ipv4.port = NET_INVALID_PORT;
    strcpy(addr.url.path, "somewhere");

    check_str_eq(net_addr_to_str(&addr), "http://127.0.0.1/somewhere");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = false;
    addr.url.type = NET_ADDR_TYPE_IPV4;
    addr.url.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.url.ipv4.port = 1234;
    memset(addr.url.path, 0, sizeof(addr.url.path));

    check_str_eq(net_addr_to_str(&addr), "http://127.0.0.1:1234");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = false;
    addr.url.type = NET_ADDR_TYPE_IPV4;
    addr.url.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.url.ipv4.port = 1234;
    strcpy(addr.url.path, "somewhere");

    check_str_eq(net_addr_to_str(&addr), "http://127.0.0.1:1234/somewhere");
}

static void test_net_addr_to_str_url_https_ipv4()
{
    struct net_addr addr;

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = true;
    addr.url.type = NET_ADDR_TYPE_IPV4;
    addr.url.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.url.ipv4.port = NET_INVALID_PORT;
    memset(addr.url.path, 0, sizeof(addr.url.path));

    check_str_eq(net_addr_to_str(&addr), "https://127.0.0.1");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = true;
    addr.url.type = NET_ADDR_TYPE_IPV4;
    addr.url.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.url.ipv4.port = NET_INVALID_PORT;
    strcpy(addr.url.path, "somewhere");

    check_str_eq(net_addr_to_str(&addr), "https://127.0.0.1/somewhere");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = true;
    addr.url.type = NET_ADDR_TYPE_IPV4;
    addr.url.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.url.ipv4.port = 1234;
    memset(addr.url.path, 0, sizeof(addr.url.path));

    check_str_eq(net_addr_to_str(&addr), "https://127.0.0.1:1234");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = true;
    addr.url.type = NET_ADDR_TYPE_IPV4;
    addr.url.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.url.ipv4.port = 1234;
    strcpy(addr.url.path, "somewhere");

    check_str_eq(net_addr_to_str(&addr), "https://127.0.0.1:1234/somewhere");
}

static void test_net_addr_to_str_url_http_hostname()
{
    struct net_addr addr;

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = false;
    addr.url.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.url.hostname.host, "www.google.com");
    addr.url.hostname.port = NET_INVALID_PORT;
    memset(addr.url.path, 0, sizeof(addr.url.path));

    check_str_eq(net_addr_to_str(&addr), "http://www.google.com");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = false;
    addr.url.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.url.hostname.host, "www.google.com");
    addr.url.hostname.port = NET_INVALID_PORT;
    strcpy(addr.url.path, "somewhere");

    check_str_eq(net_addr_to_str(&addr), "http://www.google.com/somewhere");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = false;
    addr.url.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.url.hostname.host, "www.google.com");
    addr.url.hostname.port = 1234;
    memset(addr.url.path, 0, sizeof(addr.url.path));

    check_str_eq(net_addr_to_str(&addr), "http://www.google.com:1234");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = false;
    addr.url.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.url.hostname.host, "www.google.com");
    addr.url.hostname.port = 1234;
    strcpy(addr.url.path, "somewhere");

    check_str_eq(net_addr_to_str(&addr), "http://www.google.com:1234/somewhere");
}

static void test_net_addr_to_str_url_https_hostname()
{
    struct net_addr addr;

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = true;
    addr.url.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.url.hostname.host, "www.google.com");
    addr.url.hostname.port = NET_INVALID_PORT;
    memset(addr.url.path, 0, sizeof(addr.url.path));

    check_str_eq(net_addr_to_str(&addr), "https://www.google.com");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = true;
    addr.url.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.url.hostname.host, "www.google.com");
    addr.url.hostname.port = NET_INVALID_PORT;
    strcpy(addr.url.path, "somewhere");

    check_str_eq(net_addr_to_str(&addr), "https://www.google.com/somewhere");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = true;
    addr.url.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.url.hostname.host, "www.google.com");
    addr.url.hostname.port = 1234;
    memset(addr.url.path, 0, sizeof(addr.url.path));

    check_str_eq(net_addr_to_str(&addr), "https://www.google.com:1234");

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.is_https = true;
    addr.url.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.url.hostname.host, "www.google.com");
    addr.url.hostname.port = 1234;
    strcpy(addr.url.path, "somewhere");

    check_str_eq(net_addr_to_str(&addr), "https://www.google.com:1234/somewhere");
}

static void test_resolve_hostname_str()
{
    struct net_addr_ipv4 addr;

    addr.addr = NET_INVALID_ADDR;
    addr.port = NET_INVALID_PORT;

    check_bool_true(net_resolve_hostname("www.google.com", &addr));
    check_int_neq(addr.addr, NET_INVALID_ADDR);
    check_int_eq(addr.port, NET_INVALID_PORT);

    addr.addr = NET_INVALID_ADDR;
    addr.port = NET_INVALID_PORT;

    check_bool_true(net_resolve_hostname(NET_LOCALHOST_NAME, &addr));
    check_int_eq(addr.addr, NET_LOCALHOST_ADDR);
    check_int_eq(addr.port, NET_INVALID_PORT);

    addr.addr = NET_INVALID_ADDR;
    addr.port = NET_INVALID_PORT;

    check_bool_false(net_resolve_hostname("opihjblaksdasd", &addr));
    check_int_eq(addr.addr, NET_INVALID_ADDR);
    check_int_eq(addr.port, NET_INVALID_PORT);
}

static void test_resolve_hostname_ipv4_addr()
{
    struct net_addr addr;
    struct net_addr_ipv4 res_addr;

    addr.type = NET_ADDR_TYPE_IPV4;
    addr.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.ipv4.port = 1234;

    check_bool_true(net_resolve_hostname_net_addr(&addr, &res_addr));
    check_int_eq(res_addr.addr, NET_LOCALHOST_ADDR);
    check_int_eq(res_addr.port, 1234);
}

static void test_resolve_hostname_hostname_addr()
{
    struct net_addr addr;
    struct net_addr_ipv4 res_addr;

    addr.type = NET_ADDR_TYPE_HOSTNAME;
    strcpy(addr.hostname.host, NET_LOCALHOST_NAME);
    addr.hostname.port = 1234;

    check_bool_true(net_resolve_hostname_net_addr(&addr, &res_addr));
    check_int_eq(res_addr.addr, NET_LOCALHOST_ADDR);
    check_int_eq(res_addr.port, 1234);
}

static void test_resolve_hostname_url_ipv4_addr()
{
    struct net_addr addr;
    struct net_addr_ipv4 res_addr;

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.type = NET_ADDR_TYPE_IPV4;
    addr.url.is_https = false;
    strcpy(addr.url.path, "somewhere");
    addr.url.ipv4.addr = NET_LOCALHOST_ADDR;
    addr.url.ipv4.port = 1234;

    check_bool_true(net_resolve_hostname_net_addr(&addr, &res_addr));
    check_int_eq(res_addr.addr, NET_LOCALHOST_ADDR);
    check_int_eq(res_addr.port, 1234);
}

static void test_resolve_hostname_url_hostname_addr()
{
    struct net_addr addr;
    struct net_addr_ipv4 res_addr;

    addr.type = NET_ADDR_TYPE_URL;
    addr.url.type = NET_ADDR_TYPE_HOSTNAME;
    addr.url.is_https = false;
    strcpy(addr.url.path, "somewhere");
    strcpy(addr.url.hostname.host, NET_LOCALHOST_NAME);
    addr.url.hostname.port = 1234;

    check_bool_true(net_resolve_hostname_net_addr(&addr, &res_addr));
    check_int_eq(res_addr.addr, NET_LOCALHOST_ADDR);
    check_int_eq(res_addr.port, 1234);
}

static void test_check_remote_connection()
{
    struct net_addr addr;

    check_bool_true(net_str_parse("127.0.0.1:1", &addr));
    check_bool_false(net_check_remote_connection(&addr, 2000));

    check_bool_true(net_str_parse("www.google.com:80", &addr));
    check_bool_true(net_check_remote_connection(&addr, 2000));

    check_bool_true(net_str_parse("www.google.com:22", &addr));
    check_bool_false(net_check_remote_connection(&addr, 2000));
}

static void test_check_remote_connection_ipv4()
{
    struct net_addr addr;

    check_bool_true(net_str_parse("127.0.0.1:1", &addr));
    check_bool_false(net_check_remote_connection_ipv4(&addr.ipv4, 2000));
}

TEST_MODULE_BEGIN("util/net")
TEST_MODULE_TEST(test_parse_invalid)
TEST_MODULE_TEST(test_parse_ipv4)
TEST_MODULE_TEST(test_parse_hostname)
TEST_MODULE_TEST(test_parse_url_http_ipv4)
TEST_MODULE_TEST(test_parse_url_https_ipv4)
TEST_MODULE_TEST(test_parse_url_http_hostname)
TEST_MODULE_TEST(test_parse_url_https_hostname)
TEST_MODULE_TEST(test_net_addr_to_str_ipv4)
TEST_MODULE_TEST(test_net_addr_to_str_hostname)
TEST_MODULE_TEST(test_net_addr_to_str_url_http_ipv4)
TEST_MODULE_TEST(test_net_addr_to_str_url_https_ipv4)
TEST_MODULE_TEST(test_net_addr_to_str_url_http_hostname)
TEST_MODULE_TEST(test_net_addr_to_str_url_https_hostname)
TEST_MODULE_TEST(test_resolve_hostname_str)
TEST_MODULE_TEST(test_resolve_hostname_ipv4_addr)
TEST_MODULE_TEST(test_resolve_hostname_hostname_addr)
TEST_MODULE_TEST(test_resolve_hostname_url_ipv4_addr)
TEST_MODULE_TEST(test_resolve_hostname_url_hostname_addr)
TEST_MODULE_TEST(test_check_remote_connection)
TEST_MODULE_TEST(test_check_remote_connection_ipv4)
TEST_MODULE_END()
