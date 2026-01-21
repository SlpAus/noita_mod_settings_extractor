#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <variant>
#include <vector>

#include "types.h"
#include "processor.h"

using namespace emscripten;

val setting_bool_to_val(const SettingBool &sb)
{
    auto resolved = sb.resolve();
    if (std::holds_alternative<bool>(resolved))
    {
        return val(std::get<bool>(resolved));
    }
    else
    {
        return val(std::get<int32_t>(resolved));
    }
}

struct SettingValueVisitor
{
    val operator()(std::nullptr_t) const
    {
        return val::null();
    }

    val operator()(const SettingBool &sb) const
    {
        return setting_bool_to_val(sb);
    }

    val operator()(double d) const
    {
        return val(d);
    }

    val operator()(const std::string &s) const
    {
        return val(s);
    }
};

val record_to_val(const ModSettingRecord &record)
{
    val obj = val::object();
    obj.set("name", record.name);

    obj.set("value", std::visit(SettingValueVisitor{}, record.value));
    obj.set("value_next", std::visit(SettingValueVisitor{}, record.value_next));

    return obj;
}

val result_to_val(const ModSettingsResult &result)
{
    val obj = val::object();
    obj.set("identifier", result.identifier);
    obj.set("record_count", result.record_count);

    val settings_arr = val::array();
    for (size_t i = 0; i < result.settings.size(); ++i)
    {
        settings_arr.set(i, record_to_val(result.settings[i]));
    }
    obj.set("settings", settings_arr);

    return obj;
}

val process_data_wrapper(std::string const &input_binary)
{
    std::span<const char> input_span(input_binary.data(), input_binary.size());

    auto result = process_raw_data(input_span);

    val res_obj = val::object();

    if (result)
    {
        res_obj.set("success", true);
        res_obj.set("data", result_to_val(*result));
    }
    else
    {
        res_obj.set("success", false);
        res_obj.set("error", error_to_string(result.error()));
        res_obj.set("errorCode", static_cast<int>(result.error()));
    }

    return res_obj;
}

EMSCRIPTEN_BINDINGS(my_module)
{
    function("processData", &process_data_wrapper);
}