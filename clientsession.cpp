#include "clientsession.h"
ClientSession::ClientSession(QTcpSocket *client_skt):skt(client_skt),focus_index(0){
    tmp_msg.clear();
    need_sync=true;
    focus_index=0;
    connect(skt,SIGNAL(readyRead()),this,SLOT(handle_msg()));
    connect(skt,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    connect(skt,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error_happened()));//TODO,important

    client_addr=skt->peerAddress();
}

void ClientSession::set_valid(bool valid)
{
    need_sync=!valid;
    if(!valid){
        QByteArray rt;
        rt.clear();
        DataPacket pkt;
        pkt.set_int("type",Pvd::NEED_UPDATE);
        rt.append(pkt.data().data());
        skt->write(rt,rt.size());
    }
}

void ClientSession::handle_msg(){
    int writes_num=0;
    QByteArray client_buf=skt->readAll();
    QByteArray valid_buf;
    valid_buf.clear();
    tmp_msg.append(client_buf);
    while(get_valid_buf(tmp_msg,valid_buf)) {//Get valid json object, TODO:we only check {} matches, we should check json grammar
        string rt;
        rt.clear();
        QString str_input(valid_buf);
        prt(info,"get %d bytes:",str_input.length());
        printf("%s\n",str_input.toStdString().data());
        string str=valid_buf.data();
        emit client_request(str,rt,(void *)this);
        writes_num=skt->write(rt.data(),rt.size());
        QString str_output(QByteArray(rt.data(),rt.size()));
        prt(info,"reply %d bytes:",writes_num);
        printf("%s\n",str_output.toStdString().data());
    }
}

void ClientSession::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        prt(info,"err");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        prt(info,"err");
        break;
    default:break;

    }
}
