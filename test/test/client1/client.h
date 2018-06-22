#ifndef CLIENT_H
#define CLIENT_H
#include <QObject>
#include <QtNetwork>
#include <tool.h>
#include "pvd.h"
class Client : public QObject
{
    QByteArray tmp_msg;
    Q_OBJECT
public:
    char buf[2000];
    Client()
    {
        tmp_msg.clear();
        server_ip.clear();
        tcp_socket=new QTcpSocket();
        in.setDevice(tcp_socket);
        in.setVersion(QDataStream::Qt_1_0);
        connect(tcp_socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
        connect(tcp_socket,SIGNAL(readyRead()),this,SLOT(handle_server_msg()),Qt::DirectConnection);
        connect(tcp_socket,SIGNAL(connected()),this,SLOT(handle_connected()),Qt::DirectConnection);
    }
    ~Client()
    {
        delete tcp_socket;
    }

    void get_config()
    {

        //    int request_length=Pvd::encode_configuration_request(buf);//encoder buffer
        QJsonObject obj;
        obj["type"]=Pvd::GET_CONFIG;
        QJsonDocument doc(obj);

        bool ret= send(doc.toJson());//talk to server
        if(!ret){
            prt(info,"fail send");
        }
        //    return rst.remove(0,Pvd::HEAD_LENGTH);//TODO:get the ret value;
    }
    void set_config(QByteArray cfg)
    {
        QJsonDocument doc_config=QJsonDocument::fromJson(cfg);
        QJsonObject obj;
        obj["type"]=Pvd::SET_CONFIG;
        obj["config"]=doc_config.object();
        QJsonDocument doc(obj);
        bool ret= send(doc.toJson());//talk to server
        if(!ret){
            prt(info,"fail send");
        }
    }

    void set_alg(QByteArray cfg , int index)
    {
        QJsonDocument doc_config=QJsonDocument::fromJson(cfg);
        QJsonObject obj;
        obj["type"]=Pvd::MOD_CAMERA_ALG;
        obj["channel"]=doc_config.array();
        obj["cam_index"]=index;
        QJsonDocument doc(obj);
        bool ret= send(doc.toJson());//talk to server
        if(!ret){
            prt(info,"fail send");
        }
    }
    void focus_camera(int cam_index)
    {
        //    int request_length=Pvd::encode_configuration_request(buf);//encoder buffer
        QJsonObject obj;
        obj["type"]=Pvd::CAM_OUTPUT_OPEN;
        obj["cam_index"]=cam_index;
        QJsonDocument doc(obj);

        //   bool ret= send(doc.toJson().data(),doc.toJson().length());//talk to server
        bool ret= send(doc.toJson());//talk to server
        if(!ret){
            prt(info,"fail send");
        }
    }

    void disfocus_camera(int cam_index)
    {
        //    int request_length=Pvd::encode_configuration_request(buf);//encoder buffer
        QJsonObject obj;
        obj["type"]=Pvd::CAM_OUTPUT_CLOSE;
        obj["cam_index"]=cam_index;
        QJsonDocument doc(obj);

        //  bool ret= send(doc.toJson().data(),doc.toJson().length());//talk to server
        bool ret= send(doc.toJson());//talk to server
        if(!ret){
            prt(info,"fail send");
        }
    }

    void add_camera(QString url,int index)
    {
        QJsonObject obj_cam;
        obj_cam["url"]=url;
        add_camera(obj_cam,index);
    }

    void add_camera(QJsonObject obj_cam,int index)
    {
        QJsonObject obj;
        obj["type"]=Pvd::INSERT_CAMERA;
        obj["cam_index"]=index;
        obj["camera"]=obj_cam;
        QJsonDocument doc(obj);
        int len=doc.toJson().length();
        //  bool ret= send(doc.toJson().data(),len);//talk to server
        bool ret= send(doc.toJson());//talk to server
        if(!ret){
            prt(info,"fail send");
        }
    }
    void send_msg(QByteArray ba)
    {
        // bool ret= send(ba.data(),ba.length());//talk to server
        bool ret= send(ba);//talk to server
        if(!ret){
            prt(info,"fail send");
        }
    }

    //    void add_camera(QByteArray cam_cfg,QString url)
    //    {

    //    }

    void del_camera(int index)
    {
        QJsonObject obj;
        obj["type"]=Pvd::DELETE_CAMERA;
        obj["cam_index"]=index;
        QJsonDocument doc(obj);

        // bool ret= send(doc.toJson().data(),doc.toJson().length());//talk to server
        bool ret= send(doc.toJson());//talk to server
        if(!ret){
            prt(info,"fail send");
        }

    }
    void mod_camera(QByteArray new_camera_config,int index)
    {

    }

    int count_begin_symbol(QByteArray ba)
    {
        char *tmp=ba.data();
        int sz=ba.size();
        int ret=0;
        int i;
        for( i=0;i<sz;i++){
            if(tmp[i]=='{'){
                ret++;
            }
        }
        return ret;
    }
    int count_end_symbol(QByteArray ba)
    {
        char *tmp=ba.data();
        int sz=ba.size();
        int ret=0;
        int i;
        for( i=0;i<sz;i++){
            if(tmp[i]=='}'){
                ret++;
            }
        }
    }
    void find_pos(QByteArray ba,int &endpos,int &left)
    {

    }
    bool try_get_obj_buf(QByteArray src,QByteArray &dst)
    {
        int ret=false;
        int stack=0;
        char *p_tmp=src.data();
        bool flg=false;
        //    bool flg_real_end=false;
        //char *p_start=src.data();
        dst.clear();
        dst.append(src);
        int i;
        if(count_begin_symbol(src)>0){
            for(i=0;i<src.size();i++){
                if(p_tmp[i]=='{')
                {
                    stack++;
                    flg=true;
                }
                if(p_tmp[i]=='}')
                    stack--;


                if(stack==0&&flg)
                {

                    break;
                }

            }
            if(i<src.size()){
                ret=true;
                if(src[i+1]=='\n')
                    dst.truncate(i+2);
                else
                    dst.truncate(i+i);
            }
        }
        return ret;
    }

    //dst:a sting which contain a compelete json object
    //src:a slice ofstream buffer
    //tmp_msg:last slice of buffer(maybe begining of json string)
    bool get_valid_buf(QByteArray &src,QByteArray &dst)
    {
        if(try_get_obj_buf(src,dst)){
            src.remove(0,dst.size());
            return true;
        }
        return false;

    }

public slots:
    void handle_connected()
    {
        prt(info,"connected");
        emit connect_done();
    }


    void handle_server_msg();

    void connect_to_server(QString ip)
    {
        if(ip.size()){
            //  prt(info,"state %d ",tcp_socket->state());
            if(tcp_socket->state()==QAbstractSocket::ConnectedState)
                tcp_socket->disconnectFromHost();
            //     prt(info,"new ip %s connect (%d)",server_ip.toStdString().data(),tcp_socket->state());
            if(ip==server_ip){
                prt(info,"reconnecting %s",server_ip.toStdString().data());
                tcp_socket->connectToHost(server_ip,12345);
            }else{
                prt(info,"connecting %s",ip.toStdString().data());
                server_ip=ip;
                tcp_socket->connectToHost(server_ip,12345);
            }

        }else{
        }
    }
    void  displayError(QAbstractSocket::SocketError socketError)
    {
        prt(info,"err when connecting to server");
        switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            qDebug()<<"1";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qDebug()<<"1";
            break;
        default:
            qDebug()<<"1";
        }
    }


signals:
    void signal_get_config_done(bool,string);
    void need_update_config();
    void connect_done();
    void get_ret(QByteArray);
    void send_done(QByteArray);
private:
    bool send(const QByteArray ba)
    {
        emit send_done(ba);
        bool ret=false;
        int write_bytes=0;
        int len=ba.length();
        write_bytes=tcp_socket->write(ba.data(),ba.length());
        bool flush_ret=tcp_socket->flush();//TODO,not work for flush
        if(flush_ret){
            prt(info,"flush ok");
        }else{
            prt(info,"flush err");
        }

        if(write_bytes!=len){
            prt(info,"send %d bytes in state %d , %d bytes left",write_bytes,tcp_socket->state(),len-write_bytes);
        }else{
            ret=true;
        }
        return ret;
    }
    QString server_ip;
    QTcpSocket *tcp_socket;
    QDataStream in;
    QByteArray ret_ba;
    bool need_read;
};

#endif // CLIENT_H
