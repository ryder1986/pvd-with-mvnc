#include "testpd.h"

TestPD::TestPD()
{

}




// find the intersection of "this" and "rect2", and put into "result"
bool CRect::Intersect(CRect& result,const CRect& rect2) const
{
    if( Empty() || rect2.Empty() ||
        left >= rect2.right || rect2.left >= right ||
        top >= rect2.bottom || rect2.top >= bottom )
    {
        result.Clear();
        return false;
    }
    result.left   = std::max( left, rect2.left );
    result.right  = std::min( right, rect2.right );
    result.top    = std::max( top, rect2.top );
    result.bottom = std::min( bottom, rect2.bottom );
    return true;
}

// find the union of "this" and "rect2", and put into "result"
bool CRect::Union(CRect& result,const CRect& rect2) const
{
    if(Empty())
    {
        if(rect2.Empty())
        {
            result.Clear();
            return false;
        }
        else
            result = rect2;
    }
    else
    {
        if(rect2.Empty())
            result = *this;
        else
        {
            result.left   = std::min( left, rect2.left );
            result.right  = std::max( right, rect2.right );
            result.top    = std::min( top, rect2.top );
            result.bottom = std::max( bottom, rect2.bottom );
        }
    }
    return true;
}


// End of Helper functions
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// Functions that load the two classifiers
//void LoadCascade(DetectionScanner& ds)
//{
//    std::vector<NodeDetector::NodeType> types;
//    std::vector<int> upper_bounds;
//    std::vector<std::string> filenames;

//    types.push_back(NodeDetector::CD_LIN); // first node
//    upper_bounds.push_back(256);	//100
//    filenames.push_back("combined.txt.model");
//    types.push_back(NodeDetector::CD_HIK); // second node
//    upper_bounds.push_back(353);	//353
//    filenames.push_back("combined2.txt.model");

//    ds.LoadDetector(types,upper_bounds,filenames);
//    // You can adjust these parameters for different speed, accuracy etc
//    ds.cascade->nodes[0]->thresh += 0.8;
//    ds.cascade->nodes[1]->thresh -= 0.095;
//}

void DetectionScanner::LoadDetector(std::vector<NodeDetector::NodeType>& types,std::vector<int>& upper_bounds,std::vector<std::string>& filenames)
{
    unsigned int depth = types.size();
    assert(depth>0 && depth==upper_bounds.size() && depth==filenames.size());
    if(cascade)
        delete cascade;
    cascade = new CascadeDetector;
    assert(xdiv>0 && ydiv>0);
    for(unsigned int i=0; i<depth; i++)
        cascade->AddNode(types[i],(xdiv-EXT)*(ydiv-EXT)*baseflength,upper_bounds[i],filenames[i].c_str());

    hist.Create(1,baseflength*(xdiv-EXT)*(ydiv-EXT));
}

void NodeDetector::Load(const NodeType _type,const int _featurelength,const int _upper_bound,const int _index,const char* _filename)
{
    type = _type;
    index = _index;
    filename = _filename;
    featurelength = _featurelength;
    upper_bound = _upper_bound;
    if(type==CD_LIN)
        thresh = UseSVM_CD_FastEvaluationStructure(_filename,_featurelength,classifier);
    else if(type==CD_HIK)
        thresh = UseSVM_CD_FastEvaluationStructure(_filename,_featurelength,upper_bound,classifier);

    if(type==CD_LIN) type = LINEAR;
    if(type==CD_HIK) type = HISTOGRAM;
}

void CascadeDetector::AddNode(const NodeDetector::NodeType _type,const int _featurelength,const int _upper_bound,const char* _filename)
{
    if(length==size)
    {
        int newsize = size * 2;
        NodeDetector** p = new NodeDetector*[newsize];
        assert(p!=NULL);
        std::copy(nodes,nodes+size,p);
        size = newsize;
        delete[] nodes;
        nodes = p;
    }
    nodes[length] = new NodeDetector(_type,_featurelength,_upper_bound,length,_filename);
    length++;
}

// End of functions that load the two classifiers
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// Detection functions

