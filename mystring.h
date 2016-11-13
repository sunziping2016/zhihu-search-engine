//
// Created by sun on 10/24/16.
//

#ifndef HTML_PARSER_MYSTRING_H
#define HTML_PARSER_MYSTRING_H

#include "myvector.h"
#include "myfunctional.h"

template<typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT> >
class mybasic_string: public myvector<CharT> {
public:
    typedef mybasic_string<CharT> string_type;
    typedef myvector<CharT>       vector_type;

    mybasic_string() {}
    template <class InputIterator>
    mybasic_string(InputIterator first, InputIterator last) : vector_type(first, last) {}
    template<typename CharT2>
    mybasic_string(const CharT2 *s) {
        while (*s)
            this->push_back(*s++);
    }
    mybasic_string(const string_type &other) : vector_type(other) {}
    string_type &operator += (const string_type &x) {
        this->insert(this->end(), x.begin(), x.end());
        return *this;
    }

    typename vector_type::const_pointer c_str() {
        this->push_back('\0');
        this->pop_back();
        return this->cbegin();
    }

    string_type substr(size_t pos, size_t count) const {
        return string_type(this->begin() + pos, this->begin() + pos + count);
    }
};

template<typename CharT>
mybasic_string<CharT> operator + (const mybasic_string<CharT> &lhs, const mybasic_string<CharT> &rhs) {
    mybasic_string<CharT> temp(lhs);
    temp.insert(temp.end(), rhs.begin(), rhs.end());
    return temp;
}

template<typename CharT1, typename CharT2>
mybasic_string<CharT2> operator + (const CharT1 *lhs, const mybasic_string<CharT2> &rhs) {
    return mybasic_string<CharT2>(lhs) + rhs;
}

template<typename CharT1, typename CharT2>
mybasic_string<CharT1> operator + (const mybasic_string<CharT1> &lhs, const CharT2 *rhs) {
    return lhs + mybasic_string<CharT1>(rhs);
}

template<typename CharT1, typename CharT2>
bool operator == (const CharT1 *lhs, const mybasic_string<CharT2> &rhs) {
    return mybasic_string<CharT2>(lhs) == rhs;
}

template<typename CharT1, typename CharT2>
bool operator == (const mybasic_string<CharT1> &lhs, const CharT2 *rhs) {
    return lhs == mybasic_string<CharT1>(rhs);
}


template<typename CharT1, typename CharT2>
bool operator != (const CharT1 *lhs, const mybasic_string<CharT2> &rhs) {
    return mybasic_string<CharT2>(lhs) != rhs;
}

template<typename CharT1, typename CharT2>
bool operator != (const mybasic_string<CharT1> &lhs, const CharT2 *rhs) {
    return lhs != mybasic_string<CharT1>(rhs);
}


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
template<typename CharT, typename Traits, typename Allocator>
std::basic_istream<CharT, Traits> &getall(std::basic_istream<CharT, Traits> &in, mybasic_string<CharT, Traits, Allocator> &str) {
    int ch;
    while ((ch = in.get()) != EOF)
        str.push_back(ch);
    return in;
};

typedef mybasic_string<char>     mystring;
typedef mybasic_string<wchar_t>  mywstring;
typedef mybasic_string<char16_t> myu16string;
typedef mybasic_string<char32_t> myu32string;

#endif //HTML_PARSER_MYSTRING_H
