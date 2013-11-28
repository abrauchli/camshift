#include <algorithm> // std::min
#include <deque>
#include <utility> // std::pair
#include <vector>

#include "CamShiftProcessor.hpp"
#include "LSFit.hpp"
#include "Stats.hpp"

using LS::LSFit;

class CurveFitProcessor : public CamShiftProcessor
{
public:

    CurveFitProcessor(VideoCapture &Frames, string WindowName)
        :   CamShiftProcessor(Frames, WindowName),
            HISTORY_LEN(50),
            lsf_x(true),
            lsf_y(true)
    {
    }

protected:

    const int       HISTORY_LEN;
    deque<pair<int, Point2f> >  point_history;
    LSFit<LS::CURVE_DEG_CUBIC, int, float> lsf_x;
    LSFit<LS::CURVE_DEG_CUBIC, int, float> lsf_y;
    Stats           stats;

    template <typename T> int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }

    virtual void track_results(Mat Image, const RotatedRect &TrackBox)
    {
        Point2f     pts[4];
        TrackBox.points(pts);
        Point2f center = (pts[0] + pts[2]) * 0.5;
        point_history.push_back(make_pair(frameCount, center));
        if (point_history.size() > HISTORY_LEN)
            point_history.pop_front();

        // clear history when radical direction changes happen
        size_t sx = lsf_x.size();
        size_t sy = lsf_y.size();
        if (sx >= 2 && (lsf_x.at(sx-1) - lsf_x.at(sx-2)) * (center.x - lsf_x.at(sx-1)) < 0)
            lsf_x.clear();
        if (sy >= 2 && (lsf_y[sy-1] - lsf_y[sy-2]) * (center.y - lsf_y.at(sy-1)) < 0)
            lsf_y.clear();

        // add new points to curve fitting algorithm (LSFit)
        lsf_x.push_back(frameCount, center.x);
        lsf_y.push_back(frameCount, center.y);

        // draw object location history
        typedef deque<pair<int,Point2f> >::const_reverse_iterator rev_point_it;
        for (rev_point_it it = point_history.rbegin(); it != point_history.rend(); ++it) {
            circle(Image, it->second, 4, Scalar(255,0,0), 2);
        }

        stats.print_stats(point_history);

        // predict next occurance
        vector<float> curpred; // stats
        // draw next predicted points
        for (int i = -20; i <= Stats::PREDCOUNT; ++i) {
            float x = lsf_x[frameCount + i];
            float y = lsf_y[frameCount + i];
            if (i > 0)
                curpred.push_back(sqrt(x*x+y*y)); // stats
            Point2f p(x, y);
            circle(Image, p, 4, Scalar(i < 0 ? 255 : 0,255,0), 2);
        }
        stats.add_pred(curpred); // stats
    }
    
};

