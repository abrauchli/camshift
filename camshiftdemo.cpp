#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Spline.hpp"
#include "Stats.hpp"

#include <cstdio> // sscanf
#include <deque>
#include <iostream>
#include <string>
#include <utility> // pair
#include <vector>

using namespace cv;
using namespace std;

Mat image;

bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;

static void onMouse(int event, int x, int y, int, void*)
{
    if (selectObject) {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        selection &= Rect(0, 0, image.cols, image.rows);
    }

    switch (event) {
    case CV_EVENT_LBUTTONDOWN:
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectObject = true;
        break;
    case CV_EVENT_LBUTTONUP:
        selectObject = false;
        if (selection.width > 0 && selection.height > 0) {
            trackObject = -1;
            cout << "Selection: " << selection.x << "," << selection.y << ","
                 << selection.width << "," << selection.height << endl;
        }
        break;
    }
}

static void help()
{
    cout << "\nThis is a demo that shows mean-shift based tracking\n"
            "You select a color objects such as your face and it tracks it.\n"
            "This reads from video camera (0 by default, or the camera number the user enters\n"
            "Usage: \n"
            "   camshiftdemo -c [camera_number]\n"
            "   camshiftdemo input_movie\n";

    cout << "\n\nHot keys: \n"
            "\tESC - quit the program\n"
            "\tc - stop the tracking\n"
            "\tb - switch to/from backprojection view\n"
            "\th - show/hide object histogram\n"
            "\tp - pause video\n"
            "To initialize tracking, select the object with mouse\n";
}

static const char* keys =
{
    "{c|camIdx|0  |Camera number (default 0)}"
    "{ |vmin  |10 |vmin (default 10)}"
    "{ |vmax  |256|vmin (default 256)}"
    "{s|smin  |30 |vmin (default 30)}"
    "{r|rect  |   |initial tracking square X,Y,w,h}"
    "{1|      |   |input movie}"
};

static void set_selection(const string& rect_str)
{
    int x,y,w,h;
    if (4 == sscanf(rect_str.c_str(), "%d,%d,%d,%d", &x, &y, &w, &h)) {
        selection = Rect(x,y,w,h);
        selection &= Rect(0, 0, image.cols, image.rows);
        trackObject = -1; // trigger histogram creation
    }
}

