#ifndef G_H
#define G_H

#include <cmath>

double g(double x) {
    double ans = 0;
    for (int i = 100; i >= 1; --i) {
        double temp = 0;
        for (int j = i; j >= 1; --j) temp += pow(x + j, -3.1);
        ans += sin(x + temp) / pow(1.2, i);
    }
    return ans;
}


#endif