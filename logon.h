#include <iostream>
#include <string>

using namespace std;

#define BUFSIZE 100

string getEnvVar(string path) {
    LPTSTR sysVar;
    sysVar = (LPTSTR)malloc(BUFSIZE * sizeof(TCHAR));
    GetEnvironmentVariable(path.c_str(), sysVar, BUFSIZE);
	return sysVar;
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
    if ((n = path.find("\%SystemRoot%")) != string::npos) {
		string var = getEnvVar("SystemRoot");
		path = path.substr(12).insert(0, var);
	}
	if ((n = path.find("\%Systemroot%")) != string::npos) {
		string var = getEnvVar("SystemRoot");
		path = path.substr(12).insert(0, var);
	}
    if ((n = path.find("\%USERPROFILE%")) != string::npos) {
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

