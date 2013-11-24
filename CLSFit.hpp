#include "opencv2/opencv.hpp"

using namespace cv;

template<typename X, typename Y>
class CLSFit {
public:
    CLSFit(const vector<X> &x, const vector<Y> &y) : xs(x), ys(y) {
        solve_cls();
    }

    void solve_cls() {
        int n = xs.size();
        Mat x(n, 4, DataType<Y>::type);
        Mat y(ys);

        x.col(0) = Scalar(1);
        Mat(xs).convertTo(x.col(1), DataType<Y>::type);
        pow(x.col(1), 2, x.col(2));
        pow(x.col(1), 3, x.col(3));
        solve(x, y, coef, DECOMP_QR); // alternatively DECOMP_SVD
    }

    Y interpolate(const X& x) const {
        return coef[3] * pow(x, 3) + coef[2] * pow(x, 2) + coef[1] * x + coef[0];
    }

    Y operator[](const X& x) const {
        return interpolate(x);
    }
private:
    const vector<X> &xs;
    const vector<Y> &ys;
    Vec<Y, 4> coef;
};
