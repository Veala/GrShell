#include <fstream>
#include <generator.h>
#include <analyzer.h>

#define debug

using namespace std;

ifstream projFile;
map<string,string> proData;
map<string,string> generator::genData;
map<string,string> generator::signal::sigData;
map<string,string> analyzer::anaData;

//map<string,string> anaData;
//map<string,string> oscData;
list<string> commands;

void start(analyzer& a, generator& g) {
    //a.start();
    g.start();
}

int main(int argc, char* argv[])
{
    if (argc > 1) {
        projFile.open(argv[1]);
        if ( (projFile.rdstate() & std::ifstream::failbit ) != 0 ) {
            cout << "Error opening '" << argv[1] << "'\n";
            return 0;
        }

        string k,v;
        char c;

        while (projFile.get(c)) {
            projFile.unget();
            if (c != '-') {
                getline(projFile, k, '\n');
                continue;
            }
            getline(projFile, k, ' ');
            getline(projFile, v, '\n');
            if (k.at(1) == 'p')
                proData[k] = v;
            else if (k.at(1) == 'g')
                generator::genData[k] = v;
            else if (k.at(1) == 's')
                generator::setSigData(k, v);
            else if (k.at(1) == 'a')
                analyzer::anaData[k] = v;
        }
        projFile.close();
    }

    commands.push_back("getAllP");
    commands.push_back("setP");
    commands.push_back("delP");
    commands.push_back("generator");
    commands.push_back("analyzer");
    commands.push_back("oscilloscope");
    commands.push_back("start");
    commands.push_back("commands");
    commands.push_back("exit");

    generator gen;
    analyzer ana;

    std::map<string,string>::iterator it = proData.find("-pm");
    if (it!=proData.end()) {
#ifdef debug
        cout << "start() -> it!=proData.end()" << endl;
#endif
        if (it->second == "auto") {
#ifdef debug
        cout << "it->second == auto" << endl;
#endif
            start(ana, gen);
        }
    }

    string command;
    cout << "=>";
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
            for (std::map<string,string>::iterator it=proData.begin(); it!=proData.end(); ++it)
                cout << "key: " << it->first << "; value: " << it->second << endl;
        } else if (command == "setP") {
            cout << "key:";
            string key, value; getline(cin,key);
            std::map<string,string>::iterator it = proData.find(key);
            cout << "value:"; getline(cin,value);
            proData[key] = value;
            if (it == proData.end()) cout << "new 'key-value' added" << endl;
        } else if (command == "delP") {
            cout << "key:";
            string key; getline(cin,key);
            std::map<string,string>::iterator it = proData.find(key);
            if (it == proData.end()) { cout << "Key not find" << endl; }
            else { proData.erase(it); cout << "Key was delete" << endl; }
        } else if (command == "generator") {
            gen.execShell();
        } else if (command == "analyzer") {
            ana.execShell();
        } else if (command == "oscilloscope") {

        } else if (command == "start") {
            start(ana, gen);
        }
        cout << "=>";
    }

    return 0;
}
