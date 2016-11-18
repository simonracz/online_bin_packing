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

#include "AutoAnnotator.h"

/**
    Handles command line options.
*/
class Options {
private:
    std::string mPath;
    int mDimension = 2;
    bool mAuto = false;
    bool mHelp = false;    
    cxxopts::Options options;
    void ensureConsistency() {
        mDimension = std::max(1, mDimension);
        if (mPath.length() == 0) {
            throw cxxopts::OptionException("Path can't be empty.");
        }
    }
public:
    Options() : options("annotate", "Online bin packing annotator for creating training sets") {
        options.add_options()
          ("f,file", "Results are appended to this file", cxxopts::value<std::string>(mPath)
                ->default_value("ann.txt"))
          ("d,dim", "Dimension of the items (default: 2)", cxxopts::value<int>(mDimension))
          ("a,auto", "Automatically find one optiomal solution (exponential runtime!)", cxxopts::value<bool>(mAuto))
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
    bool isAuto() const {return mAuto;}
    bool isHelp() const {return mHelp;}
    std::string helpMessage() const {return options.help({""});}
    void print() const {
        std::cout << "options = {" 
                  << "\n  file: " << mPath
                  << ",\n  dimension: " << mDimension
                  << ",\n  auto: " << mAuto
                  << ",\n  help: " << mHelp
                  << "\n}" << std::endl;
    }
};

class AnnotatorException : public std::exception {
private:
    std::string m_message;
public:
    AnnotatorException(const std::string& message) : m_message(message) {
        // empty
    }

    virtual const char* what() const noexcept {
        return m_message.c_str();
    }
};

/**
    Reads input from cmd line in the format of "[int, int, ...]"

    Returns an std::vector<int>
*/
std::vector<int> readInput() {
    std::vector<int> v;
    std::string line;
    std::getline(std::cin, line);
    std::istringstream ss{line};
    char c;
    ss >> c;
    if (c != '[') {
        throw AnnotatorException("Input error. Should start with '['.");
    }
    int i;
    c = ',';
    while (c != ']') {
        if (c != ',') {
            throw AnnotatorException("Input error. Should separate elements by ','.");
        }
        ss >> i >> c;       
        v.push_back(i);
    }
    return v;
}

/**
    Pretty prints one queue.

    Sample output:
    "
       1.         2.         3.
     -----      -----      -----    
    | 100 |    |   5 |    |   4 |
    |  71 |    |  22 |    |  14 |
     -----      -----      -----
    "

    See prettyPrintQueues() for more.
*/
void prettyPrintQueue(const std::vector<int>&queues, int offset, const Options& opts) {
    int dim = opts.getDimension();
    int length = queues.size() / 2 / dim;

    //   1.         2.         3.
    for (int i = 0; i < length; ++i) {
        std::cout.width(4);
        std::cout << std::right << (i + 1) << '.' << "      ";
    }
    std::cout << '\n';

    // -----      -----      -----    
    for (int i = 0; i < length; ++i) {
        std::cout << " -----     ";
    }
    std::cout << '\n';

    //|   1 |    |   0 |    |  55 |
    //|  71 |    |  22 |    |  14 |
    for (int d = 0; d < dim; ++d) {
        for (int i = 0; i < length; ++i) {
            std::cout << "| ";
            std::cout.width(3);
            std::cout << std::right << queues[i * dim + d + offset] << " |    ";
        }
        std::cout << '\n';
    }

    // -----      -----      -----    
    for (int i = 0; i < length; ++i) {
        std::cout << " -----     ";
    }
    std::cout << '\n';
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
    "
*/
void prettyPrintQueues(const std::vector<int>& queues, const Options& opts) {
    std::cout << "\nNodes.\n\n";
    prettyPrintQueue(queues, 0, opts);
    std::cout << "\nJobs\n\n";
    prettyPrintQueue(queues, queues.size() / 2, opts);    
}

/**
    Asks user for input to annotate the jobs.

    Unnassignable jobs should be assigned to Node 0, which is virtual node.

    Returns the annotations.
*/
std::vector<int> annotate(int length) {
    std::cout << "\nAnnotate the best distribution of the jobs on the given nodes.\n\n";
    std::cout << "Give the assigned Node number for each job. (0 means unassigned)\n\n";
    
    std::vector<int> annotations;
    annotations.reserve(length);

    std::string line;
    for (int i = 0; i < length; ++i) {
        std::cout << "Job " << (i + 1) << ". :  ";
        std::getline(std::cin, line);
        if (line.length() == 0) {        
            throw AnnotatorException("No annotations.");
        }
        std::istringstream ss{line};
        int n;
        ss >> n;
        annotations.push_back(n);
    }
    return annotations;
}

/**
    Appends the queues and it's annotations to the file specified by the 'file' cmd line option.
*/
void writeToFile(const std::string& path, const std::vector<int>& queues, const std::vector<int>& annotations) {
    auto fs = std::ofstream(path, std::ios::app|std::ios::out);
    bool notFirst = false;
    for (auto it = queues.cbegin(); it != queues.cend(); ++it) {
        if (notFirst) {
            fs << " ";
        }
        fs << *it;
        notFirst = true;
    }

    for (auto it = annotations.cbegin(); it != annotations.cend(); ++it) {        
        fs << ' ' << *it;        
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
    prettyPrintQueues(queues, opts);
    std::vector<int> annotations;
    if (opts.isAuto()) {
        auto autoAnnotator = AutoAnnotator(queues, opts.getDimension());
        annotations = autoAnnotator.annotate();
        autoAnnotator.printDistribution();
    } else {
        annotations = annotate(queues.size() / 2 / opts.getDimension());    
    }    
    writeToFile(opts.getPath(), queues, annotations);
    return 0;
}







