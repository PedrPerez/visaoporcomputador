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


bool Video::saveVideoFrames(string path) {
	Frame f;
	
	while (this->readFrame(f) == true) {
		if (f.saveFrame(path) == false)
			return false;

		cout << f.getnFrame() << "/" << this->getTotalFrames() << endl;
	}

	return true;
}