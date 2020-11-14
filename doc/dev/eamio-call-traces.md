# eamio API call traces
This document outlines the different call traces from BT5 to the eamio API depending on which type
of card reader was emulated by the backend.

Unfortunately, that's a thing but shouldn't be as eamio is considered an abstraction layer to the
actual reader implementation (magnetic slotted readers, slotted readers, wavepass readers). However,
with multiple iterations and support of these different types added incrementally, this developed
into an inconsistent mess.

Luckily, the default eamio implementation (generic input/joystick/keyboard) is not affected by this.

However, for custom implementations, especially ones that shim old hardware on top of new backends,
e.g. slotted readers on newer games requiring wavepass, or new hardware on top of old backends,
e.g. wavepass readers on magnetic slotted reader games, this becomes a problem as one implementation
will likely not work across the different combinations.

Still, if you are aware of what backend type and how these are calling your eamio implemenation,
you can at least work around that. Depending on your requirements and the type of hardware you want
to use in your implementation, you might get away with a single eamio implementation to support
all backends. But, you might need to have separate ones for the different backends.

The following call traces of the eamio API might be useful for this to better understand how eamio
is called and to support you in debugging, if something is not working as expected.

## Magnetic slotted card reader (IIDX 9 - 12)
The following pseudo-code is based on a call trace that was taken running IIDX 11 with some debug
prints outputting when the corresponding function was called.

Things to note:
* Game is hammering on the `eam_io_card_slot_cmd`

```c
while (true) {
    for (int i = 0; i < 2; i++) {
        // Part which is always executed, i.e. when the reader is in idle mode. This starts once
        // the game has initialized the readers on boot
        eam_io_poll(i)
        sensores = eam_io_get_sensor_state(i)

        // Player pressed start button and the login screen appears
        if (player_active_login(i)) {
            // Once card data is read/available, this is skipped
            if (player_no_card_data_read(i)) {
                // Opens the slot to allow inserting the card
                eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_OPEN)
            }
            
            // Yes, the backend keeps issuing read command requests as long as the card is in the
            // slot
            if (sensores & BIT_MASK_FRONT && sensores & BIT_MASK_BACK) {
                eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_READ)

                if (player_no_card_data_read(i)) {
                    read_card_data = eam_io_read_card(i)
                }
            }
        // Logout screen after the game session. Keep calling this part until the player has logged
        // out entirely which is concluded by removing the card from the slot
        } else if (player_active_logout(i)) {
            if (sensores & BIT_MASK_FRONT && sensores & BIT_MASK_BACK) {
                eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_EJECT)
            }

            eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_CLOSE)
        } else {
            // Yes, the backend is always issuing close commands when there is nothing to do
            eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_CLOSE)
        }
    }
}
```

## Slotted card reader (IIDX 13 - 18)
The following pseudo-code is based on a call trace that was taken running IIDX 13 with some debug
prints outputting when the corresponding function was called.

Things to note:
* Compared to the magnetic card reader behavior above, the game makes more sophisticated use of the
commands and usually issues them only once

```c
while (true) {
    for (int i = 0; i < 2; i++) {
        // Part which is always executed, i.e. when the reader is in idle mode. This starts once
        // the game has initialized the readers on boot
        eam_io_poll(i)
        sensores = eam_io_get_sensor_state(i)

        // Player pressed start button and the login screen appears
        if (player_active_login(i) && card_not_read_yet(i)) {            
            if (sensores & BIT_MASK_FRONT && sensores & BIT_MASK_BACK) {
                eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_READ)

                if (player_no_card_data_read(i)) {
                    read_card_data = eam_io_read_card(i)
                }
            }
        // Logout screen after the game session. Keep calling this part until the player has logged
        // out entirely which is concluded by removing the card from the slot
        } else if (player_active_logout(i)) {
            if (sensores & BIT_MASK_FRONT && sensores & BIT_MASK_BACK) {
                eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_EJECT)
                eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_CLOSE)
            } else {
                eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_CLOSE)
                // concludes logout game flow
            }
        }
    }
}
```

## Wavepass (IIDX 19+)
The following pseudo-code is based on a call trace that was taken running IIDX 27 with some debug
prints outputting when the corresponding function was called.

Things to note:
* Since there is not slot anymore, but the backend relies on a report by `eam_io_get_sensor_state`,
this call is still relevant to report back that a card touched the reader
* No part for logging out the player, since there is no card eject necessary

```c
while (true) {
    for (int i = 0; i < 2; i++) {
        // Part which is always executed, i.e. when the reader is in idle mode. This starts once
        // the game has initialized the readers on boot
        eam_io_poll(i)
        sensores = eam_io_get_sensor_state(i)

        // Player pressed start button and the login screen appears
        if (player_active_login(i) && card_not_read_yet(i)) {            
            if (sensores & BIT_MASK_FRONT && sensores & BIT_MASK_BACK) {
                eam_io_card_slot_cmd(i, EAM_IO_CARD_SLOT_CMD_READ)

                if (player_no_card_data_read(i)) {
                    read_card_data = eam_io_read_card(i)
                }
            }
        }
    }
}
```