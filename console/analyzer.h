#ifndef ANALYZER_H
#define ANALYZER_H

#include <iostream>
#include <sstream>
#include <math.h>
#include <map>
#include <visa.h>
#include <list>
#include <vector>
#include <algorithm>
#include <windows.h>

using namespace std;

class analyzer
{
public:
    analyzer();
    ~analyzer();
    static map<string,string> anaData;
    void execShell();
    void start();

protected:
    ViChar* resrc = "TCPIP::10.7.15.50::INSTR";
    int error;

private:
    ViSession session, vi;
    int openSession();
    struct state {
        short isOpenSes = 1;
        short isChangeAnaP = 1;
        short isCloseSes = 0;
    }stateAna;

    list<string> commands;
};

#endif // ANALYZER_H
