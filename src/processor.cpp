#include "processor.h"

#include <cstring>
#include <expected>
#include <span>

#include "fastlz.h"

#include "types.h"
#include "parser.h"

std::expected<ModSettingsResult, AppError> process_raw_data(std::span<const char> input)
{
    if (input.empty())
    {
        return std::unexpected(AppError::EmptyInput);
    }

    if (input.size() < 8)
    {
        return std::unexpected(AppError::HeaderTooShort);
    }

    uint32_t compressed_size = 0;
    uint32_t decompressed_size = 0;

    std::memcpy(&compressed_size, input.data(), sizeof(uint32_t));
    std::memcpy(&decompressed_size, input.data() + 4, sizeof(uint32_t));

    std::vector<char> decompressed_data(decompressed_size);
    const char *data_ptr = input.data() + 8;
    size_t remaining_size = input.size() - 8;

    if (decompressed_size < 128)
    {
        if (compressed_size != decompressed_size)
        {
            return std::unexpected(AppError::SizeMismatch);
        }
        if (remaining_size < decompressed_size)
        {
            return std::unexpected(AppError::UncompressedDataReadError);
        }
        std::memcpy(decompressed_data.data(), data_ptr, decompressed_size);
    }
    else
    {
        if (remaining_size < compressed_size)
        {
            return std::unexpected(AppError::SizeMismatch);
        }

        if (compressed_size == 0)
        {
            return std::unexpected(AppError::EmptyInput);
        }

        int result_size = fastlz_decompress(
            data_ptr,
            static_cast<int>(compressed_size),
            decompressed_data.data(),
            static_cast<int>(decompressed_data.size()));

        if (result_size != static_cast<int>(decompressed_size))
        {
            return std::unexpected(AppError::DecompressionFailed);
        }
    }

    return parse_decompressed_data(decompressed_data);
}