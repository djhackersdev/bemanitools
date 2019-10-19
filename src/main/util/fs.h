#ifndef UTIL_FS_H
#define UTIL_FS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Load a file into memory
 *
 * @param filename Path of the file to load
 * @param bytes Pointer to a pointer to return an allocated location with the
 *        loaded data to
 * @param nbytes Pointer to a variable to return the read size of the file to
 * @param text_mode True to read the file in text mode (add terminating \0),
 *        false for binary mode
 * @return True on success, false on error
 */
bool file_load(
    const char *filename, void **bytes, size_t *nbytes, bool text_mode);

/**
 * Save a buffer to a file (overwrite any existing files with the same name)
 *
 * @param filename Path to save the data to
 * @param bytes Pointer to buffer with contents to save
 * @param nbytes Number of bytes to write to the file
 * @return True on success, false on error
 */
bool file_save(const char *filename, const void *bytes, size_t nbytes);

/**
 * Check if a normal file, folder, symlink, ... exists
 *
 * @param path Path as c-string.
 * @return True if exists, false otherwise
 */
bool path_exists(const char *path);

/**
 * Check if a normal file, folder, symlink, ... exists
 *
 * @param path Path as w-string.
 * @return True if exists, false otherwise
 */
bool path_exists_wstr(const wchar_t *path);

FILE *fopen_appdata(const char *vendor, const char *filename, const char *mode);

/**
 * Create a new directory
 *
 * @param path Path to directory
 * @return True if successful, false on failure
 */
bool path_mkdir(const char *path);
char *path_next_element(char *path);

#define read_bin(f, val, size) (fread((val), (size), 1, (f)) == 1)
#define read_u8(f, val) (fread((val), 1, 1, (f)) == 1)
#define read_u16(f, val) (fread((val), 2, 1, (f)) == 1)
#define read_u32(f, val) (fread((val), 4, 1, (f)) == 1)

bool read_str(FILE *f, char **str);

#define write_bin(f, val, size) fwrite((val), (size), 1, (f))
#define write_u8(f, val) fwrite((val), 1, 1, (f))
#define write_u16(f, val) fwrite((val), 2, 1, (f))
#define write_u32(f, val) fwrite((val), 4, 1, (f))

void write_str(FILE *f, const char *str);

#endif
