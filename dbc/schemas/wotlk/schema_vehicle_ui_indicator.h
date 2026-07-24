#ifndef SCHEMA_VEHICLE_UI_INDICATOR_H
#define SCHEMA_VEHICLE_UI_INDICATOR_H

#include "dbc_schema.h"

static const DbcFieldDef vehicle_ui_indicator_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "BackgroundTex", DbcFieldType::String },
};

static const DbcSchema schema_vehicle_ui_indicator = {
    "VehicleUIIndicator",
    vehicle_ui_indicator_fields,
    sizeof(vehicle_ui_indicator_fields) / sizeof(vehicle_ui_indicator_fields[0])
};

#endif
