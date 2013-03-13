#include <serializer/string_escaper.h>

#include <boost/spirit/home/karma.hpp>
#include <string>

using namespace boost::spirit;

// http://boost-spirit.com/home/articles/karma-examples/generate-escaped-string-output-using-spirit-karma/

template <typename OutputIterator>
struct escaped_string : karma::grammar<OutputIterator, std::string(char const*)>
{
    escaped_string() : escaped_string::base_type(esc_str)
    {
        esc_char.add('\a', "\\a")('\b', "\\b")('\f', "\\f")('\n', "\\n")
                    ('\r', "\\r")('\t', "\\t")('\v', "\\v")('\\', "\\\\")
                    ('\'', "\\\'")('"', "\\\"")
            ;
        esc_str =   karma::lit(karma::_r1)
                << *(esc_char | karma::print | "\\x" << karma::hex)
                <<  karma::lit(karma::_r1)
            ;
    }
    karma::rule<OutputIterator, std::string(char const*)> esc_str;
    karma::symbols<char, char const*> esc_char;
};

std::string escape_string(const std::string& input) {
  typedef std::back_insert_iterator<std::string> sink_type;
 
  std::string generated;
  sink_type sink(generated);

  char const* quote = "";
   
  escaped_string<sink_type> g;
  karma::generate(sink, g(quote), input);

  return generated;
}
