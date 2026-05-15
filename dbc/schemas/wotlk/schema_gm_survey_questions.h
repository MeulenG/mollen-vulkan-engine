#ifndef SCHEMA_GM_SURVEY_QUESTIONS_H
#define SCHEMA_GM_SURVEY_QUESTIONS_H

#include "dbc_schema.h"

static const DbcFieldDef gm_survey_questions_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Question_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "Question_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Question" },
    { "QuestionFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_gm_survey_questions = {
    "GMSurveyQuestions",
    gm_survey_questions_fields,
    sizeof(gm_survey_questions_fields) / sizeof(gm_survey_questions_fields[0])
};

#endif
