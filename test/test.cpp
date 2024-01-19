#include "easy_json.h"
#include <cstdio>

int main()
{
    const char json_string[] = R"({"str":"1234", "num" : 4321,"bool":true,"obj":{"obj_str":"1234"},"arr":[1,2,3,4]})";

    auto * json = easy_json::JsonAny::parse(json_string);
    printf("%p\n", json);
    return 0;
}