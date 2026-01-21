#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <variant>
#include <vector>

struct SettingBool
{
    int32_t value;

    std::variant<bool, int32_t> resolve() const
    {
        if (value == 0)
        {
            return false;
        }
        if (value == 1)
        {
            return true;
        }
        return value;
    }
};

using SettingValue = std::variant<std::nullptr_t, SettingBool, double, std::string>;

struct ModSettingRecord
{
    std::string name;
    SettingValue value;
    SettingValue value_next;
};

struct ModSettingsResult
{
    uint32_t identifier;
    uint32_t record_count;
    std::vector<ModSettingRecord> settings;
};

enum class AppError
{
    IOError,
    EmptyInput,
    HeaderTooShort,
    SizeMismatch,
    DecompressionFailed,
    UncompressedDataReadError,
    ParseError_BufferOverflow,
    ParseError_UnknownType,
};

inline std::string error_to_string(AppError err)
{
    switch (err)
    {
    case AppError::IOError:
        return "Failed to read input file.";
    case AppError::EmptyInput:
        return "Input data is empty.";
    case AppError::HeaderTooShort:
        return "Input data is too short to contain a valid header.";
    case AppError::SizeMismatch:
        return "Decompressed size mismatch.";
    case AppError::DecompressionFailed:
        return "FastLZ decompression failed.";
    case AppError::UncompressedDataReadError:
        return "Failed to read uncompressed data block.";
    case AppError::ParseError_BufferOverflow:
        return "Parser error: Unexpected end of buffer.";
    case AppError::ParseError_UnknownType:
        return "Parser error: Unknown variant type tag.";
    default:
        return "Unknown error.";
    }
}