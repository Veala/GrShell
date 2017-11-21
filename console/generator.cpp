#include "generator.h"

generator::generator()
{
    //genData["-gVOLT"] = "5.0E-01";
    //genData["-gFREQ"] = "160.0E+06";

    if (signal::sigData["-sType"] == "LFM")
        sig = new signal_LFM();
    if (signal::sigData["-sType"] == "SIN")
        sig = new signal_SIN();
    commands.push_back("getAllP");
    commands.push_back("setP");
    commands.push_back("delP");
    commands.push_back("signal");
    commands.push_back("start");
    commands.push_back("commands");
    commands.push_back("exit");
}

void generator::exec()
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
        } else if (command == "delP") {
            cout << "key:";
            string key; getline(cin,key);
            std::map<string,string>::iterator it = genData.find(key);
            if (it == genData.end()) { cout << "Key not find" << endl; }
            else { genData.erase(it); cout << "Key was delete" << endl; }
        } else if (command == "signal") {
            sig->exec();
        } else if (command == "start") {
            start();
        }
        cout << "generator:";
    }
}

void generator::start()
{
    int error;
    ViSession session, vi;

    ViChar* resrc = "TCPIP0::localhost::hislip0::INSTR";
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
        //Sleep(5000); // milli seconds
        //return 1;
    }

    // Query the instrument identity

    error = viPrintf(vi, "*IDN?\n");
    ViChar buffer[5000];
    error = viScanf(vi, "%t", buffer);
    cout << "*IDN? -> " << buffer << endl;

    // Reset the instrument
    cout << "Reset instrument and setup waveform ... " << endl << endl;
    error = viPrintf(vi, "*RST\n");

    error = viPrintf(vi, ":FREQuency:RASTer %s\n", genData.find("-gFREQ")->second);
    error = viPrintf(vi, ":DAC:VOLTage %s\n", genData.find("-gVOLT")->second);
    error = viPrintf(vi, ":DAC:VOLTage:OFFSet 0\n");
    error = viPrintf(vi, ":OUTPut1:ROUTe DAC\n");

    int sampleCount = 500 * 192;
    sampleCount = 1536;
    sig->GranularityCheck(sampleCount);

    // Ensure instrument is stopped
    error = viPrintf(vi, ":ABORt\n");

    vector<ViByte> buffer1;


}

void generator::setSigData(string k, string v)
{
    signal::sigData[k] = v;
}

generator::~generator()
{
    delete sig;
}

generator::signal::signal()
{
    commands.push_back("getAllP");
    commands.push_back("setP");
    commands.push_back("delP");
    commands.push_back("commands");
    commands.push_back("exit");
}

void generator::signal::exec()
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
        } else if (command == "delP") {
            cout << "key:";
            string key; getline(cin,key);
            std::map<string,string>::iterator it = sigData.find(key);
            if (it == sigData.end()) { cout << "Key not find" << endl; }
            else { sigData.erase(it); cout << "Key was delete" << endl; }
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
//        int remainder = sampleCount % 192;
//        int quotient = sampleCount/ 192;

//        if (remainder != 0)
//            sampleCount = (quotient + 1) * 192;
//        cout << sampleCount;
    }
}

void generator::signal_SIN::GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1)
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

void generator::signal_LFM::GenerateWaveformCommands(int &sampleCount, vector<ViByte> &buffer1)
{
    // Generate a LFM wave, ensure granularity (192) and minimum samples (384)

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
