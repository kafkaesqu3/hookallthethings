// Injector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdafx.h"
#include <stdio.h>
void PrintError(LPTSTR lpszFunction);
BOOL WINAPI InjectDllInNewProc(__in LPCWSTR lpcwszDll, __in LPCWSTR targetExecutable);
BOOL WINAPI InjectDllInExistingProc(__in LPCWSTR lpcwszDll, __in LPCWSTR targetExecutable);
LPVOID WINAPI InjectDLL(__in HANDLE hProcess, __in LPCWSTR lpcwszDll);
void EnableDebugPriv();

int main()
{
    //we dont need this since its running as the same user
    //EnableDebugPriv();
    wchar_t selfdir[MAX_PATH] = { 0 };
    
    //get our current working directory
    GetModuleFileName(NULL, selfdir, MAX_PATH);
    PathRemoveFileSpec(selfdir);

    

    //get DLL to inject and name of exe to inject into
    std::wstring dllPath = std::wstring(selfdir) + TEXT("\\hooks.dll");
    std::wstring targetPath = std::wstring(selfdir) + TEXT("\\target.exe");

    //start target exe, inject DLL
    //if (InjectDllInNewProc(dllPath.c_str(), targetPath.c_str())) {
    //    printf("Dll was successfully injected.\n");
    //}
    //else {
    //    printf("Terminating the Injector app...");
    //}

    dllPath = std::wstring(selfdir) + TEXT("\\InjectMe2.dll");
    targetPath = TEXT("notepad.exe");

    if (InjectDllInExistingProc(dllPath.c_str(), targetPath.c_str())) {
        printf("Dll was successfully injected.\n");
    }
    else {
        printf("Terminating the Injector app...");
    }
    

    getchar();

    return 0;
}

BOOL WINAPI InjectDllInExistingProc(__in LPCWSTR lpcwszDll, __in LPCWSTR targetExecutable) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    HANDLE hProcess = NULL;
    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_tcsicmp(entry.szExeFile, targetExecutable) == 0)
            {
                hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                    PROCESS_CREATE_THREAD |
                    PROCESS_VM_OPERATION |
                    PROCESS_VM_WRITE, FALSE, entry.th32ProcessID);

                if (!hProcess) {
                    PrintError(_T("OpenProcess"));
                }
                break;
            }
        }
    }
    if (hProcess) {
        LPVOID injectResult = InjectDLL(hProcess, lpcwszDll);
        if (!injectResult) {
            PrintError(_T("Cannot inject"));
        }
    }
    else {
        PrintError(_T("Couldnt find process"));
        CloseHandle(hProcess);
        CloseHandle(snapshot);
        return false;
    }
    CloseHandle(hProcess);
    CloseHandle(snapshot);
    return true;
}

LPVOID WINAPI InjectDLL(__in HANDLE hProcess, __in LPCWSTR lpcwszDll) {
    
    SIZE_T nLength;
    LPVOID lpLoadLibraryW = NULL;
    LPVOID lpRemoteString;

    lpLoadLibraryW = GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"), "LoadLibraryW");

    if (!lpLoadLibraryW)
    {
        PrintError(TEXT("GetProcAddress"));
        return FALSE;
    }
    
    // Calculate the number of bytes needed for the DLL's pathname
    nLength = wcslen(lpcwszDll) * sizeof(WCHAR);

    // allocate mem for dll name
    lpRemoteString = VirtualAllocEx(hProcess, NULL, nLength + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!lpRemoteString)
    {
        PrintError(TEXT("VirtualAllocEx"));

        // close process handle
        CloseHandle(hProcess);

        return FALSE;
    }
    // write dll name
    if (!WriteProcessMemory(hProcess, lpRemoteString, lpcwszDll, nLength+1, NULL)) {

        PrintError(TEXT("WriteProcessMemory"));
        // free allocated memory
        VirtualFreeEx(hProcess, lpRemoteString, 0, MEM_RELEASE);

        // close process handle
        CloseHandle(hProcess);

        return FALSE;
    }
    // create a thread inside the target process that calls loadlibraryw to load the library with the hook 
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibraryW, lpRemoteString, NULL, NULL);

    if (!hThread) {
        PrintError(TEXT("CreateRemoteThread"));
    }
    else {
        WaitForSingleObject(hThread, 4000);

        DWORD dwExitCode = 0;
        GetExitCodeThread(hThread, &dwExitCode);
        if (dwExitCode == 0)
        {
            PrintError(_T("Error: LoadLibraryA failed."));
        }
        else
        {
            OutputDebugString(_T("Success: the remote thread was successfully created.\n"));
        }
        
    }
    //  free allocated memory
    VirtualFreeEx(hProcess, lpRemoteString, 0, MEM_RELEASE);

    return lpRemoteString;
}

BOOL WINAPI InjectDllInNewProc(__in LPCWSTR lpcwszDll, __in LPCWSTR targetPath)
{
    SIZE_T nLength;
    LPVOID lpLoadLibraryW = NULL;
    LPVOID lpRemoteString;
    STARTUPINFO             startupInfo;
    PROCESS_INFORMATION     processInformation;

    //initialize structs
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(STARTUPINFO);

    //start process in suspended mode
    if (!CreateProcess(targetPath, NULL, NULL, NULL, FALSE,
        CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInformation))
    {
        PrintError(_T("CreateProcess"));
        return FALSE;
    }

    lpLoadLibraryW = GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"), "LoadLibraryW");

    if (!lpLoadLibraryW)
    {
        PrintError(TEXT("GetProcAddress"));
        // close process handle
        CloseHandle(processInformation.hProcess);
        return FALSE;
    }

    nLength = wcslen(lpcwszDll) * sizeof(WCHAR);

    // allocate mem for dll name
    lpRemoteString = VirtualAllocEx(processInformation.hProcess, NULL, nLength + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!lpRemoteString)
    {
        PrintError(TEXT("VirtualAllocEx"));

        // close process handle
        CloseHandle(processInformation.hProcess);

        return FALSE;
    }
    // write dll name
    if (!WriteProcessMemory(processInformation.hProcess, lpRemoteString, lpcwszDll, nLength, NULL)) {

        PrintError(TEXT("WriteProcessMemory"));
        // free allocated memory
        VirtualFreeEx(processInformation.hProcess, lpRemoteString, 0, MEM_RELEASE);

        // close process handle
        CloseHandle(processInformation.hProcess);

        return FALSE;
    }
    // create a thread inside the target process that calls loadlibraryw to load the library with the hook 
    HANDLE hThread = CreateRemoteThread(processInformation.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibraryW, lpRemoteString, NULL, NULL);

    if (!hThread) {
        PrintError(TEXT("CreateRemoteThread"));
    }
    else {
        WaitForSingleObject(hThread, 4000);

        //resume suspended process
        ResumeThread(processInformation.hThread);
    }
    //  free allocated memory
    VirtualFreeEx(processInformation.hProcess, lpRemoteString, 0, MEM_RELEASE);

    // close process handle
    CloseHandle(processInformation.hProcess);

    return TRUE;
}

void EnableDebugPriv()
{
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;

    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    bool result = AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);
    if (!result) {
        PrintError(_T("AdjustTokenPrivilege"));
    }

    CloseHandle(hToken);
}

void PrintError(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);

    wprintf(L"%s", lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}