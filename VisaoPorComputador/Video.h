#pragma once
#include "libs.h"
#include "Frame.h"

class Video
{
private:
	string path;
	VideoCapture capture;
	mutex m;

public:
	Video(string path);

	bool init();

	inline int getTotalFrames() {
		return this->capture.get(CAP_PROP_FRAME_COUNT);
	}

	inline int getFrameRate() {
		return this->capture.get(cv::CAP_PROP_FPS);
	}

	inline int getWidth() {
		return this->capture.get(cv::CAP_PROP_FRAME_WIDTH);
	}

	inline int getHeight() {
		return this->capture.get(cv::CAP_PROP_FRAME_HEIGHT);
	}

	bool readFrame(Frame& f);

};

