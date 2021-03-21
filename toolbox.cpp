#include "toolbox.h"

int applyOp(int val1, int op, int val2)
{
    switch (op) {
    case 0:
        return val1 + val2;
        break;

    case 1:
        return val1 - val2;
        break;

    case 2:
        return val1 / val2;
        break;

    case 3:
        return val1 * val2;
        break;

    case 4:
        return pow(val1, val2);
        break;
    }

    return -1;
}

QString getStrOp(int op){
    switch (op) {
    case 0:
        return "+";
        break;

    case 1:
        return "-";
        break;

    case 2:
        return "/";
        break;

    case 3:
        return "*";
        break;

    case 4:
        return "^";
        break;
    }

    return "";
}

vector<int> getSuite(int start, int end, int op, int operand){
    vector<int> l;

    if(!isSuiteValid(start, end, op, operand))
        return l;

    if(start<end){
        for (int j=start; j<=end && j<=TILE_SIZE_LIMIT; j=applyOp(j, op, operand))
            l.push_back(j);
    }else if(start>end){
        for (int j=start; j>=end && j>0; j=applyOp(j, op, operand))
            l.push_back(j);
    }

    return l;
}

QString getSuiteStr(vector<int> s){
    QString res;

    if(s.size() < 2)
        return res;

    bool manual = s[0];
    int size = s[1];

    if(size==0)
        return res;

    vector<int> suite;
    if(manual){
        vector<int>::const_iterator first = s.begin() + 2;
        vector<int>::const_iterator last = s.end();
        vector<int> newVec(first, last);
        suite = newVec;
        if ((int)newVec.size() != size)
            qDebug() << "error not same size";
    }else{
        suite = getSuite(s[2], s[3], s[4], s[5]);
    }

    if(suite.empty())
        return res;

    for(auto it=suite.begin(); it!=suite.end(); ++it){
        if(it==suite.begin())
            res += QString::number(*it);
        else
            res += " - "+QString::number(*it);
    }

    if(manual){
        res = "   Manual     |      " + res;
    }else{
        res = "   [ " + QString::number(s[2]) + " -> " + QString::number(s[3]) + " ]  " + getStrOp(s[4]) + " " + QString::number(s[5]) + "      |      " + res;
    }

    return res;
}

bool isSuiteValid(int start, int end, int op, int operand){
    bool badinput = start<end && (op==1 || op==2);
    badinput = badinput || (start>end && op!=1 && op!=2);
    badinput = badinput || (op==2 && operand<2);
    return !badinput;
}

int mod(int x, int m)
{
    int r = x%m;
    return r<0 ? r+m : r;
}
