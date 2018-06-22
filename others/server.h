#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include "filedatabase.h"
#include "cameramanager.h"
#include "clientsession.h"
#include "pvd.h"
class LocationService : public QObject{
    Q_OBJECT
public:
    LocationService(){
        timer=new QTimer();
        connect(timer,SIGNAL(timeout()),this,SLOT(check_client_msg()));//TODO:maybe replace with readReady signal
        udp_skt = new QUdpSocket();
        udp_skt->bind(Pvd::SERVER_REPORTER_PORT,QUdpSocket::ShareAddress);
    }
    ~LocationService()
    {
        disconnect(timer);
        delete timer;
        delete udp_skt;
    }
    void start()
    {
        timer->start(100);
    }

    void stop()
    {
        timer->stop();
    }

public  slots:
    void check_client_msg()
    {
        QByteArray client_msg;
        char *msg;
        if(udp_skt->hasPendingDatagrams())
        {
            client_msg.resize((udp_skt->pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort;
            udp_skt->readDatagram(client_msg.data(),client_msg.size(),&sender,&senderPort);
            prt(info,"get client broadcasted code :%s",msg=client_msg.data());
            if(!strcmp(msg,"pedestrian")){
                //    send_info_to_client();
                send_info_to_client(sender);
            }
            else{
                prt(error,"client code :%s NOT MATCH pedestrian,we will not answer",msg=client_msg.data());
            }
        }else{
            //prt(debug,"searching client on port %d",Pvd::SERVER_REPORTER_PORT)
        }
    }

    void send_info_to_client()
    {
        QByteArray datagram;
        datagram.clear();
        QList <QNetworkInterface>list_interface=QNetworkInterface::allInterfaces();
        foreach (QNetworkInterface i, list_interface) {
            if(i.name()!="lo"){
                QList<QNetworkAddressEntry> list_entry=i.addressEntries();
                foreach (QNetworkAddressEntry e, list_entry) {
                    if(e.ip().protocol()==QAbstractSocket::IPv4Protocol)
                    {
                        datagram.append(QString(e.ip().toString())).append(QString(",")).\
                                append(QString(e.netmask().toString())).append(QString(",")).append(QString(e.broadcast().toString()));
                    }

                }
            }
        }
#if 1
        //broadcast
        udp_skt->writeDatagram(datagram.data(), datagram.size(),
                               QHostAddress::Broadcast, Pvd::CLIENT_REPORTER_PORT);
#else
        //send to single ip. problem in windows
#endif
    }
    void send_info_to_client(const QHostAddress &addr)
    {
        QByteArray datagram;
        datagram.clear();
        QList <QNetworkInterface>list_interface=QNetworkInterface::allInterfaces();
        foreach (QNetworkInterface i, list_interface) {
            if(i.name()!="lo"){
                QList<QNetworkAddressEntry> list_entry=i.addressEntries();
                foreach (QNetworkAddressEntry e, list_entry) {
                    if(e.ip().protocol()==QAbstractSocket::IPv4Protocol)
                    {
                        datagram.append(QString(e.ip().toString())).append(QString(",")).\
                                append(QString(e.netmask().toString())).append(QString(",")).append(QString(e.broadcast().toString()));
                    }

                }
            }
        }
#if 1
        udp_skt->writeDatagram(datagram.data(), datagram.size(),
                               addr, Pvd::CLIENT_REPORTER_PORT);
#else
        //send to single ip. problem in windows
#endif
    }
private:
    QTimer *timer;
    QUdpSocket *udp_skt;
};

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
public slots:
    int handle_client_request(string request,string &ret,void *addr);
    void new_connection();
    void delete_client(ClientSession *c)
    {
        delete c ;
        clients.removeOne(c);
    }
    void  displayError(QAbstractSocket::SocketError socketError)
    {
        switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            prt(info,"client closed");
            break;
        case QAbstractSocket::HostNotFoundError:
            prt(info,"client error");
            break;
        case QAbstractSocket::ConnectionRefusedError:
            prt(info,"client error");
            break;
        default:
            break;
        }
    }

private:
    inline void load_cfg()
    {
        string json_data;
        database->load(json_data);
        JsonValue jv=DataPacket(json_data).value();
        jv_2_cfg(jv);
    }

    inline void save_cfg()
    {
        JsonValue jv=cfg_2_jv();
        database->save(DataPacket(jv).data());
    }

    inline JsonValue cfg_2_jv()
    {
        DataPacket pkt;
        pkt.set_string("device_name",cfg.server_name);
        pkt.set_int("deviceID",cfg.dev_id);
        pkt.set_string("signal_machine_ip",cfg.sig_ip);
        pkt.set_int("signal_machine_port",cfg.sig_port);
        pkt.set_string("ntp_ip",cfg.ntp_ip);
        pkt.set_int("ntp_port",cfg.ntp_port);
        pkt.set_value("cameras",cfg.cams_cfg);
        return pkt.value();
    }

    inline void jv_2_cfg(JsonValue jv)
    {
        DataPacket pkt(jv);
        cfg.server_name=pkt.get_string("device_name");
        cfg.dev_id=pkt.get_int("deviceID");
        cfg.sig_ip=pkt.get_string("signal_machine_ip");
        cfg.sig_port= pkt.get_int("signal_machine_port");
        cfg.ntp_ip=pkt.get_string("ntp_ip");
        cfg.ntp_port=pkt.get_int("ntp_port");
        cfg.cams_cfg=pkt.get_value("cameras");
    }

    QTcpServer *server;//server for reply all clients request ,execute client cmds,like add cam,del cam, reconfigure cam,etc..
    FileDatabase *database;//hold config data;
    CameraManager *camera_manager;//hold cameras
    QList <ClientSession *> clients;//hold sessions
    Config_t cfg;
    char recv_buf[Pvd::BUFFER_MAX_LENGTH];
    char send_buf[Pvd::BUFFER_MAX_LENGTH];
    LocationService service;
};


#endif // SERVER_H
