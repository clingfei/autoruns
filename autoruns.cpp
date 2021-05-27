#include "StdAfx.h"
#include <io.h>
#include <string>
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <stdio.h>
#include <typeinfo.h>
#include <atlstr.h>
#include <comdef.h>
#include <taskschd.h>
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include <wchar.h>
#include <map>

#include "utils.h"
#include "startup.h"
#include "service.h"

#include "schedTasks.h"

using namespace std;


#pragma comment(lib, "version.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "taskschd.lib")

#define BUFSIZE 100
#define MAX_VALUE_NAME 16383

void startup();
void logon();
void QueryKey(HKEY hKey);
void service();
void driver();
void schedTasks();

vector<string> keys;
int main() {
	keys.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	keys.push_back("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run");
	keys.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
	keys.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
	keys.push_back("SOFTWARE\\Microsoft\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");

    startup();
    logon();
	service();
	driver();
	schedTasks();
	//string test = "C:\\Users\\clf\\Desktop\\VSCodeUserSetup-x64-1.56.0.exe";
	//LPCWSTR path = stringToLpcwstr(test);
	//getTimeStamp(path);
	system("pause");
    
	return 0;
}

void startup() {
    LPTSTR sysVar;
    sysVar = (LPTSTR)malloc(BUFSIZE * sizeof(TCHAR));
    GetEnvironmentVariable("USERPROFILE", sysVar, BUFSIZE);
	string filePath = sysVar;
    strcat(sysVar, "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\*");
	filePath += "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\";

    vector<string> files = getFiles(sysVar);
    cout << "User Startup:\n";

	for (int i=0; i<files.size();i++) {
        cout << files[i] << endl;
		LPCWSTR path = stringToLpcwstr(filePath + files[i]);
		getTimeStamp(path);
    }

    sysVar = (LPSTR)malloc(BUFSIZE * sizeof(TCHAR));
	filePath = sysVar;
    GetEnvironmentVariable("ProgramData", sysVar, BUFSIZE);
    strcat(sysVar, "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\*");
	filePath += "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\";
    files = getFiles(sysVar);
    cout << "ProgramStartup:\n";

    for (int i=0; i<files.size();i++) {
        cout << files[i] << endl;
		LPCWSTR path = stringToLpcwstr(filePath + files[i]);
		getTimeStamp(path);
    }
}

void logon() {
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
				image_path = Format(image_path);
				cout << "value: " << image_path << endl;
				cout << "path: " << image_path << endl;
				LPCWSTR path = stringToLpcwstr(image_path);
				getTimeStamp(path);
				cout << endl;
			}
		}
	}

}


void service(){
	HKEY rootKey = HKEY_LOCAL_MACHINE;
	LPCSTR subKey  = "SYSTEM\\CurrentControlSet\\Services";
	HKEY hKey;
	string itemKey;
	vector<string> items;
	if (ERROR_SUCCESS == RegOpenKeyEx(rootKey, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey)){
		items = getItem(hKey);
	} else {
		cout  << "RegOpenKeyEx Error!" << endl;
	}
	for (int i = 0; i < items.size(); ++i) {
		//cout << "value: " << items[i] << endl; 
		itemKey = "SYSTEM\\CurrentControlSet\\services\\" + items[i];
		DWORD start = getStart(rootKey, itemKey.c_str());
		bool autoruns = false;
		if (start < 3) 
			autoruns = true;
		DWORD type = getType(rootKey, itemKey.c_str());
		
		LPBYTE ObjectName;
		string Description, DisplayName, ImagePath;
		// type<=8 is driver, Service otherwise. 
		if ( type > 8 ) {
			ObjectName = getObjectName(rootKey, itemKey.c_str());
			Description = getDescription(rootKey, itemKey.c_str());
			ImagePath = getImagePath(rootKey, itemKey.c_str());
			DisplayName = getDisplayName(rootKey, itemKey.c_str());
			CString str = "NULL";
			DisplayName = Format(DisplayName);
			ImagePath = Format(ImagePath);
			Description = Format(Description);

			if (ObjectName == NULL) ObjectName = (LPBYTE)str.GetBuffer(str.GetLength());	
			cout << "ObjectName: " << ObjectName << "\tDescription: " << Description << "\tImagePath: " << ImagePath << "\tDisplayNmae: " << DisplayName << endl;  
			LPCWSTR path = stringToLpcwstr(ImagePath);
			string timestamp = getTimeStamp(path);
			string publisher = getPublisher(path);
			cout << "timestamp: " << timestamp << endl;
			cout << "publisher: " << publisher << endl;
		}
	}

}

