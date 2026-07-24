#ifndef SCHEMA_ATTACK_ANIM_TYPES_H
#define SCHEMA_ATTACK_ANIM_TYPES_H

#include "dbc_schema.h"

static const DbcFieldDef attack_anim_types_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
};

static const DbcSchema schema_attack_anim_types = {
    "AttackAnimTypes",
    attack_anim_types_fields,
    sizeof(attack_anim_types_fields) / sizeof(attack_anim_types_fields[0])
};

#endif
