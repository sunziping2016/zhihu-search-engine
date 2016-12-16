//
// Created by sun on 11/12/16.
//

#ifndef HTML_PARSER_MYPAIR_H
#define HTML_PARSER_MYPAIR_H

#include <ostream>
#include <type_traits>

#include <utility>

//std::pair

template<typename T1, typename T2>
struct mypair {
    typedef T1 first_type;
    typedef T2 second_type;
    first_type  first;
    second_type second;

    mypair() = default;
    mypair(const mypair &p) = default;
    mypair(mypair &&p) = default;
    template<class U1, class U2>
    mypair(const mypair<U1, U2> &p)
            : first(p.first), second(p.second) {}
    template<class U1, class U2>
    mypair(mypair<U1, U2> &&p)
            : first(std::move(p.first)), second(std::move(p.second)) {}
    mypair(const T1 &x, const T2 &y)
            : first(x), second(y) {}
    template<class U1, class U2>
    mypair(U1 &&x, U2 &&y)
            : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {}
};

template <typename T1, typename T2>
inline mypair<typename std::decay<T1>::type, typename std::decay<T2>::type> mymake_pair(T1 &&x, T2 &&y) {
    return mypair<typename std::decay<T1>::type, typename std::decay<T2>::type>(std::forward<T1>(x), std::forward<T2>(y));
}

template<typename CharT, typename T1, typename T2>
std::basic_ostream<CharT> &operator << (std::basic_ostream<CharT> &out, const mypair<T1, T2> &p)
{
    return out << out.widen('(') << p.first << out.widen(',') << out.widen(' ') << p.second << out.widen(')');
};

#endif //HTML_PARSER_MYPAIR_H
