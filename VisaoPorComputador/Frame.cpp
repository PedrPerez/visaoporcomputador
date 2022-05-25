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

void Frame::mergeRectangles(vector<Rect>& boundRect) {
    for (size_t i = 0; i < boundRect.size(); i++) {
        for (size_t j = 0; j < boundRect.size(); j++) {

            if (
                i != j && 
                (boundRect[i] & boundRect[j]).area() != 0) 
            {
                boundRect[i] = boundRect[i] | boundRect[j];
                boundRect.erase(boundRect.begin() + j);
                i = 0;
                j = 0;
                break;
            }
        }
    }
}

void Frame::findContourns(int thresh) {
    Mat canny_output;

    Canny(this->frame, canny_output, thresh, thresh * 2);
    vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    Mat drawing = this->frame;//Mat::zeros(canny_output.size(), CV_8UC3);
    vector<Rect> boundRect;
    Scalar color = Scalar(255, 51, 51);

    for (size_t i = 0; i < contours.size(); i++)
    {
        
        //drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
        Rect r = boundingRect(contours[i]);//enclose in Rect

        //Excluir rectangulos do tamanho da frame
        if (r.area() < this->frame.size().area() * 0.95) {
            boundRect.push_back(r);
        }

    }

    mergeRectangles(boundRect);
    

    for (size_t i = 0; i < boundRect.size(); i++)
    {

        rectangle(drawing, boundRect[i], color, 2, LINE_8, 0);

    }
}