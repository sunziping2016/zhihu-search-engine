#include <iostream>
#include "myvector.h"
#include "mylist.h"

using namespace std;

int main()
{
    mylist<int> i, j;
    i.push_back(1);
    i.push_back(2);
    i.push_back(3);
    i.push_back(4);
    j = i;

    mylist<int>::const_iterator iter;
    iter = j.begin();
    j.erase(iter);

    for (auto &a: j)
        cout << a << endl;
    return 0;
}