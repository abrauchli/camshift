#include "opencv2/opencv.hpp"

using namespace cv;

template<int D, typename X, typename Y>
class LSFit
{
public:
    LSFit(const vector<X> &x, const vector<Y> &y) : xs(x), ys(y) {
        solve_cls();
    }

    void solve_cls() {
        int n = xs.size();
        Mat x(n, D+1, DataType<Y>::type);
        Mat y(ys);

        x.col(0) = Scalar(1);
        Mat(xs).convertTo(x.col(1), DataType<Y>::type);
        for (int d = 2; d <= D; ++d) {
            pow(x.col(1), d, x.col(d));
        }
        solve(x, y, coef, DECOMP_QR); // alternatively DECOMP_SVD
    }

    Y interpolate(const X& x) const {
        Y sum = 0;
        for (int d = D; d >= 0; --d)
            sum += coef(d) * pow(x, d);
        return sum;
    }

    Y operator[](const X& x) const {
        return interpolate(x);
    }
private:
    const vector<X> &xs;
    const vector<Y> &ys;
    Vec<Y, D+1> coef;
};

// cpp11
// template<typename X, typename Y>
// using CLSFit = LSFit<3, X, Y>;
