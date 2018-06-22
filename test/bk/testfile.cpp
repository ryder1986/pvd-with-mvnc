
#include <iostream>
#include  "../filedatabase.h"
using namespace std ;
int main()
{
    cout << "test:"<<endl;
    string abc="123499";
    FileDatabase f("tmp3.txt");
    f.save(abc);
    string ff;
    f.load(ff);
    cout << "load:"<<ff<<endl;
    return 1;
}
