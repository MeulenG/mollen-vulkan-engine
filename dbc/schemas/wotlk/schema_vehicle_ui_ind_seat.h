#ifndef SCHEMA_VEHICLE_UI_IND_SEAT_H
#define SCHEMA_VEHICLE_UI_IND_SEAT_H

#include "dbc_schema.h"

static const DbcFieldDef vehicle_ui_ind_seat_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "VehicleUIIndicatorID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "vehicleuiindicator" },
    { "VirtualSeatIndex", DbcFieldType::UInt32 },
    { "XPos", DbcFieldType::Float },
    { "YPos", DbcFieldType::Float },
};

static const DbcSchema schema_vehicle_ui_ind_seat = {
    "VehicleUIIndSeat",
    vehicle_ui_ind_seat_fields,
    sizeof(vehicle_ui_ind_seat_fields) / sizeof(vehicle_ui_ind_seat_fields[0])
};

#endif
