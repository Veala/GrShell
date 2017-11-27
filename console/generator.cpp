#include "generator.h"

generator::generator()
{
    //genData["-gVOLT"] = "5.0E-01";
    //genData["-gFREQ"] = "160.0E+06";

    if (signal::sigData["-sType"] == "LFM")
        sig = new signal_LFM(gFreq);
    if (signal::sigData["-sType"] == "SIN")
        sig = new signal_SIN(gFreq);
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

    // Query the instrument identity
    error = viPrintf(vi, "*IDN?\n");
    ViChar buffer[5000];
    error = viScanf(vi, "%t", buffer);
    cout << "*IDN? -> " << buffer << endl;

    if(stateGen.isChangeSigP || stateGen.isChangeGenP) {
        cout << "abort" << endl;
        error = viPrintf(vi, ":ABORt\n");
        stateGen.isAborted = 1;
    }
//    if(stateGen.isChangeSigP) {
//        //cout << "Reset instrument and setup waveform ... " << endl << endl;
//        error = viPrintf(vi, "*RST\n");
//    }
    if(stateGen.isChangeGenP) {
        cout << "isChangeGenP" << endl;
        cout << genData.find("-gVolt")->second.c_str() << endl;
        //error = viPrintf(vi, ":FREQuency:RASTer %s\n", gFreq.c_str());
        error = viPrintf(vi, ":DAC:VOLTage %s\n", genData.find("-gVolt")->second.c_str());
        error = viPrintf(vi, ":DAC:VOLTage:OFFSet 0\n");
        error = viPrintf(vi, ":OUTPut1:ROUTe DAC\n");
        stateGen.isChangeGenP = 0;
    }
    if(stateGen.isChangeSigP) {
        cout << "isChangeSigP" << endl;
        //Common multiple of 48 and 64;
        int sampleCount = 500 * 192;
        //sampleCount = 1536;

        // Init sampleCount;
        vector<ViByte> buffer1;
        sig->GenerateWaveformCommands(sampleCount, buffer1);

//        cout << "test" << endl;
//        cout << gFreq.c_str() << endl;
//        cout << "sc: " << sampleCount;
        error = viPrintf(vi, ":FREQuency:RASTer %s\n", gFreq.c_str());

        error = viPrintf(vi, ":TRACe1:DELete:ALL\n");
        error = viPrintf(vi, ":TRACe1:DEFine 1,%d\n", sampleCount);

        error = viPrintf(vi, "*OPC?\n");
        error = viScanf(vi, "%t", buffer);

        ViUInt32 writtenCount;
        error = viWrite(vi, &buffer1[0], (ViUInt32)buffer1.size(), &writtenCount);
        error = viFlush(vi, VI_WRITE_BUF);
        // Switch on outputs
        error = viPrintf(vi, ":OUTPut1 on\n");

        // Select segments
        error = viPrintf(vi, ":TRACe1:SELect 1\n");


        stateGen.isChangeSigP = 0;
    }
    if (stateGen.isAborted) {
        cout << "isAborted" << endl;
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
        cout << endl << endl << "Finished." << endl;
        Sleep(5000); // milli seconds
        stateGen.isAborted = 0;
    }
}

int generator::openSession()
{
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
            ;
        Sleep(5000); // milli seconds
        return 1;
    }
    return 0;
}

void generator::setSigData(string k, string v)
{
    signal::sigData[k] = v;
}

generator::~generator()
{
    delete sig;
    if (stateGen.isCloseSes) {
        cout << "~generator() stateGen.isCloseSes" << endl;
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

void generator::signal_SIN::GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1)
{
    //GranularityCheck(sampleCount);

    // Generate a sine wave, ensure granularity (192) and minimum samples (384)

    const long Fs = stod(sigData.find("-sF")->second); //5 or 20MHz in Hz
    int N = 26;
    int Chain;
    long Fr = Fs*N;

    if (Fr < 125000000)  { Fr = 125000000;  Chain = 1; }
    if (Fr > 7500000000) { Fr = 7400000000; Chain = 0; }

    int remainder = Fr % Fs;
    int quotient = Fr / Fs;
    if (remainder != 0) { N=quotient+Chain; Fr=N*Fs;  }
    sampleCount = N*192;
    //cout << "sc: " << sampleCount << endl;

    int sampleCountCheck = 0;
    vector<ViChar> binaryValues;
    double Tr=(double)1/Fr;
    //cout << "Fr: " << Fr << endl;
    //cout << "Tr: " << Tr << endl;
    *gF = to_string(Fr);
    do
    {
        for (int i=0; i<N; ++i)
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

void generator::signal_LFM::GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1)
{
    //GranularityCheck(sampleCount);

    // Generate a LFM wave, ensure granularity (192) and minimum samples (384)

//    cout << "Test" << endl;
    const long long F_min = stod(sigData.find("-sFmin")->second); //Hz
    const long long F_max = stod(sigData.find("-sFmax")->second); //Hz
    const double long Ts = stod(sigData.find("-sT")->second); //s
    const long long Fs = (long long)1/Ts; //Hz
    const long double F_0 = (F_max + F_min)/2; //Hz
    const long double b = (long double)(F_max - F_min)/Ts; //Hz^2

//    cout << "F_min" << F_min << endl;
//    cout << "F_max" << F_max << endl;
//    cout << "Ts " << Ts << endl;
//    cout << "Fs " << Fs << endl;
//    cout << "F_0 " << F_0 << endl;
//    cout << "b " << b << endl;

    long N = 3;
    int Chain=0;
    long long Fr = (long long)F_max*N;

//    cout << "Fr " << Fr << endl;

    if (Fr < 125000000)  { Fr = 125000000;  Chain = 1; }
    if (Fr > 7500000000) { Fr = 7400000000; Chain = 0; }
    int remainder = Fr % Fs;
    int quotient = Fr / Fs;
    if (remainder != 0) { N=quotient+Chain; Fr=N*Fs;  }
    sampleCount = N*192;
    if (quotient > sampleCount) { N=quotient+Chain; Fr=N*Fs;  }
    sampleCount = N*192;
    if (sampleCount > 2E+9) cout << "Sample count > 2Gs";

    int sampleCountCheck = 0;
    vector<ViChar> binaryValues;
    long double Tr=(long double)1/Fr;
    *gF = to_string(Fr);

//    cout << "Tr " << Tr << endl;
//    cout << "N " << N << endl;

    int counter = 0;
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
        counter=0;
        sampleCountCheck += N;
    } while (sampleCount != sampleCountCheck);

    cout << "binaryValues.size(): " <<  binaryValues.size() << endl;
    // Encode the binary commands to download the waveform
    string bytes1 = string(":trac1:data 1,0,") + ScpiBlockPrefix(binaryValues.size());

    buffer1.resize(bytes1.size() + binaryValues.size());
    memcpy(&buffer1[0], bytes1.c_str(), bytes1.size());
    memcpy(&buffer1[bytes1.size()], &binaryValues[0], binaryValues.size());
}
