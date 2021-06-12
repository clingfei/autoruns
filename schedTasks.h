#pragma once
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <map>

#include <Wincrypt.h>
#include <stdlib.h>
#include <Softpub.h>

#include <vector>

#include <comdef.h>
#include <taskschd.h>

#include <comutil.h>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib,"comsupp.lib")

using namespace std;

void CreateTaskServiceInstance(ITaskService*& pTaskService) {
	HRESULT hResult = ::CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pTaskService);
	if (!SUCCEEDED(hResult))
		cout << "Error while creating TaskService instance" << endl;
}

void ConnectToTaskService(ITaskService*& pTaskService) {
    HRESULT hr;
    
    VARIANT username;
	VARIANT password;
	VARIANT server;
	VARIANT domain;
	::VariantInit(&username);
	::VariantInit(&password);
	::VariantInit(&server);
	::VariantInit(&domain);

	hr = (pTaskService)->Connect(server, username, domain, password);
	if (!SUCCEEDED(hr)) {
		cout << "Error while connecting to the TaskService" << endl;
		return;
	}

	::VariantClear(&username);
	::VariantClear(&password);
	::VariantClear(&server);
	::VariantClear(&domain);	
}

void GetRootTaskFolder(ITaskFolder*& pTaskFolder, ITaskService*& pTaskService, BSTR& bstrRootFolder) {
	HRESULT hResult = pTaskService->GetFolder(bstrRootFolder, &pTaskFolder);
	if (!SUCCEEDED(hResult)) 
		cout << "Error while getting the task root folder" << endl;
}

void EnumerateTasks(ITaskFolder*& pTaskFolder, DWORD indent, map<string, string> &res) {
	//map<string, string> res;

	HRESULT hResult, hResult1;

	BSTR bstrFolderName = NULL;
	pTaskFolder->get_Name(&bstrFolderName);
	// Get tasks in folder
	LONG lTasks = 0;
	IRegisteredTaskCollection* pTaskCollection = NULL;
	pTaskFolder->GetTasks(TASK_ENUM_HIDDEN, &pTaskCollection);
	pTaskCollection->get_Count(&lTasks); 

	// Loop through all tasks
	for (LONG i = 0; i < lTasks; i++) {
		IRegisteredTask* pTask = NULL;
		VARIANT item, actitem;
		::VariantInit(&item);
		item.vt = VT_I4;
		item.lVal = i + 1;
		hResult = pTaskCollection->get_Item(item, &pTask);
		if (SUCCEEDED(hResult)) {
			BSTR bstrTaskName = NULL;
			ITaskDefinition* taskdef = NULL;
			IActionCollection* ppact = NULL;
			IAction* pact = NULL;
			IExecAction* peact = NULL;
			BSTR bstrTaskimg = NULL;
			TASK_STATE pstate;
			//get name
			hResult = pTask->get_Path(&bstrTaskName);
			//get path
			pTask->get_Definition(&taskdef);
			taskdef->get_Actions(&ppact);
			ppact->get_Item(1, &pact);
			pact->QueryInterface(IID_IExecAction, (void**)&peact);
			//get state
			pTask->get_State(&pstate);
			//check state
			bool disabled = (pstate == TASK_STATE_DISABLED);
			bool queued = (pstate == TASK_STATE_QUEUED);
			bool ready = (pstate == TASK_STATE_READY);
			bool running = (pstate == TASK_STATE_RUNNING);
			bool unknown = (pstate == TASK_STATE_UNKNOWN);
			if (ready || running) {
				if (peact != NULL) {
					hResult1 = peact->get_Path(&bstrTaskimg);
					char* lpszText = _com_util::ConvertBSTRToString(bstrTaskimg);
					//format(lpszText);
					bstrTaskimg = _com_util::ConvertStringToBSTR(lpszText);
					if (SUCCEEDED(hResult)) {
						string taskName = (_bstr_t) bstrTaskName;
						//cout << "taskName: " << taskName << endl;
 						if (bstrTaskimg != NULL) {
							string taskPath = (_bstr_t) bstrTaskimg;
							//cout << "taskPath: " << taskPath << endl; 
							//::dic.insert(std::pair<char*, char*>(_com_util::ConvertBSTRToString(bstrTaskName), _com_util::ConvertBSTRToString(bstrTaskimg)));
							res.insert(make_pair(taskName, taskPath));
						}
						
					}
				}
				::SysFreeString(bstrTaskName);
			}
			else
				::SysFreeString(bstrTaskName);
		}
		else {
			::wprintf(L"Error while retriving task %d\n", i + 1);
		}
	}

	// Get all sub folders in current folder
	LONG lTaskFolders = 0;
	ITaskFolderCollection* pNewTaskFolderCollections = NULL;
	pTaskFolder->GetFolders(0, &pNewTaskFolderCollections);
	pNewTaskFolderCollections->get_Count(&lTaskFolders);

	// Loop through all the folders
	for (LONG i = 0; i < lTaskFolders; i++) {
		ITaskFolder* pNewTaskFolder = NULL;
		VARIANT item;
		::VariantInit(&item);
		item.vt = VT_I4;
		item.lVal = i + 1;

		pNewTaskFolderCollections->get_Item(item, &pNewTaskFolder);
		EnumerateTasks(pNewTaskFolder, indent + 3, res);
		pNewTaskFolder->Release();
	}

	pTaskCollection->Release();
}