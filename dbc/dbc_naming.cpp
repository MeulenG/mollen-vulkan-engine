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

std::string DbcPrettyName(const char* schema_name) {
    std::string result;
    if (!schema_name) return result;

    const char* s = schema_name;
    size_t n = 0;
    while (s[n]) ++n;

    for (size_t i = 0; i < n; i++) {
        char c = s[i];
        if (c == '_') {
            if (!result.empty() && result.back() != ' ') result += ' ';
            continue;
        }
        if (i > 0) {
            char prev = s[i - 1];
            char next = (i + 1 < n) ? s[i + 1] : 0;
            // Boundary rules:
            //  1. lowercase -> uppercase  ("ItemSet"  -> "Item Set")
            //  2. end of acronym: upper, upper, lower  ("GMSurvey" -> "GM Survey")
            bool boundary =
                (std::islower(static_cast<unsigned char>(prev)) &&
                 std::isupper(static_cast<unsigned char>(c))) ||
                (std::isupper(static_cast<unsigned char>(prev)) &&
                 std::isupper(static_cast<unsigned char>(c)) &&
                 next && std::islower(static_cast<unsigned char>(next)));
            if (boundary && !result.empty() && result.back() != ' ') {
                result += ' ';
            }
        }
        result += c;
    }
    return result;
}
