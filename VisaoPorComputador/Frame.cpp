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
        para resoluçao
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
        rectangle(drawing, boundRect[i], color, 2, LINE_8, 0);
        

    }
}

void Frame::identifyObjects(vector<ImageDb>& db) {
    int histSize = 256;
    float range[] = { 0, 256 }; //the upper boundary is exclusive
    const float* histRange[] = { range };
    bool retf = false;

    

    for (size_t i = 0; i < boundRect.size(); i++) {
        float corr_p = 0;
        ImageDb * result = NULL;
        double b_corr_f = 0;
        double g_corr_f = 0;
        double r_corr_f = 0;

        for (size_t j = 0; j < db.size(); j++) {
           
            Mat templateImg = imread(db[j].getFilePath());
            vector<Mat> templateImg_bgr;
            split(templateImg, templateImg_bgr);
            Mat templateImg_hist[3];

            calcHist(&templateImg_bgr[0], 1, 0, Mat(), templateImg_hist[0], 1, &histSize, histRange, true, false);
            normalize(templateImg_hist[0], templateImg_hist[0], 0, 1, NORM_MINMAX, -1, Mat());
            templateImg_hist[0].convertTo(templateImg_hist[0], CV_32F);

            calcHist(&templateImg_bgr[1], 1, 0, Mat(), templateImg_hist[1], 1, &histSize, histRange, true, false);
            normalize(templateImg_hist[1], templateImg_hist[1], 0, 1, NORM_MINMAX, -1, Mat());
            templateImg_hist[1].convertTo(templateImg_hist[1], CV_32F);

            calcHist(&templateImg_bgr[2], 1, 0, Mat(), templateImg_hist[2], 1, &histSize, histRange, true, false);
            normalize(templateImg_hist[2], templateImg_hist[2], 0, 1, NORM_MINMAX, -1, Mat());
            templateImg_hist[2].convertTo(templateImg_hist[2], CV_32F);

            Mat originImg = this->frame(boundRect[i]);
            vector<Mat> originImg_bgr;
            split(originImg, originImg_bgr);
            Mat originImg_hist[3];
            
            calcHist(&originImg_bgr[0], 1, 0, Mat(), originImg_hist[0], 1, &histSize, histRange, true, false);
            normalize(originImg_hist[0], originImg_hist[0], 0, 1, NORM_MINMAX, -1, Mat());
            originImg_hist[0].convertTo(originImg_hist[0], CV_32F);

            calcHist(&originImg_bgr[1], 1, 0, Mat(), originImg_hist[1], 1, &histSize, histRange, true, false);
            normalize(originImg_hist[1], originImg_hist[1], 0, 1, NORM_MINMAX, -1, Mat());
            originImg_hist[1].convertTo(originImg_hist[1], CV_32F);

            calcHist(&originImg_bgr[2], 1, 0, Mat(), originImg_hist[2], 1, &histSize, histRange, true, false);
            normalize(originImg_hist[2], originImg_hist[2], 0, 1, NORM_MINMAX, -1, Mat());
            originImg_hist[2].convertTo(originImg_hist[2], CV_32F);
            
            //Here:
            double b_corr = compareHist(templateImg_hist[0], originImg_hist[0], HISTCMP_CORREL);
            double g_corr = compareHist(templateImg_hist[1], originImg_hist[1], HISTCMP_CORREL);
            double r_corr = compareHist(templateImg_hist[2], originImg_hist[2], HISTCMP_CORREL);

            double corr_f = (b_corr + g_corr + r_corr) / 3;

            if (corr_p < corr_f) {
                corr_p = corr_f;
                b_corr_f = b_corr;
                g_corr_f = g_corr;
                r_corr_f = r_corr;
                result = &db[j];
            }

            retf = true;
        }
        
        stringstream ss1,ss2;
        vector<string> lines;
        //Found image
        if (result != NULL && b_corr_f > 0 && g_corr_f > 0 && r_corr_f > 0) {
            
            
            ss1 << "Object " << i << " -> Match: " << corr_p * 100 << " % [" << b_corr_f*100 << " %; " << g_corr_f * 100 << " %; " << r_corr_f * 100 << " %]";
            lines.push_back(ss1.str()); 
            ss2 << "Category : " << result->getCategory() << " -> " << result->getFilePath();
            lines.push_back(ss2.str());

            
        }
        else {
            ss1 << "Object " << i << " -> No Match!!";
            lines.push_back(ss1.str());
        }

        text_legend(lines, boundRect[i], 0.05);

    }

}


void Frame::filter_orange(Mat& m){
    Mat hsv,out;
    //Scalar low_orange(46, 85, 45);
    //Scalar high_orange(55, 100, 60);
    Scalar low_orange(0, 50, 0);
    Scalar high_orange(255, 100, 255);

    cvtColor(m, hsv, COLOR_BGR2HSV);
    
    inRange(hsv, low_orange, high_orange, out);
    
    try{
        cvtColor(out, m, COLOR_GRAY2BGR);
    }
    catch (Exception e) {
        cout << e.err;
        exit(1);
    }
}