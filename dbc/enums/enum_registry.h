#ifndef ENUM_REGISTRY_H
#define ENUM_REGISTRY_H

#include "dbc_enum.h"

// Look up a registered enum (or bitmask) table by its public name. Returns
// null if the name is unknown - callers should treat that as "fall back to
// raw integer rendering".
const DbcEnum* GetDbcEnum(const char* name);

// Iterate all registered enums. Useful for tests and debug dumps.
const DbcEnum* const* GetAllDbcEnums(uint32_t* count);

#endif // ENUM_REGISTRY_H
