#include <omp.h>
#include <iostream>
#include <queue>
#include <stack>
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

double max(double M, double gOfC, double gOfD, double (*g)(double))
{
    double maxValue;

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

bool getDeeper(double gOfC, double gOfD, double c, double d, double s, double (*g)(double), double M, double epsilon)
{
    if(((gOfC + gOfD + s * (d - c)) / 2) > (M + epsilon))
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
    double (*gPtr)(double) = &g;
    clock_t startTime;
    clock_t endTime;
    bool* busyArray;
    queue<Node> mainQueue;
    stack<Node> mainStack;

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

    if(!getDeeper(boundaryValueOfA, boundaryValueOfB, a, b, s, gPtr, M, epsilon))
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

        // master thread: initiation
        #pragma omp master
        {
            nthreads = omp_get_num_threads();
            busyArray = new bool[nthreads]();
            startTime = omp_get_wtime();
            mainQueue.push(Node(a, (a + b) / 2));
            mainQueue.push(Node((a + b) / 2, b));
        }
        #pragma omp barrier

        bool deeper = false;
        double localM = M;
        Node curTask(0, 0);
        while(true)
        {
            if(curTask.lowerBound == 0 && curTask.upperBound == 0)
            {
                omp_set_lock(&queueLock);
                    if(mainQueue.empty())
                    {   
                        busyArray[tid] = false;
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
                    curTask = mainQueue.front();
                    mainQueue.pop();
                omp_unset_lock(&queueLock); 
            }

            double lowerBound = curTask.lowerBound;
            double upperBound = curTask.upperBound;
            curTask.lowerBound = 0;
            curTask.upperBound = 0;

            double gOfLowerBound = g(lowerBound);
            double gOfUpperBound = g(upperBound);        
            localM = max(localM, gOfLowerBound, gOfUpperBound, gPtr);
            deeper = getDeeper(gOfLowerBound, gOfUpperBound, lowerBound, upperBound, s, gPtr, localM, epsilon);
                          
            omp_set_lock(&MLock);
                if(localM > M)
                {
                    M = localM;
                }
                else
                {
                    localM = M;
                }
            omp_unset_lock(&MLock);

            if(deeper)
            {
                omp_set_lock(&queueLock);
                    mainQueue.push(Node(lowerBound, (lowerBound + upperBound) / 2));
                omp_unset_lock(&queueLock);
                curTask = Node((lowerBound + upperBound) / 2, upperBound);
            }
        }
    }
    endTime = omp_get_wtime();
    cout << "Maximum: " << M << endl;
    cout << "execution time: " << (endTime - startTime) << " seconds" << endl;
    delete[] busyArray;
}
