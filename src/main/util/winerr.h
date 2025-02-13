#ifndef UTIL_WINERR_H
#define UTIL_WINERR_H

char* util_winerr_format_last_error_code();

char* util_winerr_format_error_code(int err_code);

#endif