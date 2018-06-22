#include <iostream>
#include "testcppjson.h"
#include "../cppjson/include/json/reader.h"
#include "../cppjson/include/json/value.h"
using namespace Json;
int main()
{
    FileDatabase db("../config.json");
    string data;
    bool rst=db.load(data);
    Reader r;
    Value v;
    bool rst1=r.parse(data,v);
    string s=v["signal_machine_ip"].asString();
    cout<<s<<endl;
    return 0;
}
