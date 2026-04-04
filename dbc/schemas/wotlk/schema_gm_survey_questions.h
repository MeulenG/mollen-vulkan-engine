#ifndef SCHEMA_GM_SURVEY_QUESTIONS_H
#define SCHEMA_GM_SURVEY_QUESTIONS_H

#include "dbc_schema.h"

static const DbcFieldDef gm_survey_questions_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Question_enUS",   DbcFieldType::String },
    { "Question_koKR",   DbcFieldType::String },
    { "Question_frFR",   DbcFieldType::String },
    { "Question_deDE",   DbcFieldType::String },
    { "Question_enCN",   DbcFieldType::String },
    { "Question_zhTW",   DbcFieldType::String },
    { "Question_esES",   DbcFieldType::String },
    { "Question_esMX",   DbcFieldType::String },
    { "Question_ruRU",   DbcFieldType::String },
    { "Question_jaJP",   DbcFieldType::String },
    { "Question_ptPT",   DbcFieldType::String },
    { "Question_itIT",   DbcFieldType::String },
    { "Question_Unk1",   DbcFieldType::String },
    { "Question_Unk2",   DbcFieldType::String },
    { "Question_Unk3",   DbcFieldType::String },
    { "Question_Unk4",   DbcFieldType::String },
    { "QuestionFlags",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_gm_survey_questions = {
    "GMSurveyQuestions",
    gm_survey_questions_fields,
    sizeof(gm_survey_questions_fields) / sizeof(gm_survey_questions_fields[0])
};

#endif
