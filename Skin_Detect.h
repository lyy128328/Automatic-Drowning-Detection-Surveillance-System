#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;

#define MINCB 77
#define MAXCB 127
#define MINCR 133
#define MAXCR 173
#define ERODE_SIZE 5
#define DILATE_SIZE 3
#define DEVIATION 2

void colour_segmentation(Mat &img, Mat &imgFilter, int cs_en=1, 
						 int min_Cr=MINCR, int max_Cr=MAXCR, int min_Cb=MINCB, int max_Cb=MAXCB);
void density_regularization(Mat &img, Mat &imgFilter, int dr_en=1, 
							int erode1=ERODE_SIZE, int dilate1=DILATE_SIZE);
void luminance_regularization(Mat &img, Mat &imgFilter, int lr_en=1, int deviation=DEVIATION);
void geometric_correction(Mat &img, Mat &imgFilter, int gc_en=1, 
						  int erode2=ERODE_SIZE, int dilate2=DILATE_SIZE);
void average_frame(Mat &imgFilter, Mat &imgHist, int af_en=1, int weighting=255);