#include <serializer/json/impl.h>

struct test {
  std::string _key;
  std::string _value;

  typedef std::string key_type;
  typedef std::string mapped_type;

  const std::string& key() const { return _key; }
  const std::string& value() const { return _value; }
};

template <>
struct format_override<test, JsonOutStream> {
	template <typename Stream>
  static void format(Stream& out, const test& obj) {
    ::format(out, obj._key);
    out << ":";
    ::format(out, obj._value);
  }
};

template <>
struct format_override<test, JsonInStream> {
	template <typename Stream>
  static void format(Stream& out, test& obj) {
    ::format(out, obj._key);
    out >> ":";
    ::format(out, obj._value);
  }
};

template <typename Stream>
struct format_override<test, Stream> {
  static void format(Stream& out, const test& obj) {
    ::format(out, obj._key);
    out << "->";
    ::format(out, obj._value);
  }
};

struct recursive {
  std::vector<int> vals;
  std::vector<recursive*> children;

  recursive() { }
  recursive(const std::initializer_list<int>& init) : vals(init) { }
};

template <>
struct format_override<recursive, JsonOutStream> {
	template <typename Stream>
  static void format(Stream& out, const recursive& obj) {
  	out << "{";
  	if (obj.vals.size()) {
    	out << "\"data\":";
    	::format(out, obj.vals);
    }
    if (obj.vals.size() && obj.children.size())
    	out << ",";
  	if (obj.children.size()) {
    	out << "\"children\":";
      ::format(out, obj.children);
    }
    out << "}";
  }
};

template <>
struct format_override<recursive, JsonInStream> {
	template <typename Stream>
  static void format(Stream& in, recursive& obj) {
    in >> "{";
    in >> "\"data\":";
    ::format(in, obj.vals);
    if (in)
    	in >> ",";
    in.good();

    in >> "\"children\":";
    ::format(in, obj.children);
    in.good();
    in >> "}";
  }
};

template <>
struct format_override<recursive*, JsonOutStream> {
	template <typename Stream>
  static void format(Stream& out, const recursive* obj) {
  	::format(out, *obj);
  }
};

template <>
struct format_override<recursive*, JsonInStream> {
	template <typename Stream>
  static void format(Stream& in, recursive*& obj) {
  	obj = new recursive();
  	::format(in, *obj);
  }
};

void test1() {
  constexpr auto text = R"({"First":["hello","world","goodbye"],"Second":["Meh"]})";
  JsonInStream ssi(text);
  std::unordered_map<std::string, std::set<std::string>> fill;
  format(ssi, fill);

  JsonOutStream ss;
  format(ss, fill);
  std::cout << std::endl;
}

void test2() {
	constexpr auto text = R"({"First":{"a":1,"b":2,"c":3,"d":4},"Second":{"e":5,"f":6,"g":7,"h":8}})";
  JsonInStream ssi(text);
  std::unordered_map<std::string, std::map<std::string, int>> fill;
  format(ssi, fill);

  JsonOutStream ss;
  format(ss, fill);
  std::cout << std::endl;
}

void test3() {
	constexpr auto text = R"({"Hello": "Goodbye", 	"Hello2": "Goodbye2"})";
  JsonInStream ssi(text);
  std::vector<test> fill;
  format(ssi, fill);

  JsonOutStream ss;
  format(ss, fill);
  std::cout << std::endl;
}

void test4() {
	constexpr auto text = R"(
		{
		  "data": [1, 2, 3],
		  "children": [
		  	{
		  		"data": [4, 5, 6]
		  	},
		  	{
		  		"data": [7, 8, 9]
		  	},
		  	{
		  		"children": [
		  			{
		  				"data": [10, 11]
		  			}
		  		]
		  	}
		  ]
    }
  )";
	JsonInStream ssi(text);
	recursive fill;
	format(ssi, fill);

  JsonOutStream ss;
  format(ss, fill);
	std::cout << std::endl;
}

void test5() {
	constexpr auto text = R"([1, 2, 3, 4, 5])";
	JsonInStream ssi(text);
	std::vector<int> fill;
	format(ssi, fill);

	JsonOutStream ss;
	format(ss, fill);
	std::cout << std::endl;
}

void test6() {
  JsonObject obj;
  obj["Hello"] = 0;
  JsonArray arr = {1, "hi", "bye", true, false, {"hello", "world", 1, 2, 3}};
  JsonObject obj2 = {{"Hello", 1}, {"Goodbye", 2}, {"Test", JsonObject{{"?", 2}}}};
  JsonValue val = JsonObject{{"Hello", "World"}};
  obj["Children"] = arr;
  obj["Sub-obj"] = obj2;
  obj["test"] = obj["Children"];
  static_cast<JsonArray&>(obj["test"])[0] = 2;
  static_cast<JsonObject&>(obj["Sub-obj"])["Hello"] = 2;

  // bind goodbye property to hello
  obj["Goodbye"] = obj["Hello"];
  obj["Hello"] = 1;
  //static_cast<JsonNumber&>(obj["Hello"]) = 1;

  JsonOutStream ss;
  format(ss, obj);
  std::cout << std::endl;
}

void test7() {
  constexpr auto text = R"(
{
    "glossary": {
        "title": "example glossary",
		"GlossDiv": {
            "title": "S",
			"GlossList": {
                "GlossEntry": {
                    "ID": "SGML",
					"SortAs": "SGML",
					"GlossTerm": "Standard Generalized Markup Language",
					"Acronym": "SGML",
					"Abbrev": "ISO 8879:1986",
					"GlossDef": {
                        "para": "A meta-markup language, used to create markup languages such as DocBook.",
						"GlossSeeAlso": ["GML", "XML"]
                    },
					"GlossSee": "markup"
                }
            }
        }
    }
})";

  JsonInStream ssi(text);
  JsonObject fill;
  format(ssi, fill);

	JsonOutStream ss;
	format(ss, fill);
	std::cout << std::endl;
}

int main(int argc, char* argv[]) {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
}
