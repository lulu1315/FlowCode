#include <stdio.h>
#include <math.h>
#include "imageLib/imageLib.h"
#include "flowIO.h"
#include "colorcode.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "tinyexr/tinyexr.h"

static const char *usage = "\n  usage: %s [-quiet] in.flo out.exr\n";

int verbose = 1;

int main(int argc, char *argv[])
{
    try {
	int argn = 1;
	if (argc > 1 && argv[1][0]=='-' && argv[1][1]=='q') {
	    verbose = 0;
	    argn++;
	}
	if (argn >= argc-2 && argn <= argc-1) {
	    char *flowname = argv[argn++];
	    char *exrname = argv[argn++];
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
	      printf("image = %s\n", flowname);
	      printf("size = %d, %d\n", width, height);
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

	    for (y = 0; y < height; y++) {
	      for (x = 0; x < width; x++) {
		i=y*width+x;
//		printf("x,y,i : %d,%d,%d\n",x,y,i);
		images[0][i] = im.Pixel(x,y,0);
		images[1][i] = im.Pixel(x,y,1);
		images[2][i] = 0;
		}
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
	    
//	    const char* channel_names[] = {"A", "B", "G", "R"}; // must be ABGR order.
//	    image.channel_names = channel_names;
	    
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

//	    const char* err;
	    int ret = SaveEXRImageToFile(&image, &header, exrname, &err);
	    if (ret != TINYEXR_SUCCESS) {
		  fprintf(stderr, "Save EXR err: %s\n", err);
		  return ret;
		  }
	    if (verbose) {
	      printf("Saved exr file. [ %s ] \n",  exrname);
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
