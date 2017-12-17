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

//    if (signal::sigData["-sType"] == "")
//        sig = new signal_IMP(gFreq);

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
        //long long sampleCount=0;

        // Init sampleCount;
        vector<ViByte> buffer1;
        try {
            sig->Calculate();
        }
        catch (string *error) {
            cout << error->c_str();
            delete error;
            return;
        }
        catch (...) {
            cout << "critical error\n";
            return;
        }

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
        cout << "start() -> sampleCount: " << (long int)sig->sampleCount << endl;
#endif
        error = viPrintf(vi, ":TRACe1:DEFine 1,%ld\n", (long int)sig->sampleCount);
        do {
            Sleep(50);
            error = viPrintf(vi, "*OPC?\n");
            error = viScanf(vi, "%t", buffer);
        } while(strcmp(buffer,"1\n") != 0);


        ////////////////////////////////////////////////////
        for (sig->n=1; sig->n <= sig->count; sig->n++) {
            buffer1.clear();
            sig->GenerateWaveformCommands(buffer1);
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
        }


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

void generator::signal::Calculate()
{
    Fr = (double long)N*Fs;

    int Chain = rangeCheck(Fr);
    N = (Fr / Fs + Chain);

    if (N<minN) {
        throw new string("Error: Fr/Fs < 4\n");
    } else if (N<384) {
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

#ifdef debug
    cout << "GenerateWaveformCommands() -> Fs: " << Fs << endl;
    cout << "GenerateWaveformCommands() -> Fr: " << Fr << endl;
    cout << "GenerateWaveformCommands() -> N: " << N << endl;
    cout << "GenerateWaveformCommands() -> sampleCount: " << sampleCount << endl;
#endif

    *gF = to_string((long long)Fr);
    offset=0;
    counter=0;
    i=0;
}

void generator::signal::GenerateWaveformCommands(vector<ViByte> &buffer1)
{
    // Generate a sine, LFM, impulse waves, ensure granularity (192) and minimum samples (384)

    long long sampleCountCheck = 0;
    vector<ViChar> binaryValues;
    binaryValues.clear();
    short way = 0;
    long long barrier = n*maxPortion;
    do
    {
        for (; i<N; ++i, ++counter)
        {
            if (counter == barrier || counter == sampleCount) { way = 1; break; }
            const short dac = dacValue();
            short value = (short)(dac << 4);
            binaryValues.push_back((ViChar)value);
            binaryValues.push_back((ViChar)(value >> 8));
        }
        if (way==1)
            break;
        i=0;
        sampleCountCheck += N;
    } while (sampleCount != sampleCountCheck);

    // Encode the binary commands to download the waveform
#ifdef debug
    cout << "offset: " << offset << endl;
#endif
    string bytes1 = string(":trac1:data 1,") + to_string(offset) + string(",") + ScpiBlockPrefix(binaryValues.size());
    offset+=binaryValues.size()/2;
#ifdef debug
    cout << "n: " << n << endl;
    cout << "offset: " << offset << endl;
    cout << "binaryValues.size(): " << binaryValues.size()  << endl;
#endif
    buffer1.resize(bytes1.size() + binaryValues.size());
    memcpy(&buffer1[0], bytes1.c_str(), bytes1.size());
    memcpy(&buffer1[bytes1.size()], &binaryValues[0], binaryValues.size());
}

string generator::signal::ScpiBlockPrefix(size_t blocklen)
{
    ostringstream blockLenSStr;
    blockLenSStr << blocklen;
    ostringstream resultSStr;
    resultSStr << "#" << blockLenSStr.str().size() << blockLenSStr.str();
    return resultSStr.str();
}

int generator::signal::rangeCheck(long double& Fr)
{
    if (Fr < Fr_min) Fr = Fr_min;
    if (Fr > Fr_max) Fr = Fr_max;
    if ((Fr >= Fr_min) && (Fr < Fr_max/2)) return 1;
    if ((Fr >= Fr_max/2) && (Fr <= Fr_max)) return 0;
}

void generator::signal_SIN::Calculate()
{
    Fs = stold(sigData.find("-sF")->second); //in Hz;
    minN=4; N=384;
    signal::Calculate();
    Tr = 1/Fr;
}

short generator::signal_SIN::dacValue()
{
    return (short)(2047 * sin(2 * M_PI * Fs * i * Tr));
}

void generator::signal_LFM::Calculate()
{
    F_min = stod(sigData.find("-sFmin")->second); //Hz
    F_max = stod(sigData.find("-sFmax")->second); //Hz
    Ts = stod(sigData.find("-sT")->second); //s
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

    double long dt = (t_21 - t_22)/4;
    minN = Ts/dt;
    dt = (t_21 - t_22)/10;
    N = Ts/dt;

#ifdef debug
    cout << "N...: " << N << endl;
    cout << "Fs: " << Fs << endl;
#endif

    signal::Calculate();
    Tr = 1/Fr;

#ifdef debug
    cout << "GenerateWaveformCommands() -> F_min: " << F_min << endl;
    cout << "GenerateWaveformCommands() -> F_max: " << F_max << endl;
    cout << "GenerateWaveformCommands() -> Ts: " << Ts << endl;
    cout << "GenerateWaveformCommands() -> Fs: " << Fs << endl;
    cout << "GenerateWaveformCommands() -> F_0: " << F_0 << endl;
    cout << "GenerateWaveformCommands() -> b: " << b << endl;
#endif
}

short generator::signal_LFM::dacValue()
{
    long double t = -Ts/2 + (double long)i*Tr;
    return (short)(2047 * cos(2 * M_PI * (F_0 * t + b/2 * t * t)));
}

void generator::signal_IMP::Calculate()
{
    Ts = stold(sigData.find("-sT")->second); //in s
    Fs = 1/Ts;
    minN=4; N=384;
    signal::Calculate();
}

short generator::signal_IMP::dacValue()
{
    short sign;
    if (i < N/2) sign = 1; else sign = -1;
    return sign;
}
