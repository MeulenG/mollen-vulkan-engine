#ifndef SCHEMA_QUEST_FACTION_REWARD_H
#define SCHEMA_QUEST_FACTION_REWARD_H

#include "dbc_schema.h"

static const DbcFieldDef quest_faction_reward_fields[] = {
    { "Id",        DbcFieldType::UInt32 },
    { "Value1",    DbcFieldType::Int32  },
    { "Value2",    DbcFieldType::Int32  },
    { "Value3",    DbcFieldType::Int32  },
    { "Value4",    DbcFieldType::Int32  },
    { "Value5",    DbcFieldType::Int32  },
    { "Value6",    DbcFieldType::Int32  },
    { "Value7",    DbcFieldType::Int32  },
    { "Value8",    DbcFieldType::Int32  },
    { "Value9",    DbcFieldType::Int32  },
    { "Value10",   DbcFieldType::Int32  },
};

static const DbcSchema schema_quest_faction_reward = {
    "QuestFactionReward",
    quest_faction_reward_fields,
    sizeof(quest_faction_reward_fields) / sizeof(quest_faction_reward_fields[0])
};

#endif
