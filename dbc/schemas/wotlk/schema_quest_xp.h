#ifndef SCHEMA_QUEST_XP_H
#define SCHEMA_QUEST_XP_H

#include "dbc_schema.h"

static const DbcFieldDef quest_xp_fields[] = {
    { "Id",        DbcFieldType::UInt32 },
    { "Diff1",     DbcFieldType::UInt32 },
    { "Diff2",     DbcFieldType::UInt32 },
    { "Diff3",     DbcFieldType::UInt32 },
    { "Diff4",     DbcFieldType::UInt32 },
    { "Diff5",     DbcFieldType::UInt32 },
    { "Diff6",     DbcFieldType::UInt32 },
    { "Diff7",     DbcFieldType::UInt32 },
    { "Diff8",     DbcFieldType::UInt32 },
    { "Diff9",     DbcFieldType::UInt32 },
    { "Diff10",    DbcFieldType::UInt32 },
};

static const DbcSchema schema_quest_xp = {
    "QuestXP",
    quest_xp_fields,
    sizeof(quest_xp_fields) / sizeof(quest_xp_fields[0])
};

#endif
