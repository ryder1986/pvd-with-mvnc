#include "filedatabase.h"
#include "tool.h"
FileDatabase::FileDatabase(string file_name)
{
    name=file_name;
    config.clear();
    bool ret=load(config);
    if(ret){
        //prt(info,"open profile %s ok\ncontent:\n%s",name.toStdString().data(),config.toStdString().data());
    }else{
        prt(info,"open profile %s fail",name.data());
    }
}

