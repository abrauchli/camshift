// needs Point2f, sqrt from opencv
#include "opencv2/imgproc/imgproc.hpp"

#include <cassert>
#include <cstring> // size_t
#include <deque>
#include <iostream>
#include <utility> // pair
#include <vector>

using namespace cv;
using namespace std;

class Stats {
public:
    static const int PREDCOUNT = 40;

    Stats() {
        pred = deque<vector<float> >();
        error = vector<vector<float> >(PREDCOUNT, vector<float>());
        mean = vector<float>(PREDCOUNT, 0);
        dev = vector<float>(PREDCOUNT, 0);
    }

    void print_stats(const deque<pair<int, Point2f> >& points, bool print_header = true) {
        if (points.size() < PREDCOUNT)
            return;
        // validate predictions against history
        if (print_header) {
            cout << "Prediction <mean, stddev>" << endl;
            int i(1);
            for (; i < PREDCOUNT; ++i) {
                if (i <= 5 || i % 10 == 0)
                    cout << i << "\t\t";
            }
            cout << i << endl;
        }
        vector<float> curpred = pred.front();
        int pi = 0;
        for (deque<pair<int, Point2f> >::const_reverse_iterator it = points.rbegin();
            it != points.rend() && pi < curpred.size();
            ++pi, ++it) {

            // mean
            float sqpt = sqrt(it->second.x*it->second.x+it->second.y*it->second.y);
            float e = abs(sqpt - curpred[pi]);
            error[pi].push_back(e);
            mean[pi] = (error[pi].size() * mean[pi] + e) / (error[pi].size() + 1);
            if (pi < 5 || pi % 10 == 0 || pi == PREDCOUNT -1)
                cout << mean[pi] << "\t";

            // stddev
            int s = 0;
            for (vector<float>::const_iterator eit = error[pi].begin(); eit != error[pi].end(); ++eit) {
                s += (*eit - mean[pi]) * (*eit - mean[pi]);
            }
            dev[pi] = sqrt(s / error[pi].size());
            if (pi < 5 || pi % 10 == 0 || pi == PREDCOUNT -1)
                cout << dev[pi] << "\t";
        }
        cout << endl;
    }

    /**
     * Add a new prediction,
     * trims current list to back PREDCOUNT
     */
    void add_pred(vector<float>& curpred) {
        assert(curpred.size() == PREDCOUNT);
        if (pred.size() >= PREDCOUNT)
            pred.pop_front();
        pred.push_back(curpred);
    }

private:
    deque<vector<float> > pred;
    vector<vector<float> > error;
    vector<float> mean;
    vector<float> dev;
};
