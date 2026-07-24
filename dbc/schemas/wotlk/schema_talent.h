#ifndef SCHEMA_TALENT_H
#define SCHEMA_TALENT_H

#include "dbc_schema.h"

static const DbcFieldDef talent_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "TabID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "tab" },
    { "TierID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "tier" },
    { "ColumnIndex", DbcFieldType::UInt32 },
    { "SpellRank1", DbcFieldType::UInt32 },
    { "SpellRank2", DbcFieldType::UInt32 },
    { "SpellRank3", DbcFieldType::UInt32 },
    { "SpellRank4", DbcFieldType::UInt32 },
    { "SpellRank5", DbcFieldType::UInt32 },
    { "SpellRank6", DbcFieldType::UInt32 },
    { "SpellRank7", DbcFieldType::UInt32 },
    { "SpellRank8", DbcFieldType::UInt32 },
    { "SpellRank9", DbcFieldType::UInt32 },
    { "PrereqTalent1", DbcFieldType::UInt32 },
    { "PrereqTalent2", DbcFieldType::UInt32 },
    { "PrereqTalent3", DbcFieldType::UInt32 },
    { "PrereqRank1", DbcFieldType::UInt32 },
    { "PrereqRank2", DbcFieldType::UInt32 },
    { "PrereqRank3", DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "RequiredSpellID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "requiredspell" },
    { "CategoryMask1", DbcFieldType::UInt32 },
    { "CategoryMask2", DbcFieldType::UInt32 },
};

static const DbcSchema schema_talent = {
    "Talent",
    talent_fields,
    sizeof(talent_fields) / sizeof(talent_fields[0])
};

#endif
