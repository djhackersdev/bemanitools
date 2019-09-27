#ifndef IMPORTS_EAPKI_H
#define IMPORTS_EAPKI_H

#include <stdbool.h>

#include "imports/avs.h"

typedef bool (*dll_entry_init_t)(char *, struct property_node *);
typedef bool (*dll_entry_main_t)(void);

#endif
