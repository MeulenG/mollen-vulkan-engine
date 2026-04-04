#include "dbc_file.h"
#include "dbc_schema.h"
#include <cstring>

DbcFile::DbcFile()
    : header_{}, record_data_(nullptr), string_block_(nullptr), data_size_(0) {
}

bool DbcFile::Load(const uint8_t* data, uint32_t size) {
    if (size < sizeof(dbc_header)) {
        return false;
    }

    memcpy(&header_, data, sizeof(dbc_header));

    if (header_.magic != DBC_MAGIC) {
        return false;
    }

    uint32_t expected_size = sizeof(dbc_header)
                           + (header_.record_count * header_.record_size)
                           + header_.string_block_size;
    if (size < expected_size) {
        return false;
    }

    record_data_ = data + sizeof(dbc_header);
    string_block_ = reinterpret_cast<const char*>(record_data_
                  + (header_.record_count * header_.record_size));
    data_size_ = size;

    return true;
}

uint32_t DbcFile::GetRecordCount() const {
    return header_.record_count;
}

uint32_t DbcFile::GetFieldCount() const {
    return header_.field_count;
}

uint32_t DbcFile::GetRecordSize() const {
    return header_.record_size;
}

uint32_t DbcFile::GetStringBlockSize() const {
    return header_.string_block_size;
}

const uint8_t* DbcFile::GetRecord(uint32_t index) const {
    if (index >= header_.record_count) {
        return nullptr;
    }
    return record_data_ + (index * header_.record_size);
}

const char* DbcFile::GetString(uint32_t offset) const {
    if (offset >= header_.string_block_size) {
        return "";
    }
    return string_block_ + offset;
}

uint32_t DbcFile::GetUInt32(uint32_t record, uint32_t field) const {
    const uint8_t* row = GetRecord(record);
    if (!row || field >= header_.field_count) {
        return 0;
    }
    uint32_t value;
    memcpy(&value, row + (field * sizeof(uint32_t)), sizeof(uint32_t));
    return value;
}

int32_t DbcFile::GetInt32(uint32_t record, uint32_t field) const {
    const uint8_t* row = GetRecord(record);
    if (!row || field >= header_.field_count) {
        return 0;
    }
    int32_t value;
    memcpy(&value, row + (field * sizeof(uint32_t)), sizeof(int32_t));
    return value;
}

float DbcFile::GetFloat(uint32_t record, uint32_t field) const {
    const uint8_t* row = GetRecord(record);
    if (!row || field >= header_.field_count) {
        return 0.0f;
    }
    float value;
    memcpy(&value, row + (field * sizeof(uint32_t)), sizeof(float));
    return value;
}

const char* DbcFile::GetStringField(uint32_t record, uint32_t field) const {
    uint32_t offset = GetUInt32(record, field);
    return GetString(offset);
}

uint8_t DbcFile::GetUInt8At(uint32_t record, uint32_t byte_offset) const {
    const uint8_t* row = GetRecord(record);
    if (!row || byte_offset >= header_.record_size) {
        return 0;
    }
    return row[byte_offset];
}

int8_t DbcFile::GetInt8At(uint32_t record, uint32_t byte_offset) const {
    return static_cast<int8_t>(GetUInt8At(record, byte_offset));
}

uint16_t DbcFile::GetUInt16At(uint32_t record, uint32_t byte_offset) const {
    const uint8_t* row = GetRecord(record);
    if (!row || byte_offset + 1 >= header_.record_size) {
        return 0;
    }
    uint16_t value;
    memcpy(&value, row + byte_offset, sizeof(uint16_t));
    return value;
}

int16_t DbcFile::GetInt16At(uint32_t record, uint32_t byte_offset) const {
    return static_cast<int16_t>(GetUInt16At(record, byte_offset));
}

uint32_t DbcFile::GetUInt32At(uint32_t record, uint32_t byte_offset) const {
    const uint8_t* row = GetRecord(record);
    if (!row || byte_offset + 3 >= header_.record_size) {
        return 0;
    }
    uint32_t value;
    memcpy(&value, row + byte_offset, sizeof(uint32_t));
    return value;
}

int32_t DbcFile::GetInt32At(uint32_t record, uint32_t byte_offset) const {
    return static_cast<int32_t>(GetUInt32At(record, byte_offset));
}

float DbcFile::GetFloatAt(uint32_t record, uint32_t byte_offset) const {
    const uint8_t* row = GetRecord(record);
    if (!row || byte_offset + 3 >= header_.record_size) {
        return 0.0f;
    }
    float value;
    memcpy(&value, row + byte_offset, sizeof(float));
    return value;
}

const char* DbcFile::GetStringAt(uint32_t record, uint32_t byte_offset) const {
    uint32_t offset = GetUInt32At(record, byte_offset);
    return GetString(offset);
}

uint32_t DbcFile::GetFieldOffset(const DbcSchema* schema, uint32_t field) {
    if (!schema || field >= schema->field_count) {
        return 0;
    }
    uint32_t offset = 0;
    for (uint32_t i = 0; i < field; i++) {
        offset += GetFieldTypeSize(schema->fields[i].type);
    }
    return offset;
}
