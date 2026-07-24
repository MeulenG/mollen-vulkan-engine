#ifndef SCHEMA_MAIL_TEMPLATE_H
#define SCHEMA_MAIL_TEMPLATE_H

#include "dbc_schema.h"

static const DbcFieldDef mail_template_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Subject_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "Subject_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject" },
    { "SubjectFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "Body_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "Body_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Body" },
    { "BodyFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_mail_template = {
    "MailTemplate",
    mail_template_fields,
    sizeof(mail_template_fields) / sizeof(mail_template_fields[0])
};

#endif
