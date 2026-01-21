#include "fastlz.h"
#include "formatter.h"
#include "parser.h"
#include "types.h"

#include <cstring>
#include <expected>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <vector>

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

std::expected<std::vector<char>, AppError> read_file_to_vector(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file)
    {
        return std::unexpected(AppError::IOError);
    }

    std::streamsize size = file.tellg();
    if (size < 0)
    {
        return std::unexpected(AppError::IOError);
    }

    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        return std::unexpected(AppError::IOError);
    }

    return buffer;
}

int main(int argc, char *argv[])
{
    std::ios_base::sync_with_stdio(false);

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <mod_settings.bin>" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];

    auto file_result = read_file_to_vector(input_filename);
    if (!file_result)
    {
        std::cerr << "Error: " << error_to_string(file_result.error()) << std::endl;
        return 1;
    }

    auto process_result = process_raw_data(*file_result);

    if (!process_result)
    {
        std::cerr << "Error: " << error_to_string(process_result.error()) << std::endl;
        return 1;
    }

    format_as_json(*process_result, std::cout, true);

    return 0;
}