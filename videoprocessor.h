#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "tool.h"
#include "pvd.h"
using namespace std;
using namespace cv;
class VideoProcessor
{
public:
    string alg_rst;
    VideoProcessor()
    {
    }
    virtual  void prepare(QJsonValue v)
    {
    }
    int get_id()
    {
        return channel_id;
    }
    virtual  bool process( Mat img)
    {
        return false;
    }
    virtual  string get_rst()
    {

        return alg_rst ;
    }
    virtual void init()
    {
    }
    virtual int get_percent()
    {
        return percent;
    }
    virtual int set_percent(int tmp)
    {
        percent=tmp;
    }
    virtual int get_busy_state()
    {
        return busy_state;
    }
    virtual int set_busy_state(int tmp)
    {
        busy_state=tmp;
    }
    virtual int get_exist_state()
    {
        return exist_state;
    }
    virtual int set_exist_state(bool tmp)
    {
        exist_state=tmp;
    }
protected:
    int channel_id;
    int percent;
    int busy_state;
    int exist_state;
private:


};
#endif // VIDEOPROCESSOR_H
