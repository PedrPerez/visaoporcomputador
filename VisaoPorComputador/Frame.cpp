#include "Frame.h"

Frame::Frame(Mat& frame, int nFrame) {
	this->frame = frame.clone();
	this->nFrame = nFrame;
}
