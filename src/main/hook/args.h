#pragma once

#include <windows.h>

HRESULT args_recover(int *argc, char ***argv);
void args_free(int argc, char **argv);