void driver() {
	HKEY rootKey = HKEY_LOCAL_MACHINE;
	LPCSTR subKey  = "SYSTEM\\CurrentControlSet\\Services";
	HKEY hKey;
	string itemKey;
	vector<string> items;
	if (ERROR_SUCCESS == RegOpenKeyEx(rootKey, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey)){
		items = getItem(hKey);
	} else {
		cout  << "RegOpenKeyEx Error!" << endl;
	}
	for (int i = 0; i < items.size(); ++i) {
		//cout << "value: " << items[i] << endl; 
		itemKey = "SYSTEM\\CurrentControlSet\\services\\" + items[i];
		DWORD start = getStart(rootKey, itemKey.c_str());
		bool autoruns = false;
		if (start < 3) 
			autoruns = true;
		DWORD type = getType(rootKey, itemKey.c_str());
		
		LPBYTE ObjectName;
		string Description, DisplayName, ImagePath;
		// type<=8 is driver, Service otherwise. 
		if ( type <=8 ) {
			ObjectName = getObjectName(rootKey, itemKey.c_str());
			Description = getDescription(rootKey, itemKey.c_str());
			ImagePath = getImagePath(rootKey, itemKey.c_str());
			DisplayName = getDisplayName(rootKey, itemKey.c_str());
			CString str = "NULL";
			//DisplayName = Format(DisplayName);
			ImagePath = Format(ImagePath);
			//Description = Format(Description);

			if (ObjectName == NULL) ObjectName = (LPBYTE)str.GetBuffer(str.GetLength());	
			cout << "ObjectName: " << ObjectName << "\tDescription: " << Description << "\tImagePath: " << ImagePath << "\tDisplayNmae: " << DisplayName << endl;  
			LPCWSTR path = stringToLpcwstr(ImagePath);
			string timestamp = getTimeStamp(path);
			string publisher = getPublisher(path);
			cout << "timestamp: " << timestamp << endl;
			cout << "publisher: " << publisher << endl;
		}
	}
}

void schedTasks() {
	map<string, string> res; 
	ITaskService* pTaskService = NULL;
	ITaskFolder* pTaskFolder = NULL;
	IRegisteredTaskCollection* pTaskCollection = NULL;
	BSTR bstrRootFolder = ::SysAllocString(L"\\");

	HRESULT hr = S_OK;
	ITaskScheduler *pITS;

	hr = ::CoInitialize(NULL);
	if (!SUCCEEDED(hr)) {
		cout << "Error occured when initializing COM" << endl;
		return;
	}
	hr = ::CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
	if (!SUCCEEDED(hr)) {
		cout << "Error occured when initializing security" << endl;
		return;
	}
	
	CreateTaskServiceInstance(pTaskService);
	
	ConnectToTaskService(pTaskService);
	GetRootTaskFolder(pTaskFolder, pTaskService, bstrRootFolder);
	EnumerateTasks(pTaskFolder, 0, res);

 	map<string, string>::iterator iter;
	iter = res.begin();
	while (iter != res.end()) {
		cout << "taskName: " << iter->first << endl;
		iter->second = subVarWithPath(iter->second);
		iter->second = Format(iter->second);
		cout << "taskPath: " << iter->second << endl;
		string timestamp = getTimeStamp(stringToLpcwstr(iter->second));
		cout << timestamp << endl;
		string publisher = getPublisher(stringToLpcwstr(iter->second));
		cout << "publisher: " << publisher << endl; 
		iter++;
	}
	// Cleanup
	pTaskFolder->Release();
	pTaskService->Release();
	::SysFreeString(bstrRootFolder);
	::CoUninitialize();


}