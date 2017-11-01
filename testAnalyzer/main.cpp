#include <iostream>
#include <visa.h>

using namespace std;

int main()
{
    int Span = 800;     //MHz
    int Freq = Span/2;  //MHz
    int SWT = 10;       //ms
    int RBW = 3;        //MHz
    int VBW = 300;      //kHz
    int Att = 10;       //dB
    int Ref = 60;       //dB
    //int Trace = MAX_HOLD; //MHz

    ViSession defaultRM;
    ViSession analyzer;
    const ViString analyzerString = "TCPIP::10.7.15.50::INSTR";
    const int analyzerTimeout = 1000;
    ViStatus status = viOpenDefaultRM(&defaultRM);
    cout << status << endl;
    status = viOpen(defaultRM, analyzerString, VI_NULL, VI_NULL, &analyzer);
    cout << status << endl;

    ViChar data[5000] = { 0 };
    unsigned long s = 2;
    ViPUInt32 retCnt = &s;

//    status = viWrite(analyzer, (ViBuf)"*IDN?\n", 6, retCnt);
//    cout << status << endl;
//    status = viRead(analyzer, (ViPBuf)data, 500, retCnt);
//    cout << status << endl;
//    cout << data << endl;

    viPrintf(analyzer, "*IDN?\n");
    cout << viScanf(analyzer, "%t", data);
    cout << "data: " << data;

    string str;
    cin >> str;

    status = viWrite(analyzer, (ViBuf)"*CLS\n", 5, retCnt);
    cout << status << endl;
    status = viWrite(analyzer, (ViBuf)"*RST\n", 5, retCnt);
    cout << status << endl;


    status = viWrite(analyzer, (ViBuf)"SYST:DISP:UPD OFF\n", 18, retCnt);
    cout << status << endl;

    cin >> str;

    status = viWrite(analyzer, (ViBuf)"SYST:DISP:UPD ON\n", 17, retCnt);
    cout << status << endl;

    status = viFlush(analyzer, VI_WRITE_BUF);
    cout << "Flush " << status << endl;

    status = viWrite(analyzer, (ViBuf)"SYST:SHUT\n", 10, retCnt);
    viClose(analyzer);
    viClose(defaultRM);
    return 0;
}
