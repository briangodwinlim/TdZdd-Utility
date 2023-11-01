#include <map>
#include <vector>
#include <random>
#include <iomanip>

// SAPPOROBDD
#include <ZBDD.h>
#include <SBDD_helper.h>

// TdZdd
#include <tdzdd/DdEval.hpp>
#include <tdzdd/DdSpecOp.hpp>
#include <tdzdd/DdStructure.hpp>
#include <tdzdd/spec/SapporoZdd.hpp>
#include <tdzdd/eval/ToZBDD.hpp>

#include <tdzdd/util/Graph.hpp>
#include <tdzdd/spec/PathZdd.hpp>
#include <tdzdd/util/IntSubset.hpp>
#include <tdzdd/spec/DegreeConstraint.hpp>

// Utility files
#include "MomentsEval.hpp"
#include "RandomPruning.hpp"
#include "WeightedIterator.hpp"

using namespace tdzdd;
using namespace sbddh;

// Knapsack DdSpec
class Knapsack: public DdSpec<Knapsack,int,2> {
    int const n;
    int const w;
    std::vector<int> const weights;

public:
    Knapsack(int n, int w, std::vector<int>weights)
            : n(n), w(w), weights(weights) {
    }

    int getRoot(int& state) const {
        state = w;
        return n;
    }

    int getChild(int& state, int level, int value) const {
        state -= value * weights[level];
        if (--level == 0) return (state >= 0) ? -1 : 0;
        if (state < 0) return 0;
        return level;
    }
};

std::string options[][2] = { //
        {"power <n>", "Generate a ZDD representing the powerset of the universe with <n> elements"}, //
        {"knapsack", "Generate a ZDD representing a knapsack problem with inputs given in STDIN"}, //
        {"graph <n>", "Generate a ZDD representing all undirected paths in a complete graph with <n> vertices"}, //
        {"prune <f>", "Randomly prune each arc with probability <f>"}, //
        {"moment <n>", "Compute the first <n> moments of the family"}, //
        {"sort <n>", "Get the first <n> subsets sorted by descending value"}, //
        {"zdd", "Dump resulting ZDD to STDOUT in DOT format"}, //
        {"export", "Dump resulting ZDD to STDOUT"}, //
    };

std::map<std::string,bool> opt;
std::map<std::string,int> optNum;
std::map<std::string,double> optDouble;

void usage(char const* cmd) {
    std::cerr << "usage:" << cmd << " [ <zddFile> <options>... ] \n";
    std::cerr << "options\n";
    for (unsigned i = 0; i < sizeof(options) / sizeof(options[0]); ++i) {
        std::cerr << "  -" << options[i][0];
        for (unsigned j = options[i][0].length(); j < 15; ++j) {
            std::cerr << " ";
        }
        std::cerr << ": " << options[i][1] << "\n";
    }
}

