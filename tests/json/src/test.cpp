#include <uber_test.hpp>
#include <cassert>
#include <fstream>

#include <serializer/json/impl.h>

#include "resources.h"

using namespace ut;

bool equivalent(const JsonValue&, const JsonValue&);
bool equivalent(const JsonObject&, const JsonObject&);
bool equivalent(const JsonArray&, const JsonArray&);
bool equivalent(const JsonString&, const JsonString&);
bool equivalent(const JsonNumber&, const JsonNumber&);
bool equivalent(const JsonBool&, const JsonBool&);

bool equivalent(const JsonString& v1, const JsonString& v2) {
  return v1 == v2;
}

bool equivalent(const JsonNumber& v1, const JsonNumber& v2) {
  return v1 == v2;
}

bool equivalent(const JsonBool& v1, const JsonBool& v2) {
  return v1 == v2;
}

bool equivalent(const JsonArray& v1, const JsonArray& v2) {
  if (v1.size() != v2.size())
    return false;

  for (std::size_t i = 0; i < v1.size(); ++i) {
    if (!equivalent(v1[i], v2[i]))
      return false;
  }

  return true;
}

bool equivalent(const JsonObject& v1, const JsonObject& v2) {
  if (v1.size() != v2.size())
    return false;

  for (auto& p1 : v1) {
    bool key_match = false;
    for (auto& p2 : v2) {
      if (p1.first == p2.first) {
        key_match = true;
        if (!equivalent(p1.second, p2.second))
          return false;
        break;
      }
    }
    if (!key_match)
      return false;
  }

  return true;
}

bool equivalent(const JsonValue& v1, const JsonValue& v2) {
  if (v1.type != v2.type)
    return false;

  switch (v1.type) {
    case JsonValue::OBJECT:
      return equivalent(v1.as<JsonObject>(), v2.as<JsonObject>());
    case JsonValue::ARRAY:
      return equivalent(v1.as<JsonArray>(), v2.as<JsonArray>());
    case JsonValue::STRING:
      return equivalent(v1.as<JsonString>(), v2.as<JsonString>());
    case JsonValue::NUMBER:
      return equivalent(v1.as<JsonNumber>(), v2.as<JsonNumber>());
    case JsonValue::BOOLEAN:
      return equivalent(v1.as<JsonBool>(), v2.as<JsonBool>());
    default: {
      return true;
    }
  }
}

// http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
std::string get_file_contents(const std::string& filename)
{
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return std::move(contents);
  }
}

describe(suite)
  it("should compare parsed and constructed json objects", []{
    JsonInStream ssi(text);
    JsonValue fill;
    format(ssi, fill);

    JsonObject v = {
      {"glossary", {
        {"title", "example glossary"},
        {"GlossDiv", {
          {"title", "S"},
          {"GlossList", {
            {"GlossEntry", {
              {"ID", "SGML"},
              {"SortAs", "SGML"},
              {"GlossTerm", "Standard Generalized Markup Language"},
              {"Acronym", "SGML"},
              {"Abbrev", "ISO 8879:1986"},
              {"GlossSee", "markup"},
              {"GlossDef",  {
                {"para", "A meta-markup language, used to create markup languages such as DocBook."},
                {"GlossSeeAlso", JsonArray{"GML", "XML"}}
              }},
              {"NullTest", nullptr}
            }
          }}
        }}
      }}
    }};

    assert(equivalent(fill.as<JsonObject>(), v));

    v["glossary"]["title"] = 10;
    assert(equivalent(fill.as<JsonObject>(), v) == false);

    auto num = 100;
    auto str = "sample key";
    JsonValue s = {{str, num}};
    std::cout << s[str] << std::endl;
  });

  it("should parse a very large json object", []{
    for (std::size_t i = 0; i < 10; ++i) {
      const auto& str = get_file_contents("stress.json");
      JsonInStream ssi(str);
      JsonValue fill;
      format(ssi, fill);
    }

    //std::cout << fill << std::endl;
  });
done(suite)

int main(int argc, char* argv[]) {
  parent_suite()->execute();
}
