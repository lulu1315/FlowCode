#include "opencv2/highgui.hpp"
#include "opencv2/video.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/optflow.hpp"
#include <fstream>
#include <limits>

using namespace std;
using namespace cv;
using namespace optflow;

const String keys = "{help h usage ? |      | print this message   }"
        "{@image1        |      | image1.flo}"
        "{@image2        |      | image2.exr}";
        
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
    parser.about("convert .flo file to .exr");
    if ( parser.has("help") || argc < 2 )
    {
        parser.printMessage();
        printf("EXAMPLES:\n");
        printf("./flo2exr in.flo out.exr\n");
        return 0;
    }
    
    String inputfile = parser.get<String>(0);
    String outputfile = parser.get<String>(1);

    if ( !parser.check() )
    {
        parser.printErrors();
        return 0;
    }
    
    Mat_<Point2f> flow;
    flow = optflow::readOpticalFlow(inputfile);
    
    if ( !flow.data )
    {
        printf("No flow data \n");
        return -1;
    }
        
    //write result as .exr
    Mat flow_3chan = flowTo3Channels(flow);
    cv::cvtColor(flow_3chan, flow_3chan, CV_BGR2RGB);
    cv::imwrite(outputfile, flow_3chan);
    printf("writing exr file : %s\n",outputfile.c_str());
    
}
