#include "opencv2/highgui.hpp"
#include "opencv2/video.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/optflow.hpp"

#include <limits>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

static Mat flowToDisplay(const Mat flow)
{
    Mat flow_split[2];
    Mat magnitude, angle;
    Mat hsv_split[3], hsv, rgb;
    split(flow, flow_split);
    cartToPolar(flow_split[0], flow_split[1], magnitude, angle, true);
    normalize(magnitude, magnitude, 0, 255, NORM_MINMAX);
    int width =flow.cols;
    int height=flow.rows;
    hsv_split[0] = angle; // already in degrees - no normalization needed
    hsv_split[1] = Mat::ones(angle.size(), angle.type());
    hsv_split[2] = magnitude;
    merge(hsv_split, 3, hsv);
    cvtColor(hsv, rgb, COLOR_HSV2BGR);
    
 /*   for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                Point3f val=rgb.at<Point3f>(i, j);
                cout << val  << endl;
            }
        }
 */       
    return rgb;
}

static Mat exrTo2Channels(const Mat ima)
{
    Mat ima_split[4];
    Mat flow = Mat::zeros(ima.size(), CV_32FC2);
    split(ima, ima_split);
    vector<Mat> channels;
    channels.push_back(ima_split[2]);
    channels.push_back(ima_split[1]);
    merge(channels,flow);
    return flow;
}

float mag(Point2f vec) { //magnitude of vector
    return sqrt((vec.x*vec.x)+(vec.y*vec.y));
}

Mat DrawFlow(Mat flow, Mat background, int flowcolor, int sampling, float vscale, float motiontreshold, float &maxforce) {
    int flowidth =flow.cols;
    int flowheight=flow.rows;
    int backwidth =background.cols;
    int backheight=background.rows;
    float ratio=(float)backwidth/(float)flowidth;
    //cout << "ratio : " << ratio << endl;
    Point2f getforce,pos;
    for (int i = 0; i < flowheight; i++) {
		for (int j = 0; j < flowidth; j++) {
            pos.x=j;
            pos.y=i;
            Point2f val=flow.at<Point2f>(i, j);
            if (mag(val) > maxforce) {maxforce = mag(val);}
            if (!(i%sampling) && !(j%sampling)) {
                if (mag(val) > motiontreshold) {
                    circle(background,ratio*pos,1,Scalar(flowcolor,flowcolor,flowcolor),1,CV_AA);
                    line(background,ratio*pos,(ratio*pos)+(ratio*val*vscale),Scalar(flowcolor,flowcolor,flowcolor),1,CV_AA);
                    }
            }
        }
    }
    return background;
}

int main(int argc, const char* argv[])
{
    cout << "showflow ima_in ext background[black,white,flow,ima.png] flowcolor ima_out.png" << endl;
    string image_in_name;
    string ext,back;
    string image_out_name;
    image_in_name   = argv[1];
    ext             = argv[2];
    back            = argv[3];
    int flowcolor   = atoi(argv[4]);
    image_out_name  = argv[5];
    
    int sampling = atoi(argv[6]); //10
    float vscale = atof(argv[7]); //1
    float gamma  = atof(argv[8]); //.8
    float motiontreshold  = atof(argv[9]); //.5
    
    Mat background;
    Mat_<Point2f> flow;
    float maxforce=0;
    
    //read and convert image in
    if (ext == "exr") {
        Mat exrflow = imread(image_in_name+"."+ext,-1);
        cout << "reading : " << image_in_name << "." << ext << endl;
        int flowidth =exrflow.cols;
        int floheight=exrflow.rows;
        cout << "input width/height : " << flowidth << "/" << floheight  << endl;
        flow=exrTo2Channels(exrflow);
        background = flowToDisplay(flow);
        }
    if (ext == "flo") {
        flow = optflow::readOpticalFlow(image_in_name+"."+ext);
        cout << "reading : " << image_in_name << "." << ext << endl;
        int width =flow.cols;
        int height=flow.rows;
        cout << "input width/height : " << width << "/" << height  << endl;
        background = flowToDisplay(flow);
        }
    //background
    if (back == "white") {
        background = cv::Scalar(255,255,255);
        }
    else if (back == "black") {
        background = cv::Scalar(0,0,0);
        }
    else if (back == "flow") {
        cout << "using coloured flow as background" << endl;
        }
    else {
        background = imread(back,cv::IMREAD_GRAYSCALE);
        cout << "reading : " << back << endl;
        int width =background.cols;
        int height=background.rows;
        cout << "input width/height : " << width << "/" << height  << endl;
        // Convert to double for "pow"
        cv::Mat1d dsrc;
        background.convertTo(dsrc, CV_64F);
        // Compute the "pow"
        cv::Mat1d ddst;
        cv::pow(dsrc, gamma, ddst);
        // Convert back to uchar
        ddst.convertTo(background, CV_8U);
        }
        
    //draw vectors
    
    Mat imafinal=DrawFlow(flow,background,flowcolor,sampling,vscale,motiontreshold,maxforce);
    cout << "maximum flow magnitude : " << maxforce << endl;
        
    imwrite(image_out_name, imafinal);
    cout << "writing png file : " << image_out_name << endl;
}
