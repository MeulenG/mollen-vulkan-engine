#ifndef SCHEMA_REGISTRY_H
#define SCHEMA_REGISTRY_H

#include "dbc_schema.h"

const DbcSchema* GetSchema(Expansion expansion, const char* dbc_name);
const DbcSchema* GetAllSchemas(Expansion expansion, uint32_t* count);

const DbcSchema* GetSchema(const char* dbc_name);
const DbcSchema* GetAllSchemas(uint32_t* count);

#endif // SCHEMA_REGISTRY_H
