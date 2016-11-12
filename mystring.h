//
// Created by sun on 10/24/16.
//

#ifndef HTML_PARSER_MYSTRING_H
#define HTML_PARSER_MYSTRING_H

#include <cctype> // for isspace

#include "myvector.h"
#include "myfunctional.h"

template<typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT> >
class mybasic_string: public myvector<CharT> {
public:
    typedef mybasic_string<CharT> string_type;
    typedef myvector<CharT>       vector_type;

    mybasic_string() {}
    mybasic_string(const CharT *s) {
        while (*s)
            this->push_back(*s++);
    }
    mybasic_string(const string_type &other) : vector_type(other) {}
    string_type &operator += (const string_type &x) {
        this->insert(this->end(), x.begin(), x.end());
        return *this;
    }
    string_type operator + (const string_type &x) const {
        string_type temp(*this);
        temp.insert(temp.end(), x.begin(), x.end());
        return temp;
    }

};

template<typename CharT1, typename CharT2, typename Traits, typename Allocator>
std::basic_ostream<CharT1> &operator << (std::basic_ostream<CharT1> &out, const mybasic_string<CharT2, Traits, Allocator> str)
{
    for (typename mybasic_string<CharT2, Traits, Allocator>::const_iterator iter = str.cbegin(); iter < str.cend(); ++iter)
        out << *iter;
    return out;
};

template<typename CharT, typename Traits, typename Allocator>
std::basic_istream<CharT, Traits> &getline(std::basic_istream<CharT, Traits> &in, mybasic_string<CharT, Traits, Allocator> &str, CharT delim) {
    int ch;
    str.clear();
    while ((ch = in.get()) != EOF && isspace(ch));
    if (ch != EOF) {
        str.push_back(static_cast<CharT>(ch));
        while ((ch = in.get()) != EOF && ch != delim)
            str.push_back(ch);
    }
    return in;
};

template<typename CharT, typename Traits, typename Allocator> struct myhash<mybasic_string<CharT, Traits, Allocator> > {
    std::size_t operator () (const mybasic_string<CharT, Traits, Allocator> &str) const {
        std::size_t value = 0;
        for (typename mybasic_string<CharT, Traits, Allocator>::const_iterator iter = str.cbegin(); iter < str.cend(); ++iter)
            value = 37 * value + *iter;
        return value;
    }
};


template<typename CharT, typename Traits, typename Allocator>
std::basic_istream<CharT, Traits> &getline(std::basic_istream<CharT, Traits> &in, mybasic_string<CharT, Traits, Allocator> &str) {
    return getline(in, str, in.widen('\n'));
};

typedef mybasic_string<char> mystring;
typedef mybasic_string<wchar_t> mywstring;

#endif //HTML_PARSER_MYSTRING_H
