#include "analyzer.h"

analyzer::analyzer()
{
    commands.push_back("getAllP");
    commands.push_back("setP");
    commands.push_back("delP");
    commands.push_back("start");
    commands.push_back("commands");
    commands.push_back("exit");
}

analyzer::~analyzer()
{
    if (stateAna.isCloseSes) {
        error = viPrintf(vi, "*RST\n");
        error = viClose(vi);
        error = viClose(session);
        stateAna.isCloseSes=0;
        stateAna.isOpenSes=1;
    }
}

void analyzer::execShell()
{
    string command;
    cout << "analyzer:";
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
            for (std::map<string,string>::iterator it=anaData.begin(); it!=anaData.end(); ++it)
                cout << "key: " << it->first << "; value: " << it->second << endl;
        } else if (command == "setP") {
            cout << "key:";
            string key, value; getline(cin,key);
            std::map<string,string>::iterator it = anaData.find(key);
            cout << "value:"; getline(cin,value);
            anaData[key] = value;
            if (it == anaData.end()) cout << "New 'key-value' added" << endl;
            stateAna.isChangeAnaP = 1;
        } else if (command == "delP") {
            cout << "key:";
            string key; getline(cin,key);
            std::map<string,string>::iterator it = anaData.find(key);
            if (it == anaData.end()) { cout << "Key not find" << endl; }
            else { anaData.erase(it); cout << "Key was delete" << endl; stateAna.isChangeAnaP = 1; }
        } else if (command == "start") {
            start();
        }
        cout << "analyzer:";
    }
}

void analyzer::start()
{
    if(stateAna.isOpenSes)
        if (openSession()) return; else { stateAna.isOpenSes=0; stateAna.isCloseSes=1; }

    // Query the instrument identity
    error = viPrintf(vi, "*IDN?\n");
    ViChar buffer[5000];
    error = viScanf(vi, "%t", buffer);
    cout << "*IDN? -> " << buffer << endl;

    if(stateAna.isChangeAnaP) {
        error = viPrintf(vi, ":FREQUENCY:CENTER %s\n", anaData.find("-aFREQ")->second.c_str());
        error = viPrintf(vi, ":FREQUENCY:SPAN %s\n", anaData.find("-aSPAN")->second.c_str());
        error = viPrintf(vi, ":SWE:TIME %s\n", anaData.find("-aSWT")->second.c_str());
        error = viPrintf(vi, ":BAND:RES %s\n", anaData.find("-aRBW")->second.c_str());
        error = viPrintf(vi, ":BAND:VID %s\n", anaData.find("-aVBW")->second.c_str());
        error = viPrintf(vi, ":INP:ATT %s\n", anaData.find("-aAtt")->second.c_str());
        error = viPrintf(vi, ":DISP:TRAC:Y:RLEV %s\n", anaData.find("-aRef")->second.c_str());
        error = viPrintf(vi, ":DISP:TRAC:MODE %s\n", anaData.find("-aTRACE")->second.c_str());
        error = viPrintf(vi, ":SYST:DISP:UPD ON\n");

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
        cout << endl << endl << "Finished." << endl << endl;
        Sleep(5000); // milli seconds
        stateAna.isChangeAnaP = 0;
    }
}

int analyzer::openSession()
{
    cout << "Open " << resrc << " ... ";
    error = viOpenDefaultRM(&session);
    error = viOpen(session, resrc, VI_NULL, VI_NULL, &vi);
    if (error != VI_SUCCESS)
    {
        viClose(session);
        cout
            << "failed!" << endl << endl
            << "Make sure that the R&S FSQ Analyzer is started" << endl << endl
            ;
        Sleep(5000); // milli seconds
        return 1;
    }
    return 0;
}
