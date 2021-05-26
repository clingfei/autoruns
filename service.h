#include <iostream>
#include <vector>
#include <string>

using namespace std;

#define MAX_VALUE_NAME 16383


string getSysVar(string path) {
    LPTSTR sysVar;
    sysVar = (LPTSTR)malloc(BUFSIZE * sizeof(TCHAR));
    GetEnvironmentVariable(path.c_str(), sysVar, BUFSIZE);
	return sysVar;
}

string sysVar2Path(LPBYTE ImagePath, DWORD lpcbData) {
	string path(ImagePath, ImagePath + lpcbData);
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
	if ((n = path.find("\%systemroot%")) != string::npos) {
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
	//cout << path << endl;
	return path;
}

vector<string> getItem(HKEY hKey){
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cSubKeys;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 

	DWORD i, retCode;

	TCHAR achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	retCode = RegQueryInfoKey(
        hKey,                    // key handle 
		NULL,                // buffer for class name 
        NULL,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        NULL,   // security descriptor 
        NULL);       // last write time 

	vector<string> res;

	if (cSubKeys) {
		for (int i=0, retCode = ERROR_SUCCESS; i<cSubKeys; i++) {
			cchValue = MAX_VALUE_NAME;
			achValue[0] = '\0';
			retCode = RegEnumKeyEx(hKey, i,
				achValue,
				&cchValue,
				NULL,
				NULL,
				NULL,
				NULL);

			if (retCode == ERROR_SUCCESS) {
				res.push_back(achValue);
			}
		}
	}
	else {
		cout << "There is no items under service" << endl;
	}
	return res;
}

DWORD getStart(HKEY rootKey, LPCSTR subKey) {
	DWORD lpType = REG_DWORD;
	DWORD start;
	DWORD dwValue;
	HKEY hKey;
	unsigned long retCode;
	retCode = RegOpenKeyEx(rootKey, subKey, 0, KEY_QUERY_VALUE,&hKey);
	if (retCode == ERROR_SUCCESS) 
		retCode = RegQueryValueEx(hKey, "Start", 0, &lpType, (LPBYTE)&start, &dwValue);
	RegCloseKey(hKey);
	return start;
} 

DWORD getType(HKEY rootKey, LPCSTR subKey) {
	//cout << subKey << endl;
	DWORD lpType = REG_DWORD;
	DWORD type;
	DWORD dwValue;
	HKEY hKey;
	unsigned long retCode;
	retCode = RegOpenKeyEx(rootKey, subKey, 0, KEY_QUERY_VALUE, &hKey);
	if (retCode == ERROR_SUCCESS) 
		retCode = RegQueryValueEx(hKey, "Type", 0, &lpType, (LPBYTE)&type, &dwValue);
	RegCloseKey(hKey);
	return type;
		//return type;
}

LPBYTE getObjectName(HKEY rootKey, LPCSTR subKey) {	
	HKEY hKey;
	DWORD lpcbData = 1024;
	DWORD lpType = REG_SZ;
	LPBYTE ObjectName = NULL;
	unsigned long retCode;

	retCode = RegOpenKeyEx(rootKey, subKey, 0, KEY_QUERY_VALUE, &hKey);
	if (retCode == ERROR_SUCCESS) {
		//RegQueryValueEx(hKey, "ObjectName", 0, &lpType, ObjectName, &dwValue);
		ObjectName = (LPBYTE)malloc(lpcbData); 
		retCode = RegQueryValueEx(hKey, "ObjectName", 0, &lpType, ObjectName, &lpcbData);
		if (retCode == ERROR_SUCCESS) {
			//cout << "ObjectName: " << ObjectName << endl;
			RegCloseKey(hKey);
		}
		else {
			CString str = "NULL";
			ObjectName = (LPBYTE)str.GetBuffer(str.GetLength());
		}
	}
	return ObjectName;
}

string getDescription(HKEY rootKey, LPCSTR subKey) {
	HKEY hKey;
	DWORD lpcbData = 1024;
	DWORD lpType = REG_SZ;
	LPBYTE Description = NULL;
	unsigned long retCode;

	retCode = RegOpenKeyEx(rootKey, subKey, 0, KEY_QUERY_VALUE, &hKey);
	if (retCode == ERROR_SUCCESS) {
		//RegQueryValueEx(hKey, "ObjectName", 0, &lpType, ObjectName, &dwValue);
		Description = (LPBYTE)malloc(lpcbData); 
		retCode = RegQueryValueEx(hKey, "Description", 0, &lpType, Description, &lpcbData);
		if (retCode == ERROR_SUCCESS) {
			//cout << "Description: " << Description << endl;
			RegCloseKey(hKey);
		}
		else {
			lpcbData = 4;
			CString str = "NULL";
			Description = (LPBYTE)str.GetBuffer(str.GetLength());
		}
	}
	CString str = "NULL";
	if (Description == NULL ) Description = (LPBYTE)str.GetBuffer(str.GetLength());
	string description = sysVar2Path(Description, lpcbData);
	return description;
}

string getImagePath(HKEY rootKey, LPCSTR subKey) {
	HKEY hKey;
	DWORD lpcbData = 1024;
	DWORD lpType = REG_EXPAND_SZ;
	LPBYTE ImagePath = NULL;
	unsigned long retCode;

	retCode = RegOpenKeyEx(rootKey, subKey, 0, KEY_QUERY_VALUE, &hKey);
	if (retCode == ERROR_SUCCESS) {
		//RegQueryValueEx(hKey, "ObjectName", 0, &lpType, ObjectName, &dwValue);
		ImagePath = (LPBYTE)malloc(lpcbData); 
		retCode = RegQueryValueEx(hKey, "ImagePath", 0, &lpType, ImagePath, &lpcbData);
		if (retCode == ERROR_SUCCESS) {
			//cout << "ImagePath: " << ImagePath << endl;
			RegCloseKey(hKey);
		}
		else {
			lpcbData = 4;
			CString str = "NULL";
			ImagePath = (LPBYTE)str.GetBuffer(str.GetLength());
		}
	}

	//change system variable to path
	CString str = "NULL";
	if (ImagePath == NULL) ImagePath = (LPBYTE)str.GetBuffer(str.GetLength());
	//cout << ImagePath << " " << lpcbData << endl;
	string path = sysVar2Path(ImagePath, lpcbData);

	return path;
}

string getDisplayName(HKEY rootKey, LPCSTR subKey) {
	HKEY hKey;
	DWORD lpcbData = 1024;
	DWORD lpType = REG_SZ;
	LPBYTE DisplayName = NULL;
	unsigned long retCode;

	retCode = RegOpenKeyEx(rootKey, subKey, 0, KEY_QUERY_VALUE, &hKey);
	if (retCode == ERROR_SUCCESS) {
		//RegQueryValueEx(hKey, "ObjectName", 0, &lpType, ObjectName, &dwValue);
		DisplayName = (LPBYTE)malloc(lpcbData); 
		retCode = RegQueryValueEx(hKey, "DisplayName", 0, &lpType, DisplayName, &lpcbData);
		if (retCode == ERROR_SUCCESS) {
			//cout << "ImagePath: " << ImagePath << endl;
			RegCloseKey(hKey);
		}
		else {
			lpcbData = 4;
			CString str = "NULL";
			DisplayName = (LPBYTE)str.GetBuffer(str.GetLength());
		}
	}
	CString str = "NULL";
	if (DisplayName == NULL) DisplayName = (LPBYTE)str.GetBuffer(str.GetLength());
	//cout << ImagePath << " " << lpcbData << endl;
	string name = sysVar2Path(DisplayName, lpcbData);

	return name;
}
