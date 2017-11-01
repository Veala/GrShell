//#define my
#ifdef my

#include <iostream>
#include <visa.h>

using namespace std;

int main()
{
    ViSession defaultRM, generator;
    ViChar data[5000] = { 0 };

    viOpenDefaultRM(&defaultRM);
    viOpen(defaultRM, "TCPIP0::localhost::hislip0::INSTR", VI_NO_LOCK, 10000, &generator);
    //viSetAttribute(generator, VI_ATTR_TMO_VALUE, 10000);

    //viUnlock (generator);
    viPrintf(generator, "*IDN?\n");
    cout << viScanf(generator, "%t", data);
    cout << "data: " << data;
    //cout << viRead(generator, (ViPBuf)data, 49, &xxx);

    //viPrintf(generator, "*RST\n");

    cout << viQueryf(generator, ":SYST:VERS?\n", "%t", data);
    cout << "data: " << data;

    cout << viQueryf(generator, ":SYST:LIC:EXT:LIST?\n", "%t", data);
    cout << "data: " << data;

    cout << viQueryf(generator, ":SYST:COMM:SOCK?\n", "%t", data);
    cout << "data: " << data;

    cout << viQueryf(generator, "*OPT?\n", "%t", data);
    cout << "data: " << data;

    cout << viQueryf(generator, ":STATus:QUEStionable:CONNection:ENABle?\n", "%f", data);
    cout << "data: " << data;

    cout << viQueryf(generator, ":SYST:COMM:HISL?\n", "%t", data);
    cout << "data: " << data;

    cout << viPrintf(generator, ":FREQuency:RASTer 8.6E+09\n");
    while (1) {
        viQueryf(generator, "*OPC?\n", "%t", data);
        if ((string)data != "1\n") cout << "OPC " << data;
        else break;
    }
    cout << viQueryf(generator, ":FREQ:RAST?\n", "%t", data);
    cout << "data: " << data;


    viClose(generator);
    viClose(defaultRM);
    return 0;
}

#endif

#ifndef my

#define _USE_MATH_DEFINES
#include <iostream>
#include <math.h>
#include <visa.h>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

using namespace std;

static string ScpiBlockPrefix(size_t blocklen)
{
    ostringstream blockLenSStr;
    blockLenSStr << blocklen;
    ostringstream resultSStr;
    resultSStr << "#" << blockLenSStr.str().size() << blockLenSStr.str();
    //cout << blockLenSStr.str().size() << endl;
    //cout << blockLenSStr.str() << endl;
    return resultSStr.str();
}

static void GenerateWaveformCommands(int& sampleCount, vector<ViByte>& buffer1, ViChar datax[3000])
{
    // Generate a sine wave, ensure granularity (192) and minimum samples (384)
    const int patternLength = 32;

    vector<ViChar> binaryValues;
    int sampleCountCheck = 0;
    do
    {
        for (int i = 0; i < patternLength; ++i)
        {
            const short dac = (short)(2047 * sin(2 * M_PI * i / patternLength));
            short value = (short)(dac << 4);
            //if (i==0) value+=1;
            binaryValues.push_back((ViChar)value);
            binaryValues.push_back((ViChar)(value >> 8));
        }
        sampleCountCheck += patternLength;
    } while (sampleCount != sampleCountCheck);

    // Encode the binary commands to download the waveform    
    string bytes1 = string(":trac1:data 2,0,") + ScpiBlockPrefix(binaryValues.size());

    buffer1.resize(bytes1.size() + binaryValues.size());
    memcpy(&buffer1[0], bytes1.c_str(), bytes1.size());
    memcpy(&buffer1[bytes1.size()], &binaryValues[0], binaryValues.size());
}

static void GenerateWaveformCommandsFM(int& sampleCount, vector<ViByte>& buffer1)
{
    // Generate a sine wave, ensure granularity (192) and minimum samples (384)

    vector<ViChar> binaryValues;
    int sampleCountCheck = 0;
    const float T_rast = 0.000833; //mks
    //const float T_rast = 0.00625; //mks
    const int F_min = 10; //MHz
    const int F_max = 300; //MHz
    const float Ts = T_rast * 384; //mks
    const float F_0 = (F_max + F_min)/2; //MHz
    const float b = (F_max - F_min)/Ts; //MHz^2
    do {
        for (float i = -Ts/2; i<Ts/2 ; i+=T_rast)
        {
            const short dac = (short)(2047 * sin(2 * M_PI * (F_0 * i + b/2 * i * i)));
            //cout << dac << endl;
            short value = (short)(dac << 4);
            //if (i==0) value+=1;
            binaryValues.push_back((ViChar)value);
            binaryValues.push_back((ViChar)(value >> 8));
            //sampleCountCheck+=1;
        }
        sampleCountCheck += 384;
        cout << "sampleCountCheck: " << sampleCountCheck << endl;
        string ciin2;
        cin >> ciin2;
    } while (sampleCount != sampleCountCheck);

    cout << "sampleCountCheck: " << sampleCountCheck << endl;

    // Encode the binary commands to download the waveform
    string bytes1 = string(":trac1:data 1,0,") + ScpiBlockPrefix(binaryValues.size());

    buffer1.resize(bytes1.size() + binaryValues.size());
    memcpy(&buffer1[0], bytes1.c_str(), bytes1.size());
    memcpy(&buffer1[bytes1.size()], &binaryValues[0], binaryValues.size());
}


