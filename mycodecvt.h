//
// Created by sun on 11/13/16.
//

#ifndef HTML_PARSER_MYCODECVT_H
#define HTML_PARSER_MYCODECVT_H

#include <iostream>
#include "mystring.h"

myu32string utf8_to_utf32(const mystring &str);
mystring utf32_to_utf8(const myu32string &str);

myu32string input_utf8_to_utf32(std::istream &in);
void output_utf32_to_utf8(std::ostream &out, const myu32string &text);


#endif //HTML_PARSER_MYCODECVT_H
