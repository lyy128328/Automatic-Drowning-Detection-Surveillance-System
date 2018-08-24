#include <iostream>
#include <string.h>
#include <opencv2/opencv.hpp>

#include "Skin_Detect.h"
#include "Block_Kmeans.h"
#include "Temporal_VMF.h"
#include "Foreground.h"

using namespace cv;

int cppMain(int argc, char **argv)
{
	VideoCapture cap;
	if( strcmp(argv[1],"device") ){
		cap.open(argv[1]);
	} else{
		cap.open(0);
	}
	if( !cap.isOpened() ){ // video file not open
		cerr << "Error: video not open." << endl;
		exit(EXIT_FAILURE);
	}

	int cs_en=1, min_Cr=109, max_Cr=155, min_Cb=95, max_Cb=145;
	// namedWindow("Setting", WINDOW_NORMAL);
	// createTrackbar("Colour Segmentation", "Setting", &cs_en, 1);
	// createTrackbar("Min Cr", "Setting", &min_Cr, 255);
	// createTrackbar("Max Cr", "Setting", &max_Cr, 255);
	// createTrackbar("Min Cb", "Setting", &min_Cb, 255);
	// createTrackbar("Max Cb", "Setting", &max_Cb, 255);

	// +++++++++++++++++++++++++++ VMF setup ++++++++++++++++++++++++++
	Mat tmp, mask_tmp; // not initialized
	int VMF_frame_no = 5;
	VMF_frame a;
	a.frame = new Mat[VMF_frame_no]; // initialize Mat *frame in struct VMF_frame
	a.mask = new Mat[VMF_frame_no]; // initialize Mat *mask in struct VMF_frame
	for( int i=0; i<VMF_frame_no; i++){
		cap >> a.frame[i];
		cvtColor(a.frame[i], a.mask[i], CV_BGR2YCrCb); //Convert to YUV colorspace
		colour_segmentation(a.mask[i], a.mask[i], cs_en, min_Cr, max_Cr, min_Cb, max_Cb);//, 1, 103, 110, 141, 144);
	}
	a.discard = 0; // discard index is initialized to 0
	Mat dst = Mat::zeros(a.frame[0].size(),a.frame[0].type()); // initialize output image of Temporal_VMF
	Mat tMask = Mat::zeros(a.mask[0].size(),a.mask[0].type()); // initialize mask of output image of Temporal_VMF

	Temporal_VMF myVMF(a, dst, tMask, VMF_frame_no);

	// +++++++++++++++++++++++++++ block Kmeans setup +++++++++++++++++++++++++++
	int K = 3;
	int block_size = 40;
	Mat dst2 = Mat::zeros(dst.size(),dst.type()); // initialize output image of Block_Kmeans
	Block_Kmeans myBK(dst,
					  dst2,
					  tMask,
					  block_size,
					  K,
					  TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
					  3,
					  KMEANS_PP_CENTERS);
	// int multi = 3;
	// namedWindow("SettingBK", WINDOW_NORMAL);
	// createTrackbar("BK", "Setting", &multi, 10);

	// ++++++++++++++++++++++++++ foreground setup ++++++++++++++++++++++++++++++++++
	Mat inputImg = Mat::zeros(dst2.size(),dst2.type());
	Foreground myFG( inputImg, myBK.getCenters(), block_size);
	namedWindow("FG Setting", WINDOW_NORMAL);
	int Ts = 102, Tl = 88;
	createTrackbar("Ts", "FG Setting", &Ts, 255);
	createTrackbar("Tl", "FG Setting", &Tl, 255);

	// ++++++++++++++++++++++++++ start online process ++++++++++++++++++++++++++++++
	namedWindow("Origin", WINDOW_NORMAL);
	// namedWindow("VMF", WINDOW_NORMAL);
	// namedWindow("tMask", WINDOW_NORMAL);
	// namedWindow("BK", WINDOW_NORMAL);
	namedWindow("FG", WINDOW_NORMAL);
	while(true){
		cap >> inputImg;

		tmp = inputImg.clone();

		cvtColor(tmp, mask_tmp, CV_BGR2YCrCb); //Convert to YUV colorspace
		colour_segmentation(mask_tmp, mask_tmp, cs_en, min_Cr, max_Cr, min_Cb, max_Cb);
		myVMF.online(tmp, mask_tmp);
		// myBK.setThresholdMulti(multi/10.0);
		myBK.process();
		myFG.setThreshold(Ts, Tl);
		myFG.process();

		imshow("Origin", inputImg);
		// imshow("VMF", dst);
		// imshow("tMask", tMask);
		// imshow("BK", dst2);
		imshow("FG", myFG.getMap());

		if( (char)waitKey(30)==27 ) break; // end video
	}

	return 0;
}