#include <QCoreApplication>
#include "videosource.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    vector <VideoSource *>srcs;//

  //  ("rtsp://192.168.1.95:554/av0_1");
    Mat frame;
    int loop=100;
    while(loop--){
        srcs.push_back(new VideoSource ("rtsp://192.168.1.95:554/av0_1"));



//        bool ret= src->get_frame(frame);
//abc:         ret= src->get_frame(frame);

//              if(ret){
//            prt(info,"get frame");
//        }else{
//                  goto abc;
//              }
//        delete src;
    }
    return a.exec();
}

