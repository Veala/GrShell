#include <fstream>
#include <generator.h>

using namespace std;

ifstream projFile;
map<string,string> allData;
list<string> commands;

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
            allData[k] = v;
        }
        projFile.close();
    }

    commands.push_back("getAllParam");
    commands.push_back("commands");
    commands.push_back("testType");
    commands.push_back("generator");
    commands.push_back("analyzer");
    commands.push_back("oscilloscope");
    commands.push_back("start");
    commands.push_back("exit");

    generator gen;

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
        } else if (command == "getAllParam") {
            for (std::map<string,string>::iterator it=allData.begin(); it!=allData.end(); ++it)
                cout << "key: " << it->first << "; value: " << it->second << endl;
        } else if (command == "generator") {
            gen.exec();
        } else if (command == "signal") {

        } else if (command == "analyzer") {

        } else if (command == "oscilloscope") {

        } else if (command == "start") {

        }
        cout << "=>";
    }

    return 0;
}
