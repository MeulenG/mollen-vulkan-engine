#ifndef DBC_ENUM_H
#define DBC_ENUM_H

#include <cstdint>

// A single (value, label) pair - e.g. {0, "Male"} for the Sex enum.
//
// Values are int32 so we can encode signed integer codes too (e.g. PowerType
// uses -2 for Health). DBC fields that hold these values may be UInt32 in
// storage; the editor handles the conversion.
struct DbcEnumValue {
    int32_t value;
    const char* label;
};

// A named integer enumeration referenced by DbcFieldDef::hint when
// DbcSemantic::Enum or DbcSemantic::Bitmask is set.
//
// For Bitmask, `value` is a single-bit mask (1, 2, 4, 8, ...) and `label`
// is the flag name. For plain Enum, `value` is the integer code.
struct DbcEnum {
    const char* name;
    const DbcEnumValue* values;
    uint32_t count;
};

#endif // DBC_ENUM_H
