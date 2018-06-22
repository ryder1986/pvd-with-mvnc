#ifndef SERVERINFOSEARCHER_H
#define SERVERINFOSEARCHER_H

#include <QObject>
#include <QtNetwork/QUdpSocket>
#include "tool.h"
#include <QThread>
#include "pvd.h"
class ServerReplyCheckRouting : public QObject
{
    Q_OBJECT

public slots:
    void check_reply(    QUdpSocket *udp_skt_find_server) {
        QString str;
        str.clear();
        int try_times=100;
        while(try_times--){
            if(udp_skt_find_server->hasPendingDatagrams()){
                datagram.resize((udp_skt_find_server->pendingDatagramSize()));
                udp_skt_find_server->readDatagram(datagram.data(),datagram.size());
                prt(info,"get server info : %s",datagram.data());
                server_ip.clear();
                server_ip.append(datagram.split(',')[0]);
                prt(info,"ip : %s",server_ip.toStdString().data());
                ip_list.append(server_ip);
                emit resultReady(server_ip);
            }else{
                //      prt(info,"searching");
            }
            QThread::msleep(10);
        }
    }

signals:
    void resultReady(  QString result);
private:
    //  QUdpSocket *udp_skt_find_server;

    QByteArray datagram;
    QString server_ip;
    QStringList ip_list;
};

class ServerInfoSearcher : public QObject{
    Q_OBJECT
    QThread check_thread;
    ServerReplyCheckRouting *p_checker;

public:
    ServerInfoSearcher()
    {

        udp_skt_find_server=new QUdpSocket(this);
        udp_skt_find_server->bind(Pvd::CLIENT_REPORTER_PORT,QUdpSocket::ShareAddress);
        //     connect(udp_skt_find_server,SIGNAL(readyRead()),this,SLOT(get_reply()),Qt::QueuedConnection);

        //   connect();

        p_checker=new ServerReplyCheckRouting;
        p_checker->moveToThread(&check_thread);
        connect(&check_thread,&QThread::finished,p_checker,&QObject::deleteLater);
        connect(this,SIGNAL(begin_search(QUdpSocket*)),p_checker,SLOT(check_reply(QUdpSocket*)),Qt::QueuedConnection);
        connect(p_checker,SIGNAL(resultReady(QString)),this,SLOT(ip_found(QString)),Qt::QueuedConnection);
    }
    ~ServerInfoSearcher()
    {
        check_thread.quit();
        check_thread.wait();
    }
    void broadcast_info()
    {
        QByteArray b;
        b.append("pedestrian");
        udp_skt_find_server->writeDatagram(b.data(), b.size(),
                                           QHostAddress::Broadcast, Pvd::SERVER_REPORTER_PORT);
        prt(info,"finding server ...");

    }
    void search()
    {

        check_thread.start();
        emit begin_search(udp_skt_find_server);
    }

    void search_device()
    {
        ip_list.clear();
        broadcast_info();
        search();
        //   p_find_server_thread=new std::thread(find_server);
        //   QThread::msleep(3000);

    }
    QStringList search_rst()
    {
        return ip_list;
    }
    static void find_server()
    {
        prt(info," find server thread");
        int times=10;
        while(times--){

        }
    }
signals:
    void begin_search( QUdpSocket *udp_skt_find_server);
    void find_ip(QString ip);

public slots:
    void ip_found(QString ip)
    {
        qDebug()<<ip;
       // ip_list.append(ip);
        emit find_ip(ip);
    }

//    void get_reply()
//    {
//        //  while(udp_skt->hasPendingDatagrams())
//        if(udp_skt_find_server->hasPendingDatagrams())
//        {
//            datagram.resize((udp_skt_find_server->pendingDatagramSize()));
//            udp_skt_find_server->readDatagram(datagram.data(),datagram.size());
//            prt(info,"get server info : %s",datagram.data());
//            server_ip.append(datagram.split(',')[0]);
//            ip_list.append(server_ip);
//        }
//    }

private :
    QUdpSocket *udp_skt_find_server;

    QByteArray datagram;
    QString server_ip;
    QStringList ip_list;
};

class ProcessedDataReciver: public QObject{
    Q_OBJECT
public:
    ProcessedDataReciver()
    {
        udp_skt_alg_output=new QUdpSocket(this);
        udp_skt_alg_output->bind(12346,QUdpSocket::ShareAddress);
        connect(udp_skt_alg_output,SIGNAL(readyRead()),this,SLOT(get_rst()),Qt::QueuedConnection);
    }
signals:
    void send_rst(QByteArray);
public slots:
    void get_rst()
    {
        QByteArray datagram_rst;
        if(udp_skt_alg_output->hasPendingDatagrams())
        {
            //   int size=udp_skt_alg_output->pendingDatagramSize();
            datagram_rst.resize((udp_skt_alg_output->pendingDatagramSize()));
            udp_skt_alg_output->readDatagram(datagram_rst.data(),datagram_rst.size());
            //        udp_skt_alg_output->readDatagram(sss,500);
            //          datagram_rst= udp_skt_alg_output->readAll();
#if 0
            QList <QByteArray > bl= datagram_rst.split(':');
            QByteArray b_index=bl[0];
            int index=*(b_index);

            prt(info,"get cam   %d rst",index);

            QByteArray b_loc=bl[1];
#else

         //  prt(info,"get data %s",datagram_rst.data());
            emit send_rst(datagram_rst);
#endif

            //   emit send_camera_rst(index,b_loc);
            //    QList <QByteArray > xy= b_loc.split(',');
            //            int x=xy[0].toInt();
            //            int y=xy[1].toInt();
            //           prt(info," %d : %d",x,y);

        }
    }

private:
    QUdpSocket *udp_skt_alg_output;
    QByteArray rst;
};



#endif // SERVERINFOSEARCHER_H
