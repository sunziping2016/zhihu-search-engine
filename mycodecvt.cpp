//
// Created by sun on 11/13/16.
//

#include "mycodecvt.h"

myu32string utf8_to_utf32(const mystring &str, std::size_t index) {
    myu32string result;

    while (index != str.size()) {
        std::size_t remunits;
        char nxt, msk;
        char32_t cp;
        nxt = str[index++];
        if (nxt & 0x80) {
            msk = (char) 0xe0;
            for (remunits = 1; (nxt & msk) != (msk << 1); ++remunits)
                msk = (char) ((msk >> 1) | 0x80);
        }
        else {
            remunits = 0;
            msk = 0;
        }
        cp = (char32_t) (nxt & ~msk);
        while (remunits-- > 0 && index != str.size()) {
            cp <<= 6;
            cp |= str[index++] & 0x3f;
        }
        result.push_back(cp);
    }

    return result;
}

mystring utf32_to_utf8(const myu32string &str, std::size_t index) {
    mystring result;
    while (index != str.size()) {
        char32_t cp = str[index++];
        if (cp < 0x7f)
            result.push_back((char) cp);
        else if (cp < 0x7ff) {
            result.push_back((char) ((cp >> 6) | 0xc0));
            result.push_back((char) ((cp & 0x3f) | 0x80));
        }
        else if (cp < 0xffff) {
            result.push_back((char) ((cp >> 12) | 0xe0));
            result.push_back((char) (((cp >> 6) & 0x3f) | 0x80));
            result.push_back((char) ((cp & 0x3f) | 0x80));
        }
        else {
            result.push_back((char) ((cp >> 18) | 0xf0));
            result.push_back((char) (((cp >> 12) & 0x3f) | 0x80));
            result.push_back((char) (((cp >> 6) & 0x3f) | 0x80));
            result.push_back((char) ((cp & 0x3f) | 0x80));

        }
    }
    return result;
}

myu32string input_utf8_to_utf32(std::istream &in)
{
    mystring temp;
    getall(in, temp);
    if (temp.size() >= 3 && temp[0] == '\xef' && temp[1] == '\xbb' && temp[2] == '\xbf') // BOM character
        return utf8_to_utf32(temp, 3);
    return utf8_to_utf32(temp);
}
void output_utf32_to_utf8(std::ostream &out, const myu32string &text)
{
    out << utf32_to_utf8(text);
}
