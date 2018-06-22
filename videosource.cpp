#include "videosource.h"

VideoSource::VideoSource(string path)
{
    // frame_rate=0;
    url=path;
    quit_flg=false;
    tmr=new QTimer();
    connect(tmr,SIGNAL(timeout()),this,SLOT(handle_time_out()));
    tmr->start(1000);
    this->setObjectName("video source thread");
    this->start();
}
VideoSource::~VideoSource()
{
    delete tmr;
    quit_flg=true;
    // this->exit(0);
    prt(info,"quiting  %s", url.data());
    //   this->wait();// TODO, we have risk to stuck here.
    //  this->thread()->quit();
    //   this->terminate();
    if(!this->wait(1000))
    {
        prt(info,"terminating ######!!!!!!#########################end fail   %s", url.data());

        this->terminate();
        prt(info,"############################################end fail   %s", url.data());
        this->wait();
        prt(info,"end ok   %s", url.data());

    }
    //  prt(info,"quit %s done", url.data());


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
    monitor=0;
    int flag_retry=0;
    while(!quit_flg){
        // if((!(monitor++%30))){
        //    prt(info,"flg %d",quit_flg);
        // }
        // prt(info,"runing thread %s",url.toStdString().data());
        if( vcap.isOpened()){
            flag_retry=0;
            bool rt= vcap.read(mat_rst);
            if(!rt){
                cout<<"get frame err"<<url.data()<<endl;
                prt(info,"get frame fail,restart video capture %s", url.data());
                vcap.release();  vcap=   VideoCapture( url.data());
            }
            if(mat_rst.cols==0){
                vcap.release();
                cout<<"get frame invalid"<<url.data();
                prt(info,"%s get frame error,retrying ... ", url.data());
                continue;
                prt(info,"restarting %s      ", url.data());
            }else{
                //    frame_rate++;
                lock.lock();
                if(frame_list.size()<3){
                    frame_list.push_back(mat_rst);
                }
                lock.unlock();
             //   cout<<"wait for new frame:"<<url.data()<<endl;
                if(frame_wait_time)
                    this_thread::sleep_for(chrono::milliseconds( frame_wait_time));
              //  cout<<"wait done:"<<url.data()<<endl;
            }
        }else{
            if(flag_retry++<10){
                this_thread::sleep_for(chrono::milliseconds(100));
            }else{
                this_thread::sleep_for(chrono::seconds(1));
            }
            vcap=VideoCapture( url.data());
            cout<<"open url err:"<<url.data()<<endl;

        }
    }
    prt(info,"flg %d",quit_flg);
    if( vcap.isOpened())
        vcap.release();
}

void VideoSource::handle_time_out()
{
    //    prt(info,"%s src rate %d",url.toStdString().data(),frame_rate);
    //  frame_rate=0;
}
