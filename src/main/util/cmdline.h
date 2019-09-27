#ifndef UTIL_CMDLINE_H
#define UTIL_CMDLINE_H

void args_recover(int *argc, char ***argv);
void args_free(int argc, char **argv);
char *args_join(int argc, char **argv);

#endif
