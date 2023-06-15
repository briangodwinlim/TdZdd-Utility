/*
 * A DdSpec for random pruning during ZDD construction for memory performance
 * Ideally used with Bootstrap methods
 */

#pragma once

#include <cmath>
#include <random>
#include <tdzdd/DdSpec.hpp>

namespace tdzdd {

class RandomPruning: public StatelessDdSpec<RandomPruning,2> {
    int const n;
    int const seed;
    long double const prob;
    std::default_random_engine generator;

public:
    RandomPruning(int n, long double prob, int seed = 0)
            : n(n), prob(prob), seed(seed) {
        generator.seed(seed ? seed : time(NULL));
    }

    int getRoot() const {
        return n;
    }

    int getChild(int level, int take) {
        std::uniform_real_distribution<long double> distribution(0.0, 1.0);
        if (distribution(generator) < prob) return 0;
        
        if (--level == 0) return -1;
        return level;
    }

    // Probability a given subset is in at least one of `sim` ZDD constructions
    long double getProb(int sim = 1) const {
        return 1 - std::pow(1 - std::pow(1 - prob, n), sim);
    }
};

} // namespace tdzdd