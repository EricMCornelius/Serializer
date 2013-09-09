#pragma once

#include <serializer/json/json.h>

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <iostream>

struct JsonValue;
typedef std::string JsonString;
typedef std::unordered_map<JsonString, JsonValue> JsonObject;
typedef std::vector<JsonValue> JsonArray;
typedef double JsonNumber;
typedef bool JsonBool;
struct JsonNull { };

template <>
struct has_key<JsonValue> {
  typedef void key_type;
  typedef void mapped_type;
  typedef void value_type;

  const static bool value = false;
};

template <>
struct format_override<JsonNull, JsonOutStream> {
  template <typename Stream>
  static void format(Stream& out, const JsonNull& obj) {
    out << "null";
  }
};

template <>
struct format_override<JsonNull, JsonInStream> {
  template <typename Stream>
  static void format(Stream& in, JsonNull& obj) {
    in.good();
    in >> "null";
  }
};

template <>
struct format_override<JsonBool, JsonOutStream> {
  template <typename Stream>
  static void format(Stream& out, const JsonBool& obj) {
    if (obj)
      out << "true";
    else
      out << "false";
  }
};

template <>
struct format_override<JsonBool, JsonInStream> {
  template <typename Stream>
  static void format(Stream& in, JsonBool& obj) {
    in.good();
    in >> "true";
    if (in) {
      obj = true;
      return;
    }

    in.good();
    in >> "false";
    if (in) {
      obj = false;
      return;
    }
  }
};

void concat_impl(std::ostream& out) {

}

template <typename Head, typename... Args>
void concat_impl(std::ostream& out, const Head& h, const Args&... args) {
  out << h;
  concat_impl(out, args...);
}

template <typename... Args>
std::string concat(const Args&... args) {
  std::stringstream str;
  concat_impl(str, args...);
  return str.str();
}

struct ExceptionBase : public std::runtime_error {
  template <typename... Args>
  ExceptionBase(const Args&... args) : std::runtime_error(concat(args...)) {}
};

struct AccessException : public ExceptionBase {
  using ExceptionBase::ExceptionBase;
};

struct TypeException : public ExceptionBase {
  using ExceptionBase::ExceptionBase;
};

struct QueryResult;
struct SetterResult;

struct JsonValue {
  union value_type {
    std::shared_ptr<JsonNull> null;
    std::shared_ptr<JsonObject> object;
    std::shared_ptr<JsonArray> array;
    std::shared_ptr<JsonString> string;
    std::shared_ptr<JsonNumber> number;
    std::shared_ptr<JsonBool> boolean;

    value_type() : null(nullptr) { }
    ~value_type() { }
  };

  value_type ptr;

  enum class Type : unsigned char {
    Object,
    Array,
    String,
    Number,
    Boolean,
    Null
  };

  Type type = Type::Null;

  template <typename Type>
  Type& as();

  template <typename Type>
  const Type& as() const;

  template <typename Type>
  bool is() const;

  bool has(const std::string& key) const {
    return (type == Type::Object && ptr.object->find(key) != ptr.object->end());
  }

  bool has(const char* key) const {
    return has(std::string(key));
  }

  bool has(std::size_t idx) const {
    return (type == Type::Array && idx < ptr.array->size());
  }

  JsonValue& lookup(const std::string& key) {
    return ptr.object->operator[](key);
  }

  JsonValue& lookup(std::size_t idx) {
    return ptr.array->operator[](idx);
  }

  template <typename Value>
  void set(const std::string& key, const Value& v) {
    ptr.object->operator[](key) = v;
  }

  template <typename Value>
  void set(std::size_t idx, const Value& v) {
    ptr.array->operator[](idx) = v;
  }

  template <typename Key>
  QueryResult get(const Key& key);

  template <typename Key, typename... Args>
  QueryResult get(const Key& key, const Args&... args);

  template <std::size_t size>
  SetterResult operator [] (const char key[size]);

  template <std::size_t size>
  const JsonValue& operator [] (const char key[size]) const;

  SetterResult operator [] (const char* key);

  const JsonValue& operator [] (const char* key) const;

  SetterResult operator [] (const std::string& key);

  const JsonValue& operator [] (const std::string& key) const;

  JsonValue& operator [] (const std::size_t idx);

  const JsonValue& operator [] (const std::size_t idx) const;

  JsonValue& operator [] (const int idx);

  const JsonValue& operator [] (const int idx) const;

  JsonValue()
    : type(Type::Null) { }

  JsonValue(int i) {
    *this = static_cast<double>(i);
  }

  JsonValue(double d) {
    *this = d;
  }

  JsonValue(std::string&& str) {
    *this = str;
  }

