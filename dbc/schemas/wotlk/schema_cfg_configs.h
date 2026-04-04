#ifndef SCHEMA_CFG_CONFIGS_H
#define SCHEMA_CFG_CONFIGS_H

#include "dbc_schema.h"

static const DbcFieldDef cfg_configs_fields[] = {
    { "Id",          DbcFieldType::UInt32 },
    { "RealmType",   DbcFieldType::UInt32 },
    { "PlayerKillingAllowed", DbcFieldType::UInt32 },
    { "Roleplaying", DbcFieldType::UInt32 },
};

static const DbcSchema schema_cfg_configs = {
    "Cfg_Configs",
    cfg_configs_fields,
    sizeof(cfg_configs_fields) / sizeof(cfg_configs_fields[0])
};

#endif
