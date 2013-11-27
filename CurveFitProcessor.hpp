#include <deque>
#include <utility> // pair

#include "CamShiftProcessor.hpp"
#include "Stats.hpp"
#include "Spline.hpp"

class CurveFitProcessor : public CamShiftProcessor
{
public:

    CurveFitProcessor(VideoCapture &Frames, string WindowName)
        :   CamShiftProcessor(Frames, WindowName)
    {
    }

protected:

    deque< pair<int, Point2f> >     points;
    Stats                           stats;

    virtual void track_results(Mat Image, const RotatedRect &TrackBox)
    {
        // draw ten last locations
        Point2f     pts[4];

        TrackBox.points(pts);
        Point2f center = (pts[0] + pts[2]) * 0.5;
        points.push_back(make_pair(frameCount, center));
        if (points.size() > Stats::PREDCOUNT)
            points.pop_front();
        int alpha = 255;
        for (deque<pair<int, Point2f> >::const_reverse_iterator it = points.rbegin(); it != points.rend(); ++it) {
            circle(Image, it->second, 4, Scalar(255,0,0,alpha), 2);
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
                circle(Image, p, 4, Scalar(0,255,0), 2);
            }

            stats.add_pred(curpred);
        }

    }
    
};


