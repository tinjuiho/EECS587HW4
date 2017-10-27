#include <iostream>
#include <queue>
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
    double a = 1;
    double b = 100;
    double epsilon = 0.000001;
    double s = 12;
    double M;
    double boundaryValueOfA = g(a);
    double boundaryValueOfB = g(b);
    bool findMAX = false;
    bool* busyArray;
    double (*gPtr)(double) = &g;
    clock_t startTime;
    clock_t endTime;
    queue<Node*> mainQueue;

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

    startTime = clock();
    mainQueue.push(new Node(a, (a + b) / 2));
    mainQueue.push(new Node((a + b) / 2, b));

    Node* curTask = NULL;
    bool deeper = false;
    queue<Node*> subQueue;
    int batchSize;
    while(!mainQueue.empty())
    {
            
        curTask = mainQueue.front();
        mainQueue.pop();
        double lowerBound = curTask->lowerBound;
        double upperBound = curTask->upperBound;
        delete curTask;

        M = max(M, lowerBound, upperBound, gPtr);
        deeper = getDeeper(lowerBound, upperBound, s, gPtr, M, epsilon);

        if(deeper)
        {
            mainQueue.push(new Node(lowerBound, (lowerBound + upperBound) / 2));
            mainQueue.push(new Node((lowerBound + upperBound) / 2, upperBound));
        }
        cout << "queue size: " << mainQueue.size() << endl;
        cout << "Maximum is " << M << endl;
        cout << "execution time: " << (clock() - startTime) / CLOCKS_PER_SEC << " seconds" << endl;
    }

    endTime = clock();
    cout << "queue size: " << mainQueue.size() << endl;
    cout << "Maximum is " << M << endl;
    cout << "execution time: " << (endTime - startTime) / CLOCKS_PER_SEC << " seconds" << endl;
    // delete[] busyArray;
}
