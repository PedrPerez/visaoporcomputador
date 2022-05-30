#pragma once
#include "libs.h"
#include "ImageDb.h"

class Frame
{
private:
	Mat frame;
	int nFrame;
	vector<Rect> boundRect;
	

public:
	Frame(Mat& frame, int nFrame);

	Frame(string file, int nFrame);

	Frame(){}

	inline int getnFrame() {
		return this->nFrame;
	}

	inline Mat getFrame() {
		return this->frame;
	}

	inline void drawRectangle(int x, int y, int w, int h) {
		rectangle(this->frame, Rect(x, y, w, h), Scalar(255,0,0), 3, 0);
	}

	void findContourns(int thresh);

	bool saveFrame(string file);

	bool findMatch(vector<ImageDb>& db, ImageDb& ret);

private:
	void mergeRectangles(vector<Rect>& boundRect);
	bool intersectRectangle(Rect a, Rect b, float tolerance);
	void filterRectangles(vector<Rect>& boundRect, float min_w, float min_h, float max_w, float max_h);
	void text_legend(vector<string> textLine, Rect rect, float h_offset_percentage);

};

