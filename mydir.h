//
// Created by sun on 11/13/16.
//

#ifndef HTML_PARSER_MYDIR_H
#define HTML_PARSER_MYDIR_H

#include "mylist.h"
#include "mystring.h"

mylist<mystring> mydir(const mystring &directory);

#if defined (__unix__) || defined (__APPLE__)
#define PATH_SEPARATOR "/"
#elif defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#error "Unknown system."
#endif

#endif //HTML_PARSER_MYDIR_H
