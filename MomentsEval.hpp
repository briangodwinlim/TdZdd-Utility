/*
 * A DdEval for efficient moments calculation for a family of subsets represented by a ZDD
 */

#pragma once

#include <cmath>
#include <vector>
#include <tdzdd/DdEval.hpp>

namespace tdzdd {

template<typename V = int, typename T = V>
class MomentsEval: public DdEval<MomentsEval<V,T>,std::vector<V>> {
private:
    int const n;
    V const offset, scale;
    std::vector<T> const val;

    V ncr(int n, int k) {
        if (k == 0 || n == k) return 1;
        return ncr(n - 1, k - 1) + ncr(n - 1, k);
    }

public:
    MomentsEval(std::vector<T> val, int n, V offset = 0, V scale = 1) 
         : val(val), n(n), offset(offset), scale(scale) {
    }

    void evalTerminal(std::vector<V> &v, int id) {
        v = std::vector<V>(n + 1, 0);
        if (id) {
            for (int i = 0; i <= n; ++i) {
                v[i] = std::pow(offset * scale, i);
            }
        }
    }

    void evalNode(std::vector<V> &v, int level, DdValues<std::vector<V>,2> const& values) {
        v = std::vector<V>(n + 1, 0);

        for (int n_ = 0; n_ <= n; ++n_) {
            v[n_] = values.get(0)[n_];
            for (int p = 0; p <= n_; ++p) {
                v[n_] += ncr(n_, p) * values.get(1)[n_ - p] * pow(val[level] * scale, p);
            }
        }
    }
};

} // namespace tdzdd