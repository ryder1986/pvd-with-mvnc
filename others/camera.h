#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include "filedatabase.h"

#include "videosource.h"
//#include "pddemoprocessor.h"
#include "processeddatasender.h"
//#include "pdprocessor.h"
#include "pvdprocessor.h"
#include <mutex>
using namespace cv;
class Camera : public QThread
{
    Q_OBJECT
    typedef struct alg{
        JsonValue pvd_c4;
        JsonValue pvd_hog;
        string selected_alg;
    }alg_t;

    typedef struct Camera_config{
        string url;
        int direction;
        int camera_id;
        string user_name;
        string password;
        string camera_ip;
        int camera_port;
        alg_t alg;
    }Config_t;
    Rect detect_rect;
public:
    Camera(JsonValue jv):processor(NULL)
    {
        frame_rate=0;
        quit=false;
        jv_2_cfg(jv);
        start_cam();
        tmr=new QTimer;
        connect (tmr,SIGNAL(timeout()),this,SLOT(handle_time_up()));
        tmr->start(1000);


    }
    ~Camera()
    {
        stop_cam();
    }

    Rect area_2_rect(QJsonValue area)
    {
        int x_min=10000;
        int y_min=10000;
        int x_max=0;
        int y_max=0;
        foreach (QJsonValue v, area.toArray()) {
            int x= v.toObject()["x"].toInt();
            int y= v.toObject()["y"].toInt();
            if(x<x_min)
                x_min=x;
            if(x>x_max)
                x_max=x;
            if(y<y_min)
                y_min=y;
            if(y>y_max)
                y_max=y;
        }
        return Rect(x_min,y_min,x_max-x_min,y_max-y_min);
    }
    bool modify_alg(JsonValue jv)
    {

        DataPacket pkt(jv);
        cam_cfg.alg.selected_alg= pkt.get_string("selected_alg");
        JsonValue jv_pvd_c4=  pkt.get_value("pvd_c4");
        JsonValue jv_pvd_hog= pkt.get_value("pvd_hog");
//        if(jv_selected_alg.isNull()||jv_pvd_c4.isNull()||jv_pvd_hog.isNull()){
//            return false;
//        }TODO: remove value , instead string?

        cam_cfg.alg.pvd_c4= jv_pvd_c4;
        cam_cfg.alg.pvd_hog= jv_pvd_hog;
        restart_processor();
        return true;
    }
    void modify_attr(JsonValue v)
    {
        DataPacket pkt(v);
        int di= pkt.get_int("direction");
        int no= pkt.get_int("camera_id");
        cam_cfg.direction=di;
        cam_cfg.camera_id=no;
    }

    JsonValue config()
    {
        return cfg_2_jv();
    }

    void add_watcher(QString ip)
    {
        if(ip_list.contains(ip)){

        }else{
            prt(info,"client %s require output",ip.toStdString().data());
            ip_list.append(ip);
        }

    }
    void del_watcher(QString ip)
    {
        if(ip_list.contains(ip)){
            prt(info,"client %s stop output",ip.toStdString().data());
            ip_list.removeOne(ip);
        }else{
        }
    }

private:
    void restart_processor()
    {
        mtx.lock();
        string str=cam_cfg.alg.selected_alg;
        if(processor)
            delete processor;
        if(str=="pvd_c4"){
            processor=new PvdC4Processor(cam_cfg.alg.pvd_c4);
        }else if(str=="pvd_hog"){
            processor=new PvdHogProcessor(cam_cfg.alg.pvd_hog);
        }
        mtx.unlock();
    }

    void start_cam()
    {
        src=new VideoSource(cam_cfg.url);
        restart_processor();
        start();
    }

    void stop_cam()
    {
        delete tmr;
        quit=true;
        prt(info,"stoping camera..");
        this->wait();//TODO, maybe we dont need wait?
        prt(info," camera %s stoped",this->src->get_url().data());
        delete src;
        delete processor;
        src=NULL;
        processor=NULL;
    }

    virtual JsonValue cfg_2_jv()
    {
        DataPacket pkt;
        pkt.set_string("url",cam_cfg.url);
        pkt.set_int("direction",cam_cfg.direction);
        pkt.set_int("camera_id",cam_cfg.camera_id);
        pkt.set_string("user_name",cam_cfg.user_name);
        pkt.set_string("password",cam_cfg.password);
        pkt.set_string("camera_ip",cam_cfg.camera_ip);
        pkt.set_int("camera_port",cam_cfg.camera_port);

        DataPacket pkt_alg;
        pkt_alg.set_string("selected_alg",cam_cfg.alg.selected_alg);
        pkt_alg.set_value("pvd_c4",cam_cfg.alg.pvd_c4);
        pkt_alg.set_value("pvd_hog",cam_cfg.alg.pvd_hog);
        pkt.set_value("alg",pkt_alg.value());

        return pkt.value();
    }

    virtual void jv_2_cfg(JsonValue cfg)
    {
        DataPacket pkt(cfg);
        cam_cfg.url = pkt.get_string("url");
        cam_cfg.direction=pkt.get_int("direction");
        cam_cfg.camera_id=pkt.get_int("camera_id");
        cam_cfg.user_name=pkt.get_string("user_name");
        cam_cfg.password=pkt.get_string("password");
        cam_cfg.camera_ip=pkt.get_string("camera_ip");
        cam_cfg.camera_port=pkt.get_int("camera_port");
        JsonValue alg=pkt.get_value("alg");
        DataPacket pkt_alg(alg);
        cam_cfg.alg.selected_alg=pkt_alg.get_string("selected_alg");
        cam_cfg.alg.pvd_c4=pkt_alg.get_value("pvd_c4");
        cam_cfg.alg.pvd_hog=pkt_alg.get_value("pvd_hog");
    }

    void send_out(string ba)
    {
        //  emit output(ba);
        ProcessedDataSender *s=ProcessedDataSender::get_instance();
        foreach (QString ip, ip_list) {
            s->send(ba.data(),QHostAddress(ip));
        }
    }

protected:
    void run()
    {
        int i=0;
        Mat frame;
        threadid=(int)QThread::currentThread();
        QByteArray rst;
        while(!quit){
            //   prt(info,"runing %s",cam_cfg.url.toStdString().data());
            mtx.lock();
            if(src->get_frame(frame)&&frame.cols>0&&frame.rows>0){
                frame_rate++;
                // bool ret=process(frame,rst);
                bool ret=processor->process(frame);
                send_out(processor->get_rst());
            }else{
                //prt(info,"get no frame");
            }
            mtx.unlock();
            QThread::msleep(1);
        }
        // QThread::msleep(10);
    }

signals:
    void output(QByteArray ba);
public slots:
    void handle_time_up()
    {
        frame_rate=0;
    }

private:
    QList <QString> ip_list;
    int frame_rate;
    int threadid;
    QTimer *tmr;
    VideoSource *src;
    VideoProcessor *processor;
    Config_t cam_cfg;
    bool quit;
    mutex mtx;
};

#endif // CAMERA_H
