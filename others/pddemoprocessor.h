#ifndef PDDEMOPROCESSOR_H
#define PDDEMOPROCESSOR_H

#include "videoprocessor.h"
//#include <QJsonArray>
//#include <QJsonValue>
class PdDemoProcessor : public VideoProcessor
{
public:
    PdDemoProcessor():VideoProcessor()
    {
    }
//    bool process(Mat mt,QByteArray &rst)
//    {

//        rst.clear();

//        Mat detect=mt(detect_area);
//        bool  ret=false;
//        int mat_w=mt.cols;
//        int mat_h=mt.rows;
//        vector <Rect> rects;

//        if(real_process(detect,rects)){
//            rst.append(QString::number(mat_w)).append(",").append(QString::number(mat_h)).append(":");
//            foreach (Rect r, rects) {
//                QString x_str=QString::number(r.x+detect_area.x/2);
//                QString y_str=QString::number(r.y+detect_area.y/2);
//                QString width_str=QString::number(r.width);
//                QString height_str=QString::number(r.height);
//                rst.append(x_str).append(",").append(y_str).append(",").append(width_str).append(",").append(height_str).append(":");
//            }
//            ret=true;
//        }
//        return ret;

//    }
// Mat &src_image, std::vector<cv::Rect> &result_rects
    bool real_process(Mat &mt, std::vector<cv::Rect> &result_rects)
    {
        result_rects.clear();
        CascadeClassifier cascade;
        bool ret=false;
        vector<Rect> objs;
        string cascade_name = "hogcascade_pedestrians.xml";
        if (!cascade.load(cascade_name))
        {
            prt(info,"can't load cascade");
            exit(0);
        }



        Mat frame(mt);
        //        int mat_w=frame.cols;
        //        int mat_h=frame.rows;
        string str;
        str.clear();

        if (!frame.empty())
        {    //  if (frame_num % 3 == 0)
            if (1)
            {
                resize(frame,frame,Size(frame.cols / 2, frame.rows / 2),CV_INTER_LINEAR);
                cvtColor(frame, gray_frame, CV_BGR2GRAY);
                cascade.detectMultiScale(gray_frame, objs, 1.1, 3);
                vector<Rect>::iterator it = objs.begin();
                //                if (it != objs.end() && objs.size() != 0)
                //                {
                //                    str.append(QString::number(mat_w)).append(",").append(QString::number(mat_h)).append(":");
                //                }
                while (it != objs.end() && objs.size() != 0)
                {

                    //pedestrians = frame(*it);

                    Rect rct = *it;
                    {
                        rectangle(frame, rct, Scalar(0, 255, 0), 2);
                        result_rects.push_back(rct);
                        //                        QString x_str=QString::number(rct.x);
                        //                        QString y_str=QString::number(rct.y);
                        //                        QString width_str=QString::number(rct.width);
                        //                        QString height_str=QString::number(rct.height);
                        //QString test_str=QString::number(test);
                        //                        str.append(x_str).append(",").append(y_str).append(",").append(width_str).append(",").append(height_str).append(":");
                        //                        rst.append(str.toStdString().data());

                        ret=true;
                        //  break;//TODO, now we get first one
                    }
                    it++;
                }
                objs.clear();
            }
        }
        else{
            prt(info,"opencv handle frame error !");
        }


        if(result_rects.size()>0)
            ret=true;
        return ret;

    }
private:
    Mat gray_frame;
};

#endif // PDDEMOPROCESSOR_H
