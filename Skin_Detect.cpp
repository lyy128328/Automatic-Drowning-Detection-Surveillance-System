#include "Skin_Detect.h"

/***********************************************************************
 * Step 1: Colour Segmentation
 *
 * Classify pixels as either skin or non-skin. Skin tones fall within a
 * small range of Chrominance values:
 * Cr = 133 to 173
 * Cb = 77 to 127
 **********************************************************************/
void colour_segmentation(Mat &img, Mat &imgFilter, int cs_en, 
						 int min_Cr, int max_Cr, int min_Cb, int max_Cb)
{
	if( cs_en )
		inRange(img, Scalar(0, min_Cr, min_Cb), Scalar(255, max_Cr, max_Cb), imgFilter);
}

/***********************************************************************
 * Step 2: Density Regularization
 *
 * Colour segmentation produces a very noisy image with a lot of false
 * positives. We therefore need to find areas with a high probability of
 * that belong to the facial region. Group pixels in 4x4 clusters and
 * find the sum of skin coloured pixels in each cluster.
 *
 * Classify each cluster as follows to achieve a density map:
 * 0 <= sum < 16; noise or non-facial region
 * sum = 16; facial region
 *
 * Following this perform morphology as below:
 * 1. discard all clusters at the perimeter of the frame
 * 2. Erode any full-density point if it is surrounded by less than ﬁve
 * 	  other full-density points in its local 3x3 neighborhood
 * 3. Dilate any point of either zero or intermediate density if there
 *    are more than two full-density points in its local 3x3
 *    neighborhood
 **********************************************************************/
void density_regularization(Mat &img, Mat &imgFilter, int dr_en, int erode1, int dilate1)
{
	if( !dr_en )
		return;

	Mat sum;
	sum = Mat::zeros(img.rows, img.cols, CV_8UC1);
	uchar op;
	int erode, dilate;
	for (int i = 0; i < img.rows; i += 4) //Cycle over horizontal clusters
	{
		for (int j = 0; j < img.cols; j += 4) //Cycle over vertical clusters
		{
			for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
			{
				for (int l = 0; l < 4; l++) //Cycle vertically within cluster
				{
					if (imgFilter.at<uchar>(i + k, j + l) != 0) sum.at<uchar>(i, j)++;
				}
			}
			if (sum.at<uchar>(i, j) == 0 || i == 0 || j == 0 || i == (img.rows - 4) || j == (img.cols - 4)) op = 0;
			else if (sum.at<uchar>(i, j) > 0 &&  sum.at<uchar>(i, j) < 16) op = 128;
			else op = 255;
			for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
			{
				for (int l = 0; l < 4; l++) //Cycle vertically within cluster
				{
					imgFilter.at<uchar>(i + k, j + l) = op;
				}
			}
		}
	}
	for (int i = 4; i < (img.rows - 4); i += 4) //Cycle over horizontal clusters
	{
		for (int j = 4; j < (img.cols -4); j += 4) //Cycle over vertical clusters
		{
			erode = 0;
			if (imgFilter.at<uchar>(i, j) == 255)
			{
				for (int k = -4; k < 5; k += 4)
				{
					for (int l = -4; l < 5; l += 4)
					{
						if (imgFilter.at<uchar>(i + k, j + l) == 255) erode++;
					}
				}
				if (erode < erode1)
				{
					for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
					{
						for (int l = 0; l < 4; l++) //Cycle vertically within cluster
						{
							imgFilter.at<uchar>(i + k, j + l) = 0;
						}
					}
				}
			}
		}
	}
	for (int i = 4; i < (img.rows - 4); i += 4) //Cycle over horizontal clusters
	{
		for (int j = 4; j < (img.cols - 4); j += 4) //Cycle over vertical clusters
		{
			dilate = 0;
			if (imgFilter.at<uchar>(i, j) < 255)
			{
				for (int k = -4; k < 5; k += 4)
				{
					for (int l = -4; l < 5; l += 4)
					{
						if (imgFilter.at<uchar>(i + k, j + l) == 255) dilate++;
					}
				}
				if (dilate > dilate1)
				{
					for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
					{
						for (int l = 0; l < 4; l++) //Cycle vertically within cluster
						{
							imgFilter.at<uchar>(i + k, j + l) = 255;
						}
					}
				}
			}
			for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
			{
				for (int l = 0; l < 4; l++) //Cycle vertically within cluster
				{
					if (imgFilter.at<uchar>(i + k, j + l) != 255) imgFilter.at<uchar>(i + k, j + l) = 0;
				}
			}
		}
	}
}

