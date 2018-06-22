#include "videosource.h"

VideoSource::VideoSource(string path)
{
    frame_rate=0;
    url=path;
    quit_flg=false;
    tmr=new QTimer();
    connect(tmr,SIGNAL(timeout()),this,SLOT(handle_time_out()));
    tmr->start(1000);
    this->start();
}
VideoSource::~VideoSource()
{
    quit_flg=true;
    this->wait();
    delete tmr;

}
void VideoSource::run()
{
    vcap=   VideoCapture( url);
    if(!vcap.isOpened()){
        prt(info,"fail to open %s", url.data());
    }else{
        prt(info,"ok to open %s", url.data());
    }
    QString str( url.data());
    if(str.contains("rtsp")||str.contains("http")){
        frame_wait_time=0;
    }else{
        frame_wait_time=40;
    }
    Mat mat_rst;
    int flag_retry=0;
    while(!quit_flg){
        // prt(info,"runing thread %s",url.toStdString().data());
        if( vcap.isOpened()){
            flag_retry=0;
            bool rt= vcap.read(mat_rst);
            if(!rt){
                prt(info,"get frame fail,restart video capture %s", url.data());
                vcap.release();  vcap=   VideoCapture( url.data());
            }
            if(mat_rst.cols==0){
                vcap.release();
                prt(info,"%s get frame error,retrying ... ", url.data());
                continue;
                prt(info,"restarting %s      ", url.data());
            }else{
                frame_rate++;
                if(frame_list.size()<3){
                    frame_list.push_back(mat_rst);
                }
                if(frame_wait_time)
                    this_thread::sleep_for(chrono::milliseconds( frame_wait_time));
            }
        }else{
            if(flag_retry++<10){
                this_thread::sleep_for(chrono::milliseconds(100));
            }else{
                this_thread::sleep_for(chrono::seconds(1));
            }
            vcap=VideoCapture( url.data());
        }
    }
    if( vcap.isOpened())
        vcap.release();
}
