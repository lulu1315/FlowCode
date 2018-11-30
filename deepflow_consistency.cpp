#include <cstdio>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using std::cout;
using std::endl;

int main(int argc, char **argv)
{
    
    char *process_root=     argv[1];
    char *ref_root=         argv[2];
    char *oflow_root=       argv[3];
    char *consistency_root= argv[4];
    char *output_file_root= argv[5];
    char *ext=              argv[6];
    float oflowblend =   atof(argv[7]); //0.9
    int fstart= atoi(argv[8]);
    int fend=   atoi(argv[9]);
    
    cv::Mat process,ref,oflow,consistency,mapx,mapy,warped,previous;
    
    for (int frame = fstart; frame <= fend; frame++) 
        {
        //dealing with filename
        char processname[200];
        sprintf(processname,"%s.%04d.%s",process_root,frame,ext);
        process = cv::imread(processname,CV_LOAD_IMAGE_COLOR); 
        char outputname[200];
        sprintf(outputname,"%s.%04d.%s",output_file_root,frame,ext);
        cout << "output : " << outputname << endl;
        if (frame > fstart)
            {
            //read oflow
            char oflowname[200];
            sprintf(oflowname,"%s_%04d_%04d.exr",oflow_root,frame,frame-1);
            cout << "oflow : " << oflowname << endl;
            oflow = cv::imread(oflowname, IMREAD_UNCHANGED);  
            if(! oflow.data )                              // Check for invalid input
                {
                cout <<  "Could not open or find a precomputed optical flow field" << std::endl ;
                return -1;
                }
            //read consistency
            char consistencyname[200];
            sprintf(consistencyname,"%s_%04d_%04d.pgm",consistency_root,frame,frame-1);
            cout << "consistency : " << consistencyname << endl;
            consistency = cv::imread(consistencyname, CV_LOAD_IMAGE_GRAYSCALE); 
            if(! consistency.data )                              // Check for invalid input
                {
                cout <<  "Could not open or find a consistency descriptor" << std::endl ;
                return -1;
                }
            //read previous frame
            char previousname[200];
            sprintf(previousname,"%s.%04d.%s",output_file_root,frame-1,ext);
            cout << "previousresult : " << previousname << endl;
            previous = cv::imread(previousname,CV_LOAD_IMAGE_COLOR); 
            //warp with optical flow
            warped.create( oflow.size(), CV_8UC3);
            mapx = cv::Mat::zeros(oflow.size(), CV_32FC1);
            mapy = cv::Mat::zeros(oflow.size(), CV_32FC1);
            for (int y = 0; y < oflow.rows; ++y)
                {
                for (int x = 0; x < oflow.cols; ++x)
                    {
                    Vec3f of = oflow.at<Vec3f>(y, x);
                    //cout <<  x << " " << y << "pixel : " << of  << std::endl ;
                    mapx.at<float>(y, x) = x + of.val[2];
                    mapy.at<float>(y, x) = y + of.val[1];
                    }
                }
            cv::remap(previous, warped, mapx, mapy, INTER_CUBIC , BORDER_CONSTANT, Scalar(255, 255, 255) );
            
            //reinject original through consistency
            #pragma omp parallel for
            for (int y = 0; y < previous.rows; y++) {
                for (int x = 0; x < previous.cols; x++) {
                    Vec3b W = warped.at<Vec3b>(y, x);
                    float C = consistency.at<uchar>(y, x)/255.0;
                    Vec3b R = process.at<Vec3b>(y, x);
                    //mix warped previous with result through consistency
                    Vec3b O = (W*C) + (R*(1.0-C));
                    //blend result by weight
                    Vec3b OO = O*oflowblend +R*(1.0 - oflowblend);
                    //float O = std::min(OO,R);
                    process.at<Vec3b>(y, x) = OO;
                    }
                }
            }
        //else
        //    {
        //    warped=process;
        //    }
        cout << "--> processed : frame " << frame << endl;
        cv::imwrite(outputname, process);
        //preview
        bool withpreview=0;
        if (withpreview)
            {
            cv::namedWindow("consistency");
            cv::imshow("consistency", warped);
            cv::waitKey(0);
            }
        }
}
