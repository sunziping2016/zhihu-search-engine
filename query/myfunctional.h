//
// Created by sun on 11/2/16.
//

#ifndef HTML_PARSER_MYFUNCTIONAL_H
#define HTML_PARSER_MYFUNCTIONAL_H

#include <cstddef>

template<typename T>
struct myless {
    bool operator () (const T &lhs, const T &rhs) const {
        return lhs < rhs;
    }
};

template<typename T>
struct myequal_to {
    bool operator () (const T &lhs, const T &rhs) const {
        return lhs == rhs;
    }
};


template<typename Key>
struct myhash {
    std::size_t operator () (const Key &x) const {
        return static_cast<std::size_t>(&x);
    }
};

#define TRIVIAL_HASH(Type)                   \
template<> struct myhash<Type> {             \
    std::size_t operator () (Type x) const { \
        return static_cast<std::size_t>(x);  \
    }                                        \
};

template<typename Key>
struct myhash<Key *> {
    std::size_t operator () (Key *x) const {
        return reinterpret_cast<std::size_t>(x);
    }
};

template<typename T>
inline const T &mymax(const T &a, const T &b) {
    return a > b ? a : b;
}

template<typename T>
inline T &mymax(T &a, T &b) {
    return a > b ? a : b;
}

TRIVIAL_HASH(char)
TRIVIAL_HASH(wchar_t)
TRIVIAL_HASH(char16_t)
TRIVIAL_HASH(char32_t)
TRIVIAL_HASH(signed char)
TRIVIAL_HASH(unsigned char)
TRIVIAL_HASH(short)
TRIVIAL_HASH(unsigned short)
TRIVIAL_HASH(int)
TRIVIAL_HASH(unsigned int)
TRIVIAL_HASH(long)
TRIVIAL_HASH(unsigned long)
TRIVIAL_HASH(long long)
TRIVIAL_HASH(unsigned long long)

#endif //HTML_PARSER_MYFUNCTIONAL_H
