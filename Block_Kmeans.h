#ifndef BLOCK_KMEANS_H
#define BLOCK_KMEANS_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

class Block_Kmeans{
public:
	// constructor
	Block_Kmeans(Mat &src_img_in,
				 Mat &dst_img_in, // remember to initialize dst_img_in
				 Mat &mask_in,
				 int block_size_in,
				 int K_in,
				 TermCriteria criteria_in,
				 int attempts_in,         
				 int flags_in);
	// destructor
	~Block_Kmeans();
	// process
	void process();
	// set threshold multiple
	void setThresholdMulti(float multi_in = 0.3){ multi = multi_in; };
	// get centers link
	Vec3b ***getCenters(){ return centers; };

private:
	Mat **srcImgBlock; // source image in blocks
	Mat dst_img; // destination image after process
	Mat mask; // totalMask from VMF
	int width; // width of source image
	int height; // height of source image

	float multi; // set threshold multiple in myKmeans

	int blockSize; // block size
	int rowNumBlocks; // number of blocks in a row
	int colNumBlocks; // number of blocks in a column

	int K; // number of clusters
	TermCriteria criteria; // terminate criteria
	int attempts; // ?
	int flags; // ?

	Vec3b ***centers; // centers of clusters, centers[which_row_block][which_col_block][1~K]

	// seperate the whole input image into several blocks
	void makeBlocks(Mat &src_img);
	// transform image to specific format and apply kmeans function
	void myKmeans(Mat &img_in, int block_i, int block_j);
};

#endif