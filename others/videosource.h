#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <QObject>
#include <QThread>
#include <QJsonObject>
#include <QTimer>
#include "tool.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

class VideoSource : public QThread
{
    Q_OBJECT
public:
    VideoCapture vcap;
    QList <Mat> frame_list;
    int frame_wait_time;
    //   explicit VideoSource(QJsonObject config);
    VideoSource(string path);
    ~VideoSource();
    inline string get_url()
    {
        return url;
    }

    bool get_frame(Mat &frame)
    {
        if(frame_list.size()>1){
            frame=frame_list.first();
            frame_list.pop_front();
            return true;
        }else{
            return false;
        }
    }

    bool get_size(int &w,int &h)
    {
        bool ret=false;
        if(vcap.isOpened()){
            ret=true;
            w=vcap.get(CV_CAP_PROP_FRAME_WIDTH);
            h=vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
        }
        return ret;
    }
private:
    void run();
signals:
public slots:
    void handle_time_out()
    {
        //    prt(info,"%s src rate %d",url.toStdString().data(),frame_rate);
        frame_rate=0;
    }
private:
    int frame_rate;
    string url;
    bool quit_flg;
    QTimer *tmr;
};

#endif // VIDEOSOURCE_H
