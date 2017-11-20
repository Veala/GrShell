#include "generator.h"

generator::generator()
{
    generatorData["-gVOLT"] = "5.0E-01";
    generatorData["-gFREQ"] = "160.0E+06";

    commands.push_back("getAllParam");
    commands.push_back("commands");
    commands.push_back("signal");
    commands.push_back("start");
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
        } else if (command == "getAllParam") {
            for (std::map<string,string>::iterator it=generatorData.begin(); it!=generatorData.end(); ++it)
                cout << "key: " << it->first << "; value: " << it->second << endl;
        } else if (command == "signal") {

        } else if (command == "start") {
            start();
        }
        cout << "generator:";
    }
}

void generator::start()
{

}

generator::~generator()
{

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
