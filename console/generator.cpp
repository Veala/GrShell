#include "generator.h"

generator::generator()
{
    //genData["-gVOLT"] = "5.0E-01";
    //genData["-gFREQ"] = "160.0E+06";

    if (signal::sigData["-sType"] == "LFM")
        sig = new signal_LFM(gFreq);
    if (signal::sigData["-sType"] == "SIN")
        sig = new signal_SIN(gFreq);
    if (signal::sigData["-sType"] == "IMP")
        sig = new signal_IMP(gFreq);
    sig->isChangeSigP = &stateGen.isChangeSigP;
    //sig->gF = &gFreq;
    commands.push_back("getAllP");
    commands.push_back("setP");
    commands.push_back("delP");
    commands.push_back("signal");
    commands.push_back("start");
    commands.push_back("commands");
    commands.push_back("exit");
}

void generator::execShell()
{
    string command;
    cout << "generator:";
    while(getline(cin,command)) {
        if (command == "") {
            //cout << "=>";
        } else if ((std::find(commands.begin(), commands.end(), command) == commands.end())) {
            cout << command << " is not command" << endl;
        } else if (command == "commands") {
            for (std::list<string>::iterator it = commands.begin(); it!=commands.end(); ++it)
                cout << *it << endl;
        } else if (command == "exit") {
            //code
            break;
        } else if (command == "getAllP") {
            for (std::map<string,string>::iterator it=genData.begin(); it!=genData.end(); ++it)
                cout << "key: " << it->first << "; value: " << it->second << endl;
        } else if (command == "setP") {
            cout << "key:";
            string key, value; getline(cin,key);
            std::map<string,string>::iterator it = genData.find(key);
            cout << "value:"; getline(cin,value);
            genData[key] = value;
            if (it == genData.end()) cout << "New 'key-value' added" << endl;
            stateGen.isChangeGenP = 1;
        } else if (command == "delP") {
            cout << "key:";
            string key; getline(cin,key);
            std::map<string,string>::iterator it = genData.find(key);
            if (it == genData.end()) { cout << "Key not find" << endl; }
            else { genData.erase(it); cout << "Key was delete" << endl; stateGen.isChangeGenP = 1; }
        } else if (command == "signal") {
            sig->execShell();
        } else if (command == "start") {
            start();
        }
        cout << "generator:";
    }
}

