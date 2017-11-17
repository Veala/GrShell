#ifndef GENERATOR_H
#define GENERATOR_H

#include <iostream>
#include <sstream>
#include <math.h>
#include <map>
#include <visa.h>
#include <vector>

using namespace std;

class generator
{
public:
    generator();
    map<string,string> generatorData;


protected:

private:
    class signal
    {
    public:
        signal() {}
        map<string,string> signalData;
    protected:
        virtual void GenerateWaveformCommands(int& sampleCount, vector<ViByte>& buffer1) = 0;
        string ScpiBlockPrefix(size_t blocklen);
        void GranularityCheck(int& sampleCount);
    private:

    };

    class signal_SIN : public signal
    {
    public:
        signal_SIN() : signal() {}
    protected:
        void GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1);
    private:

    };

    class signal_LFM : public signal
    {
    public:
        signal_LFM() : signal() {}
    protected:
        void GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1);

    };

    void setDefault();

};

#endif // GENERATOR_H
