#ifndef CCONFIG_UTIL_H
#define CCONFIG_UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cconfig/cconfig.h"

#include "util/log.h"

bool cconfig_util_get_int(
    struct cconfig *config,
    const char *key,
    int32_t *ret,
    int32_t default_value);

bool cconfig_util_get_float(
    struct cconfig *config, const char *key, float *ret, float default_value);

bool cconfig_util_get_bool(
    struct cconfig *config, const char *key, bool *ret, bool default_value);

bool cconfig_util_get_str(
    struct cconfig *config,
    const char *key,
    char *buffer,
    size_t len,
    const char *default_value);

bool cconfig_util_get_data(
    struct cconfig *config,
    const char *key,
    uint8_t *buffer,
    size_t len,
    const uint8_t *default_value);

void cconfig_util_set_int(
    struct cconfig *config, const char *key, int32_t value, const char *desc);

void cconfig_util_set_float(
    struct cconfig *config, const char *key, float value, const char *desc);

void cconfig_util_set_bool(
    struct cconfig *config, const char *key, bool value, const char *desc);

void cconfig_util_set_str(
    struct cconfig *config,
    const char *key,
    const char *value,
    const char *desc);

void cconfig_util_set_data(
    struct cconfig *config,
    const char *key,
    const uint8_t *value,
    size_t len,
    const char *desc);

void cconfig_util_log(struct cconfig *config, log_formatter_t log_formatter);

#endif