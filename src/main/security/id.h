#ifndef SECURITY_ID
#define SECURITY_ID

#include <stdbool.h>
#include <stdint.h>

#define SECURITY_ID_HEADER 0x01

/**
 * Structure of a security id, e.g. PCBID or EAMID.
 */
typedef struct security_id {
    uint8_t header;
    uint8_t id[8];
    uint8_t checksum;
} security_id_t;

/**
 * "Default" security id with updated header and cheacksum ready for use.
 */
extern const struct security_id security_id_default;

/**
 * Parse a stringified security id, e.g. PCBID, EAMID.
 *
 * @param str String representation of id.
 * @param id Pointer to a security_id struct to write the result to.
 * @return True on successf, false on parsing error.
 */
bool security_id_parse(const char *str, struct security_id *id);

/**
 * Stringify a security_id struct.
 *
 * @param id Id to stringify.
 * @param id_only True to stringify id only, false stringify header, id and
 *                checksum.
 * @return Allocated string with stringified data. Caller must manage memory.
 */
char *security_id_to_str(const struct security_id *id, bool id_only);

/**
 * Prepare a security id, i.e. update header and checksum.
 *
 * @param id Id to prepare.
 */
void security_id_prepare(struct security_id *id);

/**
 * Verify a provided security id, i.e. check header and checksum.
 *
 * @param id Id to verify.
 * @return True if verification is successful, false on header or checksum
 *         mismatch.
 */
bool security_id_verify(const struct security_id *id);

#endif