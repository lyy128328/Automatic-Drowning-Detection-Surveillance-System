#include "Foreground.h"

Foreground::Foreground(Mat &src_in, Vec3b ***centers_in, int block_size)
{
	width = src_in.cols; // width of source image
	height = src_in.rows; // height of source image

	blockSize = block_size;
	// compute # of blocks in a row
	rowNumBlocks = width / blockSize;
	if( width%blockSize!=0 ) rowNumBlocks++;
	// compute # of blocks in a column
	colNumBlocks = height / blockSize;
	if( height%blockSize!=0 ) colNumBlocks++;

	// allocate 2D memory for source image in blocks
	srcImgBlock = new Mat*[colNumBlocks];
	for(int i=0; i<colNumBlocks; i++)
		srcImgBlock[i] = new Mat[rowNumBlocks];

	// make entire source image into small blocks
	for( int i=0; i<colNumBlocks; i++){
		for( int j=0; j<rowNumBlocks; j++){
			srcImgBlock[i][j] = src_in( Range(i*blockSize,min((i+1)*blockSize,height)), 
								  		Range(j*blockSize,min((j+1)*blockSize,width)) );
		}
	}

	// default threshold ?????
	Ts = 180;
	Tl = 100;

	// initialization of map<uchar>
	map = Mat::zeros(height, width, CV_8UC1);;

	centers = centers_in;
}

Foreground::~Foreground()
{
	for(int i=colNumBlocks-1; i>=0; i--)
		free(srcImgBlock[i]);
	free(srcImgBlock);
}

void Foreground::process()
{
	int descrep;

	// entrie image level
	for( int block_i=0; block_i<colNumBlocks; block_i++){
		for( int block_j=0; block_j<rowNumBlocks; block_j++){
			// cout << "upblockj=" << block_j << endl;
			// block level
			Mat &nowBlock = srcImgBlock[block_i][block_j];
			for( int i=0; i<blockSize; i++){
				// cout << "i=" << i << endl;
				for( int j=0; j<blockSize; j++){
					// cout << "j=" << j << endl;

					// compare to 3 centers in nowBlock
					descrep = distance( centers[block_i][block_j][0], nowBlock.at<Vec3b>(i,j));
					for( int k=1; k<3; k++){
						// cout << "k=" << k << endl;
						descrep = min(descrep,
							          distance( centers[block_i][block_j][k], nowBlock.at<Vec3b>(i,j)));
					}
					// compare to 3 centers in eight-connected blocks
					for( int a=block_i-1; a<=block_i+1 && a!=colNumBlocks-1; a++)
						for( int b=block_j-1; b<=block_j+1 && b!=rowNumBlocks-1; b++){
							if( a!=0 && b!=0 ){ // exclude nowBlock
								// boundary
								if( a<0 ) a = 0;
								if( a>=colNumBlocks ) a = colNumBlocks-1;
								if( b<0 ) b = 0;
								if( b>=rowNumBlocks ) b = rowNumBlocks-1;

								for( int k=0; k<3; k++){
									descrep = min(descrep,
							          			  distance( centers[a][b][k], nowBlock.at<Vec3b>(i,j)));
								}
							}
						}
						
					// form map
					if( descrep>=Ts ) // Ts <= descrep, 255 white <--> 0 black
						map.at<uchar>(block_i*blockSize+i, block_j*blockSize+j) = 255;
					else if( descrep<Ts && descrep>=Tl ) // Tl =< descep < Ts
						map.at<uchar>(block_i*blockSize+i, block_j*blockSize+j) = 100;
					else // descrep < Tl
						map.at<uchar>(block_i*blockSize+i, block_j*blockSize+j) = 0;

					//cout << (int)map.at<uchar>(block_i*blockSize+i, block_j*blockSize+j);
				}
				// cout << "i=" << i << endl;
			}
			// cout << "blockj=" << block_j << endl;
			// cout << "rowNumBlocks=" << rowNumBlocks << endl;
		}
		// cout << "blocki=" << block_i << endl;
	}
	// cout << "rows:" << map.rows << "\tcols:" << map.cols << endl;

	// strict/loose threshold and connected region method

}

int Foreground::distance(Vec3b a, Vec3b b)
{
	int sum = 0;
	for( int i=0; i<3; i++)
		sum += abs((int) a.val[i] - (int) b.val[i]);
	return sum;
}