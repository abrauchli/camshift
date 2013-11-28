#include "opencv2/opencv.hpp"

using namespace cv;

namespace LS {

enum CurveDegree {
    CURVE_DEG_CONST = 1,
    CURVE_DEG_LIN,
    CURVE_DEG_QUAD,
    CURVE_DEG_CUBIC,
    CURVE_DEG_QUART,
    CURVE_DEG_QUINT
};

template<int D, typename X, typename Y>
class LSFit
{
public:

    LSFit(const vector<X> &x, const vector<Y> &y, bool w = false)
    :   xs(x),
        ys(y),
        weighted(w),
        coef(D, 1, DataType<Y>::type)
    {
        solve_ls();
    }

    void solve_ls() {
        size_t n = ys.size();
        int dim = n < D ? n : D;
        if (n < D || coef.rows < dim)
            coef.resize(dim, 0);

        Mat x(n, dim, DataType<Y>::type);
        Mat y(ys);
        assert(coef.rows >= dim && dim == x.cols);
        assert(x.rows == y.rows);

        x.col(0) = Scalar(1);
        if (dim > 1)
            Mat(xs).convertTo(x.col(1), DataType<Y>::type);
        for (int d = 2; d < dim; ++d)
            pow(x.col(1), d, x.col(d));

        if (weighted) {
            for (int i = 1; i < n; ++i) {
                x.row(i) *= i;
                y.row(i) *= i;
            }
        }
        solve(x, y, coef, DECOMP_QR); // alternatively DECOMP_SVD
    }

    Y interpolate(const X& x) const {
        Y sum(0);
        for (size_t d = 0; d < coef.rows; ++d)
            sum += coef.at<Y>(d, 0) * pow(x, d);
        return sum;
    }

    Y operator[](const X& x) const {
        return interpolate(x);
    }
private:
    bool weighted;
    const vector<X> &xs;
    const vector<Y> &ys;
    Mat coef;
};

// cpp11
// template<typename X, typename Y>
// using CLSFit = LSFit<3, X, Y>;
}
