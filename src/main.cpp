#include <expected>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "types.h"
#include "processor.h"
#include "formatter.h"

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