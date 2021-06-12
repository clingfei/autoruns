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
void service();
void driver();
void schedTasks();
void KnownDlls();
void ImageHijacks();

vector<string> keys;
int main() {
	keys.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	keys.push_back("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run");
	keys.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
	keys.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
	keys.push_back("SOFTWARE\\Microsoft\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
	string flag;
	cout << "Please input what you want to check: logon, service, driver, task, dll, image, or input exit to exit" << endl; 
	while(true) {
		cin >> flag;
		if (flag == "logon") {
			startup();
			logon();
		}	
		else if (flag == "service") {
			service();
		}
		else if (flag == "driver") {
			driver();
		}
		else if (flag == "task") {
			schedTasks();
		}
		else if (flag == "dll") {
			KnownDlls();
		}
		else if (flag == "image") {
			ImageHijacks();
		}
		else {
			break;
		}
 	}
    //startup();
    //logon();
	//service();
	//driver();
	//schedTasks();
	//KnownDlls();
	//ImageHijacks();
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
        cout << "Name: " << files[i] << endl;
		cout << "ImagePath: " << filePath+files[i] << endl;
		LPCWSTR path = stringToLpcwstr(filePath + files[i]);
		
		string timeStamp = getTimeStamp(path);
		if (timeStamp.size() > 0)
		cout << "TimeStamp: " << timeStamp << endl;
		string publisher = getPublisher(path);
		if (publisher.size() > 0)
		cout << "Publisher: " << publisher << endl;
    }

    sysVar = (LPSTR)malloc(BUFSIZE * sizeof(TCHAR));
    GetEnvironmentVariable("ProgramData", sysVar, BUFSIZE);
	filePath = "";
	filePath = sysVar;
    strcat(sysVar, "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\*");
	filePath += "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\";
	files.clear();
	files = getFiles(sysVar);
    cout << "\nProgramStartup:\n";

    for (int i=0; i<files.size();i++) {
        cout << "Name: " << files[i] << endl;
		cout << "ImagePath: " << (filePath + files[i]) << endl;
		LPCWSTR path = stringToLpcwstr(filePath + files[i]);

		string timeStamp = getTimeStamp(path);
		if (timeStamp.size() > 0)
		cout << "TimeStamp: " << timeStamp << endl;
		string publisher = getPublisher(path);
		if (publisher.size() > 0)
		cout << "Publisher: " << publisher << endl;
    }
	//HKEY hKey;
	//if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Startup"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey))
	//	QueryKey(hKey); 
	//if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Common Startup"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey))
	//	QueryKey(hKey);
	//cout << getDescription(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Startup\\calc.exe"));
}

