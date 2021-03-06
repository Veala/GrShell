#ifndef GENERATOR_H
#define GENERATOR_H

//#define debug

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

class generator
{
public:
    generator();
    ~generator();
    static map<string,string> genData;
    static void setSigData(string k,string v);
    void execShell();
    void start();

protected:
    ViChar* resrc = "TCPIP0::localhost::hislip0::INSTR";
    int error;

private:
    ViSession session, vi;
    int openSession();
    void err_handler(ViSession vi, ViStatus err);

    struct state {
        short isOpenSes = 1;
        short isChangeGenP = 1;
        short isChangeSigP = 1;
        short isAborted = 0;
        short isCloseSes = 0;
    }stateGen;

    class signal
    {
    public:
        signal(string &gFreq);
        virtual ~signal() { };
        static map<string,string> sigData;
        void execShell();
        virtual void Calculate();
        void GenerateWaveformCommands(vector<ViByte>& buffer1);
        short *isChangeSigP;
        long long count = 1;
        long long n;
        long long sampleCount;
    protected:
        string ScpiBlockPrefix(size_t blocklen);
        int rangeCheck(double long& Fr);
        virtual short dacValue() = 0;

        string *gF;
        const double long Fr_min = 125E+6;
        const double long Fr_max = 12E+9;
        double long Fr, Fs;
        const long long maxSampleCount = 2E+9;
        const long long maxPortion = 98304;
        long long N, i, offset, counter, minN ;
    private:
        list<string> commands;
    };

    class signal_SIN : public signal
    {
    public:
        signal_SIN(string &gFreq) : signal(gFreq) {}
        ~signal_SIN() { };
        void Calculate();
    protected:
        short dacValue();
    private:
        double long Tr;
    };

    class signal_LFM : public signal
    {
    public:
        signal_LFM(string &gFreq) : signal(gFreq) {}
        ~signal_LFM() { };
        void Calculate();
    protected:
        short dacValue();
    private:
        double long Ts, Tr;
        double long F_min, F_max;
        double long F_0, b;
    };

    class signal_IMP : public signal
    {
    public:
        signal_IMP(string &gFreq) : signal(gFreq) {}
        ~signal_IMP() { };
        void Calculate();
    protected:
        short dacValue();
    private:
        double long Ts;
    };

    void setDefault();
    list<string> commands;
    string gFreq;

public:
    signal *sig;
};

#endif // GENERATOR_H
