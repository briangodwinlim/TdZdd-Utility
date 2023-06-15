# TdZdd Utility

Additional header files for the [TdZdd](https://github.com/kunisura/TdZdd/) library. Provides more utilities for Zero-Suppressed Binary Decision Diagram (ZDD) applications.

## MomentsEval

A `tdzdd::DdEval` for efficient moments calculation for a family of subsets represented by a ZDD.

## WeightedIterator

An iterator for sorting a family of subsets by weight. The code is adapted from the weighted_iterator implementation of `SAPPOROBDD::ZBDD` in the [Graphillion](https://github.com/takemaru/graphillion/) library.

## RandomPruning

A `tdzdd::DdSpec` for random pruning during ZDD construction for memory performance. Ideally used with Bootstrap methods.

## Sample Applications

### Build

```
make
```

### Run

```
./run [ <zddFile> <options>... ]
```

#### Example 1

Constructs a ZDD representing the power set of the set containing 10 elements. The ZDD construction randomly prunes each arc with probability 0.1 to produce a subset of the power set. Calculates the first 3 moments of the family.

```
./run -power 10 -prune 0.1 -moment 3
```

#### Example 2

Constructs a ZDD representing valid solutions to a Knapsack problem. The problem has 10 items with random weights and values and a total capacity of 50. Prints the first 5 solutions sorted by total value.

```
./run -knapsack -sort 5 <<< "10 50"
```

#### Example 3

Constructs a ZDD representing all undirected paths in a complete graph with 5 vertices. Calculates the first 3 moments of the family of paths with respect to path length. Exports the ZDD into STDOUT in SAPPOROBDD format.

```
./run -graph 5 -moment 3 -export
```

## Related Repositories

- [TdZdd](https://github.com/kunisura/TdZdd/)
- [SAPPOROBDD](https://github.com/Shin-ichi-Minato/SAPPOROBDD)
- [sbdd_helper](https://github.com/junkawahara/sbdd_helper)
- [Graphillion](https://github.com/takemaru/graphillion/)
