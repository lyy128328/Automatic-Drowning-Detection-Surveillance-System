#include "Block_Kmeans.h"

Block_Kmeans::Block_Kmeans(Mat &src_img_in,
						   Mat &dst_img_in,
						   Mat &mask_in,
				 		   int block_size_in,
				 		   int K_in,
				 		   TermCriteria criteria_in,
				 		   int attempts_in,         
				 		   int flags_in)
{
	// check if source image is loaded sucessfully
	if( src_img_in.empty() ){
		cerr << "Error: fail to load image in Block_Kmeans." << endl;
		exit(EXIT_FAILURE);
	}
	dst_img = dst_img_in;
	mask = mask_in;
	width = src_img_in.cols; // width of source image
	height = src_img_in.rows; // height of source image

	blockSize = block_size_in;
	// compute # of blocks in a row
	rowNumBlocks = width / blockSize;
	if( width%blockSize!=0 ) rowNumBlocks++;
	// compute # of blocks in a column
	colNumBlocks = height / blockSize;
	if( height%blockSize!=0 ) colNumBlocks++;

	// parameters of kmeans algorithm
	K = K_in;
	criteria = criteria_in;
	attempts = attempts_in;
	flags = flags_in;	

	// threshold multiple in myKmeans function
	multi = 0.3;

	// allocate 3D Vec3b memory for centers
	centers = new Vec3b**[colNumBlocks];
	for( int i=0; i<colNumBlocks; i++){
		centers[i] = new Vec3b*[rowNumBlocks];
		for( int j=0; j<rowNumBlocks; j++)
			centers[i][j] = new Vec3b[K];
	}

	// now we have source image split into blocks stored in srcImgBlock
	makeBlocks(src_img_in);
}

Block_Kmeans::~Block_Kmeans()
{
	// free dynamic memory of srcImgBlock
	for( int i=colNumBlocks-1; i>=0; i--)
		free(srcImgBlock[i]);
	free(srcImgBlock);

	// free dynamic memory of centers
	for( int i=rowNumBlocks-1; i>=0; i--){
		for( int j=colNumBlocks-1; j>=0; j++)
			free(centers[i][j]);
		free(centers[i]);
	}
	free(centers);
}

void Block_Kmeans::makeBlocks(Mat &src_img)
{
	// allocate 2D memory for source image in blocks
	srcImgBlock = new Mat*[colNumBlocks];
	for(int i=0; i<colNumBlocks; i++)
		srcImgBlock[i] = new Mat[rowNumBlocks];

	// make entire source image into small blocks
	for( int i=0; i<colNumBlocks; i++){
		for( int j=0; j<rowNumBlocks; j++){
			srcImgBlock[i][j] = src_img( Range(i*blockSize,min((i+1)*blockSize,height)), 
								  		 Range(j*blockSize,min((j+1)*blockSize,width)) );
		}
	}
}

void Block_Kmeans::myKmeans(Mat &img_in, int block_i, int block_j)
{
	// reorder samples to fit in kmeans function
	int mSize = img_in.rows*img_in.cols;
	int yIndex = 0, nIndex = 0;
	Mat samples(mSize, 3, CV_32F);
	Mat labels, myCenters;
  	for( int i=0; i<img_in.rows; i++)
    	for( int j=0; j<img_in.cols; j++){
    		if( (int)mask.at<uchar>(block_i*blockSize+i,block_j*blockSize+j)==0 ){
    			// only consider masked pixels
      			for( int k=0; k<3; k++)
        	 		samples.at<float>( yIndex, k) = img_in.at<Vec3b>(i,j)[k];
        	 	yIndex++;
        	} else{
        		for( int k=0; k<3; k++)
        	 		samples.at<float>( mSize-1-nIndex, k) = img_in.at<Vec3b>(i,j)[k];
        	 	nIndex++;
        	}
        }
    int threshold = mSize * multi;
    if( yIndex>threshold ){
    	Mat samplesCrop = samples(Rect( 0, 0, 1, yIndex-1)); // resize sample
		kmeans( samplesCrop, K, labels, criteria, attempts, flags, myCenters);
	} else{
		kmeans( samples, K, labels, criteria, attempts, flags, myCenters);
	}

	for( int k=0; k<K; k++)
		for( int i=0; i<3; i++){
			centers[block_i][block_j][k].val[i] = (uchar) myCenters.at<float>(k,i);
		}
}

void Block_Kmeans::process()
{
	// apply kmeans algorithm to every small blocks
	// and get the centers of each blocks
	for( int i=0; i<colNumBlocks; i++){
		for( int j=0; j<rowNumBlocks; j++){
			myKmeans(srcImgBlock[i][j], i, j);
		}
	}

	for( int i=0; i<colNumBlocks; i++)
		for( int j=0; j<rowNumBlocks; j++)
			for( int x=0; x<blockSize; x++)
				for( int y=0; y<blockSize; y++)
					dst_img.at<Vec3b>(i*blockSize+x,j*blockSize+y) = centers[i][j][0];
}