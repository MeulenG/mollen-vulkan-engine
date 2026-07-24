#ifndef SCHEMA_PAPER_DOLL_ITEM_FRAME_H
#define SCHEMA_PAPER_DOLL_ITEM_FRAME_H

#include "dbc_schema.h"

static const DbcFieldDef paper_doll_item_frame_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SlotIcon", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "SlotNumber", DbcFieldType::UInt32 },
};

static const DbcSchema schema_paper_doll_item_frame = {
    "PaperDollItemFrame",
    paper_doll_item_frame_fields,
    sizeof(paper_doll_item_frame_fields) / sizeof(paper_doll_item_frame_fields[0])
};

#endif
