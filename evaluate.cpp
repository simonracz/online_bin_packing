// Copyright (c) 2016 Simon Racz <simonracz@gmail.com>

#include <iostream>
#include <algorithm>
#include <tuple>
#include <functional>
#include <sstream>
#include <fstream>
#include <vector>
#include <exception>
#include <memory>
#include <numeric>
#include <iterator>
#include <iomanip>

#include "cxxopts.hpp"

/**
    Handles command line options.
*/
class Options {
private:
    std::string mPathTr;
    std::string mPathPr;
    int mDimension = 0;
    int mLength = 0;
    bool mHelp = false;    
    cxxopts::Options options;
    void ensureConsistency() {
        if (mHelp) {
            return;
        }
        if (mDimension == 0) {
            throw cxxopts::OptionException("Please add the set's dimension.");
        }
        if (mLength == 0) {
            throw cxxopts::OptionException("Please add the set's queue length.");
        }
        if (mPathTr.length() == 0) {
            throw cxxopts::OptionException("Path to training set can't be empty.");
        }
        if (mPathPr.length() == 0) {
            throw cxxopts::OptionException("Path to predicted solutions can't be empty.");
        }
    }
public:
    Options() : options("evaluate", "Compares given algorithm result to the First Fit algorithm.") {
        options.add_options()
          ("t,file_tr", "Training set file with optimal solutions", cxxopts::value<std::string>(mPathTr))
          ("p,file_pr", "Predicted solutions", cxxopts::value<std::string>(mPathPr))
          ("d,dim", "Dimension of the items (required)", cxxopts::value<int>(mDimension))
          ("l,length", "Length of the queues (required)", cxxopts::value<int>(mLength))
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
    std::string getPathTr() const {return mPathTr;}
    std::string getPathPr() const {return mPathPr;}
    int getDimension() const {return mDimension;}
    int getLength() const {return mLength;}
    bool isHelp() const {return mHelp;}
    std::string helpMessage() const {return options.help({""});}
    void print() const {
        std::cout << "options = {" 
                  << "\n  file_tr: " << mPathTr
                  << ",\n  file_pr: " << mPathPr
                  << ",\n  dimension: " << mDimension
                  << ",\n  length: " << mLength
                  << ",\n  help: " << mHelp
                  << "\n}" << std::endl;
    }
};

std::vector<int> readInput(const std::string& path, int totalLength) {
    std::vector<int> ret;
    auto fs = std::ifstream(path, std::ios::in);
    std::string line;
    while (std::getline(fs, line)) {
        std::istringstream ss{line};
        int input;
        for (int i = 0; i < totalLength; ++i) {
            if (ss >> input) {
                ret.push_back(input);
            }
        }
    }
    return ret;
}

std::vector<int> wasteForOptimum(const std::vector<int>& queues, int length, int dimension) {    
    int sampleMargin = 2 * length * dimension + length * (length + 1);
    int assignmentMargin = 2 * length * dimension;
    int sampleSize = queues.size() / sampleMargin;
    std::vector<int> ret;
    ret.reserve(sampleSize);
    // samples
    for (int k = 0; k < sampleSize; ++k) {
        int waste = 0;
        // tasks
        for (int i = 0; i < length; ++i) {
            // assigned to Node 0
            if (queues[sampleMargin * k + assignmentMargin + i * (length + 1)] == 1) {
                for (int d = 0; d < dimension; ++d) {
                    waste += queues[sampleMargin * k + length * dimension + i * dimension + d];
                }
            }
        }
        ret.push_back(waste);
    }
    return ret;
}

std::vector<int> wasteForSet(const std::vector<int>& queues,
                             const std::vector<int>& prediction,
                             int length,
                             int dimension) {

    int sampleMarginInQueue = 2 * length * dimension + length * (length + 1);
    int sampleMargin = length * (length + 1);
    int sampleSize = prediction.size() / sampleMargin;
    std::vector<int> ret;
    ret.reserve(sampleSize);
    // samples
    for (int k = 0; k < sampleSize; ++k) {
        int waste = 0;
        // tasks
        for (int i = 0; i < length; ++i) {
            // assigned to Node 0
            if (prediction[sampleMargin * k + i * (length + 1)] == 1) {
                for (int d = 0; d < dimension; ++d) {
                    waste += queues[sampleMarginInQueue * k + length * dimension + i * dimension + d];
                }
            }
        }
        ret.push_back(waste);
    }
    return ret;
}

std::vector<int> extractResources(const std::vector<int>& queues, int sample, int length, int dimension) {
    std::vector<int> ret;
    ret.reserve(length * dimension);
    int sampleMargin = 2 * length * dimension + length * (length + 1);
    for (int i = 0; i < length; ++i) {
        for (int d = 0; d < dimension; ++d) {
            ret.push_back(queues[sample * sampleMargin + i * dimension + d]);
        }
    }
    return ret;
}

struct SortableTask {
    int originalIndex = 0;
    int assignment = 0;
    std::vector<int> resources;
};

double harmonicMean(const std::vector<int>& nums) {
    double sum = 0;
    for (auto& item : nums) {
        sum += 1. / (double)item;
    }
    return nums.size() / sum;
}

std::vector<SortableTask> sortedTasks(const std::vector<int>& queues, int sample, int length, int dimension) {
    std::vector<SortableTask> ret;
    ret.reserve(length);
    int sampleMargin = 2 * length * dimension + length * (length + 1);
    for (int i = 0; i < length; ++i) {
        SortableTask task;
        task.originalIndex = i;
        for (int d = 0; d < dimension; ++d) {
            task.resources.push_back(queues[sample * sampleMargin + length * dimension + i * dimension + d]);
        }
        ret.push_back(task);
    }
    std::sort(ret.begin(), ret.end(), [] (const SortableTask& left, const SortableTask& right) -> bool {
        return harmonicMean(left.resources) > harmonicMean(right.resources);
    });
    
    return ret;
}

std::vector<int> checkPrediction(const std::vector<int>& queues,
                                 const std::vector<int>& prediction,
                                 int length,
                                 int dimension) {
    std::vector<int> ret;
    ret.reserve(prediction.size());
    int sampleMarginInQueue = 2 * length * dimension + length * (length + 1);
    int sampleMargin = length * (length + 1);
    int sampleSize = prediction.size() / sampleMargin;

    // samples
    for (int k = 0; k < sampleSize; ++ k) {
        auto resources = extractResources(queues, k, length, dimension);
        // tasks
        for (int i = 0; i < length; ++i) {
            int assignment = 0;
            // assignment
            for (int j = 0; j < (length + 1); ++j) {
                if (prediction[sampleMargin * k + i * (length + 1) + j] == 1) {
                    assignment = j;
                    break;
                }
            }
            if (assignment != 0) {
                bool valid = true;
                for (int d = 0; d < dimension; ++d) {
                    resources[i * dimension + d] 
                        -= queues[sampleMarginInQueue * k + length * dimension + i * dimension + d];
                    if (resources[i * dimension + d] < 0) {
                        valid = false;
                    }
                }
                if (!valid) {
                    for (int d = 0; d < dimension; ++d) {
                        resources[i * dimension + d] 
                            += queues[sampleMarginInQueue * k + length * dimension + i * dimension + d];
                    }
                    ret.push_back(1);
                    for (int m = 0; m < length; ++m) {
                        ret.push_back(0);
                    }
                    continue;
                }
            }
            for (int m = 0; m < (length + 1); ++m) {
                ret.push_back(prediction[sampleMargin * k + i * (length + 1) + m]);
            }
        }
    }
    return ret;
}

std::vector<int> worstPrediction(int sampleSize, int length) {
    std::vector<int> ret;
    ret.reserve(sampleSize * length * (length + 1));

    for (int k = 0; k < sampleSize; ++k) {
        for (int i = 0; i < length; ++i) {
            ret.push_back(1);
            for (int j = 0; j < length; ++j) {
                ret.push_back(0);
            }
        }
    }

    return ret;
}

bool tryAssignTaskToNode(std::vector<int>& workQueue, const SortableTask& task, int nodeId) {
    bool valid = true;
    int dimension = task.resources.size();
    for (int d = 0; d < dimension; ++d) {
        workQueue[nodeId * dimension + d] -= task.resources[d];
        if (workQueue[nodeId * dimension + d] < 0) {
            valid = false;
        }
    }
    if (!valid) {
        for (int d = 0; d < dimension; ++d) {
            workQueue[nodeId * dimension + d] += task.resources[d];
        }
    }
    return valid;
}

std::vector<int> calculateFirstFit(const std::vector<int>& queues, int length, int dimension) {
    std::vector<int> ret;
    int sampleSize = queues.size() / (2 * length * dimension + length * (length + 1));
    ret.reserve(sampleSize * length * (length + 1));

    for (int k = 0; k < sampleSize; ++k) {
        auto resources = extractResources(queues, k, length, dimension);
        auto tasks = sortedTasks(queues, k, length, dimension);
        for (int i = 0; i < length; ++i) {
            for (int j = 1; j < (length + 1); ++j) {
                if (tryAssignTaskToNode(resources, tasks[i], (j - 1))) {
                    tasks[i].assignment = j;
                    break;
                }
            }
        }
        for (int i = 0; i < length; ++i) {
            // linear search, could do better
            for (auto& task : tasks) {
                if (task.originalIndex == i) {
                    for (int d = 0; d < (length + 1); ++d) {
                        if (task.assignment == d) {
                            ret.push_back(1);
                        } else {
                            ret.push_back(0);
                        }
                    }
                    break;
                }
            }
        }
    }
    return ret;
}

struct Stats {
    double mean = 0;
    double variance = 0;
};

Stats calculateStatistics(const std::vector<int>& wasteWorstPred,
                          const std::vector<int>& wasteOpt,
                          const std::vector<int>& wastePred) {
    int sampleSize = wasteOpt.size();
    Stats stats;
    double prevMean = 0;
    double mean = 0;
    double variance = 0;
    for (int i = 0; i < sampleSize; ++i) {
        prevMean = mean;
        // normalize
        double divider = wasteWorstPred[i] - wasteOpt[i];
        divider = std::max(1., divider);
        double shift = wasteOpt[i];
        double item = (wastePred[i] - shift) / divider;
        if (item < 0) {
            std::cerr << "item below 0\nopt: " << wasteOpt[i] << "\nworst: " << wasteWorstPred[i] << "\nitem: " << wastePred[i] << "\n";

        }
        if (item > 1) {
            std::cerr << "item above 1\nopt: " << wasteOpt[i] << "\nworst: " << wasteWorstPred[i] << "\nitem: " << wastePred[i] << "\n";
        }
        mean = prevMean + (item - prevMean) / (i + 1);
        variance = variance + (item - prevMean) * (item - mean);
    }    
    stats.mean = mean;
    stats.variance = variance;
    return stats;
}

void printStatistics(const std::vector<int>& wasteWorstPred,
                     const std::vector<int>& wasteOpt,
                     const std::vector<int>& wastePred,
                     const std::vector<int>& wasteFF) {
    
    int sampleSize = wasteOpt.size();
    auto predStats = calculateStatistics(wasteWorstPred, wasteOpt, wastePred);
    auto ffStats = calculateStatistics(wasteWorstPred, wasteOpt, wasteFF);

    std::cout << "\nComparison of wasted resources for First Fit (FF) and provided matches.\n";
    std::cout << "\nThe waste is normalized. Optimal solution has 0 mean and 0 std.\n\n";
    std::cout << "FF\n\nMean: " << ffStats.mean << "\nStandard deviation: " << std::sqrt(ffStats.variance / sampleSize);
    std::cout << "\n\nCustom Algorithm\n\nMean: " << predStats.mean
     << "\nStandard deviation: " << std::sqrt(predStats.variance / sampleSize) << std::endl;
    std::cout << std::endl;
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
    int length = opts.getLength();
    int dim = opts.getDimension();

    auto queues = readInput(opts.getPathTr(), 2 * length * dim + length * (length + 1));
    
    auto prediction = readInput(opts.getPathPr(), length * (length + 1));
    prediction = checkPrediction(queues, prediction, length, dim);
    int sampleSize = prediction.size() / (length * (length + 1));
    
    auto firstFit = calculateFirstFit(queues, length, dim);
    auto worstPred = worstPrediction(sampleSize, length);
    
    auto wasteOpt = wasteForOptimum(queues, length, dim);
    auto wastePred = wasteForSet(queues, prediction, length, dim);
    auto wasteWorstPred = wasteForSet(queues, worstPred, length, dim);
    auto wasteFF = wasteForSet(queues, firstFit, length, dim);

    printStatistics(wasteWorstPred, wasteOpt, wastePred, wasteFF);
    return 0;
}







