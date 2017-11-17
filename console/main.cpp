#include <iostream>
#include <map>
#include <fstream>

using namespace std;

ifstream projFile;
map<string,string> mapData;

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
            mapData[k] = v;
        }
        projFile.close();
    }

    for (std::map<string,string>::iterator it=mapData.begin(); it!=mapData.end(); ++it)
        cout << "key: " << it->first << "; value: " << it->second << endl;

    return 0;
}
