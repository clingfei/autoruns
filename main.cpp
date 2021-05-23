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
    DWORD dwRet;
    LPTSTR pszOldVal;
    pszOldVal = (LPTSTR)malloc(BUFSIZE * sizeof(TCHAR));
    dwRet = GetEnvironmentVariable("USERPROFILE", pszOldVal, BUFSIZE);
    strcat(pszOldVal, "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\*");

    vector<string> files = getFiles(pszOldVal);
    cout << "User Startup:\n";

    for (int i=0; i<files.size();i++) {
        cout << files[i] << endl;
    }

    pszOldVal = (LPSTR)malloc(BUFSIZE * sizeof(TCHAR));
    dwRet = GetEnvironmentVariable("ProgramData", pszOldVal, BUFSIZE);
    strcat(pszOldVal, "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\*");
    files = getFiles(pszOldVal);
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
    cout << "jinru";
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
    cout << "Local Machine:" << endl;
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

                cout << "name:" << achValue << endl;

                string image_path = "";

                for (j = 0; j<cchData; j++) {
                    image_path = image_path + (char)achData[j];
                }
                cout << image_path << endl;

            }
        }


    }

}