  JsonValue(const std::string& str) {
    *this = str;
  }

  JsonValue(const char* str) {
    *this = str;
  }

  JsonValue(bool b) {
    *this = b;
  }

  JsonValue(std::nullptr_t val) {
    *this = nullptr;
  }

  JsonValue(JsonArray arr) {
    *this = arr;
  }

  JsonValue(JsonObject obj) {
    *this = obj;
  }

  JsonValue(std::initializer_list<std::pair<const JsonString, JsonValue>> pairs) {
    *this = JsonObject(pairs);
  }

  JsonValue(const JsonValue& other) {
    *this = other;
  }

  JsonValue(JsonValue&& other) {
    *this = other;
  }

  ~JsonValue() {
    cleanup();
  }

  const JsonValue clone() const {
    switch (type) {
      case Type::Object:
        return *ptr.object;
        break;
      case Type::Array:
        return *ptr.array;
        break;
      case Type::String:
        return *ptr.string;
        break;
      case Type::Number:
        return *ptr.number;
        break;
      case Type::Boolean:
        return *ptr.boolean;
        break;
      default: {

      }
    }
    return nullptr;
  }

  void cleanup() {
    switch (type) {
      case Type::Object:
        ptr.object = nullptr;
        break;
      case Type::Array:
        ptr.array = nullptr;
        break;
      case Type::String:
        ptr.string = nullptr;
        break;
      case Type::Number:
        ptr.number = nullptr;
        break;
      case Type::Boolean:
        ptr.boolean = nullptr;
        break;
      default: {

      }
    }
    type = Type::Null;
  }

  bool operator == (const JsonValue& other) const;

  JsonValue& operator = (JsonObject _val) {
    ptr.object = std::make_shared<JsonObject>(_val);
    type = Type::Object;
    return *this;
  }

  JsonValue& operator = (JsonArray _val) {
    ptr.array = std::make_shared<JsonArray>(_val);
    type = Type::Array;
    return *this;
  }

  JsonValue& operator = (JsonString _val) {
    ptr.string = std::make_shared<JsonString>(_val);
    type = Type::String;
    return *this;
  }

  JsonValue& operator = (const char* _str) {
    ptr.string = std::make_shared<JsonString>(_str);
    type = Type::String;
    return *this;
  }

  JsonValue& operator = (JsonNumber _val) {
    ptr.number = std::make_shared<JsonNumber>(_val);
    type = Type::Number;
    return *this;
  }

  JsonValue& operator = (int _val) {
    ptr.number = std::make_shared<JsonNumber>(_val);
    type = Type::Number;
    return *this;
  }

  JsonValue& operator = (JsonBool _val) {
    ptr.boolean = std::make_shared<JsonBool>(_val);
    type = Type::Boolean;
    return *this;
  }

  JsonValue& operator = (const JsonNull& _val) {
    ptr.null = nullptr;
    type = Type::Null;
    return *this;
  }

  JsonValue& operator = (std::nullptr_t _val) {
    ptr.null = nullptr;
    type = Type::Null;
    return *this;
  }

  JsonValue& operator = (const JsonValue& _val) {
    if (this == &_val)
      return *this;

    cleanup();
    type = _val.type;
    switch (_val.type) {
      case Type::Object:
        ptr.object = _val.ptr.object;
        break;
      case Type::Array:
        ptr.array = _val.ptr.array;
        break;
      case Type::String:
        ptr.string = _val.ptr.string;
        break;
      case Type::Number:
        ptr.number = _val.ptr.number;
        break;
      case Type::Boolean:
        ptr.boolean = _val.ptr.boolean;
        break;
      default: {

      }
    }
    return *this;
  }
};

template <>
bool JsonValue::is<JsonObject>() const {
  return type == Type::Object;
}

template <>
bool JsonValue::is<JsonArray>() const {
  return type == Type::Array;
}

template <>
bool JsonValue::is<JsonString>() const {
  return type == Type::String;
}

template <>
bool JsonValue::is<JsonNumber>() const {
  return type == Type::Number;
}

template <>
bool JsonValue::is<JsonBool>() const {
  return type == Type::Boolean;
}

template <>
bool JsonValue::is<JsonNull>() const {
  return type == Type::Null;
}

template <>
JsonObject& JsonValue::as<JsonObject>() {
  if (!is<JsonObject>())
    throw TypeException("Object type assertion failed");
  return *ptr.object;
}

template <>
const JsonObject& JsonValue::as<JsonObject>() const {
  if (!is<JsonObject>())
    throw TypeException("Object type assertion failed");
  return *ptr.object;
}

template <>
JsonArray& JsonValue::as<JsonArray>() {
  if (!is<JsonArray>())
    throw TypeException("Array type assertion failed");
  return *ptr.array;
}

