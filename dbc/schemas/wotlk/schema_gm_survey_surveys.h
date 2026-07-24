#ifndef SCHEMA_GM_SURVEY_SURVEYS_H
#define SCHEMA_GM_SURVEY_SURVEYS_H

#include "dbc_schema.h"

static const DbcFieldDef gm_survey_surveys_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Q1", DbcFieldType::UInt32 },
    { "Q2", DbcFieldType::UInt32 },
    { "Q3", DbcFieldType::UInt32 },
    { "Q4", DbcFieldType::UInt32 },
    { "Q5", DbcFieldType::UInt32 },
    { "Q6", DbcFieldType::UInt32 },
    { "Q7", DbcFieldType::UInt32 },
    { "Q8", DbcFieldType::UInt32 },
    { "Q9", DbcFieldType::UInt32 },
    { "Q10", DbcFieldType::UInt32 },
};

static const DbcSchema schema_gm_survey_surveys = {
    "GMSurveySurveys",
    gm_survey_surveys_fields,
    sizeof(gm_survey_surveys_fields) / sizeof(gm_survey_surveys_fields[0])
};

#endif
