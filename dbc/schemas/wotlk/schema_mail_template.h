#ifndef SCHEMA_MAIL_TEMPLATE_H
#define SCHEMA_MAIL_TEMPLATE_H

#include "dbc_schema.h"

static const DbcFieldDef mail_template_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Subject_enUS",    DbcFieldType::String },
    { "Subject_koKR",    DbcFieldType::String },
    { "Subject_frFR",    DbcFieldType::String },
    { "Subject_deDE",    DbcFieldType::String },
    { "Subject_enCN",    DbcFieldType::String },
    { "Subject_zhTW",    DbcFieldType::String },
    { "Subject_esES",    DbcFieldType::String },
    { "Subject_esMX",    DbcFieldType::String },
    { "Subject_ruRU",    DbcFieldType::String },
    { "Subject_jaJP",    DbcFieldType::String },
    { "Subject_ptPT",    DbcFieldType::String },
    { "Subject_itIT",    DbcFieldType::String },
    { "Subject_Unk1",    DbcFieldType::String },
    { "Subject_Unk2",    DbcFieldType::String },
    { "Subject_Unk3",    DbcFieldType::String },
    { "Subject_Unk4",    DbcFieldType::String },
    { "SubjectFlags",    DbcFieldType::UInt32 },
    { "Body_enUS",       DbcFieldType::String },
    { "Body_koKR",       DbcFieldType::String },
    { "Body_frFR",       DbcFieldType::String },
    { "Body_deDE",       DbcFieldType::String },
    { "Body_enCN",       DbcFieldType::String },
    { "Body_zhTW",       DbcFieldType::String },
    { "Body_esES",       DbcFieldType::String },
    { "Body_esMX",       DbcFieldType::String },
    { "Body_ruRU",       DbcFieldType::String },
    { "Body_jaJP",       DbcFieldType::String },
    { "Body_ptPT",       DbcFieldType::String },
    { "Body_itIT",       DbcFieldType::String },
    { "Body_Unk1",       DbcFieldType::String },
    { "Body_Unk2",       DbcFieldType::String },
    { "Body_Unk3",       DbcFieldType::String },
    { "Body_Unk4",       DbcFieldType::String },
    { "BodyFlags",       DbcFieldType::UInt32 },
};

static const DbcSchema schema_mail_template = {
    "MailTemplate",
    mail_template_fields,
    sizeof(mail_template_fields) / sizeof(mail_template_fields[0])
};

#endif
