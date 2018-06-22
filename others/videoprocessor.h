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
    virtual  bool process( Mat img)
    {
        return false;
    }
    virtual  string get_rst()
    {
        string ba;
        return ba ;
    }
    virtual void init()
    {

    }
protected:
};


#endif // VIDEOPROCESSOR_H
