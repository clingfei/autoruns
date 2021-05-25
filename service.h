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
				unsigned long j;
				cout << "key:" << achValue << endl;
				res.push_back(achValue);
			}
		}
	}
	else {
		cout << "There is no items under service" << endl;
	}
	return res;
}