#define _USE_MATH_DEFINES
//#define debug

#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

using namespace std;
long long count = 1;
long long n;
long long sampleCount;
double long Ts, Tr;
double long F_min, F_max;
double long F_0, b;

const double long Fr_min = 125E+6;
const double long Fr_max = 12E+9;
double long Fr, Fs;
const long long maxSampleCount = 2E+9;
const long long maxPortion = 98304;
long long N, i, counter, minN;
string *error;
ofstream file;

int rangeCheck(long double& Fr)
{
    if (Fr < Fr_min) Fr = Fr_min;
    if (Fr > Fr_max) Fr = Fr_max;
    if ((Fr >= Fr_min) && (Fr < Fr_max/2)) return 1;
    if ((Fr >= Fr_max/2) && (Fr <= Fr_max)) return 0;
}

namespace LFMsignal {

void Calculate()
{
    //N = firstN;
    Fr = (double long)N*Fs;

    int Chain = rangeCheck(Fr);
    N = (Fr / Fs + Chain);

    if (N<minN) {
        throw new string("Error: Fr/Fs < 4\n");
    } else if (N<384) {
        //тут подумать, над делением на 64 и 48
        sampleCount = N*192;
        Fr=N*Fs;
    } else {
        long long remainder = N % 192;
        long long quotient  = N / 192;
        if (remainder != 0)
            N = (quotient + Chain) * 192;
        sampleCount = N;
        Fr=N*Fs;
    }
    ////////////////////////////////////////
    if (sampleCount > maxSampleCount) throw new string("Error: Sample Count > 2GSa\n");
    long long remainder = sampleCount % maxPortion;
    long long quotient  = sampleCount / maxPortion;
    if (remainder != 0) count = quotient + 1;
    else count = quotient;

    counter=0;
    i=0;
}

}

void Calculate()
{
    F_min = 10E+6; //Hz
    F_max = 800E+6; //Hz
    Ts = 5E-6; //s
    Fs = 1/Ts; //Hz
    F_0 = (F_max + F_min)/2; //Hz
    b = (F_max - F_min)/Ts; //Hz^2

    double long n_12, n_2;
    n_12 = (F_max+F_min)*Ts;
    n_2 = n_12 * (3*F_max + F_min)/(4*F_max + 4*F_min);

    double long t_21 = (-F_0-sqrtl(F_0*F_0 + b*(n_2-1)))/b;
    if (t_21>Ts/2 || t_21<-Ts/2) t_21 = (-F_0+sqrtl(F_0*F_0 + b*(n_2-1)))/b;

    double long t_22 = (-F_0-sqrtl(F_0*F_0 + b*(n_2-2)))/b;
    if (t_22>Ts/2 || t_22<-Ts/2) t_22 = (-F_0+sqrtl(F_0*F_0 + b*(n_2-2)))/b;

#ifdef debug
    cout << "t_21: " << t_21 << endl;
    cout << "t_22: " << t_22 << endl;
#endif

    double long dt = (t_21 - t_22)/4;
    minN = Ts/dt;

#ifdef debug
    cout << "dt1: " << dt << endl;
    cout << "minN: " << minN << endl;
#endif

    dt = (t_21 - t_22)/10;
    N = Ts/dt;

#ifdef debug
    cout << "dt2: " << dt << endl;
    cout << "N...: " << N << endl;
    throw new string("throw End");
#endif

    LFMsignal::Calculate();
    Tr = 1/Fr;
}

short dacValue()
{
    long double t = -Ts/2 + (double long)i*Tr;
    return (short)(2047 * cos(2 * M_PI * (F_0 * t + b/2 * t * t)));
}

void GenerateWaveformCommands()
{
    long long sampleCountCheck = 0;
    short way = 0;
    long long barrier = n*maxPortion;
    do
    {
        for (; i<N; ++i, ++counter)
        {
            if (counter == barrier || counter == sampleCount) { way = 1; break; }
            short dac = dacValue();
            file << counter << ": " << dac << endl;
        }
        if (way==1)
            break;
        i=0;
        sampleCountCheck += N;
    } while (sampleCount != sampleCountCheck);

}

int main()
{
    try {
        Calculate();
    }
    catch (string *error) {
        cout << error->c_str();
        delete error;
        return 0;
    }
    catch (...) {
        cout << "critical error\n";
        return 0;
    }

    cout << "sampleCount: " << sampleCount << endl;

    file.open("a.out");

    for (n=1; n <= count; n++) {
        GenerateWaveformCommands();
    }

    file.close();
    cout << "end" << endl;

    return 0;
}

