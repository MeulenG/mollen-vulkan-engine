#ifndef DBC_WRITER_H
#define DBC_WRITER_H

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "dbc_file.h"
#include "dbc_schema.h"

class DbcWriter {
public:
    DbcWriter();

    // Initialize empty writer from a schema
    void Init(const DbcSchema* schema);

    // Import all records from an existing read-only DBC
    bool ImportFrom(const DbcFile& dbc, const DbcSchema* schema);

    uint32_t AddRecord();
    bool RemoveRecord(uint32_t index);
    uint32_t GetRecordCount() const;

    // Field setters
    void SetUInt32(uint32_t record, uint32_t field, uint32_t value);
    void SetInt32(uint32_t record, uint32_t field, int32_t value);
    void SetFloat(uint32_t record, uint32_t field, float value);
    void SetString(uint32_t record, uint32_t field, const char* value);
    void SetUInt8(uint32_t record, uint32_t field, uint8_t value);
    void SetInt8(uint32_t record, uint32_t field, int8_t value);
    void SetUInt16(uint32_t record, uint32_t field, uint16_t value);
    void SetInt16(uint32_t record, uint32_t field, int16_t value);

    // Field getters
    uint32_t GetUInt32(uint32_t record, uint32_t field) const;
    int32_t GetInt32(uint32_t record, uint32_t field) const;
    float GetFloat(uint32_t record, uint32_t field) const;
    const char* GetString(uint32_t record, uint32_t field) const;
    uint8_t GetUInt8(uint32_t record, uint32_t field) const;
    int8_t GetInt8(uint32_t record, uint32_t field) const;
    uint16_t GetUInt16(uint32_t record, uint32_t field) const;
    int16_t GetInt16(uint32_t record, uint32_t field) const;

    // Serialize to file
    bool Save(const std::filesystem::path& path) const;

    // Serialize to memory
    std::vector<uint8_t> Serialize() const;

private:
    const DbcSchema* schema_;
    uint32_t record_size_;
    std::vector<std::vector<uint8_t>> records_;
    std::vector<std::string> strings_;
    std::unordered_map<std::string, uint32_t> string_dedup_;

    uint32_t GetFieldOffset(uint32_t field) const;
    uint32_t AddString(const char* str);
    std::pair<std::vector<uint8_t>, std::unordered_map<uint32_t, uint32_t>> BuildStringBlock() const;
};

#endif // DBC_WRITER_H
