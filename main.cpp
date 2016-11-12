#include <iostream>
#include <fstream>

#include "mycodecvt.h"
#include "mydir.h"

using namespace std;

int main()
{
    mystring filename = mystring("input/") + *mydir("input").begin();
    cout << filename << endl;
    filename.push_back('\0');
    ifstream file(filename.c_str());
    myu32string text = input_utf8_to_utf32(file);
    for (size_t i = 0; i < text.size(); ++i) {
        myu32string temp;
        temp.push_back(text[i]);
        cout << utf32_to_utf8(temp) << "\t" << text[i] << endl;
    }
    return 0;
}