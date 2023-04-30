#ifndef IMPORTS_AVS_H
#define IMPORTS_AVS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

enum property_create_flag {
    PROPERTY_FLAG_READ = 0x1,
    PROPERTY_FLAG_WRITE = 0x2,
    PROPERTY_FLAG_CREATE = 0x4,
    PROPERTY_FLAG_BINARY = 0x8,
    PROPERTY_FLAG_APPEND = 0x10,
};

enum property_node_traversal {
    TRAVERSE_PARENT = 0,
    TRAVERSE_FIRST_CHILD = 1,
    TRAVERSE_FIRST_ATTR = 2,
    TRAVERSE_FIRST_SIBLING = 3,
    TRAVERSE_NEXT_SIBLING = 4,
    TRAVERSE_PREVIOUS_SIBLING = 5,
    TRAVERSE_LAST_SIBLING = 6,
    TRAVERSE_NEXT_SEARCH_RESULT = 7,
    TRAVERSE_PREV_SEARCH_RESULT = 8,
};

enum property_type {
    PROPERTY_TYPE_VOID = 1,
    PROPERTY_TYPE_S8 = 2,
    PROPERTY_TYPE_U8 = 3,
    PROPERTY_TYPE_S16 = 4,
    PROPERTY_TYPE_U16 = 5,
    PROPERTY_TYPE_S32 = 6,
    PROPERTY_TYPE_U32 = 7,
    PROPERTY_TYPE_S64 = 8,
    PROPERTY_TYPE_U64 = 9,
    PROPERTY_TYPE_BIN = 10,
    PROPERTY_TYPE_STR = 11,
    PROPERTY_TYPE_ATTR = 46,
    PROPERTY_TYPE_BOOL = 52
};

struct property;
struct property_node;

struct avs_net_interface {
    uint8_t mac_addr[6];
    uint8_t unknown[30];
#if AVS_VERSION >= 1603
    uint8_t unknown2[6];
#endif
#if AVS_VERSION >= 1700
    uint8_t unknown3[6];
#endif
};

enum psmap_type {
    PSMAP_TYPE_S8 = 2,
    PSMAP_TYPE_U8 = 3,
    PSMAP_TYPE_S16 = 4,
    PSMAP_TYPE_U16 = 5,
    PSMAP_TYPE_S32 = 6,
    PSMAP_TYPE_U32 = 7,
    PSMAP_TYPE_S64 = 8,
    PSMAP_TYPE_U64 = 9,
    PSMAP_TYPE_STR = 10,
    PSMAP_TYPE_ATTR = 45,
    PSMAP_TYPE_BOOL = 50,
};

#define PSMAP_FLAG_HAVE_DEFAULT 0x01

struct property_psmap {
    uint8_t type;
    uint8_t flags; /* A guess. Might just be a bool. */
    uint16_t offset;
    uint32_t size;
    const char *path;
    intptr_t xdefault;
};

#define PSMAP_BEGIN(name) struct property_psmap name[] = {
#define PSMAP_REQUIRED(type, xstruct, field, path) \
    {                                              \
        type,                                      \
        0,                                         \
        offsetof(xstruct, field),                  \
        sizeof(((xstruct *) 0)->field),            \
        path,                                      \
        0,                                         \
    },

#define PSMAP_OPTIONAL(type, xstruct, field, path, xdefault) \
    {                                                        \
        type,                                                \
        PSMAP_FLAG_HAVE_DEFAULT,                             \
        offsetof(xstruct, field),                            \
        sizeof(((xstruct *) 0)->field),                      \
        path,                                                \
        (intptr_t) xdefault,                                 \
    },

#define PSMAP_END              \
    {                          \
        0xFF, 0, 0, 0, NULL, 0 \
    }                          \
    }                          \
    ;

#if AVS_VERSION >= 1500
#define AVS_LOG_WRITER(name, chars, nchars, ctx) \
    void name(const char *chars, uint32_t nchars, void *ctx)

typedef void (*avs_log_writer_t)(const char *chars, uint32_t nchars, void *ctx);
#else
#define AVS_LOG_WRITER(name, chars, nchars, ctx) \
    void name(void *ctx, const char *chars, uint32_t nchars)

typedef void (*avs_log_writer_t)(void *ctx, const char *chars, uint32_t nchars);
#endif

typedef int (*avs_reader_t)(uint32_t context, void *bytes, size_t nbytes);

#if AVS_VERSION >= 1600
/* "avs" and "std" heaps have been unified */
typedef void (*avs_boot_t)(
    struct property_node *config,
    void *com_heap,
    size_t sz_com_heap,
    void *reserved,
    avs_log_writer_t log_writer,
    void *log_context);

