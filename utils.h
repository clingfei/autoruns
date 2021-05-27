#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <map>

#include <Wincrypt.h>
#include <stdlib.h>
#include <Softpub.h>

#include <vector>
#include <string>

#include <comdef.h>
#include <taskschd.h>


#pragma comment(lib, "version.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "taskschd.lib")

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
    if ((n = path.find("\%systemroot%")) != string::npos) {
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

LPTSTR GetCertificateDescription(PCCERT_CONTEXT pCertCtx)
{
	DWORD dwStrType;
	DWORD dwCount;
	LPTSTR szSubjectRDN = NULL;

	dwStrType = CERT_X500_NAME_STR;
	dwCount = CertGetNameString(pCertCtx,
		CERT_NAME_RDN_TYPE,
		0,
		&dwStrType,
		NULL,
		0);
	if (dwCount)
	{
		szSubjectRDN = (LPTSTR)LocalAlloc(0, dwCount * sizeof(TCHAR));
		CertGetNameString(pCertCtx,
			CERT_NAME_RDN_TYPE,
			0,
			&dwStrType,
			szSubjectRDN,
			dwCount);
	}

	return szSubjectRDN;
}


string getTimeStamp(LPCWSTR path) {
    GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;  //Verify a file or object using the Authenticode policy provider.
    WINTRUST_FILE_INFO sWintrustFileInfo;           //reference:https://docs.microsoft.com/en-us/windows/win32/api/wintrust/ns-wintrust-wintrust_file_info
    WINTRUST_DATA sWintrustData;                    //reference:
    LONG hr;
    PVOID OldValue = NULL;

    memset((void*)&sWintrustFileInfo, 0x00, sizeof(WINTRUST_FILE_INFO));
    memset((void*)&sWintrustData, 0x00, sizeof(WINTRUST_DATA));

    sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    sWintrustFileInfo.pcwszFilePath = path;
    sWintrustFileInfo.hFile = NULL;

    sWintrustData.cbStruct = sizeof(WINTRUST_DATA);
    sWintrustData.dwUIChoice = WTD_UI_NONE;    //display no user interface
    sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    sWintrustData.dwUnionChoice = WTD_CHOICE_FILE;             //use the file pointed by pfile
    sWintrustData.pFile = &sWintrustFileInfo;
    sWintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    
    if (Wow64DisableWow64FsRedirection(&OldValue))
        hr = WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);
        Wow64RevertWow64FsRedirection(OldValue);

    // reference: https://docs.microsoft.com/en-us/windows/win32/api/wintrust/nf-wintrust-winverifytrust
    if (hr == TRUST_E_SUBJECT_NOT_TRUSTED) {
        cout << "TRUST_E_SUBJECT_NOT_TRUSTED" << endl;
    }
    else if (hr == TRUST_E_PROVIDER_UNKNOWN) {
        cout << "The trust provider is not recognized on this system" << endl;
    }
    else if (hr == TRUST_E_ACTION_UNKNOWN) {
        cout << "The trust provider does not support the specified action. " << endl;
    }
    else if (hr == TRUST_E_SUBJECT_FORM_UNKNOWN) {
        cout << "The trust provider does not support the form specified for the subject" << endl;
    }
    else if (hr == TRUST_E_NOSIGNATURE) {
        cout << "No signature found on the file" << endl;
    }
    else if (hr == TRUST_E_BAD_DIGEST) {
        cout << "The signature of the file is invalid" << endl;
    }
    else if (hr != S_OK) {
        _tprintf(_T("WinVerifyTrust failed with error 0x%.8X\n"), hr);
    }
    else {
        CRYPT_PROVIDER_DATA const* psProvData = NULL;
        CRYPT_PROVIDER_SGNR* psProvSigner = NULL;
        CRYPT_PROVIDER_CERT* psProvCert = NULL;
        FILETIME localFt;
        SYSTEMTIME sysTime;

        psProvData = WTHelperProvDataFromStateData(sWintrustData.hWVTStateData);
        if (psProvData) {
            psProvSigner = WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0, FALSE, 0);
            if (psProvSigner) {
                FileTimeToLocalFileTime(&psProvSigner->sftVerifyAsOf, &localFt);
                FileTimeToSystemTime(&localFt, &sysTime);
                //cout << "Signature Date = " << sysTime.wDay << " " << sysTime.wMonth 
                //<< " " << sysTime.wYear << " " << sysTime.wHour << " " << sysTime.wMinute << " " << sysTime.wSecond << endl;

                psProvCert = WTHelperGetProvCertFromChain(psProvSigner, 0);
                if (psProvCert) {
                    LPTSTR szCertDesc = GetCertificateDescription(psProvCert->pCert);
                    if (szCertDesc) {
                  //      cout << "File signer = " << szCertDesc << endl;
                        LocalFree(szCertDesc);
                    }
                }  
                
                if (psProvSigner -> csCounterSigners) {
                    FileTimeToLocalFileTime(&psProvSigner->pasCounterSigners[0].sftVerifyAsOf, &localFt);
                    FileTimeToSystemTime(&localFt, &sysTime);
                    char timestamp[1000];
                    //cout << "Signature Date = " << sysTime.wDay << " " << sysTime.wMonth 
                    //    << " " << sysTime.wYear << " " << sysTime.wHour << " " << sysTime.wMinute << " " << sysTime.wSecond << endl;
                    sprintf(timestamp, ("%.2d/%.2d/%.4d at %.2d:%2.d:%.2d\n"), sysTime.wDay, sysTime.wMonth,
                             sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
                    //cout << "timestampe: " << timestamp << endl;
					string res(timestamp, strlen(timestamp));
                    return res;
                }
            }
        }
    }   
    return "";
}

