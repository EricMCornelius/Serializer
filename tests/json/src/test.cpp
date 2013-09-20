#include <uber_test.hpp>
#include <ostream_reporter.hpp>
#include <assertions.hpp>

#include <fstream>

#include <serializer/json/impl.h>

#include "resources.h"

using namespace ut;
using namespace json;

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
    std::stringstream str;
    str << text;

    //json::InStream ssi(text);
    json::Value fill;
    //format(ssi, fill);
    str >> fill;

    json::Object v = {
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
                {"GlossSeeAlso", json::Array{"GML", "XML"}}
              }},
              {"NullTest", nullptr}
            }
          }}
        }}
      }}
    }};

    ut_assert_eq(fill.as<json::Object>(), v, "Should be equivalent");

    v["glossary"]["title"] = 10;
    ut_assert_neq(fill.as<json::Object>(), v, "Should not be equivalent");
  });

  it("should create a new object", [] {
    auto num = 100;
    auto str = "sample key";
    json::Value s = {{str, num}};
    ut_assert_eq(s["sample key"], 100);
  });

  it("should create a new array", [] {
    auto num = 100;
    auto str = "sample key";
    json::Array s = {str, num};
    ut_assert_eq(s[0], "sample key");
    ut_assert_eq(s[1], num);
  });


  it("should create a new object and assign an array to the data field", [] {
    json::Object obj = {
      {"Hello", "World"}
    };
    obj["data"] = json::Array{1, 2, 3};
    ut_assert_eq(obj["data"][0], 1);
    ut_assert_eq(obj["data"][1], 2);
    ut_assert_eq(obj["data"][2], 3);
  });

  it("should throw an exception due to invalid object field access", [] {
    const json::Value obj = {
      {"Hello", "World"}
    };

    ut_assert_throws(obj["invalid"].as<Value>(), json::AccessException);
  });

  it("should throw an exception due to invalid array access", [] {
    const json::Value obj = {
      {"data", json::Array{"Hello", "World"}}
    };

    ut_assert_throws(obj["data"][2].as<Value>(), json::AccessException);
  });

  it("should demonstrate value reference semantics", [] {
    json::Value v = {
      {"Hello", "World"}
    };
    json::Value v2 = v;
    v2["test"] = "hi";

    ut_assert_eq(v, v2);
  });

  it("should clone a value", [] {
    json::Value v = {
      {"Hello", "World"}
    };
    json::Value v2 = v.clone();
    v2["test"] = "hi";

    ut_assert_neq(v, v2);
  });

  it("should clone a const value", [] {
    const json::Value v = {
      {"Hello", "World"}
    };
    json::Value v2 = v.clone();
    v2["test"] = "hi";

    ut_assert_neq(v, v2);
  });

  it("should fail to coerce a number field to a string", [] {
    json::Value v = {
      {"test", 0}
    };

    ut_assert_throws(v["test"].as<json::String>(), json::TypeException);
  });

  it("should fail to coerce a string field to a number", [] {
    json::Value v = {
      {"test", "0"}
    };

    ut_assert_throws(v["test"].as<json::Number>(), json::TypeException);
  });

  it("should fail to coerce an object field to an array", [] {
    json::Value v = {
      {"nested", {{"test", "0"}}}
    };

    ut_assert_throws(v["nested"].as<json::Array>(), json::TypeException);
  });

  it("should fail to coerce an array field to an object", [] {
    json::Value v = {
      {"nested", json::Array{"test", "0"}}
    };

    ut_assert_throws(v["nested"].as<json::Object>(), json::TypeException);
  });

  it("should return a defaulted string value", [] {
    json::Value v = {
      {"Hello", "World"},
      {"Data", json::Array{"This","Is","A","Test"}}
    };

    ut_assert_eq(v["hello"].defaultTo("Test"), "Test");
    ut_assert_eq(v["Data"][0].defaultTo("this"), "This");
  });

  it("should return a defaulted int value", [] {
    json::Value v = {
      {"Hello", "World"}
    };

    ut_assert_eq(v["hello"].defaultTo(1), 1);
  });

  it("should return a defaulted object value", [] {
    json::Value v = {
      {"test", {{"Hello", "World"}}}
    };

    json::Object t = {
      {"default", "implementation"}
    };

    ut_assert_eq(v["missing"].defaultTo(t), t);
    ut_assert_neq(v["test"].defaultTo(t), t);
  });

  it("should not create nested object via lookup", [] {
    json::Value v = {
      {"Hello", "World"}
    };
    v["first"]["second"];

    ut_assert_eq(v["first"].defaultTo(Null()), Null());
  });

  it("should create a nested object via assignment", [] {
    json::Value v = {
      {"Hello", "World"}
    };
    v["first"]["second"] = {{"Hello", "World"}};

    ut_assert_eq(v["first"]["second"]["Hello"], "World");
  });

  it("should create a nested array via assignment", [] {
    json::Value v = {
      {"Hello", "World"}
    };
    v["first"]["second"][2] = {{"Hello", "World"}};

    ut_assert_eq(v["first"]["second"][2]["Hello"], "World");
  });

  it("should default to 1", [] {
    json::Value v = {
      {"Hello", "World"}
    };

    ut_assert_eq(v["Hello"]["test"].defaultTo(1), 1);
  });

  it("should successful retrieve a json::String field", [] {
    json::Value v = {
      {"example", {{"test", "message"}}}
    };

    ut_assert_eq(v["example"]["test"].as<String>(), "message");
  });

  it("should fail to retrieve a json::String field", [] {
    json::Value v = {
      {"example", {{"test", 1}}}
    };

    ut_assert_throws(v["example"]["test"].as<String>(), json::TypeException);
  });

  it("should fail to parse a json string", [] {
    constexpr char* input = R"(
      {"Hello",: [1,2,3],
       "World": [4,"'",5,6,0,"test"],
       "...": "..."}
    )";

    json::Value v;
    auto res = v.parse(input);

    ut_assert_eq(res, false);
    ut_assert(v.is<json::Null>());
  });

  /*
  it("should parse a very large json object", []{
    for (std::size_t i = 0; i < 10; ++i) {
      const auto& str = get_file_contents("stress.json");
      json::InStream ssi(str);
      json::Value fill;
      format(ssi, fill);
    }

    //std::cout << fill << std::endl;
  });
  */
done(suite)

int main(int argc, char* argv[]) {
  OstreamReporter rep(std::cout);
  Registry::get("root")->execute(rep);
}
