#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QTimer>
#include "filedatabase.h"
#include "videosource.h"
#include "processeddatasender.h"
#include "pvdprocessor.h"
#include "pvdhogprocessor.h"
#include <mutex>
using namespace cv;
class Camera : public QThread
{
       ProcessedDataSender *s;
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
        vector <JsonValue> chs;
        //alg_t alg;
    }Config_t;
    Rect detect_rect;
public:
    Camera(JsonValue jv);
    ~Camera()
    {
        stop_cam();
    }
    bool modify_alg(JsonValue jv);
    void modify_attr(JsonValue v);
    void add_watcher(QString ip);
    void del_watcher(QString ip);
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

    inline JsonValue config()
    {
        return cfg_2_jv();
    }
private:
    void restart_processor()
    {
        mtx.lock();
        if(processors.size()){
            foreach (VideoProcessor *p, processors) {
                delete p;
            }
            processors.clear();
        }
        foreach (JsonValue v, cam_cfg.chs) {
            DataPacket pkt(v);
            string str=pkt.get_string("selected_alg");
            int channel_id=pkt.get_int("channel_id");
#if 0
            if(str=="pvd_c4"){
                processors.push_back(new PvdC4Processor(pkt.get_value(str),channel_id));
            }else if(str=="pvd_hog"){
                processors.push_back(new PvdHogProcessor(pkt.get_value(str),channel_id));
            }
#else
               processors.push_back(new PvdMvncProcessor());
#endif
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
        if(processors.size()){
            foreach (VideoProcessor *p, processors) {
                delete p;
            }
            processors.clear();
        }
        src=NULL;
        prt(info,"delete done");
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

        DataPacket pkt_alg(cam_cfg.chs);
        pkt.set_value("channel",pkt_alg.value());

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
        JsonValue ch=pkt.get_value("channel");
        DataPacket pkt_alg(ch);
        cam_cfg.chs =pkt_alg.array_value();
    }

    void send_out(string ba)
    {
        bool send_flag=false;
        if(send_check_tick++>50){
            send_flag=true;
            send_check_tick=0;

        }

        ProcessedDataSender *s=ProcessedDataSender::get_instance();
        if(send_flag){
            //  prt(info,"sending state...");
        }
        foreach (QString ip, ip_list) {
            s->send(ba.data(),QHostAddress(ip));
            if(send_flag){
                //    prt(info,"send to %s, size(%d)",ip.toStdString().data(),ba.size());
            }
        }
    }



#define DATA_START 0x7e//1
#define ID 0x01//2
    //#define ID 0x01//2
    //#define ID 0x7d//2
#define VER 0x01//3
#define OP 0x61//4
#define CLASS 0x1f//5
    //6 data
    //7 check
#define DATA_END 0x7e//8
#define DATA_START 0x7e//
    typedef struct cdata_type{
        uint8_t no;
        uint8_t exist;
        uint8_t percent;//zhan you lv
        uint8_t busy_state;// 1-5 , judge by percent
        uint8_t valid;
    }cdata_t;

    typedef struct data_type{
        uint8_t direction;
        uint8_t channel_count;
        vector <cdata_t> channels;

    }data_t;

    vector <uint8_t>  process_protocal(data_t data )
    {
        vector <uint8_t> bs;
        bs.push_back(DATA_START);
        bs.push_back(ID);
        bs.push_back(VER);
        bs.push_back(OP);
        bs.push_back(this->cam_cfg.camera_id+0x10);
        //data
        int num=data.channels.size();
        bs.push_back(0x01);//direction
        bs.push_back(num);//channel total count
        for(int i=0;i<num;i++){
            cdata_t cd=data.channels[i];
            bs.push_back(cd.no);//area id
            bs.push_back(cd.exist);//exist
            bs.push_back(cd.percent);//percent
            bs.push_back(cd.busy_state);//busy
            bs.push_back(cd.valid);//valid
        }
        uint8_t check=0;

        if(bs.size()>1){
            check=bs[0];
            for(int i=1;i<bs.size();i++){
                check^=bs[i];
            }
        }
        bs.push_back(check);
        bs.push_back(DATA_END);


        if(bs.size()>1){

            vector <uint8_t>::iterator it=bs.begin()+1;
            while(it!=bs.end()-1){
                if(*it==0x7E)
                {
                    *it=0x7D;
                    bs.insert(it+1,1,(uint8_t)0x5E);

                }else
                    if(*it==0x7D)
                    {
                        *it=0x7D;
                        bs.insert(it+1,1,(uint8_t)0x5D);
                    }
                it++;
            }
        }
        return bs;
    }

protected:
    void run();
signals:
public slots:
    inline void handle_time_up()
    {
        //      frame_rate=0;
    }
private:
    QList <QString> ip_list;
    //    int frame_rate;
 //   int threadid;
    QTimer *tmr;
    VideoSource *src;
    //  VideoProcessor *processor;
    vector <VideoProcessor *> processors;
    Config_t cam_cfg;
    bool quit;
    mutex mtx;
    int send_check_tick;


};

#endif // CAMERA_H
