#ifndef GENERATOR_H
#define GENERATOR_H

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
        virtual void GenerateWaveformCommands(int& sampleCount, vector<ViByte>& buffer1) = 0;
        short *isChangeSigP;
    protected:
        string ScpiBlockPrefix(size_t blocklen);
        void GranularityCheck(int& sampleCount);
    private:
        list<string> commands;
        string *gF;
    };

    class signal_SIN : public signal
    {
    public:
        signal_SIN(string gFreq) : signal(gFreq) {}
        ~signal_SIN() { };
        void GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1);
    };

    class signal_LFM : public signal
    {
    public:
        signal_LFM(string gFreq) : signal(gFreq) {}
        ~signal_LFM() { };
        void GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1);
    };

    void setDefault();
    list<string> commands;
    string gFreq;

public:
    signal *sig;
};

#endif // GENERATOR_H
