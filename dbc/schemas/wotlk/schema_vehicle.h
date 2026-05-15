#ifndef SCHEMA_VEHICLE_H
#define SCHEMA_VEHICLE_H

#include "dbc_schema.h"

static const DbcFieldDef vehicle_fields[] = {
    { "Id",                    DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "TurnSpeed",             DbcFieldType::Float  },
    { "PitchSpeed",            DbcFieldType::Float  },
    { "PitchMin",              DbcFieldType::Float  },
    { "PitchMax",              DbcFieldType::Float  },
    { "SeatID1",               DbcFieldType::UInt32 },
    { "SeatID2",               DbcFieldType::UInt32 },
    { "SeatID3",               DbcFieldType::UInt32 },
    { "SeatID4",               DbcFieldType::UInt32 },
    { "SeatID5",               DbcFieldType::UInt32 },
    { "SeatID6",               DbcFieldType::UInt32 },
    { "SeatID7",               DbcFieldType::UInt32 },
    { "SeatID8",               DbcFieldType::UInt32 },
    { "MouseLookOffsetPitch",  DbcFieldType::Float  },
    { "CameraFadeDistScalarMin", DbcFieldType::Float },
    { "CameraFadeDistScalarMax", DbcFieldType::Float },
    { "CameraPitchOffset",     DbcFieldType::Float  },
    { "FacingLimitRight",      DbcFieldType::Float  },
    { "FacingLimitLeft",       DbcFieldType::Float  },
    { "MsslTrgtTurnLingering", DbcFieldType::Float  },
    { "MsslTrgtPitchLingering", DbcFieldType::Float },
    { "MsslTrgtMouseLingering", DbcFieldType::Float },
    { "MsslTrgtEndOpacity",   DbcFieldType::Float  },
    { "MsslTrgtArcSpeed",     DbcFieldType::Float  },
    { "MsslTrgtArcRepeat",    DbcFieldType::Float  },
    { "MsslTrgtArcWidth",     DbcFieldType::Float  },
    { "MsslTrgtImpactRadius1", DbcFieldType::Float },
    { "MsslTrgtImpactRadius2", DbcFieldType::Float },
    { "MsslTrgtArcTexture",   DbcFieldType::String },
    { "MsslTrgtImpactTexture", DbcFieldType::String },
    { "MsslTrgtImpactModel1", DbcFieldType::String },
    { "MsslTrgtImpactModel2", DbcFieldType::String },
    { "CameraYawOffset",      DbcFieldType::Float  },
    { "UiLocomotionType",     DbcFieldType::UInt32 },
    { "MsslTrgtImpactTexRadius", DbcFieldType::Float },
    { "VehicleUIIndicatorID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "vehicleuiindicator" },
    { "PowerDisplayID1",      DbcFieldType::UInt32 },
    { "PowerDisplayID2",      DbcFieldType::UInt32 },
    { "PowerDisplayID3",      DbcFieldType::UInt32 },
};

static const DbcSchema schema_vehicle = {
    "Vehicle",
    vehicle_fields,
    sizeof(vehicle_fields) / sizeof(vehicle_fields[0])
};

#endif
