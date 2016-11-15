#include <iostream>
#include <algorithm>
#include <vector>
#include <exception>
#include <memory>
#include <random>
#include <chrono>
#include <numeric>
#include <iterator>

#include "cxxopts.hpp"

/**
    Handles command line options.
*/
class Options {
private:
    int mLength = 10;
    int mDimension = 2;
    int mSize = 1;
    bool mNaked = false;
    double mRatio = 0.2;
    bool mHelp = false;    
    cxxopts::Options options;
    void ensureConsistency() {
        mLength = std::max(1, mLength);        
        mDimension = std::max(1, mDimension);
        mRatio = std::min(1., std::max(0., mRatio));
        mSize = std::max(1, mSize);
    }
public:
    Options() : options("generate", "Generator of node and job queues") {
        options.add_options()
          ("l,length", "length of the queues (default: 10)", cxxopts::value<int>(mLength))
          ("d,dim", "dimension of the items (default: 2)", cxxopts::value<int>(mDimension))
          ("r,ratio", "amount of empty nodes and/or jobs distributed randomly (default: 0.2)",
            cxxopts::value<double>(mRatio))
          ("s,size", "number of generated sequence pairs (default: 1)", cxxopts::value<int>(mSize))
          ("n,naked", "Naked output, no '[', ',', ']' (default: false)", cxxopts::value<bool>(mNaked))
          ("h,help", "Prints help", cxxopts::value<bool>(mHelp))
        ;
    }
    bool parseCMDLine(int argc, char* argv[]) {
        try {
            options.parse(argc, argv);            
        } catch(const cxxopts::OptionException& e) {
            std::cerr << "error parsing options: " << e.what() << std::endl;
            return false;
        }
        ensureConsistency();
        return true;
    }
    int getLength() const {return mLength;}
    int getDimension() const {return mDimension;}
    double getRatio() const {return mRatio;}
    int getSize() const {return mSize;}
    bool isNaked() const {return mNaked;}
    bool isHelp() const {return mHelp;}
    std::string helpMessage() const {return options.help({""});}
    void print() const {
        std::cout << "options = {" 
                  << "\n  length: " << mLength
                  << ",\n  dimension: " << mDimension
                  << ",\n  ratio: " << mRatio
                  << ",\n  size: " << mSize
                  << ",\n  naked: " << mNaked
                  << ",\n  help: " << mHelp
                  << "\n}" << std::endl;
    }
};

template <typename Sequence>
void print(const Sequence& sequence)
{
    std::cout << "[";
    bool notFirst = false;
    for (const auto& s : sequence) {
        if (notFirst) {
            std::cout << ", ";
        }
        std::cout << s;
        notFirst = true;
    }
    std::cout << "]\n";
}

template <typename Sequence>
void print_naked(const Sequence& sequence)
{
    bool notFirst = false;
    for (const auto& s : sequence) {
        if (notFirst) {
            std::cout << " ";
        }
        std::cout << s;
        notFirst = true;
    }
    std::cout << "\n";
}

/**
    Returns a random distribution of the empty and valid nodes and jobs.
*/
std::vector<bool> randomizedNullItems(int length, double ratio)
{
    std::vector<bool> items(2 * length);
    int nullItems = ratio * 2 * length;
    for (int i = 0; i < nullItems; ++i) {
        items[i] = 1;
    }
    
    // randomize the items
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 m(seed);
    // TODO
    // Does std::shuffle() work on vector<bool>?
    std::shuffle(items.begin(), items.end(), m);

    return items;
}

/**
    Returns a vector<int>. All elements are between 0 and 100.
    
    The hidden structure is the following:
    Two consecutive 'length' number of tuples of 'dimension' number of items.
    First tuple array represents the node resources.
    The second tupple array represents the job resources.
*/
std::vector<int> generate(const Options& opts)
{
    int length = opts.getLength();
    int dim = opts.getDimension();
    auto nullItemDistribution = randomizedNullItems(length, opts.getRatio());

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 m(seed);
    
    std::uniform_int_distribution<int> dist(0, 100);
    auto dice = std::bind(dist, m);
    
    // assembling return vector
    std::vector<int> ret;
    ret.reserve(2 * length);
    for (int i = 0; i < 2 * length; ++i) {
        if (nullItemDistribution[i]) {
            for (int d = 0; d < dim; ++d) {
                ret.push_back(0);
            }
        } else {
            for (int d = 0; d < dim; ++d) {
                ret.push_back(dice());
            }
        }
    }

    return ret;
}

int main(int argc, char* argv[]) {
    Options opts;
    if (!opts.parseCMDLine(argc, argv)) {   
        return 1;
    }
    if (opts.isHelp())
    {
        std::cout << opts.helpMessage() << std::endl;
        return 0;
    }
    
    int size = opts.getSize();
    for (int i = 0; i < size; ++i) {
        auto ret = generate(opts);
        if (opts.isNaked()) {
            print_naked(ret);
        } else {
            print(ret);
        }
    }        
}






