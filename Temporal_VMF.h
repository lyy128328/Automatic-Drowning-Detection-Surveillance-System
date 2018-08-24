#include <opencv2/opencv.hpp>
#include <cmath>

using namespace std;
using namespace cv;

typedef struct {
	Mat *frame;
	Mat *mask;
	int discard;
} VMF_frame;

class Temporal_VMF{
public:
	// constructor
	Temporal_VMF(VMF_frame &input, Mat &output, Mat &tMask, int frame_no_in); // size of input[] = output
	// destructor
	~Temporal_VMF();
	// initialization
	void init();
	// online process
	void online(Mat &new_in, Mat &inMask);


private:
	VMF_frame sequence; // a sequence of input frame
	int ***sum; // sum[frame_no][row][col]
	Mat out; // output median image
	Mat totalMask; // mask of out
	int frameNo; // frame number
	int width;
	int height;

	int distance(Vec3b a, Vec3b b);
};
