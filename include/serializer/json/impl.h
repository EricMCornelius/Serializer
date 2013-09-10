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

namespace json {

struct Value;
typedef std::string String;
typedef std::unordered_map<String, Value> Object;
typedef std::vector<Value> Array;
typedef double Number;
typedef bool Bool;
struct Null { };

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

struct Value {
  union value_type {
    std::shared_ptr<Null> null;
    std::shared_ptr<Object> object;
    std::shared_ptr<Array> array;
    std::shared_ptr<String> string;
    std::shared_ptr<Number> number;
    std::shared_ptr<Bool> boolean;

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

  bool parse(const std::string& str) {
    InStream ssi(str);
    format(ssi, *this);
    return true;
  }

  bool parse(std::istream& in) {
    InStream ssi(in);
    format(ssi, *this);
    return true;
  }

  std::string json() const {
    std::stringstream out;
    OutStream ss(out);
    format(ss, *this);
    return std::move(out.str());
  }

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

  Value& lookup(const std::string& key) {
    return ptr.object->operator[](key);
  }

  Value& lookup(std::size_t idx) {
    return ptr.array->operator[](idx);
  }

  const Value& lookup(const std::string& key) const {
    auto itr = ptr.object->find(key);
    return itr->second;
  }

  const Value& lookup(std::size_t idx) const {
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

  template <std::size_t size>
  SetterResult operator [] (const char key[size]);

  template <std::size_t size>
  QueryResult operator [] (const char key[size]) const;

  SetterResult operator [] (const char* key);

  QueryResult operator [] (const char* key) const;

  SetterResult operator [] (const std::string& key);

  QueryResult operator [] (const std::string& key) const;

  SetterResult operator [] (const std::size_t idx);

  QueryResult operator [] (const std::size_t idx) const;

  SetterResult operator [] (const int idx);

  QueryResult operator [] (const int idx) const;

  Value()
    : type(Type::Null) { }

  Value(int i) {
    *this = static_cast<double>(i);
  }

  Value(double d) {
    *this = d;
  }

  Value(std::size_t i) {
    *this = static_cast<double>(i);
  }

  Value(std::string&& str) {
    *this = str;
  }

  Value(const std::string& str) {
    *this = str;
  }

  Value(const char* str) {
    *this = str;
  }

  Value(bool b) {
    *this = b;
  }

  Value(std::nullptr_t val) {
    *this = nullptr;
  }

  Value(Array arr) {
    *this = arr;
  }

  Value(Object obj) {
    *this = obj;
  }

  Value(std::initializer_list<std::pair<const String, Value>> pairs) {
    *this = Object(pairs);
  }

  Value(const Value& other) {
    *this = other;
  }

  Value(Value&& other) {
    *this = other;
  }

  ~Value() {
    cleanup();
  }

  const Value clone() const {
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

  Value& operator = (Object _val) {
    ptr.object = std::make_shared<Object>(_val);
    type = Type::Object;
    return *this;
  }

  Value& operator = (Array _val) {
    ptr.array = std::make_shared<Array>(_val);
    type = Type::Array;
    return *this;
  }

  Value& operator = (String _val) {
    ptr.string = std::make_shared<String>(_val);
    type = Type::String;
    return *this;
  }

  Value& operator = (const char* _str) {
    ptr.string = std::make_shared<String>(_str);
    type = Type::String;
    return *this;
  }

  Value& operator = (Number _val) {
    ptr.number = std::make_shared<Number>(_val);
    type = Type::Number;
    return *this;
  }

  Value& operator = (int _val) {
    ptr.number = std::make_shared<Number>(_val);
    type = Type::Number;
    return *this;
  }

  Value& operator = (Bool _val) {
    ptr.boolean = std::make_shared<Bool>(_val);
    type = Type::Boolean;
    return *this;
  }

  Value& operator = (const Null& _val) {
    ptr.null = nullptr;
    type = Type::Null;
    return *this;
  }

  Value& operator = (std::nullptr_t _val) {
    ptr.null = nullptr;
    type = Type::Null;
    return *this;
  }

  Value& operator = (std::initializer_list<std::pair<const String, Value>> pairs) {
    *this = Object(pairs);
    return *this;
  }

  Value& operator = (const Value& _val) {
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
bool Value::is<Object>() const {
  return type == Type::Object;
}

template <>
bool Value::is<Array>() const {
  return type == Type::Array;
}

template <>
bool Value::is<String>() const {
  return type == Type::String;
}

template <>
bool Value::is<Number>() const {
  return type == Type::Number;
}

template <>
bool Value::is<Bool>() const {
  return type == Type::Boolean;
}

template <>
bool Value::is<Null>() const {
  return type == Type::Null;
}

template <>
Object& Value::as<Object>() {
  if (!is<Object>())
    throw TypeException("Object type assertion failed");
  return *ptr.object;
}

template <>
const Object& Value::as<Object>() const {
  if (!is<Object>())
    throw TypeException("Object type assertion failed");
  return *ptr.object;
}

template <>
const Object& Value::as<const Object&>() const {
  if (!is<Object>())
    throw TypeException("Object type assertion failed");
  return *ptr.object;
}

template <>
Array& Value::as<Array>() {
  if (!is<Array>())
    throw TypeException("Array type assertion failed");
  return *ptr.array;
}

template <>
const Array& Value::as<Array>() const {
  if (!is<Array>())
    throw TypeException("Array type assertion failed");
  return *ptr.array;
}

template <>
String& Value::as<String>() {
  if (!is<String>())
    throw TypeException("String type assertion failed");
  return *ptr.string;
}

template <>
const String& Value::as<String>() const {
  if (!is<String>())
    throw TypeException("String type assertion failed");
  return *ptr.string;
}

template <>
Number& Value::as<Number>() {
  if (!is<Number>())
    throw TypeException("Number type assertion failed");
  return *ptr.number;
}

template <>
const Number& Value::as<Number>() const {
  if (!is<Number>())
    throw TypeException("Number type assertion failed");
  return *ptr.number;
}

template <>
Bool& Value::as<Bool>() {
  if (!is<Bool>())
    throw TypeException("Bool type assertion failed");
  return *ptr.boolean;
}

template <>
const Bool& Value::as<Bool>() const {
  if (!is<Bool>())
    throw TypeException("Bool type assertion failed");
  return *ptr.boolean;
}

template <>
const Null& Value::as<const Null&>() const {
  if (!is<Null>())
    throw TypeException("Null type assertion failed");
  return *ptr.null;
}

template <>
Value& Value::as<Value>() {
  return *this;
}

template <>
const Value& Value::as<Value>() const {
  return *this;
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

// TODO: figure out return types here...
struct QueryResult {
  QueryResult() {}

  QueryResult(const std::list<Key>& keys, const Value& value)
    : _keys(keys), _value(&value)
  {}

  QueryResult(const std::string& key, const Value& value)
    : _value(&value)
  {
    _keys.emplace_back(key);
  }

  QueryResult(std::size_t key, const Value& value)
    : _value(&value)
  {
    _keys.emplace_back(key);
  }

  template <typename Default>
  const Default& defaultToImpl(const Default& d) const {
    return d;
  }

  const String defaultToImpl(const char* d) const {
    return d;
  }

  const Number defaultToImpl(int d) const {
    return d;
  }

  template <typename Default>
  auto defaultTo(const Default& d) const -> typename std::remove_const<decltype(defaultToImpl(d))>::type {
    const Value* root = _value;
    for (const auto& key : _keys) {
      if (key.isString) {
        if (!root->has(key.str))
          return defaultToImpl(d);
        root = &root->lookup(key.str);
      }
      else {
        if (!root->has(key.idx))
          return defaultToImpl(d);
        root = &root->lookup(key.idx);
      }
    }
    return root->as<typename std::remove_const<decltype(defaultToImpl(d))>::type>();
  }

  bool isNull() const {
    return _value == nullptr;
  }

  QueryResult& operator[](const char* key) {
    _keys.emplace_back(key);
    return *this;
  }

  QueryResult& operator[](int idx) {
    _keys.emplace_back(idx);
    return *this;
  }

  template <typename T>
  bool operator == (const T& comp) const {
    if (_value)
      return *_value == comp;
    return comp == Value();
  }

  template <typename T>
  bool operator != (const T& comp) const {
    if (_value)
      return *_value != comp;
    return comp == Value();
  }

  template <typename Type>
  const Type& as() const {
    const Value* root = _value;
    for (const auto& key : _keys) {
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

  operator const Value&() const {
    return as<Value>();
  }

  std::list<Key> _keys;
  const Value* _value = nullptr;
};

struct SetterResult {
  SetterResult(const std::string& key, Value& value)
    : _value(value)
  {
    _keys.emplace_back(key);
  }

  SetterResult(std::size_t key, Value& value)
    : _value(value)
  {
    _keys.emplace_back(key);
  }

  Value& operator = (const Value& set) {
    Value* root = &_value;
    for (const auto& key : _keys) {
      if (key.isString) {
        if (!root->is<Object>())
          *root = Object();
        root = &root->lookup(key.str);
      }
      else {
        if (!root->is<Array>())
          *root = Array();
        auto& arr = root->as<Array>();
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
  auto defaultTo(const Default& d) const -> decltype(QueryResult().defaultToImpl(d)) {
    return QueryResult(_keys, _value).defaultTo(d);
  }

  template <typename Type>
  Type& as() const {
    Value* root = &_value;
    for (const auto& key : _keys) {
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

  operator Value&() {
    return as<Value>();
  }

  operator const Value&() const {
    return as<Value>();
  }

  std::list<Key> _keys;
  Value& _value;
};

template <std::size_t size>
SetterResult Value::operator [] (const char key[size]) {
  return SetterResult(key, *this);
}

template <std::size_t size>
QueryResult Value::operator [] (const char key[size]) const {
  return QueryResult(key, *this);
}

SetterResult Value::operator [] (const char* key) {
  return SetterResult(key, *this);
}

QueryResult Value::operator [] (const char* key) const {
  return QueryResult(key, *this);
}

SetterResult Value::operator [] (const std::string& key) {
  return SetterResult(key, *this);
}

QueryResult Value::operator [] (const std::string& key) const {
  return QueryResult(key, *this);
}

SetterResult Value::operator [] (const std::size_t idx) {
  return SetterResult(idx, *this);
}

QueryResult Value::operator [] (const std::size_t idx) const {
  return QueryResult(idx, *this);
}

SetterResult Value::operator [] (const int idx) {
  return SetterResult(idx, *this);
}

QueryResult Value::operator [] (const int idx) const {
  return QueryResult(idx, *this);
}

bool equivalent(const Value&, const Value&);
bool equivalent(const Object&, const Object&);
bool equivalent(const Array&, const Array&);
bool equivalent(const String&, const String&);
bool equivalent(const Number&, const Number&);
bool equivalent(const Bool&, const Bool&);

bool equivalent(const String& v1, const String& v2) {
  return v1 == v2;
}

bool equivalent(const Number& v1, const Number& v2) {
  return v1 == v2;
}

bool equivalent(const Bool& v1, const Bool& v2) {
  return v1 == v2;
}

bool equivalent(const Array& v1, const Array& v2) {
  if (v1.size() != v2.size())
    return false;

  for (std::size_t i = 0; i < v1.size(); ++i) {
    if (!equivalent(v1[i], v2[i]))
      return false;
  }

  return true;
}

// TODO: improve efficiency here - currently O(n^2) for dictionary comparisons
bool equivalent(const Object& v1, const Object& v2) {
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

bool equivalent(const Value& v1, const Value& v2) {
  if (v1.type != v2.type)
    return false;

  switch (v1.type) {
    case Value::Type::Object:
      return equivalent(v1.as<Object>(), v2.as<Object>());
    case Value::Type::Array:
      return equivalent(v1.as<Array>(), v2.as<Array>());
    case Value::Type::String:
      return equivalent(v1.as<String>(), v2.as<String>());
    case Value::Type::Number:
      return equivalent(v1.as<Number>(), v2.as<Number>());
    case Value::Type::Boolean:
      return equivalent(v1.as<Bool>(), v2.as<Bool>());
    default: {
      return true;
    }
  }
}

bool operator == (const Value& left, const Value& right) {
  return equivalent(left, right);
};

bool operator == (const Null& left, const Null& right) {
  return true;
}

bool operator != (const Value& left, const Value& right) {
  return !(left == right);
};

typedef typename Object::value_type Pair;

std::ostream& operator << (std::ostream& out, const Value& v) {
  OutStream ss(out);
  format(ss, v);
  return out;
}

std::istream& operator >> (std::istream& in, Value& v) {
  InStream ssi(in);
  format(ssi, v);
  return in;
}

std::ostream& operator << (std::ostream& out, const QueryResult& res) {
  OutStream ss(out);
  if (res._value)
    format(ss, res._value);
  else
    format(ss, Value());
  return out;
}

std::ostream& operator << (std::ostream& out, const SetterResult& res) {
  OutStream ss(out);
  Value& v = res.as<Value>();
  format(ss, v);
  return out;
}

std::ostream& operator << (std::ostream& out, const Null& n) {
  return out << "null";
}

}

template <>
struct format_override<json::Value, json::OutStream> {
  template <typename Stream>
  static void format(Stream& out, const json::Value& value) {
    using namespace json;

    switch(value.type) {
      case Value::Type::Object:
        ::format(out, *value.ptr.object);
        break;
      case Value::Type::Array:
        ::format(out, *value.ptr.array);
        break;
      case Value::Type::String:
        ::format(out, *value.ptr.string);
        break;
      case Value::Type::Number:
        ::format(out, *value.ptr.number);
        break;
      case Value::Type::Boolean:
        ::format(out, *value.ptr.boolean);
        break;
      case Value::Type::Null:
        ::format(out, Null());
        break;
    }
  }
};

template <>
struct format_override<json::Value, json::InStream> {
  template <typename Stream>
  static void format(Stream& in, json::Value& value) {
    using namespace json;

    in.good();
    String s;
    ::format(in, s);
    if (in) {
      value = s;
      return;
    }

    in.good();
    Number n;
    ::format(in, n);
    if (in) {
      value = n;
      return;
    }

    in.good();
    Bool b;
    ::format(in, b);
    if (in) {
      value = b;
      return;
    }

    in.good();
    Null nu;
    ::format(in, nu);
    if (in) {
      value = nu;
      return;
    }

    in.good();
    Object ob;
    ::format(in, ob);
    if (in) {
      value = std::move(ob);
      return;
    }

    in.good();
    Array ar;
    ::format(in, ar);
    if (in) {
      value = std::move(ar);
      return;
    }
  }
};

template <>
struct has_key<json::Value> {
  typedef void key_type;
  typedef void mapped_type;
  typedef void value_type;

  const static bool value = false;
};

template <>
struct format_override<json::Null, json::OutStream> {
  template <typename Stream>
  static void format(Stream& out, const json::Null& obj) {
    out << "null";
  }
};

template <>
struct format_override<json::Null, json::InStream> {
  template <typename Stream>
  static void format(Stream& in, json::Null& obj) {
    in.good();
    in >> "null";
  }
};

template <>
struct format_override<json::Bool, json::OutStream> {
  template <typename Stream>
  static void format(Stream& out, const json::Bool& obj) {
    if (obj)
      out << "true";
    else
      out << "false";
  }
};

template <>
struct format_override<json::Bool, json::InStream> {
  template <typename Stream>
  static void format(Stream& in, json::Bool& obj) {
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