static void GranularityCheck(int& sampleCount)
{
    if (sampleCount < 384)
        sampleCount = 384;
    else
    {
//        int remainder = sampleCount % 192;
//        int quotient = sampleCount/ 192;

//        if (remainder != 0)
//            sampleCount = (quotient + 1) * 192;
//        cout << sampleCount;
    }
}

int main(int argc, const char* argv[])
{
    char drive[_MAX_DRIVE];
    char dir[_MAX_PATH];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    _splitpath_s(argv[0], drive, dir, fname, ext);

    // Print some information about this example
    cout
        << fname << ext << endl
        << "-----------------------------------------------------" << endl
        << "M8190 example program - connect to an instrument and" << endl
        << "setup and start a simple sine wave signal" << endl
        << endl << endl
        ;

    int error;
    ViSession session, vi;

    ViChar* resrcDefault = "TCPIP0::localhost::hislip0::INSTR";
    ViChar* resrc = resrcDefault;
    if (argc > 1)
        resrc = const_cast<ViChar*>(argv[1]);

    // Open default resource manager and instrument
    cout << "Open " << resrc << " ... ";
    error = viOpenDefaultRM(&session);
    error = viOpen(session, resrc, VI_NO_LOCK, 10000, &vi);
    if (error != VI_SUCCESS)
    {
        viClose(session);
        cout
            << "failed!" << endl << endl
            << "Make sure that the AgM8190Firmware Application is started: " << endl
            << "  Start Menu -> Keysight M8190 -> Keysight M8190" << endl << endl
            << "Start this example with one of the the SCPI Access VISA resource strings " << endl
            << "for HiSLIP or VXI-11 displayed on the AgM8190Firmware application window " << endl
            << "as parameter: " << endl
            << "  e.g. \"" << fname << ext << " TCPIP0::localhost::inst2::INSTR\"" << endl
            ;
        Sleep(5000); // milli seconds
        return 1;
    }

    // Socket connections are not supported, they need extra effort
    // to handle the termination character
    const char* rsrcClassSocket = "SOCKET";
    char rsrcClass[256] = {0};
    error = viGetAttribute(vi, VI_ATTR_RSRC_CLASS, rsrcClass);
    if (strncmp(rsrcClass, rsrcClassSocket, strlen(rsrcClassSocket)) == 0)
    {
        viClose(vi);
        viClose(session);
        cout
            << endl << endl
            << "Error: This example doesn't support socket connections." << endl  << endl
            << "Start this example with one of the the SCPI Access VISA resource strings " << endl
            << "for HiSLIP or VXI-11 displayed on the AgM8190Firmware application window " << endl
            << "as parameter: " << endl
            << "  e.g. \"" << fname << ext << " TCPIP0::localhost::inst2::INSTR\"" << endl
            ;
        Sleep(5000); // milli seconds
        return 1;
    }

    cout << endl << endl;

    // Query the instrument identity

    error = viPrintf(vi, "*IDN?\n");
    ViChar buffer[5000];
    error = viScanf(vi, "%t", buffer);
    cout << "*IDN? -> " << buffer << endl;

    // Reset the instrument
    cout << "Reset instrument and setup waveform ... " << endl << endl;
    error = viPrintf(vi, "*RST\n");

    string ciin;


    //Un-Couple Channel coupling.
    // Note: not available with revision 001 hardware
    //error = viPrintf(vi, ":INSTrument:COUPle:STATe OFF\n");

    //Set the sample rate of Channel 1 to 7.2GHz
    //error = viPrintf(vi, ":FREQuency:RASTer 125.0E+06\n");

          ViChar datax[3000] = { 0 };

//        error = viPrintf(vi, ":FREQ:RAST MIN\n");
//        viPrintf(vi, "*OPC?\n");
//        error = viPrintf(vi, ":FREQ:RAST?\n");
//        error = viScanf(vi, "%t", datax);
//        cout << datax << endl;

//        error = viPrintf(vi, ":FREQuency:RASTer MAX\n");
//        viPrintf(vi, "*OPC?\n");
//        error = viPrintf(vi, ":FREQuency:RASTer?\n");
//        error = viScanf(vi, "%t", datax);
//        cout << datax << endl;

    error = viPrintf(vi, ":FREQuency:RASTer 160.0E+06\n");
    //error = viPrintf(vi, ":FREQuency:RASTer 500.0E+06\n");
    //error = viPrintf(vi, ":FREQuency:RASTer 1200.0E+06\n");
    //cin >> ciin;

    //error = viPrintf(vi, ":FREQuency:RASTer?\n");
    //cin >> ciin;
    //error = viScanf(vi, "%t", datax);
    //cout << datax << endl;


    //Set the Amplitude of Channel 1
    error = viPrintf(vi, ":DAC:VOLTage 5.0E-01\n");

    //Set the Offset of Channel 1
    error = viPrintf(vi, ":DAC:VOLTage:OFFSet 0\n");

    //Select Output Path Channel 1
    error = viPrintf(vi, ":OUTPut1:ROUTe DAC\n");

    //Common multiple of 48 and 64;
    int sampleCount = 500 * 192;
    sampleCount = 1536;
    GranularityCheck(sampleCount);

    // Ensure instrument is stopped
    error = viPrintf(vi, ":ABORt\n");

    // Init sampleCount;
    vector<ViByte> buffer1;
    GenerateWaveformCommandsFM(sampleCount, buffer1);

    // Set up new waveforms, use binary transfer for waveforms
    error = viPrintf(vi, ":TRACe1:DELete:ALL\n");
    error = viPrintf(vi, ":SEQ1:DEL:ALL\n");

    error = viPrintf(vi, ":TRACe1:DEFine 1,%d\n", sampleCount);

    ViUInt32 writtenCount;
    error = viWrite(vi, &buffer1[0], (ViUInt32)buffer1.size(), &writtenCount);
    error = viFlush(vi, VI_WRITE_BUF);


    error = viPrintf(vi, ":TRACe1:DEFine:NEW? %d\n", sampleCount);
    cout << error;
    cin >> ciin;
    error = viScanf(vi, "%t", datax);
    cout << "Yaaa: " << datax << endl;
    cin >> ciin;

    vector<ViByte> buffer2;
    GenerateWaveformCommands(sampleCount, buffer2, datax);

    error = viWrite(vi, &buffer2[0], (ViUInt32)buffer2.size(), &writtenCount);
    cout << error;
    cin >> ciin;
    error = viFlush(vi, VI_WRITE_BUF);
    cout << error;
    cin >> ciin;

    error = viPrintf(vi, ":SEQ1:DEF:NEW? 2\n");
    error = viScanf(vi, "%t", datax);
    cin >> ciin;
    cout << datax << endl;
    cin >> ciin;
    error = viPrintf(vi, ":SEQ1:DATA 0,0,1,1,0,0,0,1535\n");
    error = viPrintf(vi, ":SEQ1:DATA 0,1,2,1,0,0,0,1535\n");
    error = viPrintf(vi, ":SOUR:FUNC1:MODE STS\n");
    error = viPrintf(vi, ":STAB1:SEQ:SEL?\n");
    error = viScanf(vi, "%t", datax);
    cin >> ciin;
    cout << "sel " << datax << endl;
    cin >> ciin;
    error = viPrintf(vi, ":STAB1:SEQ:SEL 0\n");


    // Switch on outputs
    //error = viPrintf(vi, ":OUTPut1 off\n");
    error = viPrintf(vi, ":OUTPut1 on\n");

    // Select segments
    //error = viPrintf(vi, ":TRACe1:SELect 1\n");


    // Finally start instrument, now samples are generated
    cout << "Start sample generation ... " << endl << endl;
    cin >> ciin;
    error = viPrintf(vi, ":INITiate:IMMediate\n");

    // Wait until all commands have been executed
    error = viPrintf(vi, "*OPC?\n");
    error = viScanf(vi, "%t", buffer);
    // Capture errors from instrument
    cout << "Check for errors ... " << endl;
    do
    {
        error = viPrintf(vi, ":SYSTem:ERRor?\n");
        error = viScanf(vi, "%t", buffer);
        cout << "  " << buffer;
    }while(strcmp(buffer, "0,\"No error\"\n") != 0);
    // Close handles

//    Sleep(20000); // milli seconds
//    error = viPrintf(vi, ":DAC:VOLTage 6.0E-01\n");
    //error = viPrintf(vi, ":FREQuency:RASTer 7.0E+09\n");


    error = viClose(vi);
    error = viClose(session);

    cout << endl << endl << "Finished." << endl;
    Sleep(5000); // milli seconds

    return 0;
}

#endif
