#pragma once

#include "types.h"
#include <iostream>

void format_as_json(const ModSettingsResult& result, std::ostream& out, bool pretty = true);