static const char *usage = "\n  usage: %s [-quiet] in.flo out.exr normalisation\n";

#include <stdio.h>
#include <math.h>
#include "imageLib.h"
#include "flowIO.h"
#include "colorcode.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "tinyexr.h"

int verbose = 1;

int main(int argc, char *argv[])
{
    try {
	int argn = 1;
	if (argc > 1 && argv[1][0]=='-' && argv[1][1]=='q') {
	    verbose = 0;
	    argn++;
	}
	if (argn >= argc-4 && argn <= argc-3) {
	    char *flowname = argv[argn++];
	    char *exrname = argv[argn++];
	    float normalisation = argn < argc ? atof(argv[argn++]) : -1;
	    int width, height;
	    const char* err;
//load flo image
	    CFloatImage im, fband;
	    ReadFlowFile(im, flowname);
	    CShape sh = im.Shape();
	    sh.nBands = 2;
	    width = sh.width;
	    height = sh.height;
	    if (verbose) {
	      printf("\nLoaded flow file. [ \033[22;35m%s\033[22;37m ] size = [%d, %d]\n",  flowname, width, height);
//	      printf("size = %d, %d\n", width, height);
	      printf("normalisation = %f\n", normalisation);
	      }
//exr image
	    EXRHeader header;
	    InitEXRHeader(&header);
	    EXRImage image;
	    InitEXRImage(&image);
	    image.num_channels = 3;
	    std::vector<float> images[3];
	    images[0].resize(width * height);
	    images[1].resize(width * height);
	    images[2].resize(width * height);
//im.Pixel(x, y, 0);
	    int x, y, i;
	    float normx,normy;
	    int clamped=0;
	    float maxx = -999, maxy = -999;
	    float minx =  999, miny =  999;
	    float fx,fy;
	    for (y = 0; y < height; y++) {
	      for (x = 0; x < width; x++) {
		i=y*width+x;
		fx = im.Pixel(x, y, 0);
		fy = -im.Pixel(x, y, 1);
		normx=(((fx/normalisation)+1)/2);
		if (normx > 1) {normx=1;clamped=1;}
		if (normx < 0) {normx=0;clamped=1;}
		normy=(((fy/normalisation)+1)/2);
		if (normy > 1) {normy=1;clamped=1;}
		if (normy < 0) {normy=0;clamped=1;}
//		printf("x,y,i : %d,%d,%d\n",x,y,i);
		images[0][i] = normx;
		images[1][i] = normy;
		images[2][i] = 0;
		maxx = __max(maxx, fx);
		maxy = __max(maxy, fy);
		minx = __min(minx, fx);
		miny = __min(miny, fy);
		}
	    }
	    //http://www.linuxforums.org/forum/linux-programming-scripting/88-color-console.html
	    if (verbose) {
	      if (clamped) {printf("\033[22;31mWarning ! ..... motion has been clamped\n\033[22;37m");}
	      printf("\033[22;32m min,max motion X : %.3f,%.3f\n\033[22;37m",minx,maxx);
	      printf("\033[22;32m min,max motion Y : %.3f,%.3f\n\033[22;37m",miny,maxy);
	    }
	    float* image_ptr[3];
	    image_ptr[0] = &(images[2].at(0)); // B
	    image_ptr[1] = &(images[1].at(0)); // G
	    image_ptr[2] = &(images[0].at(0)); // R
	    image.images = (unsigned char**)image_ptr;
	    image.width = width;
	    image.height = height;
	    header.num_channels = 3;
	    header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels); 
	    // Must be BGR(A) order, since most of EXR viewers expect this channel order.
	    strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
	    strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
	    strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';
	    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels); 
	    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
	    for (int i = 0; i < header.num_channels; i++) {
		header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
		header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
		}
	    int ret = SaveEXRImageToFile(&image, &header, exrname, &err);
	    if (ret != TINYEXR_SUCCESS) {
		  fprintf(stderr, "Save EXR err: %s\n", err);
		  return ret;
		  }
	    if (verbose) {
	      printf("Saved exr file. [ \033[22;35m%s\033[22;37m ] \n\n",  exrname);
	      }
	    free(header.channels);
	    free(header.pixel_types);
	    free(header.requested_pixel_types);
	} else
	    throw CError(usage, argv[0]);
    }
    catch (CError &err) {
	fprintf(stderr, err.message);
	fprintf(stderr, "\n");
	return -1;
    }

    return 0;
}