#include "easy_json.h"

#include <fstream>
#include <sstream>
#include <string>

namespace easy_json
{
    template <typename T>
    class AutoFree
    {
    public:
        AutoFree(T ** p, bool is_array)
        {
            _need_free_ptr = p;
            _is_array = is_array;
        }

        ~AutoFree()
        {
            if (_need_free_ptr == nullptr || *_need_free_ptr == nullptr)
                return;

            if (!_is_array)
                delete * _need_free_ptr;
            else
                delete[] * _need_free_ptr;
            *_need_free_ptr = nullptr;
        }

    private:
        T ** _need_free_ptr = nullptr;
        bool _is_array = false;
    };

#define AutoFree(className, instance) \
    AutoFree<className> _auto_free_##instance(&instance, false)
#define AutoFreeArray(className, instance) \
    AutoFree<className> _auto_free_array_##instance(&instance, true)

    // tools func
    std::string escape(std::string && v)
    {
        std::stringstream ss;
        for (auto & c : v)
        {
            if (c == '"')
                ss << '\\';
            ss << c;
        }
        return ss.str();
    }

    int hex_to_value(char c)
    {
        if (std::isdigit(c))
            return c - '0';

        switch (c)
        {
        case 'a':
        case 'A':
            return 0xA;
        case 'b':
        case 'B':
            return 0xB;
        case 'c':
        case 'C':
            return 0xC;
        case 'd':
        case 'D':
            return 0xD;
        case 'e':
        case 'E':
            return 0xE;
        case 'f':
        case 'F':
            return 0xF;
        default:
            return 0xFF;
        }
    }

    // sample data class
    class JsonNumber : public JsonAny
    {
    public:
        double value = 0;

        JsonNumber(const double & v) { value = v; }

        std::string dump() override
        {
            char tmp[32] = { 0 };
            snprintf(tmp, 32, "%f", to_number());
            return tmp;
        }
    };

    class JsonBoolean : public JsonAny
    {
    public:
        bool value;

        JsonBoolean(const bool & v) { value = v; }

        std::string dump() override { return to_boolean() ? "true" : "false"; }
    };

    class JsonString : public JsonAny
    {
    public:
        std::string value;

        JsonString(const std::string & v) { value = v; }

        JsonString(const char * v) { value = (v ? v : ""); }

        JsonString(const char * v, size_t n)
        {
            value = v ? std::move(std::string(v, n)) : "";
        }

        std::string dump() override { return "\"" + escape(to_str()) + "\""; }
    };

    class JsonNull : public JsonAny
    {
    public:
        JsonNull() = default;

        std::string dump() override { return "null"; }
    };

    // 
    bool JsonAny::is_string() { return dynamic_cast<JsonString *>(this) != nullptr; }
    bool JsonAny::is_boolean() { return dynamic_cast<JsonBoolean *>(this) != nullptr; }
    bool JsonAny::is_object() { return dynamic_cast<JsonObject *>(this) != nullptr; }
    bool JsonAny::is_number() { return dynamic_cast<JsonNumber *>(this) != nullptr; }
    bool JsonAny::is_array() { return dynamic_cast<JsonArray *>(this) != nullptr; }
    bool JsonAny::is_null() { return dynamic_cast<JsonNull *>(this) != nullptr; }

    // 
    std::string JsonAny::to_str() { return dynamic_cast<JsonString *>(this)->value; }
    bool JsonAny::to_boolean() { return dynamic_cast<JsonBoolean *>(this)->value; }
    int64_t JsonAny::to_integer() { return static_cast<int64_t>(to_number()); }
    double JsonAny::to_number() { return dynamic_cast<JsonNumber *>(this)->value; }
    JsonObject * JsonAny::to_object() { return dynamic_cast<JsonObject *>(this); }
    JsonArray * JsonAny::to_array() { return dynamic_cast<JsonArray *>(this); }

