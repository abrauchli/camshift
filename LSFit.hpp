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

    LSFit(bool w = false)
        :   weighted(w),
            coef(D, 1, DataType<Y>::type)
    {
    }

    void solve_ls() {
        size_t n = ys.size();
        int dim = n < D ? n : D;
        if (n < D || coef.rows < dim)
            coef.resize(dim, 0);

        Mat x(n, dim, DataType<Y>::type);
        Mat y(ys, weighted); // need a copy of data when using weights

        x.col(0) = Scalar(1);
        if (dim > 1)
            Mat(xs, weighted).convertTo(x.col(1), DataType<Y>::type);
        for (int d = 2; d < dim; ++d)
            pow(x.col(1), d, x.col(d));

        if (weighted) {
            for (int i = 1; i < n; ++i) {
                x.row(i) *= i * 0.25;
                y.row(i) *= i * 0.25;
            }
        }
        solve(x, y, coef, DECOMP_QR); // alternatively DECOMP_SVD
    }

    size_t size() {
        return ys.size();
    }

    Y at(size_t i) {
        return ys[i];
    }

    void push_back(X x, Y y, bool solve = true) {
        xs.push_back(x);
        ys.push_back(y);
        if (solve)
            solve_ls();
    }

    void clear() {
        xs.clear();
        ys.clear();
    }

    const Y interpolate(const X& x) const {
        Y sum(0);
        for (size_t d = 0; d < coef.rows; ++d)
            sum += coef.at<Y>(d, 0) * pow(x, d);
        return sum;
    }

    const Y operator[](const X& x) const {
        return interpolate(x);
    }
private:
    bool weighted;
    vector<X> xs;
    vector<Y> ys;
    Mat coef;
};

}
