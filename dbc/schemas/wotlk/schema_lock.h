#ifndef SCHEMA_LOCK_H
#define SCHEMA_LOCK_H

#include "dbc_schema.h"

static const DbcFieldDef lock_fields[] = {
    { "Id",          DbcFieldType::UInt32 },
    { "Type1",       DbcFieldType::UInt32 },
    { "Type2",       DbcFieldType::UInt32 },
    { "Type3",       DbcFieldType::UInt32 },
    { "Type4",       DbcFieldType::UInt32 },
    { "Type5",       DbcFieldType::UInt32 },
    { "Type6",       DbcFieldType::UInt32 },
    { "Type7",       DbcFieldType::UInt32 },
    { "Type8",       DbcFieldType::UInt32 },
    { "Index1",      DbcFieldType::UInt32 },
    { "Index2",      DbcFieldType::UInt32 },
    { "Index3",      DbcFieldType::UInt32 },
    { "Index4",      DbcFieldType::UInt32 },
    { "Index5",      DbcFieldType::UInt32 },
    { "Index6",      DbcFieldType::UInt32 },
    { "Index7",      DbcFieldType::UInt32 },
    { "Index8",      DbcFieldType::UInt32 },
    { "Skill1",      DbcFieldType::UInt32 },
    { "Skill2",      DbcFieldType::UInt32 },
    { "Skill3",      DbcFieldType::UInt32 },
    { "Skill4",      DbcFieldType::UInt32 },
    { "Skill5",      DbcFieldType::UInt32 },
    { "Skill6",      DbcFieldType::UInt32 },
    { "Skill7",      DbcFieldType::UInt32 },
    { "Skill8",      DbcFieldType::UInt32 },
    { "Action1",     DbcFieldType::UInt32 },
    { "Action2",     DbcFieldType::UInt32 },
    { "Action3",     DbcFieldType::UInt32 },
    { "Action4",     DbcFieldType::UInt32 },
    { "Action5",     DbcFieldType::UInt32 },
    { "Action6",     DbcFieldType::UInt32 },
    { "Action7",     DbcFieldType::UInt32 },
    { "Action8",     DbcFieldType::UInt32 },
};

static const DbcSchema schema_lock = {
    "Lock",
    lock_fields,
    sizeof(lock_fields) / sizeof(lock_fields[0])
};

#endif
