#pragma once

#include <serializer/core.h>
#include <iterator>
#include <ostream>

namespace json {

struct OutStream {
  OutStream() : buffer(std::cout) { }
  OutStream(std::ostream& buffer_) : buffer(buffer_) { }

  std::ostream& buffer;
};

template <typename T>
OutStream& operator << (OutStream& out, const T& obj) {
  out.buffer << obj;
  return out;
}

struct InStream {
  std::istringstream input_stream;
  std::istream& buffer;

  operator bool() { return buffer; }
  void good() { buffer.clear(); }

  InStream(const std::string& contents) : input_stream(contents), buffer(input_stream) { }
  InStream(std::istream& input) : buffer(input) {}
};

template <typename T>
InStream& operator >> (InStream& in, T& obj) {
  in.buffer >> obj;
  return in;
}

InStream& operator >> (InStream& in, std::string& obj) {
  char c;
  bool escape = false;
  while(in.buffer >> std::noskipws >> c) {
    if (!escape && c == '"') {
      in.buffer.putback(c);
      break;
    }
    obj += c;
    escape = (c == '\\' && !escape);
  }
  in.buffer >> std::skipws;
  return in;
}

InStream& operator >> (InStream& in, const char* str) {
  std::size_t len = std::strlen(str);
  std::size_t count = 0;
  char c;

  auto pos = in.buffer.tellg();

  while(count < len && in.buffer >> c) {
    if (std::isspace(c))
      continue;

    if (c != str[count])
      break;
    ++count;
  }
  if (count != len) {
    in.buffer.seekg(pos);
    in.buffer.setstate(std::ios_base::badbit);
  }

  return in;
}

}

template <typename U>
struct formatter<U, json::OutStream> {
  template <typename T, typename Stream>
  static auto format_impl(Stream& out, const T& t) -> typename std::enable_if<has_range<T>::value && not has_key<T>::value>::type {
    const char* sep = "";
    out << "[";
    for (const auto& itr : t) {
      out << sep;
      format(out, itr);
      sep = ",";
    }
    out << "]";
  }

  template <typename T, typename Stream>
  static auto format_impl(Stream& out, const T& t) -> typename std::enable_if<has_key<T>::value && !has_format_override<typename has_key<T>::value_type, Stream>::value>::type {
    const char* sep = "";
    out << "{";
    for (const auto& itr : t) {
      out << sep;
      format(out, getKey(itr));
      out << ":";
      format(out, getValue(itr));
      sep = ",";
    }
    out << "}";
  }

  template <typename T, typename Stream>
  static auto format_impl(Stream& out, const T& t) -> typename std::enable_if<has_key<T>::value && has_format_override<typename has_key<T>::value_type, Stream>::value>::type {
    const char* sep = "";
    out << "{";
    for (const auto& itr : t) {
      out << sep;
      format(out, itr);
      sep = ",";
    }
    out << "}";
  }

  template <typename T, typename Stream>
  static auto format_impl(Stream& out, const T& t) -> typename std::enable_if<!(has_range<T>::value || has_key<T>::value)>::type {
    out << t;
  }

  template <typename T, typename Stream>
  static auto format(Stream& out, const T& t) -> typename std::enable_if<has_format_override<T, Stream>::value>::type {
    format_override<T, Stream>::format(out, t);
  }

  template <typename T, typename Stream>
  static auto format(Stream& out, const T& t) -> typename std::enable_if<!has_format_override<T, Stream>::value>::type {
    format_impl(out, t);
  }
};

template <typename U>
struct formatter<U, json::InStream> {
  template <typename T, typename Stream>
  static auto format_impl(Stream& out, T& t) -> typename std::enable_if<has_range<T>::value && not has_key<T>::value, bool>::type {
    typedef typename has_range<T>::value_type value_type;
    auto itr = std::inserter(t, t.begin());

    if (!(out >> "["))
      return false;

    while(true) {
      value_type obj;
      if (!format(out, obj))
        break;
      *itr = obj;
      if (!(out >> ","))
        break;
    }

    out.good();
    if (!(out >> "]"))
      return false;

    return out;
  }

  template <typename T, typename Stream>
  static auto format_impl(Stream& out, T& t) -> typename std::enable_if<has_key<T>::value && !has_format_override<typename has_key<T>::value_type, Stream>::value, bool>::type {
    typedef typename has_key<T>::key_type key_type;
    typedef typename has_key<T>::mapped_type mapped_type;
    typedef typename has_key<T>::value_type value_type;
    auto itr = std::inserter(t, t.begin());

    if (!(out >> "{"))
      return false;

    while(true) {
      key_type key;
      mapped_type value;
      if (!format(out, key))
        break;
      if (!(out >> ":"))
        break;
      if (!format(out, value))
        break;
      *itr = value_type(key, value);

      if (!(out >> ","))
        break;
    }

    out.good();
    if (!(out >> "}"))
      return false;

    return out;
  }

  template <typename T, typename Stream>
  static auto format_impl(Stream& out, T& t) -> typename std::enable_if<has_key<T>::value && has_format_override<typename has_key<T>::value_type, Stream>::value, bool>::type {
    typedef typename has_key<T>::value_type value_type;
    auto itr = std::inserter(t, t.begin());

    if (!(out >> "{"))
      return false;

    while(true) {
      value_type obj;
      if (!format(out, obj))
        break;
      *itr = obj;

      if (!(out >> ","))
        break;
    }

    out.good();
    if (!(out >> "}"))
      return false;

    return out;
  }

  template <typename T, typename Stream>
  static auto format_impl(Stream& out, T& t) -> typename std::enable_if<!(has_range<T>::value || has_key<T>::value), bool>::type {
    out >> t;
    return out;
  }

  template <typename T, typename Stream>
  static auto format(Stream& out, T& t) -> typename std::enable_if<has_format_override<T, Stream>::value, bool>::type {
    format_override<T, Stream>::format(out, t);
    return out;
  }

  template <typename T, typename Stream>
  static auto format(Stream& out, T& t) -> typename std::enable_if<!has_format_override<T, Stream>::value, bool>::type {
    format_impl(out, t);
    return out;
  }
};

template <>
struct format_override<std::string, json::OutStream> {
  template <typename Stream>
  static void format(Stream& out, const std::string& obj) {
    out << "\"" << obj << "\"";
  }
};

template <>
struct format_override<std::string, json::InStream> {
  template <typename Stream>
  static void format(Stream& out, std::string& obj) {
    out >> "\"" >> obj >> "\"";
  }
};
