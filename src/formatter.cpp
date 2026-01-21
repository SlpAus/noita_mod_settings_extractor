#include "formatter.h"

#include <format>
#include <iostream>
#include <string>

namespace
{
    std::string escape_json_string(std::string_view input)
    {
        std::string result;

        for (char c : input)
        {
            switch (c)
            {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                {
                    result += std::format("\\u{:04x}", static_cast<unsigned char>(c));
                }
                else
                {
                    result += c;
                }
                break;
            }
        }
        return result;
    }

    class JsonWriter
    {
    public:
        JsonWriter(std::ostream &os, bool pretty)
            : out(os), is_pretty(pretty) {}

        void begin_object()
        {
            out << "{";
            indent_push();
        }

        void end_object()
        {
            indent_pop();
            new_line();
            out << "}";
        }

        void begin_array()
        {
            out << "[";
            indent_push();
        }

        void end_array()
        {
            indent_pop();
            new_line();
            out << "]";
        }

        void write_key(std::string_view key)
        {
            new_line();
            out << std::format("\"{}\":{}", escape_json_string(key), is_pretty ? " " : "");
        }

        void write_value(uint32_t val)
        {
            out << val;
        }

        void write_value(const SettingValue &value)
        {
            std::visit([this](auto &&arg)
                       { this->write_single_value(arg); }, value);
        }

        void write_comma()
        {
            out << ",";
        }

    private:
        std::ostream &out;
        bool is_pretty;
        int indent_level = 0;

        void indent_push()
        {
            indent_level++;
        }

        void indent_pop()
        {
            if (indent_level > 0)
            {
                indent_level--;
            }
        }

        void new_line()
        {
            if (!is_pretty)
            {
                return;
            }
            out << "\n";
            for (int i = 0; i < indent_level; ++i)
            {
                out << "  ";
            }
        }

        void write_single_value(std::nullptr_t)
        {
            out << "null";
        }

        void write_single_value(SettingBool val)
        {
            std::visit([this](auto &&arg)
                       {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, bool>)
                        {
                            out << (arg ? "true" : "false");
                        }
                        else
                        {
                            out << arg;
                        } }, val.resolve());
        }

        void write_single_value(double val)
        {
            out << std::format("{}", val);
        }

        void write_single_value(const std::string &val)
        {
            out << std::format("\"{}\"", escape_json_string(val));
        }
    };

} // namespace

void format_as_json(const ModSettingsResult &result, std::ostream &out, bool pretty)
{
    JsonWriter writer(out, pretty);

    writer.begin_object();

    writer.write_key("_header");
    writer.begin_object();
    {
        writer.write_key("identifier");
        writer.write_value(result.identifier);
        writer.write_comma();

        writer.write_key("record_count");
        writer.write_value(result.record_count);
    }
    writer.end_object();

    writer.write_comma();

    writer.write_key("settings");
    writer.begin_array();

    for (size_t i = 0; i < result.settings.size(); ++i)
    {
        const auto &record = result.settings[i];

        writer.begin_object();
        {
            writer.write_key("name");
            writer.write_value(record.name);
            writer.write_comma();

            writer.write_key("value");
            writer.write_value(record.value);
            writer.write_comma();

            writer.write_key("value_next");
            writer.write_value(record.value_next);
        }
        writer.end_object();

        if (i != result.settings.size() - 1)
        {
            writer.write_comma();
        }
    }

    writer.end_array();
    writer.end_object();

    if (pretty)
    {
        out << "\n";
    }
}