template <>
const JsonArray& JsonValue::as<JsonArray>() const {
  if (!is<JsonArray>())
    throw TypeException("Array type assertion failed");
  return *ptr.array;
}

template <>
JsonString& JsonValue::as<JsonString>() {
  if (!is<JsonString>())
    throw TypeException("String type assertion failed");
  return *ptr.string;
}

template <>
const JsonString& JsonValue::as<JsonString>() const {
  if (!is<JsonString>())
    throw TypeException("String type assertion failed");
  return *ptr.string;
}

template <>
JsonNumber& JsonValue::as<JsonNumber>() {
  if (!is<JsonNumber>())
    throw TypeException("Number type assertion failed");
  return *ptr.number;
}

template <>
const JsonNumber& JsonValue::as<JsonNumber>() const {
  if (!is<JsonNumber>())
    throw TypeException("Number type assertion failed");
  return *ptr.number;
}

template <>
JsonBool& JsonValue::as<JsonBool>() {
  if (!is<JsonBool>())
    throw TypeException("Bool type assertion failed");
  return *ptr.boolean;
}

template <>
const JsonBool& JsonValue::as<JsonBool>() const {
  if (!is<JsonBool>())
    throw TypeException("Bool type assertion failed");
  return *ptr.boolean;
}

// TODO: should make this a union type
struct Key {
  Key(const std::string& str_)
    : str(str_), isString(true) {}
  Key(int idx_)
    : idx(idx_), isString(false) {}

  std::string str;
  int idx = 0;
  bool isString = false;
};

struct QueryResult {
  QueryResult() {}

  QueryResult(JsonValue* value)
    : _value(value) {}

  template <typename Default>
  Default defaultTo(const Default& d) {
    if (!_value)
      return d;

    if (_value->is<Default>())
      return _value->as<Default>();
    return d;
  }

  JsonString defaultTo(const char* d) {
    if (_value && _value->is<JsonString>())
      return _value->as<JsonString>();
    return d;
  }

  JsonNumber defaultTo(int d) {
    if (_value && _value->is<JsonNumber>())
      return _value->as<JsonNumber>();
    return d;
  }

  bool isNull() {
    return _value == nullptr;
  }

  JsonValue* _value = nullptr;
};

struct SetterResult {
  SetterResult(const std::string& key, JsonValue& value)
    : _value(value)
    {
      _keys.emplace_back(key);
    }

  JsonValue& operator = (const JsonValue& set) {
    JsonValue* root = &_value;
    for (const auto& key : _keys) {
      if (key.isString) {
        if (!root->is<JsonObject>())
          *root = JsonObject();
        root = &root->lookup(key.str);
      }
      else {
        if (!root->is<JsonArray>())
          *root = JsonArray();
        auto& arr = root->as<JsonArray>();
        if (key.idx >= arr.size())
          arr.resize(key.idx + 1);
        root = &root->lookup(key.idx);
      }
    }
    *root = set;

    return _value;
  }

  SetterResult& operator[](const char* key) {
    _keys.emplace_back(key);
    return *this;
  }

  SetterResult& operator[](int idx) {
    _keys.emplace_back(idx);
    return *this;
  }

  template <typename Default>
  auto defaultTo(const Default& d) -> decltype(QueryResult().defaultTo(d)) {
    JsonValue* root = &_value;
    for (auto& key : _keys) {
      if (key.isString) {
        if (!root->has(key.str))
          return QueryResult().defaultTo(d);
        root = &root->lookup(key.str);
      }
      else {
        if (!root->has(key.idx))
          return QueryResult().defaultTo(d);
        root = &root->lookup(key.idx);
      }
    }
    return QueryResult(root).defaultTo(d);
  }

  template <typename Type>
  Type& as() {
    JsonValue* root = &_value;
    for (auto& key : _keys) {
      if (key.isString) {
        if (!root->has(key.str))
          throw AccessException("Invalid Key: ", key.str);
        root = &root->lookup(key.str);
      }
      else {
        if (!root->has(key.idx))
          throw AccessException("Invalid Index: ", key.idx);
        root = &root->lookup(key.idx);
      }
    }
    return root->as<Type>();
  }

  std::list<Key> _keys;
  JsonValue& _value;
};

template <std::size_t size>
SetterResult JsonValue::operator [] (const char key[size]) {
  return SetterResult(key, *this);
}

template <std::size_t size>
const JsonValue& JsonValue::operator [] (const char key[size]) const {
  const auto& obj = as<JsonObject>();
  auto itr = obj.find(key);
  if (itr == obj.end())
    throw AccessException("Invalid Key: ", key);
  return itr->second;
}

