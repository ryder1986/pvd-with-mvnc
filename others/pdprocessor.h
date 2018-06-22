#ifndef PDPROCESSOR_H
#define PDPROCESSOR_H

//#include <QObject>
//#include <QJsonArray>
//#include <QJsonValue>
#include "videoprocessor.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "testpd.h"
using namespace cv;



class PdProcessor : public VideoProcessor
{
public:
    PdProcessor():scanner(HUMAN_height,HUMAN_width,HUMAN_xdiv,HUMAN_ydiv,256,0.8),VideoProcessor()
    {
        loaded=false;
    }
//    bool process(Mat src_mt,QByteArray &rst)
//    {
//        bool ret=false;
//        rst.clear();
//        int mat_w=src_mt.cols;
//        int mat_h=src_mt.rows;
//        vector <Rect> rects;
//        if(detect_area.width+detect_area.x<src_mt.cols&& detect_area.height+detect_area.y<src_mt.rows){
//            Mat mt=src_mt(detect_area);
//            real_process(mt,rects);
//        }
//        if(rects.size()>0){
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
private:
    /*****************************************/
    // Pedestrian_ICRA.cpp
    /*****************************************/

    const int HUMAN_height = 108;
    const int HUMAN_width = 36;
    const int HUMAN_xdiv = 9;
    const int HUMAN_ydiv = 4;


    // The detector

    // ---------------------------------------------------------------------
    // Helper functions


    // compute the Sobel image "ct" from "original"

    // A simple post-process (NMS, non-maximal suppression)
    // "result" -- rectangles before merging
    //          -- after this function it contains rectangles after NMS
    // "combine_min" -- threshold of how many detection are needed to survive
    void PostProcess(std::vector<CRect>& result,const int combine_min)
    {
        std::vector<CRect> res1;
        std::vector<CRect> resmax;
        std::vector<int> res2;
        bool yet;
        CRect rectInter;

        for(unsigned int i=0,size_i=result.size(); i<size_i; i++)
        {
            yet = false;
            CRect& result_i = result[i];
            for(unsigned int j=0,size_r=res1.size(); j<size_r; j++)
            {
                CRect& resmax_j = resmax[j];
                if(result_i.Intersect(rectInter,resmax_j))
                {
                    if(  rectInter.Size()>0.6*result_i.Size()
                         && rectInter.Size()>0.6*resmax_j.Size()
                         )
                    {
                        CRect& res1_j = res1[j];
                        resmax_j.Union(resmax_j,result_i);
                        res1_j.bottom += result_i.bottom;
                        res1_j.top += result_i.top;
                        res1_j.left += result_i.left;
                        res1_j.right += result_i.right;
                        res2[j]++;
                        yet = true;
                        break;
                    }
                }
            }
            if(yet==false)
            {
                res1.push_back(result_i);
                resmax.push_back(result_i);
                res2.push_back(1);
            }
        }

        for(unsigned int i=0,size=res1.size(); i<size; i++)
        {
            const int count = res2[i];
            CRect& res1_i = res1[i];
            res1_i.top /= count;
            res1_i.bottom /= count;
            res1_i.left /= count;
            res1_i.right /= count;
        }

        result.clear();
        for(unsigned int i=0,size=res1.size(); i<size; i++)
            if(res2[i]>combine_min)
                result.push_back(res1[i]);
    }

    // If one detection (after NMS) is inside another, remove the inside one
    void RemoveCoveredRectangles(std::vector<CRect>& result)
    {
        std::vector<bool> covered;
        covered.resize(result.size());
        std::fill(covered.begin(),covered.end(),false);
        CRect inter;
        for(unsigned int i=0; i<result.size(); i++)
        {
            for(unsigned int j=i+1; j<result.size(); j++)
            {
                result[i].Intersect(inter,result[j]);
                double isize = inter.Size();
                if(isize>result[i].Size()*0.65)
                    covered[i] = true;
                if(isize>result[j].Size()*0.65)
                    covered[j] = true;
            }
        }
        std::vector<CRect> newresult;
        for(unsigned int i=0; i<result.size(); i++)
            if(covered[i]==false)
                newresult.push_back(result[i]);
        result.clear();
        result.insert(result.begin(),newresult.begin(),newresult.end());
        newresult.clear();
    }

