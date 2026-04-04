#ifndef SCHEMA_SOUND_FILTER_ELEM_H
#define SCHEMA_SOUND_FILTER_ELEM_H

#include "dbc_schema.h"

static const DbcFieldDef sound_filter_elem_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "SoundFilterID",   DbcFieldType::UInt32 },
    { "OrderIndex",      DbcFieldType::UInt32 },
    { "FilterType",      DbcFieldType::UInt32 },
    { "Param1",          DbcFieldType::Float  },
    { "Param2",          DbcFieldType::Float  },
    { "Param3",          DbcFieldType::Float  },
    { "Param4",          DbcFieldType::Float  },
    { "Param5",          DbcFieldType::Float  },
    { "Param6",          DbcFieldType::Float  },
    { "Param7",          DbcFieldType::Float  },
    { "Param8",          DbcFieldType::Float  },
    { "Param9",          DbcFieldType::Float  },
};

static const DbcSchema schema_sound_filter_elem = {
    "SoundFilterElem",
    sound_filter_elem_fields,
    sizeof(sound_filter_elem_fields) / sizeof(sound_filter_elem_fields[0])
};

#endif
