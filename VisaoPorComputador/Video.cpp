#include "Video.h"

Video::Video(string path){
		this->path = path;
}

bool Video::init() {
	capture.open(this->path);

	return capture.isOpened();
}

bool Video::readFrame(Frame& f) {
	int nFrame;
	Mat frame;

	m.lock();

	this->capture.read(frame);

	if (frame.empty()) {
		m.unlock();
		return false;
	}

	nFrame = this->capture.get(cv::CAP_PROP_POS_FRAMES);

	f = Frame(frame, nFrame);

	m.unlock();

	return true;
}