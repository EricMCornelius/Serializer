#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <map>

#include <serializer/string_escaper.h>

class EscaperTestFixture : public testing::Test {

};

const std::map<std::string, std::string> cases = {
  { "Hello World", "Hello World" },
  { "{\"Goodbye\":\"World\"}", "{\\\"Goodbye\\\":\\\"World\\\"}" }
};

TEST_F(EscaperTestFixture, EscapeTest) {
  for (auto& c : cases) {
    std::cout << escape_string(c.first) << " " << c.second << std::endl;
    ASSERT_EQ(escape_string(c.first), c.second);
  }
}
