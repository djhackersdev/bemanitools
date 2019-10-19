#ifndef UTIL_MSG_THREAD_H
#define UTIL_MSG_THREAD_H

void msg_thread_init(HINSTANCE inst);
void msg_thread_fini(void);

/* Define these callbacks */

extern void msg_window_setup(HWND hwnd);
extern LRESULT WINAPI
msg_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
extern void msg_window_teardown(HWND hwnd);

#endif
