#ifndef SCHEMA_GM_SURVEY_ANSWERS_H
#define SCHEMA_GM_SURVEY_ANSWERS_H

#include "dbc_schema.h"

static const DbcFieldDef gm_survey_answers_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "SortIndex",     DbcFieldType::UInt32 },
    { "GMSurveyQuestionID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "gmsurveyquestion" },
    { "Answer_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "Answer_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Answer" },
    { "AnswerFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_gm_survey_answers = {
    "GMSurveyAnswers",
    gm_survey_answers_fields,
    sizeof(gm_survey_answers_fields) / sizeof(gm_survey_answers_fields[0])
};

#endif
