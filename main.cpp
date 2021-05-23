#include <iostream>
#include <io.h>
#include <string>
#include <windows.h>
#include <tchar.h>
#include <vector>

using namespace std;
#define BUFSIZE 100

void startup();
vector<string> getFiles(string path);

int main() {
    startup();
    return 0;
}

void startup() {
    DWORD dwRet;
    LPTSTR pszOldVal;
    pszOldVal = (LPTSTR)malloc(BUFSIZE * sizeof(TCHAR));
    dwRet = GetEnvironmentVariable("USERPROFILE", pszOldVal, BUFSIZE);
    strcat(pszOldVal, "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\*");

    vector<string> files = getFiles(pszOldVal);
    cout << "User Startup:\n";

    for (auto file :files) {
        cout << file << endl;
    }

    pszOldVal = (LPSTR)malloc(BUFSIZE * sizeof(TCHAR));
    dwRet = GetEnvironmentVariable("ProgramData", pszOldVal, BUFSIZE);
    strcat(pszOldVal, "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\*");
    files = getFiles(pszOldVal);
    cout << "ProgramStartup:\n";

    for (auto file :files) {
        cout << file << endl;
    }
}

vector<string> getFiles(string path) {
    vector<string> files;
    _finddata_t file;
    long lf;
    if ((lf=_findfirst(path.c_str(), &file)) == -1) {
        cout<<path<<" not found!!!"<<endl;
    }
    else {
        while(_findnext(lf, &file) == 0) {
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                continue;
            if (strcmp(file.name, "desktop.ini")!=0)
                files.push_back(file.name);
        }
    }
    _findclose(lf);
    return files;
}
