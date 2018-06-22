#ifndef IPINFO_H
#define IPINFO_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkInterface>
class IpInfo
{
            QList <QNetworkInterface>list_interface;
public:
    IpInfo()
    {


      list_interface=QNetworkInterface::allInterfaces();

        foreach (QNetworkInterface i, list_interface) {
             qDebug()<< "iface   "<<i.name();
              QList<QNetworkAddressEntry> list_entry=i.addressEntries();
            foreach (QNetworkAddressEntry e, list_entry) {
                if(e.ip().protocol()==QAbstractSocket::IPv4Protocol)
                {
                   qDebug()<<  (QString(e.ip().toString()));
                            qDebug()<<      (QString(e.netmask().toString()));

                          qDebug()<<        (QString(e.broadcast().toString()));
                }

            }
        }
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
    }
};

#endif // IPINFO_H