    void LoadCascade(DetectionScanner& ds)
    {
        std::vector<NodeDetector::NodeType> types;
        std::vector<int> upper_bounds;
        std::vector<std::string> filenames;

        types.push_back(NodeDetector::CD_LIN); // first node
        //upper_bounds.push_back(256);	//100
        upper_bounds.push_back(100);	//100
        filenames.push_back("combined.txt.model");
        types.push_back(NodeDetector::CD_HIK); // second node
        //upper_bounds.push_back(117);	//353

        upper_bounds.push_back(20);	//353
        //      upper_bounds.push_back(50);	//353
        //         // upper_bounds.push_back(353);	//353
        filenames.push_back("combined2.txt.model");

        ds.LoadDetector(types,upper_bounds,filenames);
        // You can adjust these parameters for different speed, accuracy etc
        ds.cascade->nodes[0]->thresh += 0.8;
        ds.cascade->nodes[1]->thresh -= 0.095;
    }


    bool real_process( Mat &src_image, std::vector<cv::Rect> &result_rects)
    {
        bool ret=false;

        if(!loaded){

            LoadCascade(scanner);
            std::cout<<"Detectors loaded."<<std::endl;
            loaded=true;
        }

        //    int key = 0;
        //    int wait_time = 1;
        int step_size = 9;
        //   float rate = 0.5;
        bool rect_organization = true;
        IntImage<double> original;

        if(result_rects.size() != 0)
        {
            result_rects.clear();
        }


        //   while( key != 27 )
        {
            double start_time;
            start_time = cv::getTickCount();

            if( src_image.empty() )
            {
                // break;
            }

            //            if (rate < 1 && (src_image.cols >= 960 || src_image.rows >= 480))
            //            {
            //     cv::resize(src_image, src_image, cv::Size(), rate, rate);
            //            }
            //            if(detect_rect.x>=src_image.cols||detect_rect.y>=src_image.rows){

            //                detect_rect.x=src_image.cols;
            //                  detect_rect.y=src_image.rows;
            //                //exit(0);
            //            }
            //            detect_rect.width?(detect_rect.width>src_image.cols):src_image.cols;
            //            detect_rect.height?(detect_rect.height>src_image.rows):src_image.rows;

            //  if(detect_rect.width==0||detect_rect.height==0)
            //   {

            //                detect_rect.x=0;
            //                detect_rect.y=0;
            //                detect_rect.width=src_image.cols;
            //                detect_rect.height=src_image.rows;
            //   }
            cv::Mat detect_region = src_image;

            original.Load( detect_region );
            std::vector<CRect> results;
            scanner.FastScan(original, results, step_size);

            if(rect_organization)
            {
                PostProcess(results,2);
                PostProcess(results,0);
                RemoveCoveredRectangles(results);
            }
#if 0
            for(size_t i = 0; i < results.size(); i++)
            {
                cv::Rect real_position;
                real_position.x = results[i].left/2;
                real_position.y = results[i].top/2;
                real_position.width = (results[i].right - results[i].left)/2;
                real_position.height = (results[i].bottom - results[i].top)/2;

                //   cv::rectangle(detect_region, real_position, cv::Scalar(0,255,0), 2);
                result_rects.push_back(real_position);
            }
#else

            for(size_t i = 0; i < results.size(); i++)
            {
                cv::Rect real_position;
                real_position.x = results[i].left;
                real_position.y = results[i].top;
                real_position.width = (results[i].right - results[i].left);
                real_position.height = (results[i].bottom - results[i].top);

                //   cv::rectangle(detect_region, real_position, cv::Scalar(0,255,0), 2);
                result_rects.push_back(real_position);
            }


#endif
            //   rectangle(src_image, detect_rect, cv::Scalar(0,255,255), 2);		//画出检测区域
            //    cv::imshow("result",src_image);

            //  key = cv::waitKey( wait_time );

            double end_time = cv::getTickCount();
            double spend_time;
            spend_time = 1000 * (fabs(end_time - start_time) / cv::getTickFrequency());
            //    std::cout << "time : " << spend_time << " ms" << std::endl;
        }
        if(result_rects.size()>0)
            ret=true;
        return ret;

    }




private:
    bool loaded;
    DetectionScanner scanner;
};

#endif // PDPROCESSOR_H
