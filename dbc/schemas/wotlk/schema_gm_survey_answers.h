#ifndef SCHEMA_GM_SURVEY_ANSWERS_H
#define SCHEMA_GM_SURVEY_ANSWERS_H

#include "dbc_schema.h"

static const DbcFieldDef gm_survey_answers_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "SortIndex",     DbcFieldType::UInt32 },
    { "GMSurveyQuestionID", DbcFieldType::UInt32 },
    { "Answer_enUS",   DbcFieldType::String },
    { "Answer_koKR",   DbcFieldType::String },
    { "Answer_frFR",   DbcFieldType::String },
    { "Answer_deDE",   DbcFieldType::String },
    { "Answer_enCN",   DbcFieldType::String },
    { "Answer_zhTW",   DbcFieldType::String },
    { "Answer_esES",   DbcFieldType::String },
    { "Answer_esMX",   DbcFieldType::String },
    { "Answer_ruRU",   DbcFieldType::String },
    { "Answer_jaJP",   DbcFieldType::String },
    { "Answer_ptPT",   DbcFieldType::String },
    { "Answer_itIT",   DbcFieldType::String },
    { "Answer_Unk1",   DbcFieldType::String },
    { "Answer_Unk2",   DbcFieldType::String },
    { "Answer_Unk3",   DbcFieldType::String },
    { "Answer_Unk4",   DbcFieldType::String },
    { "AnswerFlags",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_gm_survey_answers = {
    "GMSurveyAnswers",
    gm_survey_answers_fields,
    sizeof(gm_survey_answers_fields) / sizeof(gm_survey_answers_fields[0])
};

#endif
