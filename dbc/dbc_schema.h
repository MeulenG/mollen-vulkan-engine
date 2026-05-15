#ifndef DBC_SCHEMA_H
#define DBC_SCHEMA_H

#include <cstdint>

enum class Expansion {
    Vanilla,
    TBC,
    WotLK,
};

enum class DbcFieldType {
    UInt32,
    Int32,
    Float,
    String,
    UInt8,
    Int8,
    UInt16,
    Int16,
};

// Semantic annotation: what does this field MEAN, beyond its storage type?
//
// Editors and tools dispatch on this to render the right widget — checkbox
// for Boolean, dropdown for Enum, picker for ForeignKey, etc. Storage and
// SQL serialization are unaffected; this only governs presentation and
// human-meaningful editing affordances.
//
// Default leaves behavior identical to a pre-annotation schema.
enum class DbcSemantic {
    Default = 0,        // numeric / string with no extra meaning
    Boolean,            // 0/1 toggle (storage is still UInt32 typically)
    Enum,               // integer code; hint = enum table name in the registry
    ForeignKey,         // hint = lowercase target table name (e.g. "chrraces")
    Color,              // RGBA8 packed into a UInt32
    Bitmask,            // hint = flag table name in the registry
    LocalizedString,    // member of a 16-locale string cluster
};

struct DbcFieldDef {
    const char* name;
    DbcFieldType type;
    DbcSemantic semantic = DbcSemantic::Default;
    const char* hint = nullptr;  // optional, meaning depends on `semantic`
};

struct DbcSchema {
    const char* dbc_name;
    const DbcFieldDef* fields;
    uint32_t field_count;
    bool packed;
};

inline uint32_t GetFieldTypeSize(DbcFieldType type) {
    switch (type) {
    case DbcFieldType::UInt8:
    case DbcFieldType::Int8:
        return 1;
    case DbcFieldType::UInt16:
    case DbcFieldType::Int16:
        return 2;
    case DbcFieldType::UInt32:
    case DbcFieldType::Int32:
    case DbcFieldType::Float:
    case DbcFieldType::String:
        return 4;
    }
    return 4;
}

inline uint32_t GetSchemaRecordSize(const DbcSchema* schema) {
    uint32_t size = 0;
    for (uint32_t i = 0; i < schema->field_count; i++) {
        size += GetFieldTypeSize(schema->fields[i].type);
    }
    return size;
}

#endif // DBC_SCHEMA_H
