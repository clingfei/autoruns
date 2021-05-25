#include <iostream>
#include <vector>
#include <string>

using namespace std;

#define MAX_VALUE_NAME 16383

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

LPBYTE getDescription(HKEY rootKey, LPCSTR subKey) {
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
			CString str = "NULL";
			Description = (LPBYTE)str.GetBuffer(str.GetLength());
		}
	}
	return Description;
}

LPBYTE getImagePath(HKEY rootKey, LPCSTR subKey) {
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
			CString str = "NULL";
			ImagePath = (LPBYTE)str.GetBuffer(str.GetLength());
		}
	}
	return ImagePath;
}

LPBYTE getDisplayName(HKEY rootKey, LPCSTR subKey) {
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
			CString str = "NULL";
			DisplayName = (LPBYTE)str.GetBuffer(str.GetLength());
		}
	}
	return DisplayName;
}