/***********************************************************************
 * Step 3: Luminance Regularization
 *
 * In a typical teleconference the brightness is nonuniform throughout
 * the facial region, while the background region tends to have a more
 * even distribution of brightness. Hence, based on this characteristic,
 * background region that was previously detected due to its skin-color
 * appearance can be further eliminated
 *
 * Calculate the standard deviation of the luminance in an 8x8 pixel
 * area (under 4:2:0 subsampling).
 * if (stddev >= 2 && step2value == 1) step3value =1;
 **********************************************************************/
void luminance_regularization(Mat &img, Mat &imgFilter, int lr_en, int deviation)
{
	if( !lr_en )
		return;

	float xbar, sse, stddev;
	int op;
	
	for (int i = 4; i < (img.rows - 4); i += 4) //Cycle over horizontal clusters
	{
		for (int j = 4; j < (img.cols - 4); j += 4) //Cycle over vertical clusters
		{
			xbar = 0;
			sse = 0;
			stddev = 0;
			op = 0;
			for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
			{
				for (int l = 0; l < 4; l++) //Cycle vertically within cluster
				{
					xbar += img.at<Vec3b>(i + k, j + l)[0];
				}
			}
			xbar /= 16;
			for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
			{
				for (int l = 0; l < 4; l++) //Cycle vertically within cluster
				{
					sse += pow(img.at<Vec3b>(i + k, j + l)[0] - xbar, 2);
				}
			}
			stddev = pow(sse / 16, 0.5);				
			if (imgFilter.at<uchar>(i, j) == 255 && (int)stddev >= deviation) op = 255;
			for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
			{
				for (int l = 0; l < 4; l++) //Cycle vertically within cluster
				{
					imgFilter.at<uchar>(i + k, j + l) = op;
				}
			}
		}
	}
}

/***********************************************************************
 * Step Four: Geometric Correction
 *
 * Further mophology:
 * 1. Pixel clusters with value 1 will remain one if surrounded by at
 *    least four clusters of value 1 in its local 3x3 neighborhood
 * 2. Pixel clusters with value 0 will be given value 1 if surrounded
 *    by at least 6 clusters of value 1 in its local 3x3 neighborhood
 *
 * Filter horizontally then vertically setting any runs of less than
 * four clusters to zero
 **********************************************************************/
void geometric_correction(Mat &img, Mat &imgFilter, int gc_en, int erode2, int dilate2)
{
	if( !gc_en )
		return;

	int erode, dilate;
	for (int i = 4; i < (img.rows - 4); i+=4) //Cycle over horizontal clusters
	{
		for (int j = 4; j < (img.cols - 4); j+=4) //Cycle over vertical clusters
		{
			erode = 0;
			if (imgFilter.at<uchar>(i, j) == 255)
			{
				for (int k = -4; k < 5; k += 4)
				{
					for (int l = -4; l < 5; l += 4)
					{
						if (imgFilter.at<uchar>(i + k, j + l) == 255) erode++;
					}
				}
				if (erode < erode2)
				{
					for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
					{
						for (int l = 0; l < 4; l++) //Cycle vertically within cluster
						{
							imgFilter.at<uchar>(i + k, j + l) = 0;
						}
					}
				}
			}
		}
	}
	for (int i = 4; i < (img.rows - 4); i+=4) //Cycle over horizontal clusters
	{
		for (int j = 4; j < (img.cols - 4); j+=4) //Cycle over vertical clusters
		{
			dilate = 0;
			if (imgFilter.at<uchar>(i, j) < 255)
			{
				for (int k = -4; k < 5; k += 4)
				{
					for (int l = -4; l < 5; l += 4)
					{
						if (imgFilter.at<uchar>(i + k, j + l) == 255) dilate++;
					}
				}
				if (dilate > dilate2)
				{
					for (int k = 0; k < 4; k++) //Cycle horizontally within cluster
					{
						for (int l = 0; l < 4; l++) //Cycle vertically within cluster
						{
							imgFilter.at<uchar>(i + k, j + l) = 255;
						}
					}
				}
			}
		}
	}
}

void average_frame(Mat &imgFilter, Mat &imgHist, int af_en, int weighting)
{
	accumulateWeighted(imgFilter, imgHist, ((double) weighting/255));
}