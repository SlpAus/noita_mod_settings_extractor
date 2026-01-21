#pragma once

#include "types.h"
#include <expected>
#include <span>

std::expected<ModSettingsResult, AppError> process_raw_data(std::span<const char> input);