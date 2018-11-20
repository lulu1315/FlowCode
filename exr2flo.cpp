#include <stdio.h>
#include <math.h>
#include "imageLib/imageLib.h"
#include "flowIO.h"
#include "colorcode.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "tinyexr/tinyexr.h"

static const char *usage = "\n  usage: %s [-quiet] in.exr out.flo\n";

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
	    char *exrname = argv[argn++];
	    char *floname = argv[argn++];
//	    float normalisation = argn < argc ? atof(argv[argn++]) : -1;
	    int width, height;
	    //exr image rgba
	    float* rgba;
	    const char* err;
//load exr image
	{
	int ret = LoadEXR(&rgba, &width, &height, exrname, &err);
	if (ret != 0) {
	  printf("err: %s\n", err);
	  return -1;
	  }
	}
	printf("image = %s\n", exrname);
	printf("size = %d, %d\n", width, height);
	
//initialize motion image
	int nBands = 2;
	CFloatImage img;
	CShape sh(width, height, nBands);
	img.ReAllocate(sh);
	int x, y;
	
//copy exr pixels value to .flo
	for (y = 0; y < height; y++) {
	  for (x = 0; x < width; x++) {
	    float fx = rgba[4 * (y * width + x) + 0];
	    float fy = rgba[4 * (y * width + x) + 1];
//      float fx = (((rgba[4 * (y * width + x) + 0])*2)-1)*normalisation;
//	    float fy = (((rgba[4 * (y * width + x) + 1])*2)-1)*normalisation;
//	    printf("x y fx fy = %d, %d , %f, %f\n", x , y , fx , fy);
	    img.Pixel(x, y, 0) = fx;
	    img.Pixel(x, y, 1) = fy;
	    }
	  }
//write .flo file
	  WriteFlowFile(img, floname);
    
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
