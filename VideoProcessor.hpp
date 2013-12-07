#ifndef __VIDEO_PROCESSOR_HPP__
#define __VIDEO_PROCESSOR_HPP__

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


#include <iostream>

using namespace cv;
using namespace std;


struct VideoProcessor
{

    VideoProcessor(VideoCapture &Frames, string WindowName)
    :   frames(Frames),
        wndname(WindowName),
        rotate(0.0),
        scale(1),
        paused(true),
        quit(false),
        selecting(false),
        frameCount(0)
    {
        namedWindow(wndname.c_str(), CV_WINDOW_AUTOSIZE);
        setMouseCallback(wndname.c_str(), on_mouse, this);
    }

    virtual ~VideoProcessor()
    {
    }

    // Applies scaling or rotational transformation to input movie.
    void SetTransform(int Rotate, int Scale)
    {
        rotate = Rotate;
        scale = Scale;
    }

    // Sets an initial selection window to select target region.
    void SetSelection(const Rect &Selection)
    {
        selection = Selection;
    }


    void Play(bool Paused)
    {
        paused = Paused;

        quit = !next_frame();
        if (!quit)
        {
            process_frame(image);

            if (selection.height > 0 && selection.width > 0)
            {
                region_selected(selection);    
            }
            
            imshow( wndname.c_str(), image );    
        }

        while ( !quit )
        {
            if (!paused)
            {
                process_frame(image);
                imshow(wndname.c_str(), image);
                quit = !next_frame();
            }

            char c = (char)waitKey(10);

            switch (c) 
            {
            case 'p':
                paused = !paused;
                break ;

            case 27:
                quit = true;
                break ;

            default:
                break ;
            }

        }

    }

    void pause()
    {
        paused = true;
    }

    void select(int x, int y, bool StartStop)
    {
        if (selecting)
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);
            
            selection.width = std::abs(x - selection.x);
            selection.height = std::abs(y - selection.y);

            roi = Mat(image, selection);
            bitwise_not(roi, roi);

            imshow(wndname.c_str(), image);
        }


        if (StartStop)
        {
            if (!selecting)
            {
                selection.x = x;
                selection.y = y;
                selection.width = 1;
                selection.height = 1;
            }
            else
            {
                if (selection.area() > 16)
                {
                    cout << "Region selected x=" << selection.x << " y=" << selection.y 
                         << " h=" << selection.height << " w=" << selection.width << endl;

                    region_selected(selection);
                }
                else
                {
                    cout << "Slection area not big enough to track." << endl;
                }

                Mat roi(image, selection);

                bitwise_not(roi, roi);
                imshow(wndname.c_str(), image);                
            }

            selecting = !selecting;
        }
    }

protected:

    virtual void process_frame(Mat image)
    {
    }

    virtual void region_selected(const Rect &Selection)
    {
    }

    int             frameCount;

private:

    VideoCapture    &frames;
    string          wndname;
    Mat             image;      // Current frame image.
    double          rotate;
    int             scale;
    bool            paused;
    bool            quit;
    bool            selecting;
    Rect            selection;

    bool next_frame()
    {
        bool    empty;
        Mat     frame;

        frameCount++;

        frames >> frame;
        empty = frame.empty();
        if (!empty)
        {
            frame.copyTo(image);

            if (rotate != 0.0)
            {
                Point2f src_center(image.cols/2.0F, image.rows/2.0F);
                Mat     rot_mat;

                rot_mat = getRotationMatrix2D(src_center, rotate, 1.0);
                warpAffine(image, image, rot_mat, image.size());
            }

            resize(image, image, Size(image.cols/scale, image.rows/scale));
        }

        return !empty;
    }


    static void on_mouse(int event, int x, int y, int, void *arg)
    {
        VideoProcessor    *vidstream = (VideoProcessor*)arg;

        switch (event) 
        {
        case CV_EVENT_LBUTTONDOWN:
            //vidstream->select(x, y, SELECTING_START);
            break;
        case CV_EVENT_LBUTTONUP:
            vidstream->select(x, y, true);
            break;
        default:
            vidstream->select(x, y, false);
            break;

        }
    }

};


#endif
