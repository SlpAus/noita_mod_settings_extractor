#pragma once

#include "types.h"
#include <span>

std::expected<ModSettingsResult, AppError> parse_decompressed_data(std::span<const char> data);