int main(int argc, const char** argv)
{
    help();

    VideoCapture cap;
    Rect trackWindow;
    int hsize = 16;
    float hranges[] = {0,180};
    const float* phranges = hranges;
    CommandLineParser parser(argc, argv, keys);
    int camNum = parser.get<int>("c");
    vmin = parser.get<int>("vmin");
    vmax = parser.get<int>("vmax");
    smin = parser.get<int>("smin");
    string file = parser.get<string>("1");

    if (file.empty()) {
        cout << "Using camera " << camNum << endl;
        cap.open(camNum);
    } else {
        cout << "Using file " << file << endl;
        cap.open(file.c_str());
    }

    if (!cap.isOpened()) {
        cout << "***Could not initialize capturing...***\n";
        cout << "Current parameter's value: \n";
        parser.printParams();
        return -1;
    }

    namedWindow("Histogram", 0);
    namedWindow("CamShift Demo", 0);
    setMouseCallback("CamShift Demo", onMouse, 0);
    createTrackbar("Vmin", "CamShift Demo", &vmin, 256, 0);
    createTrackbar("Vmax", "CamShift Demo", &vmax, 256, 0);
    createTrackbar("Smin", "CamShift Demo", &smin, 256, 0);

    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    deque<pair<int, Point2f> > points;
    bool paused = true;
    cap >> frame;
    if (!frame.empty())
        frame.copyTo(image);
    set_selection(parser.get<string>("rect"));
    int frameCount = 1;
    int eCount = 0;
    Stats stats;

    for (; !frame.empty();) {

        if (!paused) {
            frame.copyTo(image);
            ++frameCount;
            cvtColor(image, hsv, CV_BGR2HSV);

            if (trackObject) {
                int _vmin = vmin, _vmax = vmax;

                inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
                        Scalar(180, 256, MAX(_vmin, _vmax)), mask);
                int ch[] = {0, 0};
                hue.create(hsv.size(), hsv.depth());
                mixChannels(&hsv, 1, &hue, 1, ch, 1);

                if (trackObject < 0) {
                    Mat roi(hue, selection), maskroi(mask, selection);
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
                    normalize(hist, hist, 0, 255, CV_MINMAX);

                    trackWindow = selection;
                    trackObject = 1;

                    histimg = Scalar::all(0);
                    int binW = histimg.cols / hsize;
                    Mat buf(1, hsize, CV_8UC3);
                    for (int i = 0; i < hsize; ++i)
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                    cvtColor(buf, buf, CV_HSV2BGR);

                    for (int i = 0; i < hsize; ++i) {
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
                        rectangle(histimg, Point(i*binW,histimg.rows),
                                  Point((i+1)*binW,histimg.rows - val),
                                  Scalar(buf.at<Vec3b>(i)), -1, 8);
                    }
                }

                calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
                backproj &= mask;
                RotatedRect trackBox = CamShift(backproj, trackWindow,
                                                TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));
                // draw ten last locations
                Point2f pts[4];
                trackBox.points(pts);
                Point2f center = (pts[0] + pts[2]) * 0.5;
                points.push_back(make_pair(frameCount, center));
                if (points.size() > Stats::PREDCOUNT)
                    points.pop_front();
                int alpha = 255;
                for (deque<pair<int, Point2f> >::const_reverse_iterator it = points.rbegin(); it != points.rend(); ++it) {
                    circle(image, it->second, 4, Scalar(255,0,0,alpha), 2);
                    alpha *= 0.85;
                }
                stats.print_stats(points); // does nothing with < 10 points
                // predict next occurance
                if (points.size() >= 3) {
                    vector<int> frames;
                    vector<float> points_x;
                    vector<float> points_y;
                    for (deque<pair<int, Point2f> >::const_iterator it = points.begin(); it != points.end(); ++it) {
                        frames.push_back(it->first);
                        points_x.push_back(it->second.x);
                        points_y.push_back(it->second.y);
                    }
                    Spline<int, float> spl_x(frames, points_x);
                    Spline<int, float> spl_y(frames, points_y);
                    vector<float> curpred; // stats
                    // draw next predicted ten points
                    for (int i = 1; i <= Stats::PREDCOUNT; ++i) {
                        float x = spl_x[frameCount + i];
                        float y = spl_y[frameCount + i];
                        curpred.push_back(sqrt(x*x+y*y)); // stats
                        Point2f p(x, y);
                        circle(image, p, 4, Scalar(0,255,0), 2);
                    }
                    stats.add_pred(curpred);
                }
                if (trackWindow.area() <= 1) {
                    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                                  Rect(0, 0, cols, rows);
                }

                if (backprojMode)
                    cvtColor(backproj, image, CV_GRAY2BGR);
                ellipse(image, trackBox, Scalar(0,0,255), 3, CV_AA);
            }
        }

        if ((selectObject || frameCount == 1) && selection.width > 0 && selection.height > 0) {
            // causes flickering since it re-inverts itself every frame
            Mat roi(image, selection);
            bitwise_not(roi, roi);
        }

        imshow("CamShift Demo", image);
        if (showHist)
            imshow("Histogram", histimg);

        char c = (char)waitKey(10);
        if (c == 27)
            break;
        switch (c) {
        case 'b':
            backprojMode = !backprojMode;
            break;
        case 'c':
            trackObject = 0;
            histimg = Scalar::all(0);
            break;
        case 'h':
            showHist = !showHist;
            if (!showHist)
                destroyWindow("Histogram");
            else
                namedWindow("Histogram", 1);
            break;
        case 'p':
            paused = !paused;
            break;
        default:
            ;
        }

        if (!paused)
            cap >> frame;
    }

    return 0;
}
