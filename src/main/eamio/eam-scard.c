#include "eam-scard.h"
#include <stdio.h>

#include "util/log.h"

struct scard {
    SCARDCONTEXT hContext;
    LPSTR mszReaders;
    DWORD pcchReaders;
    LPSTR reader_name[EAM_UNIT_COUNT];
    SCARD_READERSTATE rs[EAM_UNIT_COUNT];
    bool mux_reader;
};

static struct scard *scard = NULL;

#define MAX_APDU_SIZE 255
#define PICC_OPERATING_PARAMS 0xDFu
BYTE PICC_OPERATING_PARAM_CMD[5] = {0xFFu, 0x00u, 0x51u, PICC_OPERATING_PARAMS, 0x00u};
#define PICC_SUCCESS 0x90u
static const BYTE UID_CMD[5] = {0xFFu, 0xCAu, 0x00u, 0x00u, 0x00u};
enum scard_atr_protocol {
    SCARD_ATR_PROTOCOL_ISO14443_PART3 = 0x03,
    SCARD_ATR_PROTOCOL_ISO15693_PART3 = 0x0B,
    SCARD_ATR_PROTOCOL_FELICA_212K = 0x11,
    SCARD_ATR_PROTOCOL_FELICA_424K = 0x12,
};

void eam_scard_init(bool mux_reader)
{
    int curReaderIdx = 0;
    LONG ret;
    bool success = false;

    if (scard != NULL) {
        // already init
        return;
    }

    // Alloc and init the instance struct
    scard = (struct scard *) malloc(sizeof(struct scard));
    if (scard == NULL) {
        goto end;
    }
    memset(scard, 0, sizeof(struct scard));
    scard->mux_reader = mux_reader;

    // Assign readers to each eam unit
    ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &scard->hContext);
    if (ret != SCARD_S_SUCCESS) {
        log_warning("SCardEstablishContext failed: %08lX", (ULONG) ret);
        goto end;
    }

    scard->pcchReaders = SCARD_AUTOALLOCATE;

    ret = SCardListReaders(
        scard->hContext, NULL, (LPSTR) &scard->mszReaders, &scard->pcchReaders);
    if (ret != SCARD_S_SUCCESS) {
        log_warning("SCardListReaders failed: %08lX", (ULONG) ret);
        goto end;
    }

    // Find up to EAM_UNIT_COUNT readers
    for (LPSTR reader = scard->mszReaders; *reader != '\0';
         reader = reader + lstrlen(reader) + 1) {

        // assign the reader
        scard->reader_name[curReaderIdx] = reader;

        // are we done?
        curReaderIdx++;
        if (curReaderIdx == EAM_UNIT_COUNT || mux_reader) {
            // all readers assigned (or we are only going to use one)
            break;
        }
    }

    // report results and initialize states
    for (int i = 0; i < EAM_UNIT_COUNT; i++) {
        // if we are muxing, assign the first reader to every unit
        if (mux_reader) {
            scard->reader_name[i] = scard->reader_name[0];
        }

        if (scard->reader_name[i] != NULL) {
            log_info("unit_no %d scard reader: %s", i, scard->reader_name[i]);
        } else {
            log_warning("Failed to find reader for unit_no %d", i);
        }

        scard->rs[i].szReader = scard->reader_name[i];
        scard->rs[i].dwCurrentState = SCARD_STATE_UNAWARE;
    }

    success = true;

end:    
    if (!success) {
        log_warning("Smartcard Reader init failed");
        eam_scard_fini();
    }
}

void eam_scard_fini()
{
    if (scard != NULL) {
        if (scard->mszReaders != NULL) {
            SCardFreeMemory(scard->hContext, scard->mszReaders);
        }

        SCardReleaseContext(scard->hContext);

        free(scard);
        scard = NULL;
    }
}

bool eam_scard_is_present(uint8_t unit_no)
{
    bool is_active;

    if (scard == NULL) {
        return false;
    }
    
    log_assert(unit_no < lengthof(scard->rs));

    if (scard->mux_reader) {
        // Determine active reader by numlock state
        is_active = (GetKeyState(VK_NUMLOCK) & 0x0001) == unit_no;
    } else {
        is_active = true;
    }

    if (is_active) {
        return (scard->rs[unit_no].dwEventState & SCARD_STATE_PRESENT) != 0;
    } else {
        return false;
    }
}

