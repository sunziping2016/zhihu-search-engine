//
// Created by sun on 11/13/16.
//

#include "mycodecvt.h"

myu32string utf8_to_utf32(const mystring &str, std::size_t index) {
	myu32string result;

	while (index != str.size()) {
		char ch = str[index++];
		if (!(ch & 0x80))
			result.push_back(ch);
		else if ((ch & 0xe0) == 0xc0 && index < str.size()) {
			result.push_back(((ch & 0x1f) << 6) + (str[index] & 0x3f));
			++index;
		}
		else if ((ch & 0xf0) == 0xe0 && index + 1 < str.size()) {
			result.push_back(((ch & 0x0f) << 12) + ((str[index] & 0x3f) << 6) + (str[index + 1] & 0x3f));
			index += 2;
		}
		else if ((ch & 0xf8) == 0xf0 && index + 2 < str.size()) {
			result.push_back(((ch & 0x07) << 18) + ((str[index] & 0x3f) << 12) + ((str[index + 1] & 0x3f) << 6) + (str[index + 2] & 0x3f));
			index += 3;
		}
		else
			break;
	}
	return result;
}

mystring utf32_to_utf8(const myu32string &str, std::size_t index) {
	mystring result;
	while (index != str.size()) {
		char32_t cp = str[index++];
		if (cp < 0x80)
			result.push_back((char)cp);
		else if (cp < 0x800) {
			result.push_back((char)((cp >> 6) | 0xc0));
			result.push_back((char)((cp & 0x3f) | 0x80));
		}
		else if (cp < 0x10000) {
			result.push_back((char)((cp >> 12) | 0xe0));
			result.push_back((char)(((cp >> 6) & 0x3f) | 0x80));
			result.push_back((char)((cp & 0x3f) | 0x80));
		}
		else {
			result.push_back((char)((cp >> 18) | 0xf0));
			result.push_back((char)(((cp >> 12) & 0x3f) | 0x80));
			result.push_back((char)(((cp >> 6) & 0x3f) | 0x80));
			result.push_back((char)((cp & 0x3f) | 0x80));
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