// initialization -- compute the Census Tranform image for CENTRIST
void DetectionScanner::InitImage(IntImage<double>& original)
{
    image = original;
    image.Sobel(sobel,false,false);
    ComputeCT(sobel,ct);
}

// combine the (xdiv-1)*(ydiv-1) integral images into a single one
void DetectionScanner::InitIntegralImages(const int stepsize)
{
    if(cascade->nodes[0]->type!=NodeDetector::LINEAR)
        return; // No need to prepare integral images

    const int hd = height/xdiv*2-2;
    const int wd = width/ydiv*2-2;
    scores.Create(ct.nrow,ct.ncol);
    scores.Zero(cascade->nodes[0]->thresh/hd/wd);
    double* linearweights = cascade->nodes[0]->classifier.buf;
    for(int i=0; i<xdiv-EXT; i++)
    {
        const int xoffset = height/xdiv*i;
        for(int j=0; j<ydiv-EXT; j++)
        {
            const int yoffset = width/ydiv*j;
            for(int x=2; x<ct.nrow-2-xoffset; x++)
            {
                int* ctp = ct.p[x+xoffset]+yoffset;
                double* tempp = scores.p[x];
                for(int y=2; y<ct.ncol-2-yoffset; y++)
                    tempp[y] += linearweights[ctp[y]];
            }
            linearweights += baseflength;
        }
    }
    scores.CalcIntegralImageInPlace();
    for(int i=2; i<ct.nrow-2-height; i+=stepsize)
    {
        double* p1 = scores.p[i];
        double* p2 = scores.p[i+hd];
        for(int j=2; j<ct.ncol-2-width; j+=stepsize)
            p1[j] += (p2[j+wd] - p2[j] - p1[j+wd]);
    }
}

// Resize the input image and then re-compute Sobel image etc
void DetectionScanner::ResizeImage()
{
    image.Resize(sobel,ratio);
    image.Swap(sobel);
    image.Sobel(sobel,false,false);
    ComputeCT(sobel,ct);
}

// The function that does the real detection
int DetectionScanner::FastScan(IntImage<double>& original,std::vector<CRect>& results,const int stepsize)
{
    if(original.nrow<height+5 || original.ncol<width+5) return 0;
    const int hd = height/xdiv;
    const int wd = width/ydiv;
    InitImage(original);
    results.clear();

    hist.Create(1,baseflength*(xdiv-EXT)*(ydiv-EXT));

    NodeDetector* node = cascade->nodes[1];
    double** pc = node->classifier.p;
    int oheight = original.nrow, owidth = original.ncol;
    CRect rect;
    while(image.nrow>=height && image.ncol>=width)
    {
        InitIntegralImages(stepsize);
        for(int i=2; i+height<image.nrow-2; i+=stepsize)
        {
            const double* sp = scores.p[i];
            for(int j=2; j+width<image.ncol-2; j+=stepsize)
            {
                if(sp[j]<=0) continue;
                int* p = hist.buf;
                hist.Zero();
                for(int k=0; k<xdiv-EXT; k++)
                {
                    for(int t=0; t<ydiv-EXT; t++)
                    {
                        for(int x=i+k*hd+1; x<i+(k+1+EXT)*hd-1; x++)
                        {
                            int* ctp = ct.p[x];
                            for(int y=j+t*wd+1; y<j+(t+1+EXT)*wd-1; y++)
                                p[ctp[y]]++;
                        }
                        p += baseflength;
                    }
                }
                double score = node->thresh;
                for(int k=0; k<node->classifier.nrow; k++) score += pc[k][hist.buf[k]];
                if(score>0)
                {
                    rect.top = i*oheight/image.nrow;
                    rect.bottom = (i+height)*oheight/image.nrow;
                    rect.left = j*owidth/image.ncol;
                    rect.right = (j+width)*owidth/image.ncol;
                    results.push_back(rect);
                }
            }
        }
        ResizeImage();
    }
    return 0;
}

// The interface function that detects pedestrians
// "filename" -- an input image
// detect pedestrians in image "filename" and save results to "outname"
// "ds" -- the detector cascade -- pass "scanner" as this parameter
// "out" -- file stream that saves the output rectangle information
//int totaltime = 0;

