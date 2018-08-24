#include "Temporal_VMF.h"

Temporal_VMF::Temporal_VMF(VMF_frame &input, Mat &output, Mat &tMask, int frame_no_in)
{
	sequence = input;
	out = output;
	totalMask = tMask;
	frameNo =  frame_no_in;
	width = sequence.frame[0].cols;
	height = sequence.frame[0].rows;
	sum = new int**[frameNo];
	for( int i=0; i<frameNo; i++){
		sum[i] = new int*[height];
		for( int j=0; j<height; j++)
			sum[i][j] = new int[width];
	}
	init();
	cout << "constructor end" << endl;
}

Temporal_VMF::~Temporal_VMF()
{
	for( int i=frameNo-1; i>=0; i--){
		for( int j=height-1; j>=0; j++)
			free(sum[i][j]);
		free(sum[i]);
	}
	free(sum);
}

void Temporal_VMF::init()
{
	int smallest, k_smallest;
	cout << "init start" << endl;
	// find all sum
	for( int i=0; i<height; i++){
		for( int j=0; j<width; j++){
			// assume first frame is median
			k_smallest = 0;
			smallest = 0;
			for( int m=1; m<frameNo; m++)
				smallest += distance((sequence.frame[0]).at<Vec3b>(i,j),(sequence.frame[m]).at<Vec3b>(i,j));

			// scan though all frame to find median
			for( int k=1; k<frameNo; k++){		
				for( int m=0; m<frameNo; m++)
					if( m!=k )
						sum[k][i][j] += distance((sequence.frame[k]).at<Vec3b>(i,j),
												 (sequence.frame[m]).at<Vec3b>(i,j));

				if( sum[k][i][j]<smallest ){
					smallest = sum[k][i][j];
					k_smallest = k;
				}
			}

			// make output median image and mask of output
			out.at<Vec3b>(i,j) = (sequence.frame[k_smallest]).at<Vec3b>(i,j);
			totalMask.at<uchar>(i,j) = (sequence.mask[k_smallest]).at<uchar>(i,j);
		}
	}
	cout << "init end" << endl;
}

void Temporal_VMF::online(Mat &new_in, Mat &inMask)
{
	int k_smallest, smallest;

	for( int i=0; i<height; i++)
		for( int j=0; j<width; j++){
			// subtract discard frame and add new_in frame
			for( int k=0; k<frameNo; k++){
				sum[k][i][j] -= distance((sequence.frame[k]).at<Vec3b>(i,j),
										 (sequence.frame[sequence.discard]).at<Vec3b>(i,j));
				sum[k][i][j] += distance((sequence.frame[k]).at<Vec3b>(i,j),
										 new_in.at<Vec3b>(i,j));
			}

			// scan through all sum to find median
			k_smallest = 0;
			smallest = sum[0][i][j];
			for( int k=1; k<frameNo; k++){			
				if( sum[k][i][j]<smallest ){
					smallest = sum[k][i][j];
					k_smallest = k;
				}
			}

			// make output median image and mask of output
			out.at<Vec3b>(i,j) = (sequence.frame[k_smallest]).at<Vec3b>(i,j);
			totalMask.at<uchar>(i,j) = (sequence.mask[k_smallest]).at<uchar>(i,j);
		}
	cout << sequence.discard << endl;

	sequence.mask[sequence.discard] = inMask.clone(); // update mask
	sequence.frame[sequence.discard] = new_in.clone(); // update frame
	// update discard index
	sequence.discard++;
	sequence.discard = sequence.discard % frameNo;
}

int Temporal_VMF::distance(Vec3b a, Vec3b b)
{
	int sum = 0;
	for( int i=0; i<3; i++)
		sum += abs((int) a.val[i] - (int) b.val[i]);
	return sum;
}