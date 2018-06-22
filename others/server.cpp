#include "server.h"

Server::Server(FileDatabase *db):database(db),camera_manager(NULL)
{

    load_cfg();
    camera_manager=new CameraManager(cfg.cams_cfg);
    bool ret=false;
    server=new QTcpServer();
    ret=server->listen(QHostAddress::Any,Pvd::SERVER_PORT);
    if(ret){
        prt(info,"Server listen on port %d success!",Pvd::SERVER_PORT);
    } else {
        prt(fatal,"Server listen on port %d failed!",Pvd::SERVER_PORT);
        exit(1);
    }
    connect(server, &QTcpServer::newConnection, this,
            &Server::new_connection);
    service.start();
}
Server::~Server()
{
    delete server;
    delete camera_manager;
}

void Server::new_connection()
{
    QTcpSocket *skt = server->nextPendingConnection();
    connect(skt, SIGNAL(disconnected()),skt, SLOT(deleteLater()));
    QString str(skt->peerAddress().toString());
    prt(info,"client %s:%d connected",str.toStdString().data(),skt->peerPort());
    ClientSession *client=new ClientSession(skt);
    connect(client,SIGNAL(socket_error(ClientSession*)),this,SLOT(delete_client(ClientSession*)));
    connect(skt,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
    connect(client,SIGNAL( client_request(string,string&,void *)),this,
            SLOT(handle_client_request(string,string&,void *)),Qt::DirectConnection);//important,in case of competition bugs
    //        client->f_client_request=bind(&Server::handle_client_request,this,placeholders::_1,placeholders::_2,placeholders::_3);
    clients.append(client);
}


int Server::handle_client_request(string request,string &ret,void *addr)
{
    ClientSession *cs=(ClientSession *)addr;
    DataPacket data_src(request);
    DataPacket data_dst;
    int type=data_src.get_int("type");
    int idx=data_src.get_int("cam_index");
    data_dst.set_value("type",type);
    data_dst.set_value("return",Pvd::RETURN::OK);
    if(!type){
        ret=data_dst.data();//tell client to update
        data_dst.set_value("return",Pvd::RETURN::PARSE_ERROR);
        return 1;
    }
    bool config_changed=false;
    switch(type){
    case Pvd::GET_CONFIG:// if client wants get config, client is ok
        cs->set_valid(true);
        break;
    case Pvd::SET_CONFIG:
    case Pvd::INSERT_CAMERA:
    case Pvd::DELETE_CAMERA:
    case Pvd::MOD_CAMERA_ALG:
    case Pvd::MOD_CAMERA_ATTR:
    case Pvd::MOD_DEVICE_ATTR:
    default:
        if(cs->is_valid())
            config_changed=true;// if client is valid , change can be made
        break;
    }

    if(config_changed){//if valid change happen, other clients will be invalid
        foreach (ClientSession *session, clients) {
            if(session!=addr)
                session->set_valid(false);
        }
    }
    if(!cs->is_valid()&&type!=Pvd::GET_CONFIG){// no valid, no update
        data_dst.set_value("return",Pvd::RETURN::NEED_UPDATE);
        ret=data_dst.data();
        return 1;
    }

    switch(type){

    case Pvd::GET_CONFIG:
    {
#if 0
        QJsonObject cfg;
        cfg_2_obj(cfg);
        data_dst.set_value("config",cfg);
#else
        JsonValue jv=cfg_2_jv();
        data_dst.set_value("config",jv);
#endif
        break;
    }

    case Pvd::SET_CONFIG:
    {
        jv_2_cfg(data_src.get_value("config"));
        save_cfg();
        camera_manager->restart_cameras(cfg.cams_cfg);
        break;
    }

    case Pvd::CAM_OUTPUT_OPEN:
    {
        if(idx>camera_manager->cameras.size()||idx<1){
            prt(info,"%d out of range ",idx);
            data_dst.set_value("return",Pvd::RETURN::INVALID_VALUE);
        }else
            camera_manager->cameras[idx-1]->add_watcher(cs->ip());
        break;
    }
    case Pvd::CAM_OUTPUT_CLOSE:
    {
        if(idx>camera_manager->cameras.size()||idx<1){
            prt(info,"%d out of range ",idx);
            data_dst.set_value("return",Pvd::RETURN::INVALID_VALUE);

        }else{
            camera_manager->cameras[idx-1]->del_watcher(cs->ip());
        }
        break;
    }

    case Pvd::MOD_CAMERA_ALG:
    {
        if(idx>camera_manager->cameras.size()||idx<1){
            prt(info,"%d out of range ",idx);
            data_dst.set_value("return",Pvd::RETURN::INVALID_VALUE);
            break;
        }
        if(camera_manager->modify_camera(idx,data_src.get_value("alg"),CameraManager::MODIFY_ALG)){
            cfg.cams_cfg=camera_manager->config();
            save_cfg();
        }else{
            data_dst.set_value("return",Pvd::RETURN::INVALID_VALUE);
        }

        break;
    }

    case Pvd::MOD_CAMERA_ATTR:
    {
        if(idx>camera_manager->cameras.size()||idx<1){
            prt(info,"%d out of range ",idx);
            data_dst.set_value("return",Pvd::RETURN::INVALID_VALUE);
            break;
        }

        if(idx<=camera_manager->cameras.size()&&idx>0){
            camera_manager->modify_attr(idx,data_src.get_value("camera_args"));
            cfg.cams_cfg=camera_manager->config();
            save_cfg();
        }

        break;
    }
    case Pvd::INSERT_CAMERA:
    {
        if(camera_manager->insert_camera(idx,data_src.get_value("camera"))){

            cfg.cams_cfg=camera_manager->config();
            save_cfg();
        }else{
            data_dst.set_value("return",Pvd::RETURN::INVALID_VALUE);
        }
        break;
    }

    case Pvd::DELETE_CAMERA:
    {
        if(idx>camera_manager->cameras.size()||idx<1){
            prt(info,"%d out of range ",idx);
            data_dst.set_value("return",Pvd::RETURN::INVALID_VALUE);
        }else{
            camera_manager->delete_camera(idx);
            cfg.cams_cfg=camera_manager->config();
            save_cfg();
        }
        break;
    }

    case Pvd::HEART:
    {
        break;
    }

    case Pvd::REBOOT:
    {

        break;
    }

    case Pvd::MOD_DEVICE_ATTR:
    {
        cfg.dev_id=data_src.get_int("deviceID");
        cfg.server_name=data_src.get_string("device_name");
        save_cfg();
        break;
    }

    default:break;
    }
    ret=data_dst.data();
    return 0;
}
