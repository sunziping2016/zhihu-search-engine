#include <iostream>
#include <fstream>

#include "mycodecvt.h"
#include "mydir.h"
#include "htmlparser.h"

using namespace std;

int main()
{
    for (auto filename:  mydir("input")) {
        cout << "Parsing " << filename << endl;
        ifstream file(("input/" + filename).c_str());
        html_dom dom(input_utf8_to_utf32(file));
        for (auto i: dom.find_all("p").find_all("strong").html())
            cout << utf32_to_utf8(i) << endl;
    }
    return 0;
}