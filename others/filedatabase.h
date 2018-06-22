#ifndef FILEDATABASE_H
#define FILEDATABASE_H
#include <QObject>
#include <QFile>

#include "tool.h"
#
class FileDatabase
{
private:
//    void save(QByteArray data)
//    {
//        QFile *f=new QFile(name);
//        bool ret = f->open(QIODevice::ReadWrite|QIODevice::Truncate);
//        if(!ret){
//            prt(info,"fail to open %s",name.toStdString().data());
//            delete f;
//        }
//        f->write(data);
//        f->close();
//    }
//    bool load(QByteArray &data)
//    {

//        QFile *f=new QFile(name);
//        bool ret = f->open(QIODevice::ReadOnly);
//        if(!ret){
//            delete f;
//            return ret;
//        }
//        data=f->readAll();
//        f->close();
//        return ret;
//    }
#if 0
bool save_file(string data)
{
    QFile *f=new QFile(QString(name.data()));
    bool ret = f->open(QIODevice::ReadWrite|QIODevice::Truncate);
    if(!ret){
        prt(info,"fail to write %s",name.data());
        delete f;
        return false;
    }else{
        prt(info,"ok to write %s",name.data());
    }
    QByteArray ba(data.data(),data.size());
    f->write(ba);
    f->close();
}
bool load_file(string &data)
{

    QFile *f=new QFile(QString(name.data()));
    bool ret = f->open(QIODevice::ReadOnly);
    if(!ret){
        delete f;
        prt(info,"fail to load %s",name.data());
        return false;
    }else{
        prt(info,"ok to load %s",name.data());
    }
    QByteArray ba=f->readAll();
    data.append(ba.data());
    f->close();
    return ret;
}
#else
void save_file(const string data)
{
    FILE *f=fopen(name.data(),"wb");
    int comp=fwrite(data.data(),1,data.size(),f);
    if(comp!=data.size())
        prt(info,"error in saving file");
    fclose(f);
}
bool load_file(string &data)
{
    data.clear();
    FILE *f=fopen(name.data(),"rb");
    if(f==NULL)
        return false;
    static char ss[2];
    while(fread(ss,1,1,f)==1)
    {
        ss[1]=0;
        data.append(ss);
    }
    fclose(f);
    return true;
}
#endif
public:
FileDatabase(string file_name);
//    void save(QJsonObject node)
//    {
//        QJsonDocument json_doc_new(node);
//        save(json_doc_new.toJson());
//    }
//    void load(QJsonObject &node)
//    {
//        QByteArray ba;
//        QJsonDocument json_doc_new;
//        if(load(ba)){
//            json_doc_new=QJsonDocument::fromJson(ba);
//            node=json_doc_new.object();
//        }else{
//        }
//    }


void save(string json_data)
{

    save_file(json_data);
}
bool load(string &json_data)
{
    return    load_file(json_data);

}
private:
string name;
string config;
};

#endif // FILEDATABASE_H