LPCWSTR stringToLpcwstr(string path) {
    size_t size = path.length();
    wchar_t *buffer = new wchar_t[size+1];
    MultiByteToWideChar(CP_ACP, 0, path.c_str(), size, buffer, size * sizeof(wchar_t));
    buffer[size] = 0;
    return buffer;
}

string Format(string str) {
    int i = 0;
    if (str[0] == '\"') {
        i = str.find("\"", 1);
        if (i != string::npos) 
            str = str.substr(1, i-1);
        else 
            str = str.substr(1);
    } 
    i = str.find(" ");
    if (i != string::npos && i+1 < str.length() && str[i+1] == '-')
        str = str.substr(0, i);
    i = str.find(",");
    if (i != string::npos)
        str = str.substr(0, i);
    i = str.find("%");
    if (i != string::npos)
        str = str.replace(i, 1, "");
    return str;
}

string getPublisher(LPCWSTR path) {
    GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;  //Verify a file or object using the Authenticode policy provider.
    WINTRUST_FILE_INFO sWintrustFileInfo;           //reference:https://docs.microsoft.com/en-us/windows/win32/api/wintrust/ns-wintrust-wintrust_file_info
    WINTRUST_DATA sWintrustData;                    //reference:
    LONG hr;
    PVOID OldValue = NULL;

    memset((void*)&sWintrustFileInfo, 0x00, sizeof(WINTRUST_FILE_INFO));
    memset((void*)&sWintrustData, 0x00, sizeof(WINTRUST_DATA));

    sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    sWintrustFileInfo.pcwszFilePath = path;
    sWintrustFileInfo.hFile = NULL;

    sWintrustData.cbStruct = sizeof(WINTRUST_DATA);
    sWintrustData.dwUIChoice = WTD_UI_NONE;    //display no user interface
    sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    sWintrustData.dwUnionChoice = WTD_CHOICE_FILE;             //use the file pointed by pfile
    sWintrustData.pFile = &sWintrustFileInfo;
    sWintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    
    if (Wow64DisableWow64FsRedirection(&OldValue))
        hr = WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);
        Wow64RevertWow64FsRedirection(OldValue);

    // reference: https://docs.microsoft.com/en-us/windows/win32/api/wintrust/nf-wintrust-winverifytrust
    if (hr == TRUST_E_SUBJECT_NOT_TRUSTED) {
        cout << "TRUST_E_SUBJECT_NOT_TRUSTED" << endl;
    }
    else if (hr == TRUST_E_PROVIDER_UNKNOWN) {
        cout << "The trust provider is not recognized on this system" << endl;
    }
    else if (hr == TRUST_E_ACTION_UNKNOWN) {
        cout << "The trust provider does not support the specified action. " << endl;
    }
    else if (hr == TRUST_E_SUBJECT_FORM_UNKNOWN) {
        cout << "The trust provider does not support the form specified for the subject" << endl;
    }
    else if (hr == TRUST_E_NOSIGNATURE) {
        cout << "No signature found on the file" << endl;
    }
    else if (hr == TRUST_E_BAD_DIGEST) {
        cout << "The signature of the file is invalid" << endl;
    }
    else if (hr != S_OK) {
        _tprintf(_T("WinVerifyTrust failed with error 0x%.8X\n"), hr);
    }
    else {
        CRYPT_PROVIDER_DATA const* psProvData = NULL;
        CRYPT_PROVIDER_SGNR* psProvSigner = NULL;
        CRYPT_PROVIDER_CERT* psProvCert = NULL;
        FILETIME localFt;
        SYSTEMTIME sysTime;

        psProvData = WTHelperProvDataFromStateData(sWintrustData.hWVTStateData);
        if (psProvData) {
            psProvSigner = WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0, FALSE, 0);
            if (psProvSigner) {
                FileTimeToLocalFileTime(&psProvSigner->sftVerifyAsOf, &localFt);
                FileTimeToSystemTime(&localFt, &sysTime);
                //cout << "Signature Date = " << sysTime.wDay << " " << sysTime.wMonth 
                //<< " " << sysTime.wYear << " " << sysTime.wHour << " " << sysTime.wMinute << " " << sysTime.wSecond << endl;

                psProvCert = WTHelperGetProvCertFromChain(psProvSigner, 0);
                if (psProvCert) {
                    LPTSTR szCertDesc = GetCertificateDescription(psProvCert->pCert);
                    if (szCertDesc) {
                        //cout << "File signer = " << szCertDesc << endl;
                        //LocalFree(szCertDesc);
                        string res = szCertDesc;
                        LocalFree(szCertDesc);
                        int i = res.find("CN=");
                        string result;
                        for (int j=i+3; j<res.size(); j++) {
                            if (res[j] == ',') 
                                break;
                            result.push_back(res[j]);
                        }
                        return result;
                    }
                }  
            }
        }
    }   
    return "";
}


