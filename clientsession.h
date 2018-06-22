#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H
#include <QObject>
#include <QTcpSocket>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include "processeddatasender.h"
#include "tool.h"
#include "pvd.h"
class ClientSession:public QObject{
    QByteArray tmp_msg;
    Q_OBJECT
public:
    bool need_sync;
    ClientSession(QTcpSocket *client_skt);
    ~ClientSession()
    {
    }
    void set_valid(bool valid);

    bool is_valid()
    {
        return !need_sync;
    }

public slots:
    void error_happened()
    {
        emit socket_error(this);
    }

    inline int count_begin_symbol(QByteArray ba)
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
    inline bool try_get_obj_buf(QByteArray src,QByteArray &dst)
    {
        int ret=false;
        int stack=0;
        char *p_tmp=src.data();
        bool flg=false;
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
    inline bool get_valid_buf(QByteArray &src,QByteArray &dst)
    {
        if(try_get_obj_buf(src,dst)){
            src.remove(0,dst.size());
            return true;
        }
        return false;
    }

    void handle_msg();
    void displayError(QAbstractSocket::SocketError socketError);
    inline QString ip()
    {
        return client_addr.toString();
    }

signals :
    int get_server_config(char *buf);
    void socket_error(ClientSession *c);
    int client_request(string request,string &ret,void *);
public:
private:
    char *rcv_buf;
    char send_buf[Pvd::BUFFER_LENGTH];
    QTcpSocket *skt;
    QTimer *timer;
    QHostAddress client_addr;
    int focus_index;
};

#endif // CLIENTSESSION_H
