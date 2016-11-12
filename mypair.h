//
// Created by sun on 11/12/16.
//

#ifndef HTML_PARSER_MYPAIR_H
#define HTML_PARSER_MYPAIR_H

#include <ostream>

template<typename T1, typename T2>
struct mypair {
    typedef T1 first_type;
    typedef T2 second_type;
    first_type  first;
    second_type second;

    mypair() {}
    template<class U1, class U2>
    mypair(const mypair<U1, U2> &p)
            : first(p.first), second(p.second) {}
    template<class U1, class U2>
    mypair(const U1 &x, const U2 &y)
            : first(x), second(y) {}
};

template<typename T1, typename T2 >
mypair<T1, T2> mymake_pair(T1 t, T2 u) { // not using decay to be compatible with c++98
    return mypair<T1, T2>(t, u);
};

template<typename CharT, typename T1, typename T2>
std::basic_ostream<CharT> &operator << (std::basic_ostream<CharT> &out, const mypair<T1, T2> &p)
{
    return out << out.widen('(') << p.first << out.widen(',') << out.widen(' ') << p.second << out.widen(')');
};

#endif //HTML_PARSER_MYPAIR_H
