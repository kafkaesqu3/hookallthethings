#include "pch.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include <easyhook.h>
#include <tchar.h>
#include <sstream>

#if _WIN64
#pragma comment(lib, "EasyHook64.lib")
#else
#pragma comment(lib, "EasyHook32.lib")
#endif

using namespace std;

BOOL WINAPI WriteFileHook(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
BOOL WINAPI CreateProcessHook(LPCTSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL                  bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
BOOL WINAPI WriteFileHook(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
    const char* pBuf = "Hooked";
    DWORD buflen = sizeof(pBuf);
    return WriteFile(hFile, pBuf, buflen, lpNumberOfBytesWritten, lpOverlapped);
}

int WriteToFile(LPTSTR filename, const char* text) {
    HANDLE hFile = CreateFile(filename,                // name of the write
        GENERIC_WRITE,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        OPEN_EXISTING | FILE_APPEND_DATA,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        std::wcout << "Err: CreateFile";
        return 1;
    }

    DWORD bytesWritten;
    bool result = WriteFile(hFile, text, strlen(text), &bytesWritten, NULL);
    if (!result) {
        std::wcout << "Err: WriteFile";
        CloseHandle(hFile);
        return 1;
    }
    CloseHandle(hFile);
    return 0;
}


BOOL WINAPI CreateProcessHook(LPCTSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCTSTR lpCurrentDirectory,
    LPSTARTUPINFO lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation) {

    LPTSTR filename = _T("C:\\users\\david\\CreateProcess.log");
    ostringstream out;
    out << "CreateProcess: " << lpApplicationName << ", " << lpCommandLine << "...";
    WriteToFile(filename, out.str().c_str());

    return CreateProcessHook(lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation
    );

}

BOOL DoWriteFileHooks(REMOTE_ENTRY_INFO* inRemoteInfo) {
    std::cout << "Injected by process Id: " << inRemoteInfo->HostPID << "...";

    // Perform hooking
    HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook
    LPVOID RealWriteFile = GetProcAddress(GetModuleHandle(TEXT("kernel32")), "WriteFile");
    std::cout << "WriteFile found at address: " << RealWriteFile << "...";

    // Install the hook
    NTSTATUS result = LhInstallHook(
        RealWriteFile,
        WriteFileHook,
        NULL,
        &hHook);
    if (FAILED(result))
    {
        std::wstring s(RtlGetLastErrorString());
        std::wcout << "Failed to install hook: ";
        std::wcout << s;
    }
    else
    {
        std::cout << "Hook 'WriteFileHook installed successfully.";
    }

    // If the threadId in the ACL is set to 0,
    // then internally EasyHook uses GetCurrentThreadId()
    ULONG ACLEntries[1] = { 0 };

    // Disable the hook for the provided threadIds, enable for all others
    LhSetExclusiveACL(ACLEntries, 1, &hHook);

    return true;
}

BOOL DoCreateProcessHooks(REMOTE_ENTRY_INFO* inRemoteInfo) {
    std::cout << "Injected by process Id: " << inRemoteInfo->HostPID << "...";

    // Perform hooking
    HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook
    LPVOID RealCreateProcess = GetProcAddress(GetModuleHandle(TEXT("kernel32")), "CreateProcess");
    std::cout << "CreateProcess found at address: " << RealCreateProcess << "...";

    // Install the hook
    NTSTATUS result = LhInstallHook(
        RealCreateProcess,
        CreateProcessHook,
        NULL,
        &hHook);
    if (FAILED(result))
    {
        std::wstring s(RtlGetLastErrorString());
        std::wcout << "Failed to install hook: ";
        std::wcout << s;
    }
    else
    {
        std::cout << "Hook 'CreateProcessHook installed successfully.";
    }

    // If the threadId in the ACL is set to 0,
    // then internally EasyHook uses GetCurrentThreadId()
    ULONG ACLEntries[1] = { 0 };

    // Disable the hook for the provided threadIds, enable for all others
    LhSetExclusiveACL(ACLEntries, 1, &hHook);

    return true;
}

extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO * inRemoteInfo);

void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo)
{
    DoWriteFileHooks(inRemoteInfo);

    
}