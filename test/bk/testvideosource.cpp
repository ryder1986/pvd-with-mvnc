#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "../tool.h"
#include "../videosource.h"
using namespace cv;
static char test_buf[2]={"1"};
#if 1
void save_file(char * data,int posi)
{
    FILE *f=fopen("test.y","a");
    fseek(f,posi,SEEK_SET);
int comp=fwrite(data,1,640*480,f);
    //    int comp=fwrite(test_buf,1,1,f);
    test_buf[0]++;
       //if(comp!=1)
    prt(info,"error in saving file:%d",comp);
    fclose(f);
}
void read_file(char * data,int posi)
{
    FILE *f=fopen("test.y","rb");
    fseek(f,posi,SEEK_SET);
    int comp=fread(data,480,640,f);
    //if(comp!=1)
    prt(info,"error in reading file:%d",comp);
    fclose(f);
}
//    bool load_file(char  &data)
//    {
//        data.clear();
//        FILE *f=fopen(name.data(),"rb");
//        if(f==NULL)
//            return false;
//        static char ss[2];
//        while(fread(ss,1,1,f)==1)
//        {
//            ss[1]=0;
//            data.append(ss);
//        }
//        fclose(f);
//        return true;
//    }
static  int opened=0;
static  int pos=0;
void save_gray(Mat mt)
{
    //        cv::imshow("ss",mt);
    //        cv::waitKey(10);
    if(pos==100)
    {
        cout<<"end";
        return;
    }
    //   static char buf[640*480];
    if(!opened){
        opened=1;
    }else{
        cv::cvtColor(mt,mt,CV_BGR2GRAY);
        save_file((char *)mt.data,640*480*pos++);
    }
}
void read_gray(char *buf)
{
    if(pos==100)
    {
        cout<<"end";
        return;
    }
    //  static char buf[640*480];
    if(!opened){
        opened=1;
    }else{
        //   cv::cvtColor(mt,mt,CV_BGR2GRAY);
        read_file((char *)buf,640*480*pos++);
    }
}

#endif
int main()
{
#if 0
    VideoSource src("/media/211/videos/video-demo/8s-640x480-gop.mp4");
    Mat f;
    while(1)
    {
        if( src.get_frame(f))
        {   cv::imshow("111",f);
            cv::waitKey(10);

            save_gray(f);
        }
    }
#else
    char buf[640*480];
    while(1)
    {

        read_gray(buf);
        cv::Mat img1(480,640,CV_8UC1,buf) ;
        //   cv::cvtColor(img1,img1,CV_BGR2GRAY);
        //    this_thread::sleep_for(chrono::milliseconds(100));
        cv::imshow("111",img1);
        cv::waitKey(300);
    }
#endif
    return 0;
}
