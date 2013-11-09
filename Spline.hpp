/*
 * Adapted from code by Devin Lane:
 * "THE BEER-WARE LICENSE" (Revision 42): Devin Lane wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return. */

#include <assert.h>
#include <vector>

template<typename X, typename Y>
class Spline {
public:
    Spline() {}; // invalid spline
    Spline(const std::vector<X>& xs, const std::vector<Y>& ys) {
        assert(xs.size() >= 3);
        vecsize_t n = xs.size() -1;
        std::vector<X> a(n), b(n), d(n);
        std::vector<X> c(n+1), l(n+1), u(n+1), z(n+1), h(n+1);
        l[0] = Y(1);
        u[0] = z[0] = Y(0);
        h[0] = xs[1] - xs[0];
        for (vecsize_t i = 1; i < n; ++i) {
            h[i] = xs[i+1] - xs[i];
            l[i] = Y(2 * (xs[i+1] - xs[i-1])) - Y(h[i-1]) * u[i-1];
            u[i] = Y(h[i]) / l[i];
            a[i] = (Y(3) / Y(h[i])) * (ys[i+1] - ys[i]) - (Y(3) / Y(h[i-1])) * (ys[i] - ys[i-1]);
            z[i] = (a[i] - Y(h[i-1]) * z[i-1]) / l[i];
        }

        for (vecsize_t j = n-1; j >= 0; --j) {
            c[j] = z[j] - u[j] * c[j+1];
            b[j] = (ys[j+1] - ys[j]) / Y(h[j]) - (Y(h[j]) * (c[j+1] + Y(2) * c[j])) / Y(3);
            d[j] = (c[j+1] - c[j]) / Y(3 * h[j]);
        }

        for (vecsize_t i = 0; i < n; i++) {
            mElements.push_back(Element(xs[i], ys[i], b[i], c[i], d[i]));
        }
    }

    Y operator[](const X& x) const {
        return interpolate(x);
    }
        
    Y interpolate(const X& x) const {
        if (mElements.size() == 0)
            return Y();

        typename std::vector<element_type>::const_iterator it;
        it = std::lower_bound(mElements.begin(), mElements.end(), element_type(x));
        if (it != mElements.begin()) {
            --it;
        }

        return it->eval(x);
    }

    std::vector<Y> operator[](const std::vector<X>& xs) const {
        return interpolate(xs);
    }

    /** Evaluate at multiple locations, assuming xs is sorted ascending */
    std::vector<Y> interpolate(const std::vector<X>& xs) const {
        if (mElements.size() == 0)
            return std::vector<Y>(xs.size());
        
        typename std::vector<X>::const_iterator it;
        typename std::vector<element_type>::const_iterator it2;
        it2 = mElements.begin();
        std::vector<Y> ys;
        for (it = xs.begin(); it != xs.end(); ++it) {
            it2 = std::lower_bound(it2, mElements.end(), element_type(*it));
            if (it2 != mElements.begin())
                --it2;

            ys.push_back(it2->eval(*it));
        }

        return ys;
    }

typedef typename std::vector<X>::difference_type vecsize_t;

protected:
    class Element {
        public:
            X x;
            Y a, b, c, d;

            Element(X _x)
                : x(_x) {}
            Element(X _x, Y _a, Y _b, Y _c, Y _d)
                : x(_x), a(_a), b(_b), c(_c), d(_d) {}

            Y eval(const X& xx) const {
                X xix(xx - x);
                return a + b * xix + c * (xix * xix) + d * (xix * xix * xix);
            }

            bool operator<(const Element& e) const {
                return x < e.x;
            }

            bool operator<(const X& xx) const {
                return x < xx;
            }
    };
    typedef Element element_type;
    std::vector<element_type> mElements;
};
