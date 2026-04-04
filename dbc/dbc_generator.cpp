#include "dbc_generator.h"
#include <cstdio>
#include <cctype>
#include <cstring>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;

static std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

static std::string ToPascalCase(const std::string& str) {
    std::string result;
    bool next_upper = true;
    for (char c : str) {
        if (c == '_') {
            next_upper = true;
        } else if (next_upper) {
            result += (char)toupper(c);
            next_upper = false;
        } else {
            result += c;
        }
    }
    return result;
}

static std::string ToUpperGuard(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (isalnum(c)) {
            result += (char)toupper(c);
        } else {
            result += '_';
        }
    }
    return result;
}

static std::string CppTypeName(DbcFieldType type) {
    switch (type) {
    case DbcFieldType::UInt32:
        return "uint32_t";
    case DbcFieldType::Int32:
        return "int32_t";
    case DbcFieldType::Float:
        return "float";
    case DbcFieldType::String:
        return "const char*";
    case DbcFieldType::UInt8:
        return "uint8_t";
    case DbcFieldType::Int8:
        return "int8_t";
    case DbcFieldType::UInt16:
        return "uint16_t";
    case DbcFieldType::Int16:
        return "int16_t";
    }
    return "uint32_t";
}

static bool IsPadding(const char* name) {
    return name && name[0] == '_' && strncmp(name, "_pad", 4) == 0;
}

static std::string EscapeString(const char* str) {
    std::string result;
    if (!str) {
        return "";
    }
    for (const char* p = str; *p; ++p) {
        switch (*p) {
        case '\\':
            result += "\\\\";
            break;
        case '"':
            result += "\\\"";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            if ((unsigned char)*p < 0x20) {
                char buf[8];
                snprintf(buf, sizeof(buf), "\\x%02x", (unsigned char)*p);
                result += buf;
            } else {
                result += *p;
            }
            break;
        }
    }
    return result;
}

static void WriteFieldValue(FILE* out, const DbcFile& dbc, const DbcSchema* schema,
                            uint32_t record, uint32_t field) {
    if (schema->packed) {
        uint32_t offset = DbcFile::GetFieldOffset(schema, field);
        switch (schema->fields[field].type) {
        case DbcFieldType::UInt8:
            fprintf(out, "%u", dbc.GetUInt8At(record, offset));
            break;
        case DbcFieldType::Int8:
            fprintf(out, "%d", dbc.GetInt8At(record, offset));
            break;
        case DbcFieldType::UInt16:
            fprintf(out, "%u", dbc.GetUInt16At(record, offset));
            break;
        case DbcFieldType::Int16:
            fprintf(out, "%d", dbc.GetInt16At(record, offset));
            break;
        case DbcFieldType::UInt32:
            fprintf(out, "%u", dbc.GetUInt32At(record, offset));
            break;
        case DbcFieldType::Int32:
            fprintf(out, "%d", dbc.GetInt32At(record, offset));
            break;
        case DbcFieldType::Float:
            fprintf(out, "%.6ff", dbc.GetFloatAt(record, offset));
            break;
        case DbcFieldType::String:
            fprintf(out, "\"%s\"", EscapeString(dbc.GetStringAt(record, offset)).c_str());
            break;
        }
    } else {
        switch (schema->fields[field].type) {
        case DbcFieldType::UInt32:
            fprintf(out, "%u", dbc.GetUInt32(record, field));
            break;
        case DbcFieldType::Int32:
            fprintf(out, "%d", dbc.GetInt32(record, field));
            break;
        case DbcFieldType::Float: {
            float val = dbc.GetFloat(record, field);
            fprintf(out, "%.6ff", val);
            break;
        }
        case DbcFieldType::String:
            fprintf(out, "\"%s\"", EscapeString(dbc.GetStringField(record, field)).c_str());
            break;
        default:
            fprintf(out, "0");
            break;
        }
    }
}

