#include "clientsession.h"
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
