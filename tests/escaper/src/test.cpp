#include <iostream>
#include <string>
#include <map>

#include <serializer/string_escaper.h>
#include <uber_test.hpp>

using namespace ut;

describe(suite)
  const std::map<std::string, std::string> cases = {
    { "Hello World", "Hello World" },
    { "{\"Goodbye\":\"World\"}", "{\\\"Goodbye\\\":\\\"World\\\"}" }
  };

  it("Should verify all stored cases", [=]{
    for (auto& c : cases)
      assert(escape_string(c.first) == c.second);
  });
done(suite)

int main(int argc, char* argv[]) {
  OstreamReporter rep(std::cout);
  Registry::get("root")->execute(rep);
}