bool GenerateHeader(const DbcFile& dbc, const DbcSchema* schema,
                    const fs::path& output_dir, bool quiet) {
    if (!schema) {
        return false;
    }

    if (schema->packed) {
        uint32_t expected = GetSchemaRecordSize(schema);
        if (expected != dbc.GetRecordSize()) {
            if (!quiet) {
                printf("  Skipping %s: schema record size %u, DBC record size %u\n",
                       schema->dbc_name, expected, dbc.GetRecordSize());
            }
            return false;
        }
    } else {
        if (schema->field_count != dbc.GetFieldCount()) {
            if (!quiet) {
                printf("  Skipping %s: schema has %u fields, DBC has %u\n",
                       schema->dbc_name, schema->field_count, dbc.GetFieldCount());
            }
            return false;
        }
    }

    std::string name_lower = ToLower(schema->dbc_name);
    std::string name_pascal = ToPascalCase(schema->dbc_name);
    std::string guard = ToUpperGuard(schema->dbc_name) + "_GENERATED_H";
    std::string struct_name = name_pascal + "Entry";
    std::string filename = name_lower + ".h";

    fs::path output_path = output_dir / filename;
    fs::create_directories(output_dir);

    FILE* out = fopen(output_path.string().c_str(), "w");
    if (!out) {
        printf("  Failed to open output file: %s\n", output_path.string().c_str());
        return false;
    }

    fprintf(out, "#ifndef %s\n", guard.c_str());
    fprintf(out, "#define %s\n\n", guard.c_str());
    fprintf(out, "#include <cstdint>\n");
    fprintf(out, "#include <unordered_map>\n\n");

    fprintf(out, "struct %s {\n", struct_name.c_str());
    for (uint32_t f = 0; f < schema->field_count; f++) {
        if (IsPadding(schema->fields[f].name)) {
            continue;
        }
        fprintf(out, "    %s %s;\n",
                CppTypeName(schema->fields[f].type).c_str(),
                schema->fields[f].name);
    }
    fprintf(out, "};\n\n");

    fprintf(out, "static const %s %s_data[] = {\n", struct_name.c_str(), name_lower.c_str());
    for (uint32_t r = 0; r < dbc.GetRecordCount(); r++) {
        fprintf(out, "    { ");
        bool first = true;
        for (uint32_t f = 0; f < schema->field_count; f++) {
            if (IsPadding(schema->fields[f].name)) {
                continue;
            }
            if (!first) {
                fprintf(out, ", ");
            }
            first = false;
            WriteFieldValue(out, dbc, schema, r, f);
        }
        fprintf(out, " },\n");
    }
    fprintf(out, "};\n\n");

    bool has_id = false;
    for (uint32_t f = 0; f < schema->field_count; f++) {
        if (!IsPadding(schema->fields[f].name)) {
            has_id = (strcmp(schema->fields[f].name, "Id") == 0);
            break;
        }
    }

    if (has_id) {
        fprintf(out, "static const std::unordered_map<uint32_t, const %s*>& Get%sMap() {\n",
                struct_name.c_str(), name_pascal.c_str());
        fprintf(out, "    static std::unordered_map<uint32_t, const %s*> map;\n", struct_name.c_str());
        fprintf(out, "    if (map.empty()) {\n");
        fprintf(out, "        for (const auto& entry : %s_data) {\n", name_lower.c_str());
        fprintf(out, "            map[entry.Id] = &entry;\n");
        fprintf(out, "        }\n");
        fprintf(out, "    }\n");
        fprintf(out, "    return map;\n");
        fprintf(out, "}\n\n");

        fprintf(out, "static const %s* Get%sEntry(uint32_t id) {\n",
                struct_name.c_str(), name_pascal.c_str());
        fprintf(out, "    const auto& map = Get%sMap();\n", name_pascal.c_str());
        fprintf(out, "    auto it = map.find(id);\n");
        fprintf(out, "    if (it != map.end()) {\n");
        fprintf(out, "        return it->second;\n");
        fprintf(out, "    }\n");
        fprintf(out, "    return nullptr;\n");
        fprintf(out, "}\n\n");
    }

    fprintf(out, "#endif // %s\n", guard.c_str());

    fclose(out);

    if (!quiet) {
        printf("  Generated %s (%u records)\n", output_path.string().c_str(), dbc.GetRecordCount());
    }
    return true;
}
