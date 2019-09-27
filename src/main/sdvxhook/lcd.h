#ifndef SDVXHOOK_LCD_H
#define SDVXHOOK_LCD_H

#include <windows.h>

#include "hook/iohook.h"

void lcd_init(void);
void lcd_fini(void);
HRESULT lcd_dispatch_irp(struct irp *irp);

#endif