void generator::start()
{
    if(stateGen.isOpenSes)
        if (openSession()) return; else { stateGen.isOpenSes=0; stateGen.isCloseSes=1; }

    ViChar buffer[5000];

    error = viPrintf(vi, "*CLS\n");
    do {
        Sleep(50);
        error = viPrintf(vi, "*OPC?\n");
        error = viScanf(vi, "%t", buffer);
    } while(strcmp(buffer,"1\n") != 0);

    // Query the instrument identity
    error = viPrintf(vi, "*IDN?\n");
    error = viScanf(vi, "%t", buffer);
    cout << "*IDN? -> " << buffer << endl;

    if(stateGen.isChangeSigP || stateGen.isChangeGenP) {

#ifdef debug
        cout << "start() -> generator state: isChangeSigP || isChangeGenP" << endl;
#endif

        error = viPrintf(vi, ":ABORt\n");
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

        stateGen.isAborted = 1;
    }
//    if(stateGen.isChangeSigP) {
//        //cout << "Reset instrument and setup waveform ... " << endl << endl;
//        error = viPrintf(vi, "*RST\n");
//    }
    if(stateGen.isChangeGenP) {

#ifdef debug
        cout << "start() -> generator state: isChangeGenP: -gVolt: " << genData.find("-gVolt")->second.c_str() << endl;
#endif
        //error = viPrintf(vi, ":FREQuency:RASTer %s\n", gFreq.c_str());
        error = viPrintf(vi, ":DAC:VOLTage %s\n", genData.find("-gVolt")->second.c_str());
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

        error = viPrintf(vi, ":DAC:VOLTage:OFFSet 0\n");
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

        error = viPrintf(vi, ":OUTPut1:ROUTe DAC\n");
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

        stateGen.isChangeGenP = 0;
    }
    if(stateGen.isChangeSigP) {

#ifdef debug
        cout << "start() -> generator state: isChangeSigP" << endl;
#endif

        //Common multiple of 48 and 64;
        long long sampleCount=0;

        // Init sampleCount;
        vector<ViByte> buffer1;
        sig->GenerateWaveformCommands(sampleCount, buffer1);
#ifdef debug
        cout << "start() -> gFreq.c_str(): " << gFreq.c_str() << endl;
#endif
        error = viPrintf(vi, ":FREQuency:RASTer %s\n", gFreq.c_str());
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

        error = viPrintf(vi, ":TRACe1:DELete:ALL\n");
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

#ifdef debug
        cout << "start() -> sampleCount: " << (long int)sampleCount << endl;
#endif
        error = viPrintf(vi, ":TRACe1:DEFine 1,%ld\n", (long int)sampleCount);
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

        ViUInt32 writtenCount;
        ViUInt32 NToWrite = (ViUInt32)buffer1.size();
        error = viWrite(vi, &buffer1[0], (ViUInt32)buffer1.size(), &writtenCount);
        while (writtenCount != NToWrite) {
            cout << "writen: " << writtenCount << endl;
            cout << "error: " << error << endl;
            Sleep(50);
        }
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

        error = viFlush(vi, VI_WRITE_BUF);
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

        // Switch on outputs
        error = viPrintf(vi, ":OUTPut1 on\n");

        // Select segments
        error = viPrintf(vi, ":TRACe1:SELect 1\n");

        stateGen.isChangeSigP = 0;
    }
    if (stateGen.isAborted) {

#ifdef debug
        cout << "start() -> generator state: isAborted" << endl;
#endif

        error = viPrintf(vi, ":INITiate:IMMediate\n");
        // Wait until all commands have been executed
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);

        // Capture errors from instrument
        cout << "Check for errors ... " << endl;
        do
        {
            error = viPrintf(vi, ":SYSTem:ERRor?\n");
            error = viScanf(vi, "%t", buffer);
            cout << "  " << buffer;
        }while(strcmp(buffer, "0,\"No error\"\n") != 0);
        cout << endl << endl << "Finished." << endl << endl;
        Sleep(5000); // milli seconds
        stateGen.isAborted = 0;
    }
}

int generator::openSession()
{
    cout << "Open " << resrc << " ... ";
    error = viOpenDefaultRM(&session);
    error = viOpen(session, resrc, VI_NO_LOCK, VI_NULL, &vi);
    if (error != VI_SUCCESS)
    {
        viClose(session);
        cout
            << "failed!" << endl << endl
            << "Make sure that the AgM8190Firmware Application is started: " << endl
            << "  Start Menu -> Keysight M8190 -> Keysight M8190" << endl << endl
            ;
        Sleep(5000); // milli seconds
        return 1;
    }
    error = viSetAttribute(vi, VI_ATTR_TMO_VALUE, (ViAttrState)0x1f40);
    cout << "open: error" << error << endl;
    if (error < VI_SUCCESS) {
        err_handler(vi, error);
        viClose(session);
        cout << "Timeout is too small" << endl << endl;
        Sleep(5000); // milli seconds
        return 1;
    }
    return 0;
}

void generator::err_handler(ViSession vi, ViStatus err)
{
    char err_msg[1024]={0};
    viStatusDesc (vi, err, err_msg);
    printf ("ERROR = %s\n", err_msg);
    return;
}

void generator::setSigData(string k, string v)
{
    signal::sigData[k] = v;
}

generator::~generator()
{
    delete sig;
    if (stateGen.isCloseSes) {
        error = viPrintf(vi, ":ABORt\n");
        error = viPrintf(vi, "*RST\n");
        error = viClose(vi);
        error = viClose(session);
        stateGen.isCloseSes=0;
        stateGen.isOpenSes=1;
    }
}