//int DetectHuman(const char* filename,DetectionScanner& ds)
//{
//    std::vector<CRect> results;
//    IntImage<double> original;
//    //    original.Load(filename);

//    ds.FastScan(original,results,2);
//    PostProcess(results,2);
//    PostProcess(results,0);
//    RemoveCoveredRectangles(results);

//    cvNamedWindow("show");
//    {
//        IplImage* iplImage = NULL;
//        iplImage = cvLoadImage(filename);
//        for(unsigned int i=0; i<results.size(); i++)
//            cvRectangle(iplImage,cvPoint(results[i].left,results[i].top),cvPoint(results[i].right,results[i].bottom),CV_RGB(255,0,0),2);
//        cvShowImage("show",iplImage);
//        cvWaitKey();
//        cvReleaseImage(&iplImage);
//    }
//    return (int)results.size();
//}

// End of Detection functions
// ---------------------------------------------------------------------
//#define DEBUG_MODLE		1// 1 means self-defined detect area, 0 means whole area
//void detect_pedestrians(Mat &src_image, cv::Rect &detect_ROI, std::vector<cv::Rect> &result_rects)
//{
//    LoadCascade(scanner);
//    std::cout<<"Detectors loaded."<<std::endl;
//    int key = 0;
//    int wait_time = 1;
//    int step_size = 9;
//    float fx = 0.5;
//    bool rect_organization = true;
//    IntImage<double> original;

//    if(result_rects.size() != 0)
//    {
//        result_rects.clear();
//    }


//    while( key != 27 )
//    {
//        double start_time;
//        start_time = cv::getTickCount();

//                if( src_image.empty() )
//        {
//            break;
//        }

//        if (fx < 1 && (src_image.cols > 960 || src_image.rows > 480))
//        {
//                        cv::resize(src_image, src_image, cv::Size(), fx, fx);
//        }

//        cv::Mat detect_region = src_image(detect_ROI);

//#ifdef 	DEBUG_MODLE			//指定矩形检测区域模式
//        original.Load(detect_region);
//        std::vector<CRect> results;
//        scanner.FastScan(original, results, step_size);

//        if(rect_organization)
//        {
//            PostProcess(results,2);
//            PostProcess(results,0);
//            RemoveCoveredRectangles(results);
//        }

//        for(size_t i = 0; i < results.size(); i++)
//        {
//            cv::Rect real_position;
//            real_position.x = results[i].left + detect_ROI.x;
//            real_position.y = results[i].top + detect_ROI.y;
//            real_position.width = results[i].right - results[i].left;
//            real_position.height = results[i].bottom - results[i].top;

//            cv::rectangle(src_image, real_position, cv::Scalar(0,255,0), 2);
//            result_rects.push_back(real_position);
//        }

//        rectangle(src_image, detect_ROI, cv::Scalar(0,255,255), 2);		//画出检测区域

//        cv::imshow("result",src_image);

//#else					//不指定矩形检测区域模式
//        original.Load( src );
//        std::vector<CRect> results;
//        scanner.FastScan(original, results, step_size);

//        if(rect_organization)
//        {
//            PostProcess(results,2);
//            PostProcess(results,0);
//            RemoveCoveredRectangles(results);
//        }

//        for(size_t i = 0; i < results.size(); i++)
//        {
//            cv::Rect real_position;
//            real_position.x = results[i].left;
//            real_position.y = results[i].top;
//            real_position.width = results[i].right - results[i].left;
//            real_position.height = results[i].bottom - results[i].top;

//            cv::rectangle(src_image, real_position, cv::Scalar(0,255,0), 2);
//            result_rects.push_back(real_position);
//        }

//        cv::imshow("result",src_image);
//#endif

//        key = cv::waitKey( wait_time );

//        double end_time = cv::getTickCount();
//        double spend_time;
//        spend_time = 1000 * (fabs(end_time - start_time) / cv::getTickFrequency());
//        std::cout << "time : " << spend_time << " ms" << std::endl;
//    }

//}
