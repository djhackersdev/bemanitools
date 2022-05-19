#ifndef DDRHOOK1_FILESYSTEM_H
#define DDRHOOK1_FILESYSTEM_H

/**
 * DDR sets the current directory on boot to "D:/HDX" and then uses
 * relative file paths for everything else.
 * This function is a helper function to make it easier to turn a path
 * into an absolute path relative to the launcher ("D:/HDX/ddr_datecode")
 * and not the current working directory ("D:/HDX") so that settings
 * can be saved in the same manner as every other game.
 */
char *ddrhook1_get_launcher_path_parts();

void ddrhook1_filesystem_hook_init();

#endif