generator::signal::signal(string &gFreq)
{
    gF=&gFreq; //Hz
    commands.push_back("getAllP");
    commands.push_back("setP");
    commands.push_back("delP");
    commands.push_back("commands");
    commands.push_back("exit");
}

void generator::signal::execShell()
{
    string command;
    cout << "signal:";
    while(getline(cin,command)) {
        if (command == "") {
            //cout << "=>";
        } else if ((std::find(commands.begin(), commands.end(), command) == commands.end())) {
            cout << command << " is not command" << endl;
        } else if (command == "commands") {
            for (std::list<string>::iterator it = commands.begin(); it!=commands.end(); ++it)
                cout << *it << endl;
        } else if (command == "exit") {
            //code
            break;
        } else if (command == "getAllP") {
            for (std::map<string,string>::iterator it=sigData.begin(); it!=sigData.end(); ++it)
                cout << "key: " << it->first << "; value: " << it->second << endl;
        } else if (command == "setP") {
            cout << "key:";
            string key, value; getline(cin,key);
            std::map<string,string>::iterator it = sigData.find(key);
            cout << "value:"; getline(cin,value);
            sigData[key] = value;
            if (it == sigData.end()) cout << "new 'key-value' added" << endl;
            *isChangeSigP = 1;
        } else if (command == "delP") {
            cout << "key:";
            string key; getline(cin,key);
            std::map<string,string>::iterator it = sigData.find(key);
            if (it == sigData.end()) { cout << "Key not find" << endl; }
            else { sigData.erase(it); cout << "Key was delete" << endl; *isChangeSigP = 1; }
        }
        cout << "signal:";
    }
}

string generator::signal::ScpiBlockPrefix(size_t blocklen)
{
    ostringstream blockLenSStr;
    blockLenSStr << blocklen;
    ostringstream resultSStr;
    resultSStr << "#" << blockLenSStr.str().size() << blockLenSStr.str();
    return resultSStr.str();
}

void generator::signal::GranularityCheck(int &sampleCount)
{
    if (sampleCount < 384)
        sampleCount = 384;
    else
    {
        int remainder = sampleCount % 192;
        int quotient = sampleCount/ 192;

        if (remainder != 0)
            sampleCount = (quotient + 1) * 192;
        //cout << sampleCount;
    }
}

void generator::signal_SIN::GenerateWaveformCommands(long long &sampleCount, vector<ViByte> &buffer1)
{
    //GranularityCheck(sampleCount);

    // Generate a sine wave, ensure granularity (192) and minimum samples (384)

    const long long Fs = stod(sigData.find("-sF")->second); //5 or 20MHz in Hz
    long N = 26;
    int Chain;
    long long Fr = (long long)Fs*N;

    if (Fr < 125000000)  { Fr = 125000000;  Chain = 1; }
    if (Fr > 7500000000) { Fr = 7400000000; Chain = 0; }

    long int remainder = Fr % Fs;
    long int quotient = Fr / Fs;
    if (remainder != 0) { N=quotient+Chain; Fr=N*Fs;  }
    sampleCount = (long long)N*192;

#ifdef debug
    cout << "GenerateWaveformCommands() -> sampleCount: " << sampleCount << endl;
#endif

    long long sampleCountCheck = 0;
    vector<ViChar> binaryValues;
    binaryValues.clear();
    long double Tr=(long double)1/Fr;

#ifdef debug
    cout << "GenerateWaveformCommands() -> Fr: " << Fr << endl;
    cout << "GenerateWaveformCommands() -> Tr: " << Tr << endl;
#endif

    *gF = to_string(Fr);
    do
    {
        for (long double i=0; i<N; ++i)
        {
            const short dac = (short)(2047 * sin(2 * M_PI * Fs * i * Tr));
            //cout << dac << endl;
            short value = (short)(dac << 4);
            binaryValues.push_back((ViChar)value);
            binaryValues.push_back((ViChar)(value >> 8));
        }
        sampleCountCheck += N;
    } while (sampleCount != sampleCountCheck);

    // Encode the binary commands to download the waveform
    string bytes1 = string(":trac1:data 1,0,") + ScpiBlockPrefix(binaryValues.size());

    buffer1.resize(bytes1.size() + binaryValues.size());
    memcpy(&buffer1[0], bytes1.c_str(), bytes1.size());
    memcpy(&buffer1[bytes1.size()], &binaryValues[0], binaryValues.size());
}

