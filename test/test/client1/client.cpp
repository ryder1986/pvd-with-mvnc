#include "client.h"
void Client::handle_server_msg()
{
    ret_ba=tcp_socket->readAll();
    QByteArray valid_buf;
    valid_buf.clear();
    tmp_msg.append(ret_ba);
    while(get_valid_buf(tmp_msg,valid_buf)) {
        DataPacket pkt(string(valid_buf.data()));
        prt(info,"get %d bytes ",valid_buf.size());
        if(valid_buf.size()>0)
            need_read=true;
        int op=pkt.get_int("type");
        emit get_ret(pkt.data().data());
        switch(op)
        {
        case Pvd::GET_CONFIG:
        {
            emit signal_get_config_done(true,DataPacket(pkt.get_value("config")).data());
        }
            break;
        case Pvd::NEED_UPDATE:
            need_update_config();
            break;
        default:break;
        }
    }
}
