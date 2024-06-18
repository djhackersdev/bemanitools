#ifndef CORE_LOG_BT_EXT_H
#define CORE_LOG_BT_EXT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Helper to setup the bemanitools log implementation with a stdout sink.
 */
void core_log_bt_ext_init_with_stdout();

/**
 * Helper to setup the bemanitools log implementation with a stderr sink.
 */
void core_log_bt_ext_init_with_stderr();

/**
 * Helper to setup the bemanitools log implementation with a OutputDebugStr
 * sink.
 */
void core_log_bt_ext_init_with_debug();

/**
 * Helper to setup the bemanitools log implementation with a file sink
 *
 * @param path Path to the log file to write the log output to
 * @param append If true, then append to an existing file, false to overwrite
 *        any existing file
 * @param rotate If true, rotates an existing log file and creates a new one
 *        for this session
 * @param max_rotations Max number of rotations for the log files
 */
void core_log_bt_ext_init_with_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations);

/**
 * Helper to setup the bemanitools log implementation with a stdout and file
 * sink
 *
 * Important: This combined sink is guarded by a mutex to avoid data races on
 * logging to two different sinks.
 *
 * @param path Path to the log file to write the log output to
 * @param append If true, then append to an existing file, false to overwrite
 *        any existing file
 * @param rotate If true, rotates an existing log file and creates a new one
 *        for this session
 * @param max_rotations Max number of rotations for the log files
 */
void core_log_bt_ext_init_with_stdout_and_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations);

void core_log_bt_ext_init_with_stderr_and_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations);

void core_log_bt_ext_init_async_with_stderr();

void core_log_bt_ext_init_async_with_stderr_and_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations);

#endif