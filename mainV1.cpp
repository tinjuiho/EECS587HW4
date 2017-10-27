#include <omp.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "g.h"
using namespace std;

class node
{   public:
        double lowerBound;
        double upperBound;
        node* link;
        node(double lowerBoundValue, double upperBoundValue)
        {
            this->lowerBound = lowerBoundValue;
            this->upperBound = upperBoundValue;
            link = NULL;
        };
};
class queueList
{
    public:
        node* front;
        node* rear;
        int length;
        void insert(double lowerBound, double upperBound)
        {
            node* newNode = new node(lowerBound, upperBound);
            if(this->front == NULL)
            {
                this->front = newNode;
            }
            else
            {
                this->rear->link = newNode;
            }
            this->rear = newNode;
            length++;
        };
        void insert(node* newNode)
        {
            if(this->front == NULL)
            {
                this->front = newNode;
            }
            else
            {
                this->rear->link = newNode;
            }
            this->rear = newNode;
            length++;
        };
        node* del()
        {
            node* frontNode = NULL;
            if(this->front == NULL)
            {
                ;
            }
            else
            {
                frontNode = this->front;
                this->front = this->front->link;
                length--;
            }

            return frontNode;
        };
        queueList()
        {
            front = NULL;
            rear = NULL;
            length = 0;
        }

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
    double* queueFrontPtr = NULL;
    double* queueBackPtr = NULL;
    // int problemNum = 0;
    bool findMAX = false;
    bool* busyArray;
    double (*gPtr)(double) = &g;
    double startTime;
    double endTime;
    queueList mainQueue;
    int subQueueLimit = 2;

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
    // problemNum++;

    int nthreads;
    int tid;
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
            omp_set_lock(&queueLock);
                mainQueue.insert(a, (a + b) / 2);
                mainQueue.insert((a + b) / 2, b);
            omp_unset_lock(&queueLock);
            int iteration = 0;
            bool allRestOrNot;
            while(!findMAX)
            {
                omp_set_lock(&queueLock);
                    // check whether finding a maximum or not
                    if(mainQueue.front == NULL)
                    {
                        allRestOrNot = true;
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
                            findMAX = true;
                        }
                    }
                omp_unset_lock(&queueLock);
                iteration++;
                if(iteration % 1000000 == 0)
                {
                    cout << "iteration: " << iteration << endl;
                    cout << "M: " << M << endl;
                    cout << "queue size: " << mainQueue.length << endl;
                }
            }
        }
        // #pragma omp barrier

        if(tid != 0)
        {
            // thread id is even
            // if(tid % 2 == 0)
            // {
                // node* curTask = NULL;
                // bool deeper = false;
                // queueList subQueue;
                // int batchSize;
                // while(!findMAX)
                // {
                //     if(subQueue.front == NULL)
                //     {
                //         omp_set_lock(&queueLock);
                //             batchSize = mainQueue.length / nthreads + 1;
                //             for(int i = 0; i < batchSize; i++)
                //             {
                //                 node* newNode = mainQueue.del();
                //                 if(newNode != NULL)
                //                 {
                //                     subQueue.insert(newNode);
                //                 }
                //                 else
                //                 {
                //                     break;
                //                 }
                //             }
                //             if(subQueue.front == NULL)
                //             {
                //                 busyArray[tid] = false;
                //                 omp_unset_lock(&queueLock);
                //                 continue;
                //             }
                //             busyArray[tid] = true;
                //         omp_unset_lock(&queueLock);
                //     }

                //     curTask = subQueue.del();
                //     double lowerBound = curTask->lowerBound;
                //     double upperBound = curTask->upperBound;
                //     delete curTask;

                //     omp_set_lock(&MLock);
                //         M = max(M, lowerBound, upperBound, gPtr);
                //         deeper = getDeeper(lowerBound, upperBound, s, gPtr, M, epsilon);
                //     omp_unset_lock(&MLock);

                //     if(deeper)
                //     {
                //         subQueue.insert(lowerBound, (lowerBound + upperBound) / 2);
                //         subQueue.insert((lowerBound + upperBound) / 2, upperBound);
                //     }
                //     else
                //     {
                //         curTask = NULL;
                //     }

                //     if(subQueue.length > subQueueLimit)
                //     {
                //         omp_set_lock(&queueLock);
                //             for(int i = 0; i < subQueueLimit/2; i++)
                //             {
                //                 mainQueue.insert(subQueue.del());
                //             }
                //         omp_unset_lock(&queueLock);
                //     }
                // }
            // }
            // else if(tid % 2 != 0) // thread id is odd
            // {
                node* curTask = NULL;
                bool deeper = false;
                while(!findMAX)
                {
                    omp_set_lock(&queueLock);
                    if(curTask == NULL)
                    {
                        curTask = mainQueue.del();
                        if(curTask == NULL)
                        {
                            busyArray[tid] = false;
                            omp_unset_lock(&queueLock);
                            continue;
                        }
                    }
                    busyArray[tid] = true;
                    omp_unset_lock(&queueLock);

                    double lowerBound = curTask->lowerBound;
                    double upperBound = curTask->upperBound;
                    delete curTask;
                    
                    omp_set_lock(&MLock);
                    M = max(M, lowerBound, upperBound, gPtr);
                    deeper = getDeeper(lowerBound, upperBound, s, gPtr, M, epsilon);
                    omp_unset_lock(&MLock);

                    if(deeper)
                    {
                        omp_set_lock(&queueLock);
                        mainQueue.insert(lowerBound, (lowerBound + upperBound) / 2);
                        // ql.insert((lowerBound + upperBound) / 2, upperBound);
                        omp_unset_lock(&queueLock);
                        curTask = new node((lowerBound + upperBound) / 2, upperBound);
                    }
                    else
                    {
                        curTask = NULL;
                    }
                }
            // }
        }


        // #pragma omp barrier
        // for(int i=0; i < nthreads; i++)
        // {
        //     if(busyArray[i])
        //     {
        //         cout << "got it!" << endl;
        //     }
        //     else
        //     {
        //         cout << "false" << endl;
        //     }
        // }
    }// All threads join master thread and disband

    endTime = omp_get_wtime();
    cout << "queue size: " << mainQueue.length << endl;
    cout << "Maximum is " << M << endl;
    cout << "execution time: " << (endTime - startTime) << " seconds" << endl;
    delete[] busyArray;
}

    // int nthreads, tid;
    // /* Fork a team of threads giving them their own copies of variables */
    // #pragma omp parallel private(nthreads, tid)
    // {
    //     /* Obtain thread number */
    //     tid = omp_get_thread_num();
    //     printf("Hello World from thread = %d\n", tid);

    //     /* Only master thread does this */
    //     if (tid == 0) 
    //     {
    //         nthreads = omp_get_num_threads();
    //         printf("Number of threads test= %d\n", nthreads);
    //     }
    // }  
    // /* All threads join master thread and disband */
