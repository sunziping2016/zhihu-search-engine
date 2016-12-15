//
// Created by sun on 11/14/16.
//

#ifndef HTML_PARSER_HTMLDATA_H
#define HTML_PARSER_HTMLDATA_H

#include <cstddef>
#include "myhashset.h"
#include "myhashmap.h"
#include "mystring.h"

extern const myhashset<myu32string> void_elements;
extern const myhashmap<myu32string, char32_t> escape_string;

#endif //HTML_PARSER_HTMLDATA_
