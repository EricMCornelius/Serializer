#pragma once

#include <serializer/core.h>
#include <iterator>
#include <ostream>
#include <iomanip>
#include <istream>
#include <limits>

namespace json {

struct LiteralWrapper {
  template <std::size_t size_>
  constexpr LiteralWrapper(const char (&str_)[size_])
    : str(str_), size(size_ - 1) { }

  const char* str;
  std::size_t size;
};

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

  operator bool() { return static_cast<bool>(buffer); }
  void good() { buffer.clear(); }
  void bad() { buffer.setstate(std::ios_base::badbit); }

  template <typename T>
  InStream& trim(T&& obj) {
    buffer >> std::ws;
    *this >> obj;
    buffer >> std::ws;
    return *this;
  }

  std::string remaining() {
    auto pos = buffer.tellg();
    std::stringstream str;
    str << buffer.rdbuf();
    buffer.good();
    buffer.seekg(pos, buffer.beg);
    return str.str();
  }

  InStream(const std::string& contents) : input_stream(contents), buffer(input_stream) { }
  InStream(std::istream& input) : buffer(input) {}
};

template <typename T>
InStream& operator >> (InStream& in, T& obj) {
  auto pos = in.buffer.tellg();
  in.buffer >> obj;
  if (!in) {
    in.good();
    in.buffer.seekg(pos);
    in.bad();
  }
  return in;
}

inline InStream& operator >> (InStream& in, std::string& obj) {
  bool escape = false;
  char c = in.buffer.get();
  while (in) {
    if (!escape && c == '"') {
      in.buffer.putback(c);
      break;
    }
    obj += c;
    escape = (c == '\\' && !escape);
    c = in.buffer.get();
  }
  return in;
}

inline InStream& operator >> (InStream& in, const char* str) {
  char buf[4096];

  auto len = std::strlen(str);
  auto count = in.buffer.readsome(&buf[0], len);

  buf[count] = '\0';

  if (count != len) {
    for (auto c = count - 1; c >= 0; --c)
      in.buffer.putback(buf[c]);
    in.bad();
    return in;
  }

  for (auto idx = 0u; idx < count; ++idx) {
    if (buf[idx] != str[idx]) {
      for (auto c = count - 1; c >= 0; --c)
        in.buffer.putback(buf[c]);
      in.bad();
      return in;
    }
  }

  return in;
}

inline InStream& operator >> (InStream& in, char c) {
  auto n = in.buffer.get();
  if (n == c) {
    return in;
  }
  in.buffer.putback(n);
  in.bad();
  return in;
}

inline InStream& operator >> (InStream& in, const LiteralWrapper& lit) {
  char buf[4096];
  auto count = in.buffer.readsome(&buf[0], lit.size);

  if (count != lit.size) {
    for (auto c = count - 1; c >= 0; --c)
      in.buffer.putback(buf[c]);
    in.bad();
    return in;
  }

  for (auto idx = 0u; idx < count; ++idx) {
    if (buf[idx] != lit.str[idx]) {
      for (auto c = count - 1; c >= 0; --c)
        in.buffer.putback(buf[c]);
      in.bad();
      return in;
    }
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

    if (!(out.trim('[')))
      return false;

    do {
      value_type obj;
      if (format(out, obj))
        *itr = obj;
    }
    while(out.trim(','));

    out.good();
    out.trim(']');
    return out;
  }

  template <typename T, typename Stream>
  static auto format_impl(Stream& out, T& t) -> typename std::enable_if<has_key<T>::value && !has_format_override<typename has_key<T>::value_type, Stream>::value, bool>::type {
    typedef typename has_key<T>::key_type key_type;
    typedef typename has_key<T>::mapped_type mapped_type;
    typedef typename has_key<T>::value_type value_type;
    auto itr = std::inserter(t, t.begin());

    if (!(out.trim('{')))
      return false;

    do {
      key_type key;
      mapped_type value;
      format(out, key);
      out.trim(':');
      format(out, value);

      if (!out)
        break;

      *itr = value_type(key, value);
    }
    while(out.trim(','));

    out.good();
    out.trim('}');
    return out;
  }

  template <typename T, typename Stream>
  static auto format_impl(Stream& out, T& t) -> typename std::enable_if<has_key<T>::value && has_format_override<typename has_key<T>::value_type, Stream>::value, bool>::type {
    typedef typename has_key<T>::value_type value_type;
    auto itr = std::inserter(t, t.begin());

    if (!(out.trim('{')))
      return false;

    do {
      value_type obj;
      if (format(out, obj))
        *itr = obj;
    }
    while(out.trim(','));

    out.good();
    out.trim('}');
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
struct format_override<double, json::OutStream> {
  template <typename Stream>
  static void format(Stream& out, double val) {
    // format as an integer if there is no decimal part
    if (val == (int64_t) val)
      out << std::fixed << (int64_t) val;
    else
      out << std::setprecision(std::numeric_limits<double>::digits10) << val;
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
    out >> '"' >> obj >> '"';
  }
};
