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
    static const size_t PREDCOUNT = 10;

    Stats() {
        pred = deque<vector<float> >();
        error = vector<vector<float> >(10, vector<float>());
        mean = vector<float>(10, 0);
        dev = vector<float>(10, 0);
    }

    void print_stats(const deque<pair<int, Point2f> >& points) {
        if (points.size() < PREDCOUNT)
            return;
        // validate predictions against history
        cout << "Prediction <mean, stddev>" << endl;
        cout << "1\t\t2\t\t3\t\t4\t\t5\t\t6\t\t7\t\t8\t\t9\t\t10" << endl;
        vector<float> curpred = pred.front();
        int pi = 0;
        for (deque<pair<int, Point2f> >::const_reverse_iterator it = points.rbegin();
            it != points.rend();
            ++pi, ++it) {

            // mean
            float sqpt = sqrt(it->second.x*it->second.x+it->second.y*it->second.y);
            float e = sqpt - curpred[pi];
            e = sqrt(e*e);
            error[pi].push_back(e);
            mean[pi] = (error[pi].size() * mean[pi] + e) / (error[pi].size() + 1);
            cout << mean[pi] << "\t";

            // stddev
            int s = 0;
            for (vector<float>::const_iterator eit = error[pi].begin(); eit != error[pi].end(); ++eit) {
                s += (*eit - mean[pi]) * (*eit - mean[pi]);
            }
            dev[pi] = sqrt(s / error[pi].size());
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
