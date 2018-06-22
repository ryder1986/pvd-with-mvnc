#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include "filedatabase.h"
#include "cameramanager.h"
#include "clientsession.h"
#include "pvd.h"
#include "locationservice.h"
class Server:public QObject
{
    Q_OBJECT
    typedef struct configure{
        string server_name;
        int dev_id;
        string sig_ip;
        int sig_port;
        string ntp_ip;
        int ntp_port;
        JsonValue cams_cfg;
    }Config_t;
public:
    Server(FileDatabase *db);
    ~Server();
    void start()
    {
        service.start();
    }
public slots:
    int handle_client_request(string request,string &ret,void *addr);//do all request
    void new_connection();//client connecting
    void delete_client(ClientSession *c);
    void displayError(QAbstractSocket::SocketError socketError);
private:
    inline void load_cfg();
    inline void save_cfg();
    inline JsonValue cfg_2_jv();
    inline void jv_2_cfg(JsonValue jv);

    QTcpServer *server;//server for reply all clients request ,execute client cmds,like add cam,del cam, reconfigure cam,etc..
    FileDatabase *database;//hold config data;
    CameraManager *camera_manager;//hold cameras
    QList <ClientSession *> clients;//hold sessions
    Config_t cfg;//hold config
    char recv_buf[Pvd::BUFFER_MAX_LENGTH];
    char send_buf[Pvd::BUFFER_MAX_LENGTH];
    LocationService service;
};


#endif // SERVER_H