SetterResult JsonValue::operator [] (const char* key) {
  return SetterResult(key, *this);
}

const JsonValue& JsonValue::operator [] (const char* key) const {
  const auto& obj = as<JsonObject>();
  auto itr = obj.find(key);
  if (itr == obj.end())
    throw AccessException("Invalid Key: ", key);
  return itr->second;
}

SetterResult JsonValue::operator [] (const std::string& key) {
  return SetterResult(key, *this);
}

const JsonValue& JsonValue::operator [] (const std::string& key) const {
  const auto& obj = as<JsonObject>();
  auto itr = obj.find(key);
  if (itr == obj.end())
    throw AccessException("Invalid Key: ", key);
  return itr->second;
}

JsonValue& JsonValue::operator [] (const std::size_t idx) {
  auto& arr = as<JsonArray>();
  if (idx >= arr.size())
    throw AccessException("Invalid Index: ", idx);
  return arr[idx];
}

const JsonValue& JsonValue::operator [] (const std::size_t idx) const {
  const auto& arr = as<JsonArray>();
  if (idx >= arr.size())
    throw AccessException("Invalid Index: ", idx);
  return arr[idx];
}

template <typename Key>
QueryResult JsonValue::get(const Key& key) {
  if (has(key))
    return QueryResult(&lookup(key));
  return QueryResult();
}

template <typename Key, typename... Args>
QueryResult JsonValue::get(const Key& key, const Args&... args) {
  if (has(key))
    return lookup(key).get(args...);
  return QueryResult();
}

JsonValue& JsonValue::operator [] (const int idx) {
  return (*this)[static_cast<std::size_t>(idx)];
}

const JsonValue& JsonValue::operator [] (const int idx) const {
  return (*this)[static_cast<std::size_t>(idx)];
}

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

// TODO: improve efficiency here - currently O(n^2) for dictionary comparisons
bool equivalent(const JsonObject& v1, const JsonObject& v2) {
  if (v1.size() != v2.size())
    return false;

  for (const auto& p1 : v1) {
    bool key_match = false;
    for (const auto& p2 : v2) {
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
    case JsonValue::Type::Object:
      return equivalent(v1.as<JsonObject>(), v2.as<JsonObject>());
    case JsonValue::Type::Array:
      return equivalent(v1.as<JsonArray>(), v2.as<JsonArray>());
    case JsonValue::Type::String:
      return equivalent(v1.as<JsonString>(), v2.as<JsonString>());
    case JsonValue::Type::Number:
      return equivalent(v1.as<JsonNumber>(), v2.as<JsonNumber>());
    case JsonValue::Type::Boolean:
      return equivalent(v1.as<JsonBool>(), v2.as<JsonBool>());
    default: {
      return true;
    }
  }
}

bool JsonValue::operator == (const JsonValue& other) const {
  return equivalent(*this, other);
};

typedef typename JsonObject::value_type JsonPair;


template <>
struct format_override<JsonValue, JsonOutStream> {
  template <typename Stream>
  static void format(Stream& out, const JsonValue& value) {
    switch(value.type) {
      case JsonValue::Type::Object:
        ::format(out, *value.ptr.object);
        break;
      case JsonValue::Type::Array:
        ::format(out, *value.ptr.array);
        break;
      case JsonValue::Type::String:
        ::format(out, *value.ptr.string);
        break;
      case JsonValue::Type::Number:
        ::format(out, *value.ptr.number);
        break;
      case JsonValue::Type::Boolean:
        ::format(out, *value.ptr.boolean);
        break;
      case JsonValue::Type::Null:
        ::format(out, JsonNull());
        break;
    }
  }
};

std::ostream& operator << (std::ostream& out, const JsonValue& v) {
  JsonOutStream ss(out);
  format(ss, v);
  return out;
}

template <>
struct format_override<JsonValue, JsonInStream> {
  template <typename Stream>
  static void format(Stream& in, JsonValue& value) {
    in.good();
    JsonString s;
    ::format(in, s);
    if (in) {
      value = s;
      return;
    }

    in.good();
    JsonNumber n;
    ::format(in, n);
    if (in) {
      value = n;
      return;
    }

    in.good();
    JsonBool b;
    ::format(in, b);
    if (in) {
      value = b;
      return;
    }

    in.good();
    JsonNull nu;
    ::format(in, nu);
    if (in) {
      value = nu;
      return;
    }

    in.good();
    JsonObject ob;
    ::format(in, ob);
    if (in) {
      value = std::move(ob);
      return;
    }

    in.good();
    JsonArray ar;
    ::format(in, ar);
    if (in) {
      value = std::move(ar);
      return;
    }
  }
};
