//
// Created by sun on 10/24/16.
//

#ifndef HTML_PARSER_MYSTRING_H
#define HTML_PARSER_MYSTRING_H

#include <iostream>

#include "myvector.h"
#include "myfunctional.h"

template<typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT> >
class mybasic_string: public myvector<CharT> {
public:
    typedef mybasic_string<CharT, Traits, Allocator> string_type;
    typedef myvector<CharT>                          vector_type;

    mybasic_string() = default;
    mybasic_string(const string_type &other) = default;
    string_type &operator = (const string_type &other) = default;
    mybasic_string(string_type &&other) = default;
    string_type &operator = (string_type &&other) = default;
    template <class InputIterator>
    mybasic_string(InputIterator first, InputIterator last) : vector_type(first, last) {}
    template<typename CharT2>
    mybasic_string(const CharT2 *s) {
        while (*s)
            this->push_back(*s++);
    }
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

    std::size_t find(const string_type &str, std::size_t pos = 0) const {
        if (str.empty())
            return 0;
        int *pre_array = new int[str.size()];
        pre_array[0] = -1;
        for (std::size_t i = 1; i < str.size(); ++i) {
            int k = pre_array[i - 1];
            while (k >= 0 && pre_array[k] != pre_array[i - 1])
                k = pre_array[k];
            pre_array[i] = k + 1;
        }
        int k = 0;
        while (pos < this->size()) {
            if (k == -1) {
                ++pos;
                k = 0;
            } else if ((*this)[pos] == str[k]) {
                ++pos;
                ++k;
                if (static_cast<std::size_t>(k) == str.size()) {
                    return pos - str.size();
                }
            } else
                k = pre_array[k];
        }
        return this->size();
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

template<typename CharT, typename Traits, typename Allocator>
std::basic_ostream<CharT> &operator << (std::basic_ostream<CharT> &out, const mybasic_string<CharT, Traits, Allocator> &str)
{
    mybasic_string<char, Traits, Allocator> copy(str);
    out << copy.c_str();
    return out;
};

template<typename CharT, typename Traits, typename Allocator>
std::basic_ostream<CharT> &operator << (std::basic_ostream<CharT> &out, mybasic_string<CharT, Traits, Allocator> &&str)
{
    mybasic_string<char, Traits, Allocator> copy(std::move(str));
    out << copy.c_str();
    return out;
};


template<typename CharT, typename Traits, typename Allocator>
std::basic_istream<CharT, Traits> &getline(std::basic_istream<CharT, Traits> &in, mybasic_string<CharT, Traits, Allocator> &str, CharT delim) {
    int ch;
    str.clear();
    while ((ch = in.get()) != EOF && ch != delim)
        str.push_back(static_cast<char>(ch));
    return in;
};

template<typename CharT, typename Traits, typename Allocator> struct myhash<mybasic_string<CharT, Traits, Allocator> > {
    std::size_t operator () (const mybasic_string<CharT, Traits, Allocator> &str) const {
        std::size_t value = 0, step = str.size() / 5;
        if (step == 0)
            step = 1;
        for (std::size_t i = 0; i < str.size(); i += step)
            value = 37 * value + str[i];
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
    str.clear();
    while ((ch = in.get()) != EOF)
        str.push_back(static_cast<char>(ch));
    return in;
};

typedef mybasic_string<char>     mystring;
typedef mybasic_string<wchar_t>  mywstring;
typedef mybasic_string<char16_t> myu16string;
typedef mybasic_string<char32_t> myu32string;

inline bool isspace32(char32_t ch) {
	return ch == ' ' || (ch >= '\t' && ch <= '\r');
}

inline bool isdigit32(char32_t ch) {
	return ch >= '0' && ch <= '9';
}

inline bool isupper32(char32_t ch) {
    return ch >= 'A' && ch <= 'Z';
}

inline bool isgraph32(char32_t ch) {
    return ch >= '!' && ch <= '~';
}

inline char32_t tolower32(char32_t ch) {
	return isupper32(ch) ? ch - 'A' + 'a' : ch;
}

#endif //HTML_PARSER_MYSTRING_H
