#include <omp.h>
#include <iostream>
#include <queue>
#include <stack>
#include <ctime>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include "g.h"
using namespace std;

class Node
{   
    public:
        double lowerBound;
        double upperBound;
        Node(double lowerBoundValue, double upperBoundValue)
        {
            this->lowerBound = lowerBoundValue;
            this->upperBound = upperBoundValue;
        };
};

double max(double M, double c, double d, double (*g)(double))
{
    double maxValue;
    double gOfC = g(c);
    double gOfD = g(d);

    if(M > gOfC)
    {
        maxValue = M;
    }
    else
    {
        maxValue = gOfC;
    }
    if(gOfD > maxValue)
    {
        maxValue = gOfD;
    }

    return maxValue;
}
double potentialMax(double c, double d, double s, double (*g)(double))
{
    return (g(c) + g(d) + s * (d - c)) / 2;
}
bool getDeeper(double c, double d, double s, double (*g)(double), double M, double epsilon)
{
    if(potentialMax(c, d, s, g) > (M + epsilon))
    {
        return true;
    }
    else
    {
        return false;
    }
}

int main (int argc, char *argv[]) 
{
    omp_lock_t queueLock;
    omp_lock_t MLock;
    omp_init_lock(&queueLock);
    omp_init_lock(&MLock);

    double a = 1;
    double b = 100;
    double epsilon = 0.000001;
    double s = 12;
    double M;
    double boundaryValueOfA = g(a);
    double boundaryValueOfB = g(b);
    bool findMAX = false;
    double (*gPtr)(double) = &g;
    clock_t startTime;
    clock_t endTime;
    queue<Node> mainQueue;
    bool* busyArray;

    // Initialization of M
    if(boundaryValueOfA > boundaryValueOfB)
    {
        M = boundaryValueOfA;
    }
    else
    {
        M = boundaryValueOfB;
    }

    cout << "initial M: " << M << endl;

    if(!getDeeper(a, b, s, gPtr, M, epsilon))
    {
        cout << "Maximum is on the boundary at first" << endl;
        cout << "M: " << M << endl;

        return 0;
    }

    int nthreads;
    int tid;
    int iteration = 0;
    #pragma omp parallel private(tid)
    {
        // get individual thread id
        tid = omp_get_thread_num();
        nthreads = omp_get_num_threads();

        // master thread: initiation
        #pragma omp master
        {
            busyArray = new bool[nthreads]();
            startTime = omp_get_wtime();
            mainQueue.push(Node(a, (a + b) / 2));
            mainQueue.push(Node((a + b) / 2, b));



            for(int i = 0; i < nthreads * 100; i++)
            {
                if(!mainQueue.empty())
                {
                    Node curTask = mainQueue.front();
                    mainQueue.pop();
                    double lowerBound = curTask.lowerBound;
                    double upperBound = curTask.upperBound;
                    M = max(M, lowerBound, upperBound, gPtr);
                    bool deeper = getDeeper(lowerBound, upperBound, s, gPtr, M, epsilon);
                    if(deeper)
                    {
                        mainQueue.push(Node(lowerBound, (lowerBound + upperBound) / 2));
                        mainQueue.push(Node((lowerBound + upperBound) / 2, upperBound));
                    }
                }
                else
                {
                    cout << "something wrong! mainQueue could not be empty for now" << endl;
                }
            }
        }
        #pragma omp barrier

        // Node curTask();
        bool deeper = false;
        queue<Node> subQueue;
        stack<Node> subStack;
        int batchSize;
        // int subQueueLimit = 1000;

        if(tid % 2 == 0)
        {
            while(true)
            {
                if(subQueue.empty())
                {
                    omp_set_lock(&queueLock);
                        if(mainQueue.empty())
                        {   
                            bool allRestOrNot = true;
                            for(int i = 0; i < nthreads; i++)
                            {
                                if(busyArray[i])
                                {
                                    allRestOrNot = false;
                                    break;
                                }
                            }
                            if(allRestOrNot)
                            {
                                omp_unset_lock(&queueLock);
                                break;
                            }
                            else
                            {
                                omp_unset_lock(&queueLock);
                                continue;
                            }
                        }
                        busyArray[tid] = true;
                        batchSize = mainQueue.size() / nthreads + 1;
                        for(int i = 0; i < batchSize; i++)
                        {
                            if(!mainQueue.empty())
                            {
                                subQueue.push(mainQueue.front());
                                mainQueue.pop();
                            }
                            else
                            {
                                break;
                            }
                        }
                    omp_unset_lock(&queueLock);         
                }
                Node curTask = subQueue.front();
                subQueue.pop();
                double lowerBound = curTask.lowerBound;
                double upperBound = curTask.upperBound;
                // delete curTask;

                omp_set_lock(&MLock);
                    M = max(M, lowerBound, upperBound, gPtr);
                    deeper = getDeeper(lowerBound, upperBound, s, gPtr, M, epsilon);
                omp_unset_lock(&MLock);

                if(deeper)
                {
                    subQueue.push(Node(lowerBound, (lowerBound + upperBound) / 2));
                    subQueue.push(Node((lowerBound + upperBound) / 2, upperBound));
                }

                if(subQueue.size() > batchSize * 3)
                {
                    omp_set_lock(&queueLock);
                        for(int i = 0; i < batchSize; i++)
                        {
                            mainQueue.push(subQueue.front());
                            subQueue.pop();
                        }
                    omp_unset_lock(&queueLock);
                }

                iteration++;
                if(iteration % 2000 == 0)
                {
                    cout << "tid: " << tid << endl;
                    cout << "iteration: " << iteration << endl;
                    cout << "execution time: " << omp_get_wtime() - startTime << endl;
                    cout << "mainQueueSize: " << mainQueue.size() << endl;
                    cout << "subQueueSize: " << subQueue.size() << endl;
                    cout << "M: " << M << endl; 
                }
            }
        }
        else
        {
            while(true)
            {
                if(subStack.empty())
                {
                    omp_set_lock(&queueLock);
                        if(mainQueue.empty())
                        {   
                            bool allRestOrNot = true;
                            for(int i = 0; i < nthreads; i++)
                            {
                                if(busyArray[i])
                                {
                                    allRestOrNot = false;
                                    break;
                                }
                            }
                            if(allRestOrNot)
                            {
                                omp_unset_lock(&queueLock);
                                break;
                            }
                            else
                            {
                                omp_unset_lock(&queueLock);
                                continue;
                            }
                        }
                        busyArray[tid] = true;
                        batchSize = mainQueue.size() / nthreads + 1;
                        for(int i = 0; i < batchSize; i++)
                        {
                            if(!mainQueue.empty())
                            {
                                subStack.push(mainQueue.front());
                                mainQueue.pop();
                            }
                            else
                            {
                                break;
                            }
                        }
                    omp_unset_lock(&queueLock);         
                }
                Node curTask = subStack.top();
                subStack.pop();
                double lowerBound = curTask.lowerBound;
                double upperBound = curTask.upperBound;
                // delete curTask;

                omp_set_lock(&MLock);
                    M = max(M, lowerBound, upperBound, gPtr);
                    deeper = getDeeper(lowerBound, upperBound, s, gPtr, M, epsilon);
                omp_unset_lock(&MLock);

                if(deeper)
                {
                    subStack.push(Node(lowerBound, (lowerBound + upperBound) / 2));
                    subStack.push(Node((lowerBound + upperBound) / 2, upperBound));
                }

                if(subStack.size() > batchSize * 3)
                {
                    omp_set_lock(&queueLock);
                        for(int i = 0; i < batchSize; i++)
                        {
                            mainQueue.push(subStack.top());
                            subStack.pop();
                        }
                    omp_unset_lock(&queueLock);
                }

                iteration++;
                if(iteration % 2000 == 0)
                {
                    cout << "tid: " << tid << endl;
                    cout << "iteration: " << iteration << endl;
                    cout << "execution time: " << omp_get_wtime() - startTime << endl;
                    cout << "mainQueueSize: " << mainQueue.size() << endl;
                    cout << "subQueueSize: " << subStack.size() << endl;
                    cout << "M: " << M << endl; 
                }
            }
        }
    }

    endTime = omp_get_wtime();
    cout << "queue size: " << mainQueue.size() << endl;
    cout << "Maximum is " << M << endl;
    cout << "execution time: " << (endTime - startTime) << " seconds" << endl;
    delete[] busyArray;
}
