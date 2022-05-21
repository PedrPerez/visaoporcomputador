#include "Frame.h"

Frame::Frame(Mat& frame, int nFrame) {
	this->frame = frame.clone();
	this->nFrame = nFrame;
}


bool Frame::saveFrame(string path) {
	stringstream filePath;
	filePath << path << this->nFrame << ".png";

	return imwrite(filePath.str(), this->frame);

}