void generator::signal_LFM::GenerateWaveformCommands(long long &sampleCount, vector<ViByte> &buffer1)
{
    //GranularityCheck(sampleCount);

    // Generate a LFM wave, ensure granularity (192) and minimum samples (384)

    const long long F_min = stod(sigData.find("-sFmin")->second); //Hz
    const long long F_max = stod(sigData.find("-sFmax")->second); //Hz
    const double long Ts = stod(sigData.find("-sT")->second); //s
    const long long Fs = (long long)(1/Ts + 1); //Hz
    const long double F_0 = (F_max + F_min)/2; //Hz
    const long double b = (long double)(F_max - F_min)/Ts; //Hz^2

#ifdef debug
    cout << "GenerateWaveformCommands() -> F_min: " << F_min << endl;
    cout << "GenerateWaveformCommands() -> F_max: " << F_max << endl;
    cout << "GenerateWaveformCommands() -> Ts: " << Ts << endl;
    cout << "GenerateWaveformCommands() -> Fs: " << Fs << endl;
    cout << "GenerateWaveformCommands() -> F_0: " << F_0 << endl;
    cout << "GenerateWaveformCommands() -> b: " << b << endl;
#endif

    long N = 3;
    int Chain=0;
    long long Fr = (long long)F_max*N;

#ifdef debug
    cout << "GenerateWaveformCommands() -> Fr: " << Fr << endl;
#endif

    if (Fr < 125000000)  { Fr = 125000000;  Chain = 1; }
    if (Fr > 7500000000) { Fr = 7400000000; Chain = 0; }
    long int remainder = Fr % Fs;
    long int quotient = Fr / Fs;

#ifdef debug
    cout << "GenerateWaveformCommands() -> Fr: " << Fr << endl;
    cout << "GenerateWaveformCommands() -> remainder: " << remainder << endl;
    cout << "GenerateWaveformCommands() -> quotient: " << quotient << endl;
#endif

    if (remainder != 0) { N=quotient+Chain; Fr=N*Fs;  }

#ifdef debug
    cout << "GenerateWaveformCommands() -> Fr: " << Fr << endl;
#endif

    sampleCount = (long long)N*192;
    if (quotient > sampleCount) { N=quotient+Chain; Fr=N*Fs;  }

#ifdef debug
    cout << "GenerateWaveformCommands() -> Fr: " << Fr << endl;
#endif

    sampleCount = (long long)N*192;

    if (sampleCount > 2E+9) {

#ifdef debug
        cout << "GenerateWaveformCommands(): Sample count > 2Gs" << endl;
#endif

        long X = N / 64;
        X = X / 48;
        N = X * 48 *64;
        Fr=N*Fs;
        sampleCount = (long long)N;
    }

#ifdef debug
    cout << "GenerateWaveformCommands() -> Fr: " << Fr << endl;
#endif

    long long sampleCountCheck = 0;
    vector<ViChar> binaryValues;
    binaryValues.clear();
    long double Tr=(long double)1/Fr;
    *gF = to_string(Fr);

#ifdef debug
    cout << "GenerateWaveformCommands() -> Tr: " << Tr << endl;
    cout << "GenerateWaveformCommands() -> N: " << N << endl;
#endif

    //int counter = 0;
    do {
        //for (long double i = -Ts/2; i<Ts/2 ; i+=Tr)
        for (long double i = 0; i<N ; ++i)
        {
            //const short dac = (short)(2047 * cos(2 * M_PI * (F_0 * i + b/2 * i * i)));
            long double t = -(long double)Ts/2 + i*Tr;
            const short dac = (short)(2047 * cos(2 * M_PI * (F_0 * t + b/2 * t * t)));
            //cout << dac << endl;
            //counter++;
            short value = (short)(dac << 4);
            binaryValues.push_back((ViChar)value);
            binaryValues.push_back((ViChar)(value >> 8));
        }
        //cout << counter << endl;
        //counter=0;
        sampleCountCheck += N;
    } while (sampleCount != sampleCountCheck);

#ifdef debug
    cout << "GenerateWaveformCommands() -> binaryValues.size(): " << binaryValues.size() << endl;
#endif

    // Encode the binary commands to download the waveform
    string bytes1 = string(":trac1:data 1,0,") + ScpiBlockPrefix(binaryValues.size());

    buffer1.resize(bytes1.size() + binaryValues.size());
    memcpy(&buffer1[0], bytes1.c_str(), bytes1.size());
    memcpy(&buffer1[bytes1.size()], &binaryValues[0], binaryValues.size());
}

