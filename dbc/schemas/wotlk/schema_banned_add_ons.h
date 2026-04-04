#ifndef SCHEMA_BANNED_ADD_ONS_H
#define SCHEMA_BANNED_ADD_ONS_H

#include "dbc_schema.h"

static const DbcFieldDef banned_add_ons_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "NameMD5_1",     DbcFieldType::UInt32 },
    { "NameMD5_2",     DbcFieldType::UInt32 },
    { "NameMD5_3",     DbcFieldType::UInt32 },
    { "NameMD5_4",     DbcFieldType::UInt32 },
    { "VersionMD5_1",  DbcFieldType::UInt32 },
    { "VersionMD5_2",  DbcFieldType::UInt32 },
    { "VersionMD5_3",  DbcFieldType::UInt32 },
    { "VersionMD5_4",  DbcFieldType::UInt32 },
    { "LastModified",  DbcFieldType::UInt32 },
    { "Flags",         DbcFieldType::UInt32 },
};

static const DbcSchema schema_banned_add_ons = {
    "BannedAddOns",
    banned_add_ons_fields,
    sizeof(banned_add_ons_fields) / sizeof(banned_add_ons_fields[0])
};

#endif
