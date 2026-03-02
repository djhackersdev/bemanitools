/**
 * Smartcard reader support for eamio
 */
#ifndef EAMIO_SCARD_H
#define EAMIO_SCARD_H

#include <stdint.h>
#include <stdbool.h>
#include <windows.h>
#include <winioctl.h>
#include <winscard.h>

#include "eam-impl.h"

void eam_scard_init(bool mux_reader);
void eam_scard_fini(void);

/**
 * Indicates if a card is currently present on the reader 
 */
bool eam_scard_is_present(uint8_t unit_no);

/**
 * Read a present card into card_id buffer
 */
uint8_t eam_scard_read(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes);

/**
 * Poll smartcard reader state
 */
void eam_scard_poll(uint8_t unit_no);

/**
 * Return the name of the reader or "Not Present" if no reader is bound to the
 * unit. Intended for use in the config ui.
 */
LPSTR eam_scard_get_reader_name(uint8_t unit_no);

#endif