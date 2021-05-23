#include "StdAfx.h"
#include <iostream>
#include <io.h>
#include <string>
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <stdio.h>
#include <typeinfo.h>


using namespace std;
#define BUFSIZE 100
#define MAX_VALUE_NAME 16383

void startup();
vector<string> getFiles(string path);
void queryRegKey();
void QueryKey(HKEY hKey);
string getEnvVar(string path);
string subVarWithPath(string path);

vector<string> keys;
int main() {
    keys.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    keys.push_back("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run");
    keys.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
    keys.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
    keys.push_back("SOFTWARE\\Microsoft\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
    startup();
    queryRegKey();
    system("pause");
    return 0;
}

void startup() {
    LPTSTR sysVar;
    sysVar = (LPTSTR)malloc(BUFSIZE * sizeof(TCHAR));
    GetEnvironmentVariable("USERPROFILE", sysVar, BUFSIZE);
    strcat(sysVar, "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\*");

    vector<string> files = getFiles(sysVar);
    cout << "User Startup:\n";

    for (int i=0; i<files.size();i++) {
        cout << files[i] << endl;
    }

    sysVar = (LPSTR)malloc(BUFSIZE * sizeof(TCHAR));
    GetEnvironmentVariable("ProgramData", sysVar, BUFSIZE);
    strcat(sysVar, "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\*");
    files = getFiles(sysVar);
    cout << "ProgramStartup:\n";

    for (int i=0; i<files.size();i++) {
        cout << files[i] << endl;
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

void queryRegKey() {
    HKEY hKey;
    cout << "CurrentUser: " << endl;
    for (int i=0; i<keys.size(); i++) {
        cout << "subkeys:" << keys[i] << endl;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, _T(keys[i].c_str()), 0, KEY_READ | KEY_WOW64_64KEY, &hKey)) {
            QueryKey(hKey);
        }
        //else {
        //	cout <<  RegOpenKeyEx(HKEY_CURRENT_USER, keys[i].c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
        //}
        if (hKey != NULL){
            RegCloseKey(hKey);
        }
    }
    cout << "\nLocal Machine:" << endl;
    for (int i=0; i<keys.size(); i++) {
        cout << "subkeys:" << keys[i] << endl;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T(keys[i].c_str()), 0, KEY_READ | KEY_WOW64_64KEY, &hKey)) {
            QueryKey(hKey);
        }
        //else {
        //	cout <<  RegOpenKeyEx(HKEY_LOCAL_MACHINE, keys[i].c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
        //}
        if (hKey != NULL){
            RegCloseKey(hKey);
        }
    }
}

// reference : https://docs.microsoft.com/en-us/windows/win32/sysinfo/enumerating-registry-subkeys

void QueryKey(HKEY hKey) {
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name
    DWORD    cchClassName = MAX_PATH;  // size of class string
    DWORD    cSubKeys = 0;               // number of subkeys
    DWORD    cbMaxSubKey;              // longest subkey size
    DWORD    cchMaxClass;              // longest class string
    DWORD    cValues;              // number of values for key
    DWORD    cchMaxValue;          // longest value name
    DWORD    cbMaxValueData;       // longest value data
    DWORD    cbSecurityDescriptor; // size of security descriptor
    FILETIME ftLastWriteTime;      // last write time

    DWORD i, retCode;

    TCHAR achValue[MAX_VALUE_NAME];
    BYTE achData[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD cchData = MAX_VALUE_NAME;
    DWORD type;

    // Get the class name and the value count.
    retCode = RegQueryInfoKey(
            hKey,                    // key handle
            achClass,                // buffer for class name
            &cchClassName,           // size of class string
            NULL,                    // reserved
            &cSubKeys,               // number of subkeys
            &cbMaxSubKey,            // longest subkey size
            &cchMaxClass,            // longest class string
            &cValues,                // number of values for this key
            &cchMaxValue,            // longest value name
            &cbMaxValueData,         // longest value data
            &cbSecurityDescriptor,   // security descriptor
            &ftLastWriteTime);       // last write time

    if (cValues) {

        /*
        LSTATUS RegEnumValueA(
  HKEY    hKey,
  DWORD   dwIndex,
  LPSTR   lpValueName,
  LPDWORD lpcchValueName,
  LPDWORD lpReserved,
  LPDWORD lpType,
  LPBYTE  lpData,
  LPDWORD lpcbData
);
        */
        cout << "number of values: " << cValues << endl;
        for (int i=0, retCode = ERROR_SUCCESS; i<cValues; i++) {
            cchValue = MAX_VALUE_NAME;
            cchData = MAX_VALUE_NAME;
            achValue[0] = '\0';
            achData[0] = '\0';
            retCode = RegEnumValue(hKey, i,
                                   achValue,
                                   &cchValue,
                                   NULL,
                                   &type,
                                   achData,
                                   &cchData);

            if (retCode == ERROR_SUCCESS) {
                unsigned long j;

                cout << "key:" << achValue << endl;

                string image_path = "";

                for (j = 0; j<cchData; j++) {
                    image_path = image_path + (char)achData[j];
                }
                image_path = subVarWithPath(image_path);
                cout << "value: " << image_path << endl;
                cout << endl;
            }
        }
    }

}

string subVarWithPath(string path) {
    int n = 0;
    if ((n = path.find("%ProgramFiles%")) != string::npos) {
        string var = getEnvVar("ProgramFiles");
        path = path.substr(14).insert(0, var);
    }
    if ((n = path.find("%PROGRAMFILES%")) != string::npos) {
        string var = getEnvVar("PROGRAMFILES");
        path = path.substr(14).insert(0, var);
    }
    if ((n = path.find("%windir%")) != string::npos) {
        string var = getEnvVar("windir");
        path = path.substr(8).insert(0, var);
    }
    if ((n = path.find("%SystemRoot%")) != string::npos) {
        string var = getEnvVar("SystemRoot");
        path = path.substr(12).insert(0, var);
    }
    if ((n = path.find("%USERPROFILE%")) != string::npos) {
        string var = getEnvVar("USERPROFILE");
        path = path.substr(13).insert(0, var);
    }
    if ((n = path.find("%ProgramData%")) != string::npos) {
        string var = getEnvVar("ProgramData");
        path = path.substr(13).insert(0, var);
    }
    if ((n = path.find("\\")) == string::npos)
        path = path.insert(0, "C:\\Windows\\System32\\");
    return path;
}

string getEnvVar(string path) {
    LPTSTR sysVar;
    sysVar = (LPTSTR)malloc(BUFSIZE * sizeof(TCHAR));
    GetEnvironmentVariable(path.c_str(), sysVar, BUFSIZE);
    return sysVar;
}