#include "cameramanager.h"

CameraManager::CameraManager(JsonValue cfg,QObject *parent) : QObject(parent)
{
    cameras.clear();
    jv_2_cfg(cfg);
    start_cams();
}

CameraManager::~CameraManager()
{
    stop_cams();
}
void CameraManager::restart_cameras(JsonValue cfg)
{
    jv_2_cfg(cfg);//set config
    stop_cams();//stop all cameras
    start_cams();//start all cameras with new config
}

bool CameraManager::insert_camera(int index,JsonValue cfg)
{
    bool ret=false;
    if(index>cameras.size()+1)
    {
        index=cameras.size()+1;
    }
    if(index<1){
        prt(fatal,"index %d out of range",index);
    }else{
        prt(fatal,"insert cam  %d  ",index);
        cameras.insert(index-1,new Camera(cfg));
        cam_cfgs.insert(index-1,cfg);
        ret=true;
    }
    return ret;
}
void CameraManager::delete_camera(int index)
{
    if(check_op_index(index)){
        prt(fatal,"del cam  %d  ",index);
        delete cameras[index-1];
        cameras.removeAt(index-1);
        cam_cfgs.removeAt(index-1);
    }else{

    }
}
bool CameraManager::modify_camera(int index,JsonValue v,int mod_type)
{
    bool ret=false;
    if(index<1|index>cameras.size()){
        prt(error,"index out of rean");
        return ret;
    }

    if(cameras[index-1]->modify_alg(v)){
        ret=true;
    }else{
        return ret;
    }

    cam_cfgs[index-1]=cameras[index-1]->config();
    return ret;
}
void CameraManager::modify_attr(int index,JsonValue v)
{
    cameras[index-1]->modify_attr(v);
    cam_cfgs[index-1]= cameras[index-1]->config();
}
