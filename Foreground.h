#include <opencv2/opencv.hpp>
#include <cmath>

using namespace std;
using namespace cv;

class Foreground{
public:
	// constructor
	Foreground(Mat &src_in, Vec3b ***centers_in, int block_size);
	// destructor
	~Foreground();
	// process
	void process();
	// set threshold
	void setThreshold(int Ts_in, int Tl_in){ Ts = Ts_in;
											 Tl = Tl_in; };
	// get 0 1 2 map
	Mat &getMap(){ return map; };

private:
	Mat **srcImgBlock; // input image in blocks
	Mat map; // 2 threshold(Ts Tl) 3 state(2 1 0)
	Vec3b ***centers; // centers of blocks, eg. centers[row][col][0~2], directly link to centers_in

	int blockSize; // size of a block
	int rowNumBlocks; // number of blocks in a row
	int colNumBlocks; // number of blocks in a column

	int width; // width of source image
	int height; // height of source image

	int Ts; // strict threshold
	int Tl; // loose threshold

	int distance(Vec3b a, Vec3b b); // measure distance between two colors
};