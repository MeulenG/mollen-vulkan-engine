#ifndef SCHEMA_GM_SURVEY_CURRENT_SURVEY_H
#define SCHEMA_GM_SURVEY_CURRENT_SURVEY_H

#include "dbc_schema.h"

static const DbcFieldDef gm_survey_current_survey_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "GMSurveySurveyID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "gmsurveysurvey" },
};

static const DbcSchema schema_gm_survey_current_survey = {
    "GMSurveyCurrentSurvey",
    gm_survey_current_survey_fields,
    sizeof(gm_survey_current_survey_fields) / sizeof(gm_survey_current_survey_fields[0])
};

#endif
