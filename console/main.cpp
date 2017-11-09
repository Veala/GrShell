#include <iostream>
#include <vector>
#include <fstream>
#include <conio.h>

using namespace std;

string mode = "hand";
string projStr = "none";
vector<string> avalibleParameters;
vector<string> avalibleModes;
string checkedParam, checkedValue;
ifstream projFile;

void checkParameter(string param) {
    checkedParam = "fail";
    for (int i=0; i< avalibleParameters.size(); i++)
        if (avalibleParameters[i] == param) {
            checkedParam = param;
            break;
        }
}

bool checkValue(string value) {
    if (checkedParam == "-m") {
        for (int i=0; i< avalibleModes.size(); i++)
            if (avalibleModes[i] == value) {
                mode = value;
                return true;
            }
        return false;
    } else if (checkedParam == "-p") {
        projFile.open(value.c_str());
        if (!projFile.is_open()) {
            projFile.close();
            return false;
        }
        //code
        projFile.close();
        projStr = value;
        return true;
    }
}

int main(int argc, char* argv[])
{
    avalibleParameters.push_back("-m");
    avalibleParameters.push_back("-p");
    avalibleModes.push_back("hand");
    avalibleModes.push_back("auto");
    int counter=0;
    while (counter < argc-1) {
        counter++;
        checkParameter(string(argv[counter]));
        if (checkedParam == "fail") {
            cout << "Incorrect parameter: " << argv[counter] << endl;
            break;
        }
        counter++;
        if (counter>argc-1) {
            cout << "Not parameter value specified: " << checkedParam.c_str() << endl;
            break;
        }
        if (!checkValue(string(argv[counter]))) {
            cout << "Invalid parameter value: " << checkedParam.c_str() << endl;
            break;
        }
    }

    cout << mode << endl;
    cout << projStr << endl;

    return 0;
}