    //
    JsonAny * JsonAny::str(const char * value) { return new JsonString(value); }
    JsonAny * JsonAny::str(const char * value, int length) { return new JsonString(value, length); }
    JsonAny * JsonAny::boolean(bool value) { return new JsonBoolean(value); }
    JsonAny * JsonAny::integer(int64_t value) { return new JsonNumber(static_cast<double>(value)); }
    JsonAny * JsonAny::number(double value) { return new JsonNumber(value); }
    JsonAny * JsonAny::null() { return new JsonNull(); }
    JsonObject * JsonAny::object() { return new JsonObject(); }
    JsonArray * JsonAny::array() { return new JsonArray(); }

    // Object
    JsonObject::JsonObject()
    {
    }

    JsonObject::~JsonObject()
    {
        for (auto & p : properties)
            delete p.second;
        properties.clear();
    }

    size_t JsonObject::count() const
    {
        return properties.size();
    }

    std::string JsonObject::key_at(int index) const
    {
        std::string ret;
        try
        {
            ret = properties.at(index).first;
        }
        catch (...)
        {
            return "";
        }
        return ret;
    }

    JsonAny * JsonObject::value_at(int index) const
    {
        JsonAny * ret = nullptr;
        try
        {
            ret = properties.at(index).second;
        }
        catch (...)
        {
            return nullptr;
        }
        return ret;
    }

    JsonObject * JsonObject::set_property(const char * key, JsonAny * value)
    {
        if (nullptr == value || nullptr == key)
            return this;

        for (auto it = properties.begin(); it != properties.end(); ++it)
        {
            if (it->first == key)
            {
                delete it->second;
                properties.insert(it, std::make_pair(key, value));
                properties.erase(it);
                return this;
            }
        }
        properties.push_back(std::make_pair(key, value));
        return this;
    }

    JsonAny * JsonObject::get_property(const char * key) const
    {
        if (nullptr == key)
            return nullptr;

        for (const auto & property : properties)
        {
            if (property.first == key)
                return property.second;
        }
        return nullptr;
    }

    std::string JsonObject::dump()
    {
        if (properties.empty())
            return "{}";

        std::stringstream ss;
        ss << '{';
        for (const auto & property : properties)
        {
            ss << '\\' << property.first << ":\\" << property.second->dump() << ',';
        }

        std::streampos size = ss.tellp();
        size -= 1;
        ss.seekp(size);
        ss.put('}');
        return ss.str();
    }


    // Array
    JsonArray::JsonArray()
    {
    }

    JsonArray::~JsonArray()
    {
        for (auto & item : properties)
            delete item;
        properties.clear();
    }

    size_t JsonArray::count() const
    {
        return properties.size();
    }

    JsonAny * JsonArray::at(int index) const
    {
        JsonAny * ret = nullptr;
        try
        {
            ret = properties.at(index);
        }
        catch (...)
        {
            return nullptr;
        }
        return ret;
    }

    JsonArray * JsonArray::add(JsonAny * value)
    {
        properties.push_back(value);
        return this;
    }

    std::string JsonArray::dump()
    {
        std::stringstream ss;
        ss << '[';
        for (const auto & property : properties)
        {
            ss << property->dump() << ',';
        }

        std::streampos size = ss.tellp();
        size -= 1;
        ss.seekp(size);
        ss.put(']');
        return ss.str();
    }

    // Parse
    class JsonParser
    {
    private:
        const char * str_start = nullptr;
        const char * str_end = nullptr;
        const char * p = nullptr;

        uint32_t flag = 0;
        JsonAny * root = nullptr;
        int err = 0;

    protected:
#define RETURN_ERROR(code) { err = code; return false; }

        char skip_space()
        {
            while (p < str_end)
            {
                if (*p == ' ' || *p == '\r' || *p == '\n')
                {
                    ++p;
                    continue;;
                }
                break;
            }
            return *p;
        }

        bool parse_unicode(std::string & value)
        {
            ++p;
            uint8_t uc_b1, uc_b2, uc_b3, uc_b4;
            if (str_end - p < 4 ||
                (uc_b1 = hex_to_value(*p++)) == 0xFF ||
                (uc_b2 = hex_to_value(*p++)) == 0xFF ||
                (uc_b3 = hex_to_value(*p++)) == 0xFF ||
                (uc_b4 = hex_to_value(*p++)) == 0xFF)
                return false;

            uc_b1 = (uc_b1 << 4) | uc_b2;
            uc_b2 = (uc_b3 << 4) | uc_b4;
            uint32_t uchar = (uc_b1 << 8) | uc_b2;

            if ((uchar & 0xF800) == 0xD800)
            {
                uint32_t uchar2;

                if (str_end - p < 6 || (*++p) != '\\' || (*++p) != 'u' ||
                    (uc_b1 = hex_to_value(*++p)) == 0xFF ||
                    (uc_b2 = hex_to_value(*++p)) == 0xFF ||
                    (uc_b3 = hex_to_value(*++p)) == 0xFF ||
                    (uc_b4 = hex_to_value(*++p)) == 0xFF)
                    return false;

                uc_b1 = (uc_b1 << 4) | uc_b2;
                uc_b2 = (uc_b3 << 4) | uc_b4;
                uchar2 = (uc_b1 << 8) | uc_b2;

                uchar = 0x010000 | ((uchar & 0x3FF) << 10) | (uchar2 & 0x3FF);
            }

            if (uchar <= 0x7F)
            {
                value += uchar & 0XFF;
            }
            else if (uchar <= 0x7FF)
            {
                value += 0XC0 | (uchar >> 6);
                value += 0X80 | (uchar & 0X3F);
            }
            else if (uchar <= 0x7FF)
            {
                value += 0XE0 | (uchar >> 12);
                value += 0X80 | ((uchar >> 6) & 0x3F);
                value += 0X80 | (uchar & 0X3F);
            }
            else if (uchar <= 0xFFFF)
            {
                value += 0XF0 | (uchar >> 18);
                value += 0X80 | ((uchar >> 12) & 0x3F);
                value += 0X80 | ((uchar >> 6) & 0x3F);
                value += 0X80 | (uchar & 0X3F);
            }
            return true;
        }

        bool parse_escape_character(std::string & value)
        {
            ++p;
            switch (*p)
            {
            case '\"':
                value.push_back('\"');
                ++p;
                return true;
            case '\\':
                value.push_back('\\');
                ++p;
                return true;
            case '/':
                value.push_back('/');
                ++p;
                return true;
            case 'b':
                value.push_back('\b');
                ++p;
                return true;
            case 'f':
                value.push_back('\f');
                ++p;
                return true;
            case 'n':
                value.push_back('\n');
                ++p;
                return true;
            case 'r':
                value.push_back('\r');
                ++p;
                return true;
            case 't':
                value.push_back('\t');
                ++p;
                return true;
            case 'u':
                return parse_unicode(value);
            default:
                return false;
            }
        }

        bool parse_string(std::string & value)
        {
            value.clear();
            ++p;
            while (*p != '\0')
            {
                if (*p == '\\')
                {
                    parse_escape_character(value);
                    continue;
                }

                if (*p == '\"')
                    break;

                value.push_back(*p++);
            }
            ++p;
            return true;
        }

        bool parse_string(JsonString *& value)
        {
            std::string tmp_string;
            if (!parse_string(tmp_string))
                return false;
            value = static_cast<JsonString *>(JsonAny::str(tmp_string.c_str()));
            return true;
        }

        bool parse_array(JsonArray *& value)
        {
            JsonArray * array = JsonAny::array();
            AutoFreeArray(JsonArray, array);
            ++p;

            // empty array
            if (skip_space() == ']')
            {
                value = array;
                array = nullptr;
                return true;
            }

            JsonAny * array_value = nullptr;
            while (true)
            {
                array_value = nullptr;
                if (!parse_value(array_value))
                    return false;

                array->add(array_value);
                switch (skip_space())
                {
                case ',':
                    ++p;
                    continue;
                case']':
                    break;
                default:
                    RETURN_ERROR(-1)
                }
                if (*p == ']')
                    break;
            }
            ++p;
            array_value = array;
            array = nullptr;
            return true;
        }

        bool parse_object(JsonObject *& value)
        {
            JsonObject * obj = JsonAny::object();
            AutoFree(JsonObject, obj);
            ++p;

            // empty object
            if (skip_space() == '}')
            {
                value = obj;
                obj = nullptr;
                return true;
            }

            std::string key;
            JsonAny * obj_value = nullptr;

            while (true)
            {
                if (skip_space() != '\"')
                    return false;
                if (!parse_string(key))
                    return false;
                if (skip_space() != ':')
                    return false;

                ++p;
                obj_value = nullptr;
                if (!parse_value(obj_value))
                    return false;

                obj->set_property(key.c_str(), obj_value);
                switch (skip_space())
                {
                case ',':
                    ++p;
                    continue;
                case '}':
                    break;
                default:
                    return false;
                }

                if (*p == '}')
                    break;
            }
            ++p;
            value = obj;
            obj = nullptr;
            return true;
        }

        bool parse_boolean(JsonBoolean *& value, bool type)
        {
            int str_size = type ? 4 : 5;
            if (0 != strncmp(p, type ? "true" : "false", str_size))
                return false;

            p += str_size;
            value = static_cast<JsonBoolean *>(JsonAny::boolean(type));
            return true;
        }

        bool parse_null(JsonNull *& value)
        {
            if (0 != strncmp(p, "null", 4))
                return false;
            p += 4;
            value = static_cast<JsonNull *>(JsonAny::null());
            return true;
        }

        bool parse_number(JsonNumber *& value)
        {
            const char * num_str_end = p;
            char * end_ptr = nullptr;
            double ret = 0;
            try
            {

                ret = strtod(num_str_end, &end_ptr);
            }
            catch (...)
            {
                return false;
            }
            p = end_ptr ? end_ptr : p;

            value = static_cast<JsonNumber *>(JsonAny::number(ret));
            return true;
        }

        bool parse_value(JsonAny *& json_value)
        {
            char c = skip_space();
            switch (c)
            {
            case '\"':
            {
                JsonString * str_value = nullptr;
                if (!parse_string(str_value))
                    return false;
                json_value = str_value;
                break;
            }
            case '[':
            {
                JsonArray * array_value = nullptr;
                if (!parse_array(array_value))
                    return false;
                json_value = array_value;
                break;
            }
            case '{':
            {
                JsonObject * object_value = nullptr;
                if (!parse_object(object_value))
                    return false;
                json_value = object_value;
            }
            break;
            case 't':
            {
                JsonBoolean * true_value = nullptr;
                if (!parse_boolean(true_value, true))
                    return false;
                json_value = true_value;
            }
            break;
            case 'f':
            {
                JsonBoolean * false_value = nullptr;
                if (!parse_boolean(false_value, false))
                    return false;
                json_value = false_value;
                break;
            }
            case 'n':
            {
                JsonNull * null_value = nullptr;
                if (!parse_null(null_value))
                    return false;
                json_value = null_value;
                break;
            }
            default:
            {
                JsonNumber * number_value = nullptr;
                if (!parse_number(number_value))
                    return false;
                json_value = number_value;
            }
            }
            return true;
        }

    public:
        JsonParser(const char * json_string, size_t str_len)
        {
            str_start = json_string;
            str_end = str_start + str_len;
        }

        bool parse()
        {
            p = str_start;
            if (str_end - str_start >= 3 &&
                str_start[0] == 0XEF &&
                str_start[1] == 0XBB &&
                str_start[2] == 0XBF) // UTF-8 BOM
                p += 3;

            if (*p == '{')
            {
                JsonObject * obj = nullptr;
                if (!parse_object(obj))
                    return false;

                root = obj;
                return true;
            }

            if (*p == '[')
            {
                JsonArray * json_array = nullptr;
                if (!parse_array(json_array))
                    return false;

                root = json_array;
                return true;
            }

            return false;
        }

        JsonAny * result() const { return root; }
        int error() const { return err; }
    };

    JsonAny * JsonAny::parse(const char * str)
    {
        JsonParser parser(str, strlen(str));
        return parser.parse() ? parser.result() : nullptr;
    }

    JsonAny * JsonAny::parse_file(const char * str)
    {
        std::ifstream ifs(str);
        std::string buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        return parse(buf.c_str());
    }
} // namespace easy_json
