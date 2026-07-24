#ifndef SCHEMA_GM_SURVEY_QUESTIONS_H
#define SCHEMA_GM_SURVEY_QUESTIONS_H

#include "dbc_schema.h"

static const DbcFieldDef gm_survey_questions_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Question_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "Question_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Question", "Identity" },
    { "QuestionFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_gm_survey_questions = {
    "GMSurveyQuestions",
    gm_survey_questions_fields,
    sizeof(gm_survey_questions_fields) / sizeof(gm_survey_questions_fields[0])
};

#endif