void logon() {
    HKEY hKey;
	//cout << "CurrentUser: " << endl;
	for (int i=0; i<keys.size(); i++) {
		//cout << "\nHKCU\\" << keys[i] << endl;
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, _T(keys[i].c_str()), 0, KEY_READ | KEY_WOW64_64KEY, &hKey)) {
			QueryLogon(hKey, "\nHKCU\\" + keys[i], keys[i]);
		}
		//else {
		//	cout <<  RegOpenKeyEx(HKEY_CURRENT_USER, keys[i].c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
		//}
		if (hKey != NULL){
			RegCloseKey(hKey);
		}
	}
	
	for (int i=0; i<keys.size(); i++) {
		//cout << "\nHKLM\\" << keys[i] << endl;
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T(keys[i].c_str()), 0, KEY_READ | KEY_WOW64_64KEY, &hKey)) {
			QueryLogon(hKey, "\nHKLM\\"+keys[i], keys[i]);
		}
		//else {
		//	cout <<  RegOpenKeyEx(HKEY_LOCAL_MACHINE, keys[i].c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
		//}
		if (hKey != NULL){
			RegCloseKey(hKey);
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
			//ImagePath = Format(ImagePath);
			Description = Format(Description);

			if (ObjectName == NULL) {
				if ((Description=="NULL") && (ImagePath=="NULL") && (DisplayName == "NULL"))
					continue;
 				ObjectName = (LPBYTE)str.GetBuffer(str.GetLength());	
			}
			int n;
			if ((n = ImagePath.find("svchost.exe")) != string::npos) {
				HKEY svcKey;
				string svcPath = itemKey + "\\Parameters";
				cout << endl;
				cout << "ObjectName: " << ObjectName << "\nDescription: " << Description << "\nImagePath: " << ImagePath << "\nDisplayName: " << DisplayName << endl;  
				string ParameterPath = GetSvchost(rootKey, svcPath.c_str());
				if (ParameterPath != "") {
					//cout << ParameterPath << endl;
					cout << "Parameter: " << ParameterPath << endl;
					ParameterPath = Format(ParameterPath);
					LPCWSTR path = stringToLpcwstr(ParameterPath);
					string timestamp = getTimeStamp(path);
					string publisher = getPublisher(path);
					continue;
				}
			}
			else {
				cout << endl;
				cout << "ObjectName: " << ObjectName << "\nDescription: " << Description << "\nImagePath: " << ImagePath << "\nDisplayName: " << DisplayName << endl;  
			}
			ImagePath = Format(ImagePath);
			LPCWSTR path = stringToLpcwstr(ImagePath);
			string timestamp = getTimeStamp(path);
			string publisher = getPublisher(path);
			//cout << "timestamp: " << timestamp << endl;
			//cout << "publisher: " << publisher << endl;
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
		if ( type <= 8 ) {
			ObjectName = getObjectName(rootKey, itemKey.c_str());
			Description = getDescription(rootKey, itemKey.c_str());
			ImagePath = getImagePath(rootKey, itemKey.c_str());
			DisplayName = getDisplayName(rootKey, itemKey.c_str());
			CString str = "NULL";
			ImagePath = driverVar2Path(ImagePath);
			DisplayName = Format(DisplayName);
			ImagePath = Format(ImagePath);
			Description = Format(Description);

			if (ObjectName == NULL) {
				if ((Description=="NULL") && (ImagePath=="NULL") && (DisplayName == "NULL"))
					continue;
 				ObjectName = (LPBYTE)str.GetBuffer(str.GetLength());	
			}
			cout << endl;
			cout << "ObjectName: " << ObjectName << "\nDescription: " << Description << "\nImagePath: " << ImagePath << "\nDisplayNmae: " << DisplayName << endl;  
			LPCWSTR path = stringToLpcwstr(ImagePath);
			string timestamp = getTimeStamp(path);
			string publisher = getPublisher(path);
			//cout << "timestamp: " << timestamp << endl;
			//cout << "publisher: " << publisher << endl;
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
		//cout << "TimeStamp: " << timestamp << endl;
		string publisher = getPublisher(stringToLpcwstr(iter->second));
		//cout << "publisher: " << publisher << endl; 
		iter++;
		cout << endl;
	}
	// Cleanup
	pTaskFolder->Release();
	pTaskService->Release();
	::SysFreeString(bstrRootFolder);
	::CoUninitialize();


}

void KnownDlls() {
	HKEY hKey = HKEY_LOCAL_MACHINE;

	LPCSTR subKey = "System\\CurrentControlSet\\Control\\Session Manager\\KnownDlls";
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey)) {
		QueryKeys(hKey);
	}
	else cout << "error." << endl;
}

void ImageHijacks() {
	HKEY hKey = HKEY_LOCAL_MACHINE;
	DWORD retCode;
	vector<string> items;
	LPCSTR subKey = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options";
	string itemKey;
	retCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
	if (ERROR_SUCCESS == retCode) {
		items = getItem(hKey);
		//cout << items.size();
		for(int i=0; i<items.size(); ++i) {
			//strcat(subKey, items[i].c_str());
			itemKey = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\" + items[i];
			retCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, itemKey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
		//	cout << itemKey.c_str() << endl;
			if (retCode == ERROR_SUCCESS) {
				QueryImageHijacks(hKey);
			}
			else cout << retCode << endl;
		}
			
	} 
	//else cout << retCode << endl; 
	subKey = "SOFTWARE\\Classes\\htmlfile\\shell\\open\\command";
	retCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
	if (ERROR_SUCCESS == retCode) 
		QueryKeys(hKey);
	else cout << retCode << endl;
}


