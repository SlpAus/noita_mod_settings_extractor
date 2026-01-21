#include "parser.h"

#include <bit>
#include <cstring>
#include <expected>
#include <span>

namespace
{
    std::expected<uint32_t, AppError> read_big_endian_u32(const unsigned char *&ptr, const unsigned char *end_ptr)
    {
        if (ptr + 4 > end_ptr)
        {
            return std::unexpected(AppError::ParseError_BufferOverflow);
        }
        uint32_t value_be;
        std::memcpy(&value_be, ptr, 4);
        ptr += 4;
        return std::byteswap(value_be);
    }

    std::expected<double, AppError> read_big_endian_double(const unsigned char *&ptr, const unsigned char *end_ptr)
    {
        if (ptr + 8 > end_ptr)
        {
            return std::unexpected(AppError::ParseError_BufferOverflow);
        }
        uint32_t dword1_be, dword2_be;
        std::memcpy(&dword1_be, ptr, 4);
        std::memcpy(&dword2_be, ptr + 4, 4);
        uint32_t swapped_dword1 = std::byteswap(dword1_be);
        uint32_t swapped_dword2 = std::byteswap(dword2_be);

        uint64_t final_le_bits = (static_cast<uint64_t>(swapped_dword1) << 32) | swapped_dword2;
        double value;
        std::memcpy(&value, &final_le_bits, 8);
        ptr += 8;
        return value;
    }

    std::expected<std::string, AppError> parse_string(const unsigned char *&ptr, const unsigned char *end_ptr)
    {
        auto len_res = read_big_endian_u32(ptr, end_ptr);
        if (!len_res)
        {
            return std::unexpected(len_res.error());
        }

        uint32_t len = *len_res;

        if (ptr + len > end_ptr)
        {
            return std::unexpected(AppError::ParseError_BufferOverflow);
        }

        std::string value(reinterpret_cast<const char *>(ptr), len);
        ptr += len;
        return value;
    }

    std::expected<SettingValue, AppError> parse_value_by_type(uint32_t type_tag, const unsigned char *&ptr, const unsigned char *end_ptr)
    {
        switch (type_tag)
        {
        case 0: // nil
            return nullptr;
        case 1: // bool
        {
            auto res = read_big_endian_u32(ptr, end_ptr);
            if (!res)
            {
                return std::unexpected(res.error());
            }
            return SettingBool{ static_cast<int32_t>(*res) };
        }
        case 2: // number
            return read_big_endian_double(ptr, end_ptr);
        case 3: // string
            return parse_string(ptr, end_ptr);
        default:
            return std::unexpected(AppError::ParseError_UnknownType);
        }
    }
} // namespace

std::expected<ModSettingsResult, AppError> parse_decompressed_data(std::span<const char> data)
{
    ModSettingsResult result;

    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data.data());
    const unsigned char *end_ptr = ptr + data.size();

    auto id_res = read_big_endian_u32(ptr, end_ptr);
    if (!id_res)
    {
        return std::unexpected(id_res.error());
    }
    result.identifier = *id_res;

    auto count_res = read_big_endian_u32(ptr, end_ptr);
    if (!count_res)
    {
        return std::unexpected(count_res.error());
    }
    result.record_count = *count_res;

    result.settings.reserve(result.record_count);

    for (uint32_t i = 0; i < result.record_count; ++i)
    {
        ModSettingRecord record;

        auto name_res = parse_string(ptr, end_ptr);
        if (!name_res)
        {
            return std::unexpected(name_res.error());
        }
        record.name = *name_res;

        auto type_res = read_big_endian_u32(ptr, end_ptr);
        if (!type_res)
        {
            return std::unexpected(type_res.error());
        }
        uint32_t value_type = *type_res;

        auto next_type_res = read_big_endian_u32(ptr, end_ptr);
        if (!next_type_res)
        {
            return std::unexpected(next_type_res.error());
        }
        uint32_t value_next_type = *next_type_res;

        auto val_res = parse_value_by_type(value_type, ptr, end_ptr);
        if (!val_res)
        {
            return std::unexpected(val_res.error());
        }
        record.value = *val_res;

        auto next_val_res = parse_value_by_type(value_next_type, ptr, end_ptr);
        if (!next_val_res)
        {
            return std::unexpected(next_val_res.error());
        }
        record.value_next = *next_val_res;

        result.settings.push_back(std::move(record));
    }

    return result;
}