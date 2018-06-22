#include "locationservice.h"


void LocationService::check_client_msg()
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

void LocationService::send_info_to_client()
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

void LocationService::send_info_to_client(const QHostAddress &addr)
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
