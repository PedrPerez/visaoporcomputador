#include "Frame.h"

Frame::Frame(Mat& frame, int nFrame) {
	this->frame = frame.clone();
	this->nFrame = nFrame;
}

Frame::Frame(string file, int nFrame) {
	this->frame = imread(file);
	this->nFrame = nFrame;
}

bool Frame::saveFrame(string path) {
	stringstream filePath;
	filePath << path << this->nFrame << ".png";

	return imwrite(filePath.str(), this->frame);

}

void Frame::findContourns(int thresh) {
    Mat canny_output;

    Canny(this->frame, canny_output, thresh, thresh * 2);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
    Mat drawing = this->frame;//Mat::zeros(canny_output.size(), CV_8UC3);
    for (size_t i = 0; i < contours.size(); i++)
    {
        Scalar color = Scalar(255, 51, 51);
        drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
    }


}