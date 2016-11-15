#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <exception>
#include <memory>
#include <random>
#include <chrono>
#include <numeric>
#include <iterator>
#include <iomanip>
#include <exception>

#include "cxxopts.hpp"

/**
    Handles command line options.
*/
class Options {
private:
    std::string mPath;
    int mDimension = 2;
    bool mHelp = false;    
    cxxopts::Options options;
    void ensureConsistency() {
        mDimension = std::max(1, mDimension);
        if (mFilePath.length() == 0) {
            throw cxxopts::OptionException();
        }
    }
public:
    Options() : options("annotate", "Online bin packing annotator for learning algorithms") {
        options.add_options()
          ("f,file", "Results are appended to this file", cxxopts::value<std::string>(mPath)
                ->default_value("ann.txt"))
          ("d,dim", "Dimension of the items (default: 2)", cxxopts::value<int>(mDimension))
          ("h,help", "Prints help", cxxopts::value<bool>(mHelp))
        ;
    }
    bool parseCMDLine(int argc, char* argv[]) {
        try {
            options.parse(argc, argv);
            ensureConsistency();
        } catch(const cxxopts::OptionException& e) {
            std::cerr << "error parsing options: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
    std::string getPath() const {return mPath;}
    int getDimension() const {return mDimension;}
    bool isHelp() const {return mHelp;}
    std::string helpMessage() const {return options.help({""});}
    void print() const {
        std::cout << "options = {" 
                  << "\n  file: " << mPath
                  << ",\n  dimension: " << mDimension
                  << ",\n  help: " << mHelp
                  << "\n}" << std::endl;
    }
};

/**
    Reads input from cmd line in the format of "[int, int, ...]"

    Returns std::vector<int>
*/
std::vector<int> readInput() {
    std::vector<int> v;
    std::string line;
    std::getline(std::cin, line);
    std::istringstream ss{line};
    char c;
    ss >> c;
    if (c != '[') {
        throw std::exception("Input error. Should start with '['.");
    }
    int i;
    c = ',';
    while (c != ']') {
        if (c != ',') {
            throw std::exception("Input error. Should separate elements by ','.");
        }
        ss >> i >> c;       
        v.push_back(i);
    }
    return v;
}

/**
    Sample output:
    "
    Nodes

       1.         2.         3.
     -----      -----      -----    
    | 100 |    |   5 |    |   4 |
    |  71 |    |  22 |    |  14 |
     -----      -----      -----

    Jobs

       1.         2.         3.
     -----      -----      -----
    |   1 |    |   0 |    |  55 |
    |  20 |    |   0 |    |   9 |
     -----      -----      -----

    Annotate the best distribution of the jobs on the given nodes.
    "

    Returns the length of the queues.
*/
int prettyPrintQueues(const std::vector<int>& queues, const options& opts) {
    std::cout << "\nNode resources.\n\n"
    int dim = opts.getDimension();

    int length = queues.size() / 2 / dim;
    
    return length;
}

/**
    Asks user for input to annotate the jobs.

    Unnassignable jobs can/should be assigned to Node 0, which is not a valid node.

    Returns the annotations.
*/
std::vector<int> annotate(length) {
    std::cout << "\nT = (" << target.first << ", " << target.second << ")\n";
    std::cout << "End coordinates of path, as 'x, y' (default is T): ";
    std::string line;
    std::getline(std::cin, line);
    std::istringstream ss{line};
    char c;
    if (line.length() == 0) {        
        return target;
    }
    ss >> target.first >> c;
    if (c != ',') {
        throw std::exception("Input error. Should separate elements by ','.");        
    }
    ss >> target.second;
    return target;
}

/**
    Appends the queues and it's annotations to the file specified by the 'file' cmd line option.
*/
void writeToFile(const std::string& filePath, const std::vector<int>& queues, const std::vector<int>& annotations) {
    auto fs = std::ofstream(filePath, std::ios::app|std::ios::out);
    bool notFirst = false;
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        if (notFirst) {
            fs << " ";
        }
        fs << *it;
        notFirst = true;
    }
    int mSize = (-3 + std::sqrt(9 - 4 * 3 * (1 - map.size()))) / 6;
    for (int y = -mSize; y <= mSize; ++y) {
        for (int x = -mSize; x <= mSize; ++x) {
            int z = -x - y;
            if (z > mSize || z < -mSize) {
                continue;
            }
            fs << " ";
            if (x == P.first && y == P.second) {
                fs << "1";
                continue;
            }
            fs << "0";
        }
    }
    fs << "\n";
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
    // opts.print();
    auto queues = readInput();
    auto length = prettyPrintQueues(queues, opts);
    auto annotations = annotate(length);
    writeToFile(opts.getFilePath(), queues, annotations);
    return 0;
}







