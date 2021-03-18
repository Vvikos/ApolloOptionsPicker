#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QDebug>
#include <iostream>
#include <math.h>
#include <omp.h>

#define TILE_SIZE_LIMIT 10000

using namespace std;

typedef vector<vector<int>> Matrix;

int applyOp(int val1, int op, int val2);
QString getStrOp(int op);
vector<int> getSuite(int start, int end, int op, int operand);
QString getSuiteStr(vector<int> s);
bool isSuiteValid(int start, int end, int op, int operand);
int mod(int x, int m);

#endif // TOOLBOX_H
