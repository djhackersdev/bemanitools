#include "avs-util/error.h"

#include "util/defs.h"

struct avs_util_avs_error_str {
    avs_error error;
    const char *msg;
};

static const char *avs_util_error_unknown = "unknown error";

// Source: https://github.com/spicetools/spicetools
static const struct avs_util_avs_error_str AVS_UTIL_ERRORS[] = {
    {0x80092000, "invalid type"},
    {0x80092001, "type cannot use as array"},
    {0x80092002, "invalid"},
    {0x80092003, "too large data size"},
    {0x80092004, "too small buffer size"},
    {0x80092005, "passcode 0 is not allowed"},
    {0x80092040, "invalid node name"},
    {0x80092041, "invalid attribute name"},
    {0x80092042, "reserved attribute name"},
    {0x80092043, "cannot find node/attribute"},
    {0x80092080, "cannot allocate node"},
    {0x80092081, "cannot allocate node value"},
    {0x80092082, "cannot allocate mdigest for finger-print"},
    {0x80092083, "cannot allocate nodename"},
    {0x800920C0, "node type differs"},
    {0x800920C1, "node type is VOID"},
    {0x800920C2, "node is array"},
    {0x800920C3, "node is not array"},
    {0x80092100, "node is create-disabled"},
    {0x80092101, "node is read-disabled"},
    {0x80092102, "node is write-disabled"},
    {0x80092103, "flag is already locked"},
    {0x80092104, "passcode differs"},
    {0x80092105, "insert_read() is applied to attribute"},
    {0x80092106, "part_write() is applied to attribute"},
    {0x80092107, "MODE_EXTEND flag differs"},
    {0x80092140, "root node already exists"},
    {0x80092141, "attribute cannot have children"},
    {0x80092142, "node/attribute already exists"},
    {0x80092143, "number of nodes exceeds 65535"},
    {0x80092144, "cannot interpret as number"},
    {0x80092145, "property is empty"},
    {0x80092180, "I/O error"},
    {0x80092181, "unexpected EOF"},
    {0x80092182, "unknown format"},
    {0x800921C0, "broken magic"},
    {0x800921C1, "broken metadata"},
    {0x800921C2, "broken databody"},
    {0x800921C3, "invalid type"},
    {0x800921C4, "too large data size"},
    {0x800921C5, "too long node/attribute name"},
    {0x800921C6, "attribute name is too long"},
    {0x800921C7, "node/attribute already exists"},
    {0x80092200, "invalid encoding"},
    {0x80092201, "invalid XML token"},
    {0x80092202, "XML syntax error"},
    {0x80092203, "start tag / end tag mismatch"},
    {0x80092204, "too large node data (__size mismatch)"},
    {0x80092205, "too deep node tree"},
    {0x80092206, "invalid type"},
    {0x80092207, "invalid size"},
    {0x80092208, "invalid count"},
    {0x80092209, "invalid value"},
    {0x8009220A, "invalid node name"},
    {0x8009220B, "invalid attribute name"},
    {0x8009220C, "reserved attribute name"},
    {0x8009220D, "node/attribute already exists"},
    {0x8009220E, "too many elements in node data"},
    {0x80092240, "JSON syntax error"},
    {0x80092241, "invalid JSON literal"},
    {0x80092242, "invalid JSON number"},
    {0x80092243, "invalid JSON string"},
    {0x80092244, "invalid JSON object name"},
    {0x80092245, "object name already exists"},
    {0x80092246, "too long JSON object name"},
    {0x80092247, "too deep JSON object/array nesting"},
    {0x80092248, "cannot convert JSON array to property"},
    {0x80092249, "cannot convert empty JSON object to property"},
    {0x8009224A, "root node already exists"},
    {0x8009224B, "cannot convert root node to TYPE_ARRAY"},
    {0x8009224C, "name represents reserved attribute"},
    {0x80092280, "finger-print differs"},
    {0x800922C0, "operation is not supported"}};

const char *avs_util_error_str(avs_error error)
{
    int i;

    for (i = 0; i < lengthof(AVS_UTIL_ERRORS); i++) {
        if (error == AVS_UTIL_ERRORS[i].error) {
            return AVS_UTIL_ERRORS[i].msg;
        }
    }

    return avs_util_error_unknown;
}