int main(int argc, char *argv[]) {
    for (unsigned i = 0; i < sizeof(options) / sizeof(options[0]); ++i) {
        opt[options[i][0]] = false;
    }

    std::string zddFile;
    try {
        for (int i = 1; i < argc; ++i) {
            std::string s = argv[i];
            if (s[0] == '-') {
                s = s.substr(1);

                if (opt.count(s)) {
                    opt[s] = true;
                }
                else if (i + 1 < argc && opt.count(s + " <n>")) {
                    opt[s] = true;
                    optNum[s] = std::stoi(argv[++i]);
                }
                else if (i + 1 < argc && opt.count(s + " <f>")) {
                    opt[s] = true;
                    optDouble[s] = std::stof(argv[++i]);
                }
                else {
                    throw std::exception();
                }
            }
            else if (zddFile.empty()) {
                zddFile = s;
            }
            else {
                throw std::exception();
            }
        }
    }
    catch (std::exception& e) {
        usage(argv[0]);
        return 1;
    }

    DdStructure<2> dd;
    std::vector<int> val;   // val[l] gives the value of the variable at level l; val[0] is not used
    
    MessageHandler::showMessages();
    MessageHandler mh;
    mh.begin("ZDD Construction\n");
    try {
        // Read ZDD from file
        if (!zddFile.empty()) {
            BDD_Init(1024, 1024 * 1024 * 1024);
            FILE* fp = fopen(zddFile.c_str(), "r");
            if (fp == NULL) {
                throw std::runtime_error("File " + zddFile + " cannot be opened.");
                return 1;
            }

            ZBDD dd_s = ZBDD_Import(fp);
            fclose(fp);
            dd = DdStructure<2>(SapporoZdd(dd_s));
            dd.zddReduce();
            val = std::vector<int>(dd.topLevel() + 1, 1);   // v(x) = 1 for all x \in U
        }

        // Power set
        else if (opt["power"]) {
            dd = DdStructure<2>(optNum["power"]);
            dd.zddReduce();
            val = std::vector<int>(optNum["power"] + 1, 1);   // v(x) = 1 for all x \in U
        }

        // Knapsack problem
        else if (opt["knapsack"]) {
            mh << "Input n and w: ";
            int n, w;
            std::cin >> n >> w;
            if (n <= 0 || w < 0) throw std::runtime_error("Invalid inputs");
            
            srand(time(0));
            std::vector<int> weights;
            for (int i = 0; i < n + 1; ++i) {
                weights.push_back(rand() % 10 + 1); // w(x) ~ U{1,10} for all x \in U
                val.push_back(rand() % 10 + 1);     // v(x) ~ U{1,10} for all x \in U
            }
            dd = DdStructure<2>(Knapsack(n, w, weights));
            dd.zddReduce();

            mh << "Knapsack (w, v): ";
            for (int i = 1; i < n + 1; ++i) mh << "(" << weights[i] << ", " << val[i] << ") ";
            mh << "\n";
        }

        // Complete graph
        else if (opt["graph"]) {
            Graph g;
            for (int i = 0; i < optNum["graph"]; ++i) {
                g.addEdge(std::to_string(i), "center");
                for (int j = i + 1; j < optNum["graph"]; ++j) {
                    g.addEdge(std::to_string(i), std::to_string(j));
                }
            }
            g.update();

            CycleZdd cycle(g);
            IntRange ZeroOrTwo(0, 2, 2);
            IntRange OnlyTwo(2, 2);
            DegreeConstraint dc(g, &ZeroOrTwo);
            dc.setConstraint("center", &OnlyTwo);
            dd = DdStructure<2>(dc);
            dd.zddReduce();
            dd.zddSubset(cycle);
            dd.zddReduce();

            val = std::vector<int>(dd.topLevel() + 1, 1);      // v(e) = 1 for all e \in E
            for (int i = 1; i <= dd.topLevel(); ++i) {
                if (g.edgeLabel(dd.topLevel() - i).find("center") != std::string::npos) val[i] = 0;
            }
        }

        // Randomly prune ZDD
        if (opt["prune"]) {
            RandomPruning spec(dd.topLevel(), optDouble["prune"]);
            dd.zddSubset(spec);
            dd.zddReduce();
            mh << "Subset Prune Probability = " << 1.0 - spec.getProb() << "\n";
        }
        
        // Output zdd to STDOUT
        if (opt["zdd"] && dd.size()) dd.dumpDot(std::cout, "ZDD");
        if (opt["export"] && dd.size()) dd.dumpSapporo(std::cout);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    mh.end("finished");

    // Print moments
    if (opt["moment"]) {
        mh.begin("Moments Calculation\n");
        try{
            std::vector<__int128_t> moments = dd.evaluate(MomentsEval<__int128_t,int>(val, optNum["moment"]));
            mh << "Cardinality = " << static_cast<long long>(moments.front()) << "\n";
            std::map<int,std::string> suffix = {{1,"st"}, {2,"nd"}, {3,"rd"}};
            for (int n = 1; n <= optNum["moment"]; ++n) {
                if (suffix.count(n) == 0) suffix[n] = "th";
                mh << n << suffix[n] << " moment = " << std::fixed << std::setprecision(5) 
                    << static_cast<long double>(moments[n]) / static_cast<long double>(moments.front()) << "\n";
            }
        }
        catch (std::exception& e) {
            std::cerr << e.what() << "\n";
            return 1;
        }
        mh.end("finished");
    }

    // Sort subsets
    if (opt["sort"]) {
        mh.begin("Sorting Subsets\n");
        try {
            // Convert to ZBDD
            BDD_Init(1024, 1024 * 1024 * 1024);
            for (int i = 0; i < dd.topLevel(); ++i) BDD_NewVar();
            ZBDD dd_s = dd.evaluate(ToZBDD());
            assert(dd_s.Top() == dd.topLevel());

            weighted_iterator<int> it(dd_s, val);
            for (int i = 1; i <= std::min(static_cast<__int128_t>(optNum["sort"]), dd.evaluate(ZddCardinality<__int128_t>())); ++i) {
                mh << "Solution #" << i << ", " << "Value = " 
                    << std::fixed << std::setprecision(5) << it.curr_weight();
                mh << "\nElements: ";
                for (auto x: *it) mh << x << " ";
                mh << "\n";
                it.next();
            }
        }
        catch (std::exception& e) {
            std::cerr << e.what() << "\n";
            return 1;
        }
        mh.end("finished");
    }

    return 0;
}
