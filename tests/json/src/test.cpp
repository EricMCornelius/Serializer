#include <uber_test.hpp>

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
  return "";
}

suite(json_tests)
  it("should parse a string", [] {
    std::stringstream str;
    str << "\"test\"";

    Value val;
    str >> val;

    ut_assert(val.is<String>());
  });

  it("should parse a number", [] {
    std::stringstream str;
    str << "3.14159";

    Value val;
    str >> val;

    ut_assert(val.is<Number>());
  });

  it("should parse a null", [] {
    std::stringstream str;
    str << "null";

    Value val;
    str >> val;

    ut_assert(val.is<Null>());
  });

  it("should parse false", [] {
    std::stringstream str;
    str << "false";

    Value val;
    str >> val;

    ut_assert(val.is<Bool>());
  });

  it("should parse true", [] {
    std::stringstream str;
    str << "true";

    Value val;
    str >> val;

    ut_assert(val.is<Bool>());
  });

  it("should parse an object", [] {
    std::stringstream str;
    str << "{}";

    Value val;
    str >> val;

    ut_assert(val.is<Object>());
  });

  it("should parse an array", [] {
    std::stringstream str;
    str << "[]";

    Value val;
    str >> val;

    ut_assert(val.is<Array>());
  });

  it("should create an Array", [] {
    std::stringstream str;
    str << "[]";

    Value val;
    str >> val;

    ut_assert(val.is<Array>());
  });

  it("should compare parsed and constructed json objects", []{
    std::stringstream str;
    str << text;

    Value fill;
    str >> fill;

    Object v = {
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
                {"GlossSeeAlso", Array{"GML", "XML"}}
              }},
              {"NullTest", nullptr}
            }
          }}
        }}
      }}
    }};

    ut_assert_eq(fill.as<Object>(), v, "Should be equivalent");

    v["glossary"]["title"] = 10;
    ut_assert_neq(fill.as<Object>(), v, "Should not be equivalent");
  });

  it("should create a new object", [] {
    auto num = 100;
    auto str = "sample key";
    Value s = {{str, num}};
    ut_assert_eq(s["sample key"], 100);
  });

  it("should create a new array", [] {
    auto num = 100;
    auto str = "sample key";
    Array s = {str, num};
    ut_assert_eq(s[0], "sample key");
    ut_assert_eq(s[1], num);
  });


  it("should create a new object and assign an array to the data field", [] {
    Object obj = {
      {"Hello", "World"}
    };
    obj["data"] = Array{1, 2, 3};
    ut_assert_eq(obj["data"][0], 1);
    ut_assert_eq(obj["data"][1], 2);
    ut_assert_eq(obj["data"][2], 3);
  });

  it("should throw an exception due to invalid object field access", [] {
    const Value obj = {
      {"Hello", "World"}
    };

    ut_assert_throws(obj["invalid"].as<Value>(), AccessException);
  });

  it("should throw an exception due to invalid array access", [] {
    const Value obj = {
      {"data", Array{"Hello", "World"}}
    };

    ut_assert_throws(obj["data"][2].as<Value>(), AccessException);
  });

  it("should demonstrate value reference semantics", [] {
    Value v = {
      {"Hello", "World"}
    };
    Value v2 = v;
    v2["test"] = "hi";

    ut_assert_eq(v, v2);
  });

  it("should clone a value", [] {
    Value v = {
      {"Hello", "World"}
    };
    Value v2 = v.clone();
    v2["test"] = "hi";

    ut_assert_neq(v, v2);
  });

  it("should clone a const value", [] {
    const Value v = {
      {"Hello", "World"}
    };
    Value v2 = v.clone();
    v2["test"] = "hi";

    ut_assert_neq(v, v2);
  });

  it("should fail to coerce a number field to a string", [] {
    Value v = {
      {"test", 0}
    };

    ut_assert_throws(v["test"].as<String>(), TypeException);
  });

  it("should fail to coerce a string field to a number", [] {
    Value v = {
      {"test", "0"}
    };

    ut_assert_throws(v["test"].as<Number>(), TypeException);
  });

  it("should fail to coerce an object field to an array", [] {
    Value v = {
      {"nested", {{"test", "0"}}}
    };

    ut_assert_throws(v["nested"].as<Array>(), TypeException);
  });

  it("should fail to coerce an array field to an object", [] {
    Value v = {
      {"nested", Array{"test", "0"}}
    };

    ut_assert_throws(v["nested"].as<Object>(), TypeException);
  });

  it("should return a defaulted string value", [] {
    Value v = {
      {"Hello", "World"},
      {"Data", Array{"This","Is","A","Test"}}
    };

    ut_assert_eq(v["hello"].defaultTo("Test"), "Test");
    ut_assert_eq(v["Data"][0].defaultTo("this"), "This");
  });

  it("should return a defaulted int value", [] {
    Value v = {
      {"Hello", "World"}
    };

    ut_assert_eq(v["hello"].defaultTo(1), 1);
  });

  it("should return a defaulted object value", [] {
    Value v = {
      {"test", {{"Hello", "World"}}}
    };

    Object t = {
      {"default", "implementation"}
    };

    ut_assert_eq(v["missing"].defaultTo(t), t);
    ut_assert_neq(v["test"].defaultTo(t), t);
  });

  it("should not create nested object via lookup", [] {
    Value v = {
      {"Hello", "World"}
    };
    v["first"]["second"];

    ut_assert_eq(v["first"].defaultTo(Null()), Null());
  });

  it("should create a nested object via assignment", [] {
    Value v = {
      {"Hello", "World"}
    };
    v["first"]["second"] = {{"Hello", "World"}};

    ut_assert_eq(v["first"]["second"]["Hello"], "World");
  });

  it("should create a nested array via assignment", [] {
    Value v = {
      {"Hello", "World"}
    };
    v["first"]["second"][2] = {{"Hello", "World"}};

    ut_assert_eq(v["first"]["second"][2]["Hello"], "World");
  });

  it("should default to 1", [] {
    Value v = {
      {"Hello", "World"}
    };

    ut_assert_eq(v["Hello"]["test"].defaultTo(1), 1);
  });

  it("should successful retrieve a String field", [] {
    Value v = {
      {"example", {{"test", "message"}}}
    };

    ut_assert_eq(v["example"]["test"].as<String>(), "message");
  });

  it("should fail to retrieve a String field", [] {
    Value v = {
      {"example", {{"test", 1}}}
    };

    ut_assert_throws(v["example"]["test"].as<String>(), TypeException);
  });

  it("should fail to parse a json string", [] {
    constexpr const char* input = R"(
      {"Hello",: [1,2,3],
       "World": [4,"'",5,6,0,"test"],
       "...": "..."}
    )";

    Value v;
    auto res = v.parse(input);

    ut_assert_eq(res, false);
    ut_assert(v.is<Null>());
  });

  /*
  it("should parse a very large json object", []{
    for (std::size_t i = 0; i < 10; ++i) {
      const auto& str = get_file_contents("stress.json");
      InStream ssi(str);
      Value fill;
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
