#ifndef SCHEMA_TEAM_CONTRIBUTION_POINTS_H
#define SCHEMA_TEAM_CONTRIBUTION_POINTS_H

#include "dbc_schema.h"

static const DbcFieldDef team_contribution_points_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Data", DbcFieldType::Float },
};

static const DbcSchema schema_team_contribution_points = {
    "TeamContributionPoints",
    team_contribution_points_fields,
    sizeof(team_contribution_points_fields) / sizeof(team_contribution_points_fields[0])
};

#endif
