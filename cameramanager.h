#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QList>
#include <QDebug>
#include "pvd.h"
#include "camera.h"
class CameraManager : public QObject
{
    Q_OBJECT
public:
    enum{
        MODIFY_ALG,
        MODIFY_URL,
        MODIFY_DIRECTION,
        MODIFY_ARGS
    };
    friend class Server;
    explicit CameraManager(JsonValue config,QObject *parent = 0);
    ~CameraManager();
    JsonValue config()
    {
        return cfg_2_jv();
    }
public:
    void restart_cameras(JsonValue cfg);
    bool insert_camera(int index,JsonValue cfg);
    void delete_camera(int index);
    bool modify_camera(int index,JsonValue v,int mod_type);
    void modify_attr(int index,JsonValue v);
private:
    void start_cams()
    {
        foreach (JsonValue v, cam_cfgs)
        {
            cameras.append(new Camera(v));
        }
    }

    void stop_cams()
    {
        prt(info,"cam size %d",cameras.size());
        foreach (Camera *c, cameras)
        {
            delete c;
            prt(info,"cam size %d",cameras.size());
        }
        cameras.clear();
        prt(info,"cam size %d",cameras.size());
    }

    bool check_op_index(int index)
    {
        if(cameras.size()==0)
            return false;
        if(index<1||index>cameras.size()){
            prt(fatal,"index %d out of range(1-%d)",index,cameras.size());
            return false;
        }
        return true;
    }

    //trans config to json value
    inline JsonValue cfg_2_jv()
    {
        vector<JsonValue> vec;
        foreach (JsonValue v, cam_cfgs) {
            vec.push_back(v);
        }
        DataPacket pkt(vec);
        return pkt.value();
    }

    inline void jv_2_cfg(JsonValue jv)
    {
        DataPacket pkt(jv);
        cam_cfgs.clear();
        foreach (JsonValue v, pkt.array_value())
        {
            cam_cfgs.push_back(v);
        }
    }

private:
    QList <Camera *>cameras;
    QList <JsonValue> cam_cfgs;
};

#endif // CAMERAMANAGER_H
