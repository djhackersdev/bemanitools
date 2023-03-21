#ifndef IIDXHOOK_SETTINGS_H
#define IIDXHOOK_SETTINGS_H

#include "hook/iohook.h"

/**
 * Remaps the paths for storing "settings data" that are usually stored on the
 * drives d:\, e:\ and f:\ by the game. This defaults to the local folders
 * d\, e\ and f\.
 *
 * Required hook for 9th to Lincle.
 */
void settings_hook_init(void);

/**
 * Change the path where settings file are stored.
 *
 * @param path Destination path for storage, relative or absolute.
 */
void settings_hook_set_path(const char *path);

/**
 * iohook dispatch function
 */
HRESULT settings_hook_dispatch_irp(struct irp *irp);

#endif
