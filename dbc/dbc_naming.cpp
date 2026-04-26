#include "dbc_naming.h"

#include <cctype>

std::string DbcTableName(const char* schema_name) {
    std::string result;
    if (!schema_name) return result;
    for (const char* p = schema_name; *p; ++p) {
        result += static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
    }
    return result;
}

std::string DbcColumnName(const char* field_name) {
    std::string result;
    if (!field_name) return result;
    for (const char* p = field_name; *p; ++p) {
        unsigned char c = static_cast<unsigned char>(*p);
        if (std::isupper(c) && !result.empty() && result.back() != '_') {
            result += '_';
        }
        result += static_cast<char>(std::tolower(c));
    }
    return result;
}
