#include <iostream>

#include "AutoAnnotator.h"


/**
    Calculates the wasted resources for the given task distribution.
*/
int AutoAnnotator::calculateWaste(const std::vector<int>& distribution) {
    int ret = 0;
    for (int i = 0; i < mLength; ++i) {
        if (distribution[i] == mLength) {
            for (int d = 0; d < mDimension; ++d) {
                ret += mQueues[mDimension * mLength + i * mDimension + d];
            }
        }
    }
    return ret;
}

std::vector<int> AutoAnnotator::formatDistribution(std::vector<int> distribution) {
    for (int i = 0; i < mLength; ++i) {
        if (distribution[i] == mLength) {
            distribution[i] = 0;
            continue;
        }
        distribution[i] += 1;
    }
    return distribution;
}

/**
    Pretty prints the stored best distribution.
*/
void AutoAnnotator::printDistribution() {
    auto printDist = formatDistribution(mBestDistribution);
    for (int i = 0; i < mLength; ++i) {
        std::cout << "Job " << (i + 1) << ". : " << printDist[i] << "\n";
    }
}

/**
    Calculates one optimal solution.
*/
std::vector<int> AutoAnnotator::annotate() {
    std::vector<int> distribution(mLength, mLength);
    std::vector<int> workQueue = mQueues;

    calculateOptimum(workQueue, distribution, 0);
    return formatDistribution(mBestDistribution);
}

/**
    Assigns a task with 'taskId' to the node with 'nodeId'.

    Returns whether node had enough resources for the task.
    In case not, the operation is no op.
*/
bool AutoAnnotator::tryAssignTaskToNode(std::vector<int>& workQueue, int taskId, int nodeId) {
    if (nodeId == mLength) {
        return true;
    }

    bool valid = true;
    for (int d = 0; d < mDimension; ++d) {
        workQueue[nodeId * mDimension + d] -= workQueue[mLength * mDimension + taskId * mDimension + d];
        if (workQueue[nodeId * mDimension + d] < 0) {
            valid = false;
        }
    }
    if (!valid) {
        removeAssignedTaskFromNode(workQueue, taskId, nodeId);        
    }
    return valid;
}

void AutoAnnotator::removeAssignedTaskFromNode(std::vector<int>& workQueue, int taskId, int nodeId) {
    if (nodeId == mLength) {
        return;
    }

    for (int d = 0; d < mDimension; ++d) {
        workQueue[nodeId * mDimension + d] += workQueue[mLength * mDimension + taskId * mDimension + d];
    }
}

void AutoAnnotator::checkAndSaveDistribution(const std::vector<int>& distribution) {
    int waste = calculateWaste(distribution);
    if (waste < mBestWaste) {
        mBestDistribution = distribution;
        mBestWaste = waste;
    }
}

bool AutoAnnotator::taskIsEmpty(int taskId) {
	for (int d = 0; d < mDimension; ++d) {
		if (mQueues[mDimension * mLength + taskId * mDimension + d] != 0) {
			return false;
		}
	}
	return true;
}

void AutoAnnotator::calculateOptimum(std::vector<int>& workQueue,
                                     std::vector<int>& distribution,
                                     int taskId) {
    
    if (taskId == mLength) {
        checkAndSaveDistribution(distribution);
        return;
    }

    if (taskIsEmpty(taskId)) {
    	calculateOptimum(workQueue, distribution, taskId + 1);
    	return;
    }

    // need step when task is not assigned!
    for (int i = 0; i <= mLength; ++i) {
        if (tryAssignTaskToNode(workQueue, taskId, i)) {
            distribution[taskId] = i;
            calculateOptimum(workQueue, distribution, taskId + 1);
            removeAssignedTaskFromNode(workQueue, taskId, i);
            distribution[taskId] = mLength;
        }
    }

    return;
}