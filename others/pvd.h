#ifndef PVD_H
#define PVD_H
#include <cstring>
#include <QtCore>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <json/json.h>
#include <json/value.h>
using namespace std;
#if 0
//typedef  QString string_t;
typedef  QJsonValue JsonValue;
#else
//typedef  string string_t;
using  namespace Json ;
typedef  Value JsonValue;

#endif
class Pvd{
public:
    enum LENGTH_FIXED_VALUE{
        STR_LENGTH=100,
        PATH_LENGTH=1000,
        BUFFER_LENGTH=1000,
        BUFFER_MAX_LENGTH=3000
    };
    enum CMD{
        GET_CONFIG=1,
        SET_CONFIG,
        INSERT_CAMERA,
        DELETE_CAMERA,
        MOD_CAMERA_ATTR,
        MOD_CAMERA_ALG,
        CAM_OUTPUT_OPEN,
        CAM_OUTPUT_CLOSE,
        HEART,
        REBOOT,
        MOD_DEVICE_ATTR
    };
    enum RETURN{
        OK=0,
        PARSE_ERROR,
        NEED_UPDATE,
        INVALID_VALUE
    };
    enum PORTS{
        SERVER_PORT=12345,
        SERVER_DATA_OUTPUT_PORT=12346,
        CLIENT_REPORTER_PORT=12347,
        SERVER_REPORTER_PORT=12348
    };
};
#if 0
class DataPacket{
public:
    DataPacket(string data)
    {
        QJsonDocument doc=QJsonDocument::fromJson(QByteArray(data.data(),data.size()));
        jv=doc.object();
    }
    DataPacket()
    {

    }
    DataPacket(JsonValue v)
    {
        jv=v;
    }
    string data()
    {
        QJsonObject obj=jv.toObject();
        QJsonDocument doc(obj);
        return doc.toJson().data();
    }
 //   template <typename tp>
//    void set_value(QString name,tp value)
//    {
//        QJsonObject obj=jv.toObject();
//        obj[name]=value;
//        jv=obj;

//    }
//    void set_string(QString name,QString value)
//    {
//        QJsonObject obj=jv.toObject();
//        obj[name]=value;
//        jv=obj;

//    }
    DataPacket(vector<JsonValue> v)
    {
        QJsonArray ar;
        int i=0;
        int sz=v.size();
        for(;i<sz;i++){
            ar.append(v[i]);
        }

        QJsonObject obj=jv.toObject();

        jv=ar;
    }

    void set_array(QString name,vector<JsonValue> v)
    {
        QJsonArray ar;
        int i=0;
        int sz=v.size();
        for(;i<sz;i++){
            ar.append(v[i]);
        }

        QJsonObject obj=jv.toObject();
        obj[name]=ar;
        jv=obj;
    }
    vector<JsonValue> get_array(QString name)
    {
        QJsonArray ar= get_value(name).toArray();
        vector<JsonValue> v;


        int i=0;
        int sz=ar.size();
        for(;i<sz;i++){
            v.push_back(ar[i]);
        }

        return v;
    }
    int get_int(QString name)
    {
        return get_value(name).toInt();
    }


    bool get_bool(QString name)
    {
        return get_value(name).toBool();
    }


    string get_string(QString name)
    {
      //  return string(get_value(name).toString().toStdString().data());
        return string(get_value(name).toString().toUtf8());
    }

    JsonValue get_value(QString name)
    {
        QJsonObject obj=jv.toObject();
        return obj[name];
    }



    void set_int(QString name,int v)
    {
        QJsonObject obj=jv.toObject();

        obj[name]=v;
        jv=obj;
    }


    void  set_bool(QString name ,bool v)
    {
        QJsonObject obj=jv.toObject();

        obj[name]=v;
        jv=obj;
    }


    void set_string(QString name,string v)
    {
        QJsonObject obj=jv.toObject();

        obj[name]=QString(v.data());
        jv=obj;
    }

    void set_value(QString name,JsonValue v)
    {
        QJsonObject obj=jv.toObject();

        obj[name]=v;
        jv=obj;
    }




    vector <JsonValue>  array_value()
    {
        int sz=jv.toArray().size();
        vector <JsonValue> vec;
        for(int i=0;i<sz;i++)
        {
            vec.push_back(jv.toArray()[i]);
        }
        return vec;
    }


    JsonValue value()
    {
        return jv;
    }
private:

    //  QJsonObject obj;
    JsonValue jv;
};
//    char *str2utf8str(string_t str)
//    {
//         return str.toUtf8().data();
//    }

#else

class DataPacket{
public:
    DataPacket(string json_data)
    {
        JsonValue v;
        Reader r;
        bool rst=r.parse(json_data,v);
        val=v;
    }
    DataPacket(JsonValue v)
    {
        val = v;
    }
    DataPacket()
    {
    }
    DataPacket(  vector<JsonValue>   ar)
    {
        JsonValue v;
        int sz=ar.size();
        for(int i=0;i<sz;i++){
            v[i]=ar[i];
        }
        val=v;
    }
//    template <typename tp>
//    void set_value(string name,tp value)
//    {
//        val[name]=value;
//    }



    void set_int(string name,int v)
    {


        val[name]=v;

    }


    void  set_bool(string name ,bool v)
    {

        val[name]=v;
    }


    void set_string(string name,string v)
    {
        val[name]=v;
    }

        void set_value(string name,JsonValue value)
        {
            val[name]=value;
        }


    JsonValue get_value(string name)
    {
        return val[name];
    }

    int get_int(string name)
    {
        return get_value(name).asInt();
    }


    bool get_bool(string name)
    {
        return get_value(name).asBool();
    }


    string get_string(string name)
    {
        return string(get_value(name).asString());
    }

    vector<JsonValue>  get_array(string name)
    {
        JsonValue v=get_value(name);
        vector<JsonValue>  ar;
        int sz=v.size();
        for(int i=0;i<sz;i++){
           ar.push_back( v[i]);
        }
        return ar;
    }
    void set_array(string name,vector<JsonValue> ar)
    {
        JsonValue v;
        int sz=ar.size();
        for(int i=0;i<sz;i++){
            v.append(ar[i]);
        }
        set_value(name,v);
    }
    JsonValue value()
    {
        return val;
    }
    vector <JsonValue>  array_value()
    {
        int sz=val.size();
        vector <JsonValue> vec;
        for(int i=0;i<sz;i++)
        {
            vec.push_back(val[i]);
        }
        return vec;
    }
    string data()
    {
       // Reader r;
        FastWriter  w;
        return  w.write(val);

    }
private:
    JsonValue val;
};
//char *str2utf8str(string_t str)
//{
//     return str.data();
//}

#endif
#endif // PD_H