void generator::signal_IMP::GenerateWaveformCommands(long long &sampleCount, vector<ViByte> &buffer1)
{
    //GranularityCheck(sampleCount);

    // Generate a impulse wave, ensure granularity (192) and minimum samples (384)

    const double long Ts = stod(sigData.find("-sT")->second); //100ms in s
    const double long Fs = (double long)1/Ts;

    long long N = 12;
    double long Fr = (double long)N*Fs;

    int Chain;
    if (Fr < 125000000)  { Fr = 125000000;  Chain = 1; }
    if (Fr > 12000000000) { Fr = 12000000000; Chain = 0; }
    if ((Fr >= 125000000) && (Fr < 6500000000)) Chain = 1;
    if ((Fr >= 6500000000) && (Fr <= 12000000000)) Chain = 0;

    N = (Fr / Fs + Chain);
    sampleCount = N*192;
    ////////////////////////////////////////
    if (sampleCount > 2E+6) {
        long long X = N/64 + Chain;
        X = X/48 + Chain;
        N = X * 48 *64;
        sampleCount = N;
    }
    Fr=N*Fs;

#ifdef debug
    cout << "GenerateWaveformCommands() -> Fs: " << Fs << endl;
    cout << "GenerateWaveformCommands() -> Fr: " << Fr << endl;
    cout << "GenerateWaveformCommands() -> N: " << N << endl;
    cout << "GenerateWaveformCommands() -> sampleCount: " << sampleCount << endl;
#endif

    long long sampleCountCheck = 0;
    vector<ViChar> binaryValues;
    binaryValues.clear();

    *gF = to_string((long long)Fr);
    do
    {
        for (long long i=0; i<N; ++i)
        {
            short sign;
            if (i < N/2) sign = 1; else sign = -1;
            const short dac = (short)(2047 * sign);
            //cout << dac << endl;
            short value = (short)(dac << 4);
            binaryValues.push_back((ViChar)value);
            binaryValues.push_back((ViChar)(value >> 8));
        }
        sampleCountCheck += N;
    } while (sampleCount != sampleCountCheck);

#ifdef debug
    cout << "GenerateWaveformCommands() -> binaryValues.size(): " << binaryValues.size() << endl;
#endif

    // Encode the binary commands to download the waveform
    string bytes1 = string(":trac1:data 1,0,") + ScpiBlockPrefix(binaryValues.size());

    buffer1.resize(bytes1.size() + binaryValues.size());
    memcpy(&buffer1[0], bytes1.c_str(), bytes1.size());
    memcpy(&buffer1[bytes1.size()], &binaryValues[0], binaryValues.size());
}
