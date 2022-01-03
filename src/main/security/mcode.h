#ifndef SECURITY_MCODE_H
#define SECURITY_MCODE_H

#include <stdbool.h>

/* Used to fill up empty spaces */
#define SECURITY_MCODE_FIELD_BLANK ' '
#define SECURITY_MCODE_FIELD_NULL '\0'

/* Header */
#define SECURITY_MCODE_HEADER 'G'

/* Unkn */
#define SECURITY_MCODE_UNKN_C 'C'
#define SECURITY_MCODE_UNKN_E 'E'
#define SECURITY_MCODE_UNKN_K 'K'
#define SECURITY_MCODE_UNKN_N 'N'
#define SECURITY_MCODE_UNKN_Q 'Q'

/* Games */
#define SECURITY_MCODE_GAME_LEN 3

/* IIDX */
#define SECURITY_MCODE_GAME_IIDX_1 "863"
#define SECURITY_MCODE_GAME_IIDX_1_CLUB "896"
#define SECURITY_MCODE_GAME_IIDX_SUB "983"
#define SECURITY_MCODE_GAME_IIDX_CLUB_2 "984"
#define SECURITY_MCODE_GAME_IIDX_2 "985"
#define SECURITY_MCODE_GAME_IIDX_3 "992"
#define SECURITY_MCODE_GAME_IIDX_4 "A03"
#define SECURITY_MCODE_GAME_IIDX_5 "A17"
#define SECURITY_MCODE_GAME_IIDX_6 "B4U"
#define SECURITY_MCODE_GAME_IIDX_7 "B44"
#define SECURITY_MCODE_GAME_IIDX_8 "C44"
#define SECURITY_MCODE_GAME_IIDX_9 "C02"
#define SECURITY_MCODE_GAME_IIDX_10 "D01"
#define SECURITY_MCODE_GAME_IIDX_11 "E11"
#define SECURITY_MCODE_GAME_IIDX_12 "ECO"
#define SECURITY_MCODE_GAME_IIDX_13 "FDD"
#define SECURITY_MCODE_GAME_IIDX_14 "GLD"
#define SECURITY_MCODE_GAME_IIDX_15 "HDD"
#define SECURITY_MCODE_GAME_IIDX_16 "I00"
#define SECURITY_MCODE_GAME_IIDX_17 "JDJ"
#define SECURITY_MCODE_GAME_IIDX_18 "JDZ"
#define SECURITY_MCODE_GAME_IIDX_19 "KDZ"
#define SECURITY_MCODE_GAME_IIDX_20 "LDJ"
#define SECURITY_MCODE_GAME_IIDX_21 "LDJ"
#define SECURITY_MCODE_GAME_IIDX_22 "LDJ"
#define SECURITY_MCODE_GAME_IIDX_23 "LDJ"
#define SECURITY_MCODE_GAME_IIDX_24 "LDJ"

/* Jubeat */
#define SECURITY_MCODE_GAME_JB_1 "H44"
#define SECURITY_MCODE_GAME_JB_3 "J44"

/* Region */
#define SECURITY_MCODE_REGION_ASIA 'A'
#define SECURITY_MCODE_REGION_JAPAN 'J'

/* Cabinet */
#define SECURITY_MCODE_CABINET_A 'A'
#define SECURITY_MCODE_CABINET_C 'C'

/* Revision */
#define SECURITY_MCODE_REVISION_A 'A'
#define SECURITY_MCODE_REVISION_B 'B'
#define SECURITY_MCODE_REVISION_C 'C'
#define SECURITY_MCODE_REVISION_D 'D'
#define SECURITY_MCODE_REVISION_E 'E'
#define SECURITY_MCODE_REVISION_F 'F'
#define SECURITY_MCODE_REVISION_G 'G'

/**
 * Structure to represent a Konami mcode which is used to identify games and
 * different hardware/software revisions.
 */
struct security_mcode {
    char header;
    char unkn;
    /* Identify the game and version, e.g. IIDX 12 */
    char game[3];
    char region;
    char cabinet;
    char revision;
};

/**
 * Default mcode used to identify white eamuse roundplugs.
 */
extern const struct security_mcode security_mcode_eamuse;

/**
 * Parse an mcode from a string representation.
 *
 * @param str String to parse.
 * @param mcode Pointer to a security_mcode struct to write the result to.
 * @return True on success, false on parsing error.
 */
bool security_mcode_parse(const char *str, struct security_mcode *mcode);

/**
 * Stringify an mcode.
 *
 * @param mcode Mcode to stringify.
 * @return String containing the stringified mcode. Caller has to manage memory.
 */
char *security_mcode_to_str(const struct security_mcode *mcode);

#endif
