#ifndef LOCATIONSERVICE_H
#define LOCATIONSERVICE_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkInterface>
#include "pvd.h"
class LocationService : public QObject{
    Q_OBJECT
public:
    LocationService(){
        timer=new QTimer();
        timer->setObjectName("timer-reply clients");
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
    inline void start()
    {
        timer->start(100);
    }

    inline void stop()
    {
        timer->stop();
    }

public  slots:
    void check_client_msg();
    void send_info_to_client();
    void send_info_to_client(const QHostAddress &addr);
private:
    QTimer *timer;
    QUdpSocket *udp_skt;
};


#endif // LOCATIONSERVICE_H
