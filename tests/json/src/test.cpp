#include <uber_test.hpp>
#include <ostream_reporter.hpp>

#include <cassert>
#include <fstream>

#include <serializer/json/impl.h>

#include "resources.h"

using namespace ut;

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

    equivalent(fill.as<JsonObject>(), v);

    v["glossary"]["title"] = 10;
    equivalent(fill.as<JsonObject>(), v) == false;
  });

  it("should create a new object", [] {
    auto num = 100;
    auto str = "sample key";
    JsonValue s = {{str, num}};
    std::cout << s << std::endl;
  });

  it("should create a new array", [] {
    auto num = 100;
    auto str = "sample key";
    JsonArray s = {str, num};
    std::cout << s << std::endl;
  });

  it("should create a new object and assign an array to the data field", [] {
    JsonObject obj = {
      {"Hello", "World"}
    };
    obj["data"] = JsonArray{1, 2, 3};
    std::cout << obj << std::endl;
  });

  it("should throw an exception due to invalid object field access", [] {
    const JsonValue obj = {
      {"Hello", "World"}
    };

    std::cout << obj["invalid"] << std::endl;
  });

  it("should throw an exception due to invalid array access", [] {
    const JsonValue obj = {
      {"data", JsonArray{"Hello", "World"}}
    };

    std::cout << obj["data"][2] << std::endl;
  });

  it("should demonstrate value reference semantics", [] {
    JsonValue v = {
      {"Hello", "World"}
    };
    JsonValue v2 = v;
    v2["test"] = "hi";

    std::cout << v << " " << v2 << std::endl;
  });

  it("should clone a value", [] {
    JsonValue v = {
      {"Hello", "World"}
    };
    JsonValue v2 = v.clone();
    v2["test"] = "hi";

    std::cout << v << " " << v2 << std::endl;
  });

  it("should clone a const value", [] {
    const JsonValue v = {
      {"Hello", "World"}
    };
    JsonValue v2 = v.clone();
    v2["test"] = "hi";

    std::cout << v << " " << v2 << std::endl;
  });

  it("should fail to coerce a number field to a string", [] {
    JsonValue v = {
      {"test", 0}
    };

    auto field = v["test"].as<JsonString>();
  });

  it("should fail to coerce a string field to a number", [] {
    JsonValue v = {
      {"test", "0"}
    };

    auto field = v["test"].as<JsonNumber>();
  });

  it("should fail to coerce an object field to an array", [] {
    JsonValue v = {
      {"nested", {{"test", "0"}}}
    };

    auto field = v["nested"].as<JsonArray>();
  });

  it("should fail to coerce an array field to an object", [] {
    JsonValue v = {
      {"nested", JsonArray{"test", "0"}}
    };

    auto field = v["nested"].as<JsonObject>();
  });

  it("should return a defaulted string value", [] {
    JsonValue v = {
      {"Hello", "World"},
      {"Data", JsonArray{"This","Is","A","Test"}}
    };

    std::cout << v.get("hello").defaultTo("Test") << " " << v.get("Data", 0).defaultTo("this") << std::endl;
  });

  it("should return a defaulted int value", [] {
    JsonValue v = {
      {"Hello", "World"}
    };

    std::cout << v.get("hello").defaultTo(1) << std::endl;
  });

  it("should return a defaulted object value", [] {
    JsonValue v = {
      {"test", {{"Hello", "World"}}}
    };

    JsonObject t = {
      {"default", "implementation"}
    };

    std::cout << v.get("missing").defaultTo(t) << " " << v.get("test").defaultTo(t) << std::endl;
  });

  it("should create not create nested object via lookup", [] {
    JsonValue v = {
      {"Hello", "World"}
    };
    v["first"]["second"];

    std::cout << v << std::endl;
  });

  it("should create a nested object via assignment", [] {
    JsonValue v = {
      {"Hello", "World"}
    };
    v["first"]["second"] = {{"Hello", "World"}};

    std::cout << v << std::endl;
  });

  it("should create a nested array via assignment", [] {
    JsonValue v = {
      {"Hello", "World"}
    };
    v["first"]["second"][2] = {{"Hello", "World"}};

    std::cout << v << std::endl;
  });

  it("should default to 1", [] {
    JsonValue v = {
      {"Hello", "World"}
    };
    std::cout << v["Hello"]["test"].defaultTo(1) << std::endl;
  });

  it("should successful retrieve a JsonString field", [] {
    JsonValue v = {
      {"example", {{"test", "message"}}}
    };
    std::cout << v["example"]["test"].as<JsonString>() << std::endl;
  });

  it("should fail to retrieve a JsonString field", [] {
    JsonValue v = {
      {"example", {{"test", 1}}}
    };
    std::cout << v["example"]["test"].as<JsonString>() << std::endl;
  });


  /*
  it("should parse a very large json object", []{
    for (std::size_t i = 0; i < 10; ++i) {
      const auto& str = get_file_contents("stress.json");
      JsonInStream ssi(str);
      JsonValue fill;
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
