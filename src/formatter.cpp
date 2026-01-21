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

    enum class ContextType
    {
        Root,
        Object,
        Array
    };

    class JsonWriter
    {
    public:
        JsonWriter(std::ostream &os, bool pretty)
            : out(os), is_pretty(pretty)
        {
            context_stack.push_back(ContextType::Root);
        }

        void begin_object()
        {
            prepare_new_element();
            out << "{";
            indent_push();
            context_stack.push_back(ContextType::Object);
            has_element_written = false;
        }

        void end_object()
        {
            context_stack.pop_back();
            indent_pop();
            if (has_element_written)
            {
                new_line();
            }
            out << "}";
            has_element_written = true;
        }

        void begin_array()
        {
            prepare_new_element();
            out << "[";
            indent_push();
            context_stack.push_back(ContextType::Array);
            has_element_written = false;
        }

        void end_array()
        {
            context_stack.pop_back();
            indent_pop();
            if (has_element_written)
            {
                new_line();
            }
            out << "]";
            has_element_written = true;
        }

        void write_key(std::string_view key)
        {
            if (has_element_written)
            {
                write_comma();
            }
            new_line();
            out << std::format("\"{}\":{}", escape_json_string(key), is_pretty ? " " : "");
            after_key = true;
        }

        void write_value(uint32_t val)
        {
            prepare_new_element();
            out << val;
            has_element_written = true;
        }

        void write_value(const SettingValue &value)
        {
            prepare_new_element();
            std::visit([this](auto &&arg)
                       { this->write_single_value(arg); }, value);
            has_element_written = true;
        }

    private:
        std::ostream &out;
        bool is_pretty;
        int indent_level = 0;
        std::vector<ContextType> context_stack;
        bool has_element_written = false;
        bool after_key = false;

        ContextType current_context() const
        {
            return context_stack.back();
        }

        void prepare_new_element()
        {
            if (after_key)
            {
                after_key = false;
            }
            else 
            {
                if (has_element_written)
                {
                    write_comma();
                }
                if (current_context() == ContextType::Array)
                {
                    new_line();
                }
            }
        }

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

        void write_comma()
        {
            out << ",";
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

    writer.write_key("identifier");
    writer.write_value(result.identifier);

    writer.write_key("record_count");
    writer.write_value(result.record_count);

    writer.write_key("settings");
    writer.begin_array();

    for (size_t i = 0; i < result.settings.size(); ++i)
    {
        const auto &record = result.settings[i];

        writer.begin_object();
        {
            writer.write_key("name");
            writer.write_value(record.name);

            writer.write_key("value");
            writer.write_value(record.value);

            writer.write_key("value_next");
            writer.write_value(record.value_next);
        }
        writer.end_object();
    }

    writer.end_array();
    writer.end_object();

    if (pretty)
    {
        out << "\n";
    }
}