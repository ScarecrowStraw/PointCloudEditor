#include <pcl/apps/point_cloud_editor/qrcodelocate.h>
#include <QObject>

bool
QRCodeLocater::QRCode_Recognize(cv::Mat &image, std::string &code_type,std::string &code_data)
{
    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    // cv::Mat image = cv::imread(s);
    cv::Mat imageGray;
    cv::cvtColor(image,imageGray,CV_RGB2GRAY);
    int width = imageGray.cols;
    int height = imageGray.rows;
    auto *raw = imageGray.data;
    Image imageZbar(static_cast<unsigned int>(width), static_cast<unsigned int>(height), "Y800", raw,
                    static_cast<unsigned long>(width * height));
    scanner.scan(imageZbar); 
    Image::SymbolIterator symbol = imageZbar.symbol_begin();
    if(imageZbar.symbol_begin()==imageZbar.symbol_end())
    {
        return false;
    }
    for(;symbol != imageZbar.symbol_end();++symbol)
    {
        std::cout<<symbol->get_type_name()<<"data is"<<symbol->get_data()<<std::endl;
        code_type=symbol->get_type_name();
        code_data=symbol->get_data();
        return true;
    }
}

cv::Point
QRCodeLocater::Center_cal(vector<vector<cv::Point> > contours, int i)
{
    int centerx = 0, centery = 0, n = static_cast<int>(contours[i].size());

    centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
    centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
    cv::Point point1 =cv::Point(centerx, centery);
    return point1;
}

QRCodeLocater::QRCodeLocater(){}


bool
QRCodeLocater::locate_recognize(std::string path,std::string &code_type,std::string &code_data)
{
    rng=cv::RNG(123456);
    src = cv::imread(path);
    cv::Mat src_all = src.clone();


    cv::cvtColor(src, src_gray, CV_BGR2GRAY);

    cv::blur(src_gray, src_gray, cv::Size(3, 3));

    cv::equalizeHist(src_gray, src_gray);
    //    cv::namedWindow("src_gray");
    //    cv::imshow("src_gray", src_gray);


    cv::Scalar color = cv::Scalar(1, 1, 255);
    cv::Mat threshold_output;
    vector<vector<cv::Point> > contours, contours2;
    vector<cv::Vec4i> hierarchy;
    cv::Mat drawing = cv::Mat::zeros(src.size(), CV_8UC3);
    cv::Mat drawing2 = cv::Mat::zeros(src.size(), CV_8UC3);
    cv::Mat drawingAllContours = cv::Mat::zeros(src.size(), CV_8UC3);

    cv::threshold(src_gray, threshold_output, 112, 255, cv::THRESH_BINARY);

    //    cv::namedWindow("Threshold_output");
    //    cv::imshow("Threshold_output", threshold_output);

    cv::findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));

    int c = 0, ic = 0, k = 0, area = 0;

    int parentIdx = -1;
    for (int i = 0; i < contours.size(); i++) {

        cv::drawContours(drawingAllContours, contours, parentIdx, CV_RGB(255, 255, 255), 1, 8);
        if (hierarchy[i][2] != -1 && ic == 0) {
            parentIdx = i;
            ic++;
        } else if (hierarchy[i][2] != -1) {
            ic++;
        } else if (hierarchy[i][2] == -1) {
            ic = 0;
            parentIdx = -1;
        }

        if (ic >= 2) {

            contours2.push_back(contours[parentIdx]);

            cv::drawContours(drawing, contours, parentIdx,
                             CV_RGB(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), 1, 8);
            ic = 0;
            parentIdx = -1;
        }
    }

    for (int i = 0; i < contours2.size(); i++)
        cv::drawContours(drawing2, contours2, i,
                         CV_RGB(rng.uniform(100, 255), rng.uniform(100, 255), rng.uniform(100, 255)), -1, 4,
                         hierarchy[k][2], 0, cv::Point());


    cv::Point point[3];
    for (int i = 0; i < contours2.size(); i++) {
        point[i] = Center_cal(contours2, i);
    }

    if(contours2.size()>=3	){
        area = static_cast<int>(contourArea(contours2[1]));
        int area_side = cvRound(sqrt(double(area)));
        for (int i = 0; i < contours2.size(); i++) {

            cv::line(drawing2, point[i % contours2.size()], point[(i + 1) % contours2.size()], color, area_side / 2, 8);
        }
    }


    //    cv::namedWindow("DrawingAllContours");
    //    cv::imshow("DrawingAllContours", drawingAllContours);

    //    cv::namedWindow("Drawing2");
    //    cv::imshow("Drawing2", drawing2);

    //    cv::namedWindow("Drawing");
    //    cv::imshow("Drawing", drawing);


    cv::Mat gray_all, threshold_output_all;
    vector<vector<cv::Point> > contours_all;
    vector<cv::Vec4i> hierarchy_all;
    cv::cvtColor(drawing2, gray_all, CV_BGR2GRAY);


    cv::threshold(gray_all, threshold_output_all, 45, 255, cv::THRESH_BINARY);
    cv::findContours(threshold_output_all, contours_all, hierarchy_all, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE,
                     cv::Point(0, 0));


    cv::Point2f fourPoint2f[4];

    qDebug()<<contours_all.size();
    if(contours_all.size()>0){
        cv::RotatedRect rectPoint = cv::minAreaRect(contours_all[0]);
        cv::Rect myRect = cv::boundingRect(contours_all[0]);

        rectPoint.points(fourPoint2f);

        for (int i = 0; i < 4; i++) {
            cv::line(src_all, fourPoint2f[i % 4], fourPoint2f[(i + 1) % 4], cv::Scalar(20, 21, 237), 1);
        }

        cv::namedWindow("Src_all");
        //   cv::setWindowProperty("Src_all", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
        //cv::setWindowProperty("Src_all", CV_WND_PROP_FULLSCREEN, CV_WINDOW_NORMAL);
        cv::imshow("Src_all", src_all);

        char resultFileNameSring[100];
        sprintf(resultFileNameSring, "QRCode_Locate_result.png");

        cv::Mat resultImage = cv::Mat(src_all, myRect);
        //cv::imwrite(resultFileNameSring, resultImage);

        return QRCode_Recognize(resultImage,code_type,code_data);
    }
    else
        return false;
}
