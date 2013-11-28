#ifndef __CAM_SHIFT_PROCESSOR_HPP__
#define __CAM_SHIFT_PROCESSOR_HPP__

#include "VideoProcessor.hpp"

class CamShiftProcessor : public VideoProcessor
{

public:

    CamShiftProcessor(VideoCapture &Frames, string WindowName)
    :   VideoProcessor(Frames, WindowName),
        smin(30),
        vmin(10),
        vmax(256),
        hsize(16),
        histimg( Mat::zeros(200, 320, CV_8UC3) ),
        tracking(false),
        phranges(hranges)
    {
        hranges[0] = 0;
        hranges[1] = 180;
    }

    void SetThresholds(int VMin, int VMax, int SMin)
    {
        vmin = VMin;
        vmax = VMax;
        smin = SMin;
    }

  
protected:

    int         smin;
    int         vmin;
    int         vmax;
    int         hsize;
    Mat         hsv;
    Mat         hue;
    Mat         mask;
    Mat         hist;
    Mat         histimg;
    Mat         backproj;
    bool        tracking;
    Rect        trackWindow;
    RotatedRect trackBox;
    float       hranges[2];
    const float *phranges;


    virtual Rect search_window(Mat Image, const RotatedRect &TrackBox, const Rect &TrackWindow)
    {
        return TrackWindow;
    }

    virtual void track_results(Mat Image, const RotatedRect &TrackBox)
    {
        ellipse(Image, TrackBox, Scalar(0,0,255), 3, CV_AA);                              
    }

    virtual void process_frame(Mat image)
    {
        int ch[] = {0, 0};


        cvtColor(image, hsv, CV_BGR2HSV);
        inRange( hsv, 
                 Scalar(0, smin, min(vmin,vmax)), 
                 Scalar(180, 256, max(vmin, vmax)), 
                 mask );

        hue.create(hsv.size(), hsv.depth());

        mixChannels(&hsv, 1, &hue, 1, ch, 1);

        if (tracking)
        {
            calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
            backproj &= mask;

            trackWindow = search_window(image, trackBox, trackWindow);

            trackBox = CamShift(backproj, trackWindow,
                                 TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));


            if (trackWindow.area() <= 1) 
            {
                int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                   trackWindow.x + r, trackWindow.y + r) &
                              Rect(0, 0, cols, rows);
            }

            // todo - fixme
            if (false)
                cvtColor(backproj, image, CV_GRAY2BGR);

            track_results(image, trackBox);
        }
    }


    virtual void region_selected(const Rect &Region)
    {
        Mat     roi(hue, Region), 
                maskroi(mask, Region);
        int     binW;

        calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
        normalize(hist, hist, 0, 255, CV_MINMAX);

        trackWindow = Region;

        histimg = Scalar::all(0);
        binW = histimg.cols / hsize;
        Mat buf(1, hsize, CV_8UC3);
        for (int i = 0; i < hsize; ++i)
        {
            buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
        }

        cvtColor(buf, buf, CV_HSV2BGR);

        for (int i = 0; i < hsize; ++i) 
        {
            int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
            rectangle(histimg, Point(i*binW,histimg.rows),
                      Point((i+1)*binW,histimg.rows - val),
                      Scalar(buf.at<Vec3b>(i)), -1, 8);
        }

        tracking = true;
    }

};

#endif
