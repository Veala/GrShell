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

using namespace std;

class generator
{
public:
    generator();
    ~generator();
    static map<string,string> genData;
    static void setSigData(string k,string v);
    void exec();
    void start();

protected:

private:
    class signal
    {
    public:
        signal();
        virtual ~signal() { };
        static map<string,string> sigData;
        void exec();
    protected:
        virtual void GenerateWaveformCommands(int& sampleCount, vector<ViByte>& buffer1) = 0;
        string ScpiBlockPrefix(size_t blocklen);
        void GranularityCheck(int& sampleCount);
    private:
        list<string> commands;

    };

    class signal_SIN : public signal
    {
    public:
        signal_SIN() : signal() {}
        ~signal_SIN() { };
    protected:
        void GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1);
    private:

    };

    class signal_LFM : public signal
    {
    public:
        signal_LFM() : signal() {}
        ~signal_LFM() { };
    protected:
        void GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1);

    };

    void setDefault();
    list<string> commands;

public:
    signal *sig;
};

#endif // GENERATOR_H
