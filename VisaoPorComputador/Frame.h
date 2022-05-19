#pragma once
#include "libs.h"

class Frame
{
private:
	Mat frame;
	int nFrame;

public:
	Frame(Mat& frame, int nFrame);

	inline Frame(){}

	inline int getnFrame() {
		return this->nFrame;
	}

	inline Mat getFrame() {
		return this->frame;
	}
};

