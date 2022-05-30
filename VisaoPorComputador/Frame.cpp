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

bool Frame::intersectRectangle(Rect a, Rect b, float tolerance) {
    Rect aa, bb;
    int offset_a_w = a.width * tolerance;
    int offset_a_h = a.height * tolerance;

    int offset_b_w = b.width * tolerance;
    int offset_b_h = b.height * tolerance;

    aa.x = a.x - offset_a_w < 0 ? a.x : a.x - offset_a_w;
    aa.y = a.y - offset_a_h < 0 ? a.y : a.y - offset_a_h;
    
    aa.width = a.width + offset_a_w >= this->frame.size().width ? this->frame.size().width - 1  : a.width + offset_a_w;
    aa.height = a.height + offset_a_h >= this->frame.size().height ? this->frame.size().height - 1 : a.height + offset_a_h;

    bb.x = b.x - offset_b_w < 0 ? b.x : b.x - offset_b_w;
    bb.y = b.y - offset_b_h < 0 ? b.y : b.y - offset_b_h;

    bb.width = b.width + offset_b_w >= this->frame.size().width ? this->frame.size().width - 1 : b.width + offset_b_w;
    bb.height = b.height + offset_b_h >= this->frame.size().height ? this->frame.size().height - 1 : b.height + offset_b_h;

    return (aa & bb).area() != 0;
}

void Frame::filterRectangles(vector<Rect>& boundRect, float min_w, float min_h, float max_w, float max_h) {
    int min_area_w = min_w * this->frame.size().width; 
    int min_area_h = min_h * this->frame.size().height;
    int max_area_w = max_w * this->frame.size().width;
    int max_area_h = max_h * this->frame.size().height;

    for (size_t i = 0; i < boundRect.size(); i++) {
        if (
            boundRect[i].width <= min_area_w  ||
            boundRect[i].width >= max_area_w  ||
            boundRect[i].height <= min_area_h ||
            boundRect[i].height >= max_area_h
            
            ) {

            boundRect.erase(boundRect.begin() + i);
            i = i - 1;
        }
    }

}   

void Frame::text_legend(vector<string> textLine, Rect rect,float h_offset_percentage) {
    
    for(size_t i = 0; i < textLine.size(); i++){
        putText(this->getFrame(), textLine.at(i),
            cv::Point(rect.x, rect.y + rect.height + (h_offset_percentage * this->getFrame().size().height) + (h_offset_percentage*0.75 * this->getFrame().size().height)*i ),
            cv::FONT_HERSHEY_SIMPLEX,
            0.5, cv::Scalar(0, 0, 0), 2);
    }
}
void Frame::mergeRectangles(vector<Rect>& boundRect) {
    for (size_t i = 0; i < boundRect.size(); i++) {
        for (size_t j = 0; j < boundRect.size(); j++) {

            if (
                i != j && 
                (
                    (boundRect[i].x != boundRect[j].x && boundRect[i].y != boundRect[j].y && boundRect[i].width != boundRect[j].width && boundRect[i].height != boundRect[j].height) &&
                    
                    (
                        (boundRect[i] & boundRect[j]).area() != 0)                           //Intercecao direta 
                        ||
                        this->intersectRectangle(boundRect[i], boundRect[j], 0.1 ) == true // Intercecao com tolerancia
                    )

                )
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
    vector<vector<Point> > contours;

    Canny(this->frame, canny_output, thresh, thresh * 2);
    vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    Mat drawing = this->frame;//Mat::zeros(canny_output.size(), CV_8UC3);
    
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
    filterRectangles(boundRect, 0.06, 0.1, 0.37, 0.66); //Valores derivados no excel
    /*
        Calculo da percentagem para filtrar
        para resolušao
            1280 x 720
            100% x 100%

            Apos analise no gimp, 
            no minimo uma laranja tem que ter
                75 x 75
                6% x 10%

            no maximo:
                475 x 475
                37% x 66%

    */

    for (size_t i = 0; i < boundRect.size(); i++)
    {
        stringstream ss;
        vector<string> lines;

        ss << "[" << boundRect[i].x << "][" << boundRect[i].y + boundRect[i].height + 5 << "]\n\tTeste!";
        rectangle(drawing, boundRect[i], color, 2, LINE_8, 0);
        
        lines.push_back(ss.str());
        lines.push_back("teste1");
        lines.push_back("teste2");

        text_legend(lines, boundRect[i], 0.05);

    }
}

bool Frame::findMatch(vector<ImageDb>& db, ImageDb& ret) {

    for (size_t i = 0; i < boundRect.size(); i++) {
        cout << i << " Square : " << endl;
        for (size_t j = 0; j < db.size(); j++) {
           
            Mat result;
            Mat templateImg = imread(db[j].getFilePath());
            Mat originImg = this->frame(boundRect[i]);

            
            matchTemplate(originImg, templateImg, result, TM_CCORR_NORMED);
            normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

            double minVal; double maxVal; Point minLoc; Point maxLoc;
            Point matchLoc;

            minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

            cout << db[j].getFilePath() << " : " << endl;
            cout << "\tminVal: " << minVal << endl;
            cout << "\tmaxVal: " << maxVal << endl;
            cout << "\tminLoc: " << minLoc << endl;
            cout << "\tmaxLoc: " << maxLoc << endl;
            


        }
        cout << "--------------" << endl;
    }

    return true;
}