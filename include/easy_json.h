#pragma once
#include <deque>
#include <string>

namespace easy_json {
    class JsonArray;
    class JsonObject;

    class JsonAny
    {
    protected:
        JsonAny() = default;

    public:
        virtual ~JsonAny() = default;

        bool is_string();
        bool is_boolean();
        bool is_number();
        bool is_object();
        bool is_array();
        bool is_null();

        std::string to_str();
        bool to_boolean();
        int64_t to_integer();
        double to_number();
        JsonObject * to_object();
        JsonArray * to_array();

        virtual std::string dump() = 0;

    public:
        static JsonAny * str(const char * value = nullptr);
        static JsonAny * str(const char * value, int length);
        static JsonAny * boolean(bool value = false);
        static JsonAny * integer(int64_t value = 0);
        static JsonAny * number(double value = 0.0);
        static JsonAny * null();
        static JsonObject * object();
        static JsonArray * array();

        static JsonAny * parse(const char * str);
        static JsonAny * parse_file(const char * str);
    };

    class JsonObject : public JsonAny
    {

    public:
        virtual ~JsonObject() override;

        size_t count() const;
        std::string key_at(int index) const;
        JsonAny * value_at(int index) const;
        virtual std::string dump() override;

        JsonObject * set_property(const char * key, JsonAny * value);
        JsonAny * get_property(const char * key) const;

    private:
        friend class JsonAny;
        typedef std::pair<std::string, JsonAny *> JsonObjectPropertyType;
        std::deque<JsonObjectPropertyType> properties;    // keep order
        JsonObject();
    };

    class JsonArray : public JsonAny
    {
    public:
        virtual ~JsonArray();
        size_t count() const;
        JsonAny * at(int index) const;
        JsonArray * add(JsonAny * value);
        virtual std::string dump() override;

    private:
        friend class JsonAny;
        std::deque<JsonAny *> properties;
        JsonArray();
    };
} // namespace easy_json