uint8_t eam_scard_read(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes)
{
    uint8_t card_type = EAM_IO_CARD_NONE;

    if (scard == NULL) {
        return card_type;
    }

    log_assert(unit_no < lengthof(scard->rs));
    log_assert(card_id != NULL);
    log_assert(nbytes == EAM_CARD_NBYTES);

    /**
     * This function was lifted from scard.cpp in spice2x where its called 
     * scard_update(), then modified for use in the eam io api. Changes in 
     * support of BT5 coding standards were not made to minimize changes to
     * the original code. Error handling was however added so as to 
     * disconnect from the card reader.
     * 
     * @note The logic for the UID fix option in spice is not present
     */

    // Connect to the smart card.
    LONG lRet = 0;
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;
    
    lRet = SCardConnect(scard->hContext, scard->rs[unit_no].szReader, SCARD_SHARE_EXCLUSIVE, 
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);

    if (lRet != SCARD_S_SUCCESS) {
        log_warning("error connecting to the card: %ld", lRet);
        return card_type;
    }

    // set the reader params
    lRet = 0;
    const SCARD_IO_REQUEST *pci = dwActiveProtocol == SCARD_PROTOCOL_T1 ? SCARD_PCI_T1 : SCARD_PCI_T0;
    DWORD cbRecv = MAX_APDU_SIZE;
    BYTE pbRecv[MAX_APDU_SIZE];
    if ((lRet = SCardTransmit(hCard, pci, PICC_OPERATING_PARAM_CMD, sizeof(PICC_OPERATING_PARAM_CMD),
                              NULL, pbRecv, &cbRecv)) != SCARD_S_SUCCESS) {
        log_warning("error setting PICC params: %ld", lRet);
        goto End;
    }

    if (cbRecv > 2 && pbRecv[0] != PICC_SUCCESS && pbRecv[1] != PICC_OPERATING_PARAMS) {
        log_warning("PICC params not valid 0x%02X != 0x%02X",
                pbRecv[1], PICC_OPERATING_PARAMS);
        goto End;
    }

    // Read ATR to determine card type.
    TCHAR szReader[200];
    DWORD cchReader = 200;
    BYTE atr[32];
    DWORD cByteAtr = 32;
    lRet = SCardStatus(hCard, szReader, &cchReader, NULL, NULL, atr, &cByteAtr);
    if (lRet != SCARD_S_SUCCESS) {
        log_warning("error getting card status: %ld",lRet);
        goto End;
    }

    // Only care about 20-byte ATRs returned by arcade-type smart cards
    if (cByteAtr != 20) {
        log_warning("ignoring card with len(atr) = %ld", cByteAtr);
        goto End;
    }

    // Figure out if we should reverse the UID returned by the card based on the
    // ATR protocol
    BYTE cardProtocol = atr[12];
    BOOL shouldReverseUid = false;
    if (cardProtocol == SCARD_ATR_PROTOCOL_ISO15693_PART3) {
        log_info("card protocol: ISO15693_PART3");
        shouldReverseUid = true;
    } else if (cardProtocol == SCARD_ATR_PROTOCOL_ISO14443_PART3) {
        log_info("card protocol: ISO14443_PART3");
    } else if (cardProtocol == SCARD_ATR_PROTOCOL_FELICA_212K) {
        log_info("card protocol: FELICA_212K");
    } else if (cardProtocol == SCARD_ATR_PROTOCOL_FELICA_424K) {
        log_info("card protocol: FELICA_424K");
    } else {
        log_warning("Unknown NFC Protocol: 0x%02X", cardProtocol);
        //return;
    }

    // Read UID
    cbRecv = MAX_APDU_SIZE;
    if ((lRet = SCardTransmit(hCard, pci, UID_CMD, sizeof(UID_CMD), NULL, pbRecv, &cbRecv)) != SCARD_S_SUCCESS) {
        log_warning("error querying card UID: %ld", lRet);
        goto End;
    }

    // check response
    // 2 bytes minimum for response code
    // 3+ bytes for response code plus one byte card number
    if (cbRecv < 3) {
        if (cbRecv < 2) {
            log_warning("UID query failed - not enough bytes (%ld)", cbRecv);
        } else {
            log_warning("UID query failed - received {0x%02X} {0x%02X} (%ld bytes)", pbRecv[0], pbRecv[1], cbRecv);
        }
        goto End;
    }

    if (cbRecv < 8) {
        log_misc("padding card uid to 8 bytes by adding zeroes at the end");
        memset(&pbRecv[cbRecv], 0, 8 - cbRecv);
    } else if (cbRecv > 8) {
        log_misc("taking first 8 bytes of len(uid) = %ld", cbRecv);
    }

    // Copy UID to struct, reversing if necessary
    if (shouldReverseUid) {
        for (DWORD i = 0; i < 8; i++) {
            card_id[i] = pbRecv[7 - i];
        }
    } else {
        memcpy(card_id, pbRecv, 8);
    }

    log_info(
        "reader unit no: %d, card number to be inserted into game: "
        "%02X%02X %02X%02X %02X%02X %02X%02X",
        unit_no,
        card_id[0],
        card_id[1],
        card_id[2],
        card_id[3],
        card_id[4],
        card_id[5],
        card_id[6],
        card_id[7]);
  
    if (card_id[0] == 0xe0 && card_id[1] == 0x04) {
        card_type = EAM_IO_CARD_ISO15696;
    } else {
        card_type = EAM_IO_CARD_FELICA;
    }

End:
    SCardDisconnect(hCard, SCARD_LEAVE_CARD);

    return card_type;
}

void eam_scard_poll(uint8_t unit_no)
{
    SCARD_READERSTATE *rs;

    if (scard != NULL) {
        log_assert(unit_no < lengthof(scard->rs));

        rs = &scard->rs[unit_no];

        if (rs->szReader == NULL) {
            // no reader present for this unit
            return;
        }

        SCardGetStatusChange(scard->hContext, 0, rs, 1);
    }
}

LPSTR eam_scard_get_reader_name(uint8_t unit_no)
{
    if (scard != NULL) {
        log_assert(unit_no < lengthof(scard->rs));
    
        if (scard->rs[unit_no].szReader != NULL) {
            return scard->reader_name[unit_no];
        }        
    }

    return "Not Present";
}