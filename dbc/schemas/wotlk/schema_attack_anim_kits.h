#ifndef SCHEMA_ATTACK_ANIM_KITS_H
#define SCHEMA_ATTACK_ANIM_KITS_H

#include "dbc_schema.h"

static const DbcFieldDef attack_anim_kits_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "Animation",      DbcFieldType::UInt32 },
    { "Type",           DbcFieldType::UInt32 },
    { "Flags",          DbcFieldType::UInt32 },
    { "Unknown",        DbcFieldType::UInt32 },
};

static const DbcSchema schema_attack_anim_kits = {
    "AttackAnimKits",
    attack_anim_kits_fields,
    sizeof(attack_anim_kits_fields) / sizeof(attack_anim_kits_fields[0])
};

#endif
