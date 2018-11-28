#include "opencv2/highgui.hpp"
#include "opencv2/video.hpp"
#include "opencv2/optflow.hpp"
#include "opencv2/core/ocl.hpp"
#include <fstream>
#include <limits>

using namespace std;
using namespace cv;
using namespace optflow;

const String keys = "{help h usage ? |      | print this message   }"
        "{@image1        |      | image1}"
        "{@image2        |      | image2}"
        "{@output        |      | output}"
        "{d downsample   |1.0| downsample factor for image1 and image2}"
        "{g gpu          |0| use OpenCL}"
        "{e exr          |0| write output as exr file}";
        
static Mat flowToDisplay(const Mat flow)
{
    Mat flow_split[2];
    Mat magnitude, angle;
    Mat hsv_split[3], hsv, rgb;
    split(flow, flow_split);
    cartToPolar(flow_split[0], flow_split[1], magnitude, angle, true);
    normalize(magnitude, magnitude, 0, 1, NORM_MINMAX);
    hsv_split[0] = angle; // already in degrees - no normalization needed
    hsv_split[1] = Mat::ones(angle.size(), angle.type());
    hsv_split[2] = magnitude;
    merge(hsv_split, 3, hsv);
    cvtColor(hsv, rgb, COLOR_HSV2BGR);
    return rgb;
}

static Mat flowTo3Channels(const Mat flow)
{
    Mat flow_split[2],rgb;
    Mat blue_chan = Mat::zeros(flow.size(), CV_32FC1);
    split(flow, flow_split);
    vector<Mat> channels;
    channels.push_back(flow_split[0]);
    channels.push_back(flow_split[1]);
    channels.push_back(blue_chan);
    merge(channels,rgb);
    return rgb;
}

int main( int argc, char** argv )
{
    CommandLineParser parser(argc, argv, keys);
    parser.about("OpenCV deepflow calculation");
    if ( parser.has("help") || argc < 2 )
    {
        parser.printMessage();
        printf("EXAMPLES:\n");
        printf("./deepflow_opencv im1.png im2.png out.flo\n");
        printf("./deepflow_opencv --gpu im1.png im2.png out.flo\n");
        printf("./deepflow_opencv --exr --gpu im1.png im2.png out.exr\n");
        printf("\t use -d to downsample the flow computation (-d=.5 will compute at half resolution)\n");
        printf("\t compute flow field between im1 and im2 with opencv deepflow method\n");
        return 0;
    }
    
    String inputfile1 = parser.get<String>(0);
    String inputfile2 = parser.get<String>(1);
    String outputfile = parser.get<String>(2);
    const bool useGpu = parser.get<bool>("gpu");
    const bool useExr = parser.get<bool>("exr");
    double downsample  = parser.get<double>("downsample");
    

    
    if ( !parser.check() )
    {
        parser.printErrors();
        return 0;
    }
    
    String method ="deepflow";
    const bool display_images = 0;
    
    cv::ocl::setUseOpenCL(useGpu);
    printf("OpenCL Enabled: %u\n", useGpu && cv::ocl::haveOpenCL());
    
    Mat i1, i2;
    Mat_<Point2f> flow;
    i1 = imread(inputfile1, 1);
    i2 = imread(inputfile2, 1);
    
    if ( !i1.data || !i2.data )
    {
        printf("No image data \n");
        return -1;
    }
    if ( i1.size() != i2.size() || i1.channels() != i2.channels() )
    {
        printf("Dimension mismatch between input images\n");
        return -1;
    }
    // 8-bit images expected by all algorithms
    if ( i1.depth() != CV_8U )
        i1.convertTo(i1, CV_8U);
    if ( i2.depth() != CV_8U )
        i2.convertTo(i2, CV_8U);
    cvtColor(i1, i1, COLOR_BGR2GRAY);
    cvtColor(i2, i2, COLOR_BGR2GRAY);
    
    Size originalsize=i1.size();
    
    if (downsample !=1.0) {
        printf("downsampling input frames : x%f\n",downsample);
        resize(i1,i1, Size(), downsample, downsample, INTER_CUBIC);
        resize(i2,i2, Size(), downsample, downsample, INTER_CUBIC);
    }
    
    flow = Mat(i1.size[0], i1.size[1], CV_32FC2);
    
    Ptr<DenseOpticalFlow> algorithm;
    if ( method == "deepflow" )
        algorithm = createOptFlow_DeepFlow();
    
    double startTick, time;
    startTick = (double) getTickCount(); // measure time
    
    if (useGpu)
        algorithm->calc(i1, i2, flow.getUMat(ACCESS_RW));
    else
        algorithm->calc(i1, i2, flow);
    
    time = ((double) getTickCount() - startTick) / getTickFrequency();
    printf("Time [s]: %.3f\n", time);
    
    if(display_images)
    {
        Mat flow_image = flowToDisplay(flow);
        namedWindow( "Computed flow", WINDOW_AUTOSIZE );
        imshow( "Computed flow", flow_image );
        waitKey(0);
    }
    
    if (downsample !=1.0) {
        printf("upsampling flow to original size: x%f\n",1/downsample);
        resize(flow,flow, originalsize, 0, 0, INTER_CUBIC);
    }
        
    if (useExr) {
    //write result as .exr
    Mat flow_3chan = flowTo3Channels(flow);
    cv::cvtColor(flow_3chan, flow_3chan, CV_BGR2RGB);
    cv::imwrite(outputfile, flow_3chan);
    printf("writing exr file : %s\n",outputfile.c_str());
    }
    else {
    //write result as .flow
    optflow::writeOpticalFlow(outputfile,flow);
    printf("writing flo file : %s\n",outputfile.c_str());
    }
    
}
