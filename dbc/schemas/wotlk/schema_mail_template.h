#ifndef SCHEMA_MAIL_TEMPLATE_H
#define SCHEMA_MAIL_TEMPLATE_H

#include "dbc_schema.h"

static const DbcFieldDef mail_template_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Subject_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "Subject_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Subject", "Identity" },
    { "SubjectFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "Body_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "Body_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Body", "Identity" },
    { "BodyFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_mail_template = {
    "MailTemplate",
    mail_template_fields,
    sizeof(mail_template_fields) / sizeof(mail_template_fields[0])
};

#endif