void avs_boot(
    struct property_node *config,
    void *com_heap,
    size_t sz_com_heap,
    void *reserved,
    avs_log_writer_t log_writer,
    void *log_context);
#else
typedef void (*avs_boot_t)(
    struct property_node *config,
    void *std_heap,
    size_t sz_std_heap,
    void *avs_heap,
    size_t sz_avs_heap,
    avs_log_writer_t log_writer,
    void *log_context);

void avs_boot(
    struct property_node *config,
    void *std_heap,
    size_t sz_std_heap,
    void *avs_heap,
    size_t sz_avs_heap,
    avs_log_writer_t log_writer,
    void *log_context);
#endif

void avs_shutdown(void);

typedef uint32_t avs_desc;

void log_body_fatal(const char *module, const char *fmt, ...);
void log_body_info(const char *module, const char *fmt, ...);
void log_body_misc(const char *module, const char *fmt, ...);
void log_body_warning(const char *module, const char *fmt, ...);
void log_boot(avs_log_writer_t log_writer, void *log_context);
void log_change_level(int level);

int avs_net_ctrl(int ioctl, void *bytes, uint32_t nbytes);

int avs_thread_create(
    int (*proc)(void *), void *ctx, uint32_t sz_stack, unsigned int priority);
void avs_thread_destroy(int thread_id);
void avs_thread_exit(int result);
void avs_thread_join(int thread_id, int *result);

uint32_t property_read_query_memsize(
    avs_reader_t reader, uint32_t context, int unk0, int unk1);
struct property *property_create(int flags, void *buffer, uint32_t buffer_size);
struct property_node *property_search(
    struct property *prop, struct property_node *root, const char *path);
int property_insert_read(
    struct property *prop,
    struct property_node *root,
    avs_reader_t reader,
    uint32_t context);
int property_mem_write(struct property *prop, void *bytes, int nbytes);
void *property_desc_to_buffer(struct property *prop);
void property_file_write(struct property *prop, const char *path);
int property_set_flag(struct property *prop, int flags, int mask);
void property_destroy(struct property *prop);

int property_psmap_import(
    struct property *prop,
    struct property_node *root,
    void *dest,
    const struct property_psmap *psmap);
int property_psmap_export(
    struct property *prop,
    struct property_node *root,
    const void *src,
    const struct property_psmap *psmap);

struct property_node *property_node_clone(
    struct property *parent_prop,
    struct property_node *parent_node,
    struct property_node *src,
    bool deep);
struct property_node *property_node_create(
    struct property *prop,
    struct property_node *parent,
    int type,
    const char *key,
    ...);
void property_node_name(struct property_node *node, char *chars, int nchars);
const char *property_node_refdata(struct property_node *node);
int property_node_refer(
    struct property *prop,
    struct property_node *node,
    const char *name,
    enum property_type type,
    void *bytes,
    uint32_t nbytes);
void property_node_remove(struct property_node *node);
enum property_type property_node_type(struct property_node *node);
struct property_node *property_node_traversal(
    struct property_node *node, enum property_node_traversal direction);
void property_node_datasize(struct property_node *node);

bool std_getenv(const char *key, char *val, uint32_t nbytes);
void std_setenv(const char *key, const char *val);

struct avs_stat {
    uint64_t st_atime;
    uint64_t st_mtime;
    uint64_t st_ctime;
    uint32_t unk1;
    uint32_t filesize;
    struct stat padding;
};

enum avs_file_mode {
    AVS_FILE_READ = 1,
    AVS_FILE_WRITE = 2,
    AVS_FILE_CREATE = 0x10,
    AVS_FILE_TRUNCATE = 0x20,
    AVS_FILE_EXCLUSIVE = 0x80,
};

enum avs_file_flag {
    AVS_FILE_FLAG_SHARE_READ = 0x124,
    AVS_FILE_FLAG_SHARE_WRITE = 0x92,
};

enum avs_seek_origin {
    AVS_SEEK_SET = 0,
    AVS_SEEK_CUR = 1,
    AVS_SEEK_END = 2,
};

avs_desc avs_fs_open(const char *path, uint16_t mode, int flags);
int avs_fs_close(avs_desc desc);
size_t avs_fs_read(avs_desc desc, char *buf, uint32_t sz);
int avs_fs_lseek(avs_desc desc, long pos, int whence);
int avs_fs_lstat(const char *path, struct avs_stat *st);
int avs_fs_copy(const char *src, const char *dest);
int avs_fs_addfs(void *filesys_struct);
int avs_fs_mount(
    const char *mountpoint, const char *fsroot, const char *fstype, void *data);

bool avs_is_active();

#endif
