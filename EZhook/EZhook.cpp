#include "pch.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include <easyhook.h>
#include <tchar.h>
#include <sstream>
#include <strsafe.h>

#if _WIN64
#pragma comment(lib, "EasyHook64.lib")
#else
#pragma comment(lib, "EasyHook32.lib")
#endif

using namespace std;

BOOL WINAPI WriteFileHook(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
BOOL WINAPI CreateProcessWHook(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL                  bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
void PrintError(LPTSTR lpszFunction);
BOOL WINAPI WriteFileHook(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
    const char* pBuf = "Hooked";
    DWORD buflen = sizeof(pBuf);
    return WriteFile(hFile, pBuf, buflen, lpNumberOfBytesWritten, lpOverlapped);
}

int WriteToFile(LPWSTR filename, const wchar_t* text) {
    HANDLE hFile = CreateFile(filename,                // name of the write
        FILE_APPEND_DATA,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        FILE_APPEND_DATA,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        std::wcout << "Err: CreateFile";
        return 1;
    }

    DWORD bytesWritten;
    bool result = WriteFile(hFile, text, wcslen(text) * sizeof(wchar_t), &bytesWritten, NULL);
    if (!result) {
        std::wcout << "Err: WriteFile";
        CloseHandle(hFile);
        return 1;
    }
    CloseHandle(hFile);
    return 0;
}


BOOL WINAPI CreateProcessWHook(LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCTSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation) {

    LPWSTR filename = L"C:\\users\\david\\CreateProcess.log";
    wstringstream out;
    wstringstream wout;
    wout << "CreateProcessW: " << lpCommandLine << endl;
    wchar_t* outstring = _wcsdup(wout.str().c_str());
    WriteToFile(filename, outstring);

    return CreateProcessW(lpApplicationName,
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
    
    HMODULE hCreateProc = GetModuleHandle(TEXT("kernel32"));
    if (!hCreateProc) {
        PrintError(_T("GetModuleHandle"));
    }

    LPVOID RealCreateProcessW = GetProcAddress(hCreateProc, "CreateProcessW");
    if (RealCreateProcessW == NULL) {
        PrintError(_T("GetProcAddress"));
    }

    std::cout << "CreateProcessW found at address: " << RealCreateProcessW << "...";

    // Install the hook
    NTSTATUS result = LhInstallHook(
        RealCreateProcessW,
        CreateProcessWHook,
        NULL,
        &hHook);
    if (FAILED(result))
    {
        std::wstring s(RtlGetLastErrorString());
        std::wcout << "Failed to install hook: ";
        std::wcout << s << "...";
    }
    else
    {
        std::cout << "Hook 'CreateProcessHook installed successfully...";
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
    //DoWriteFileHooks(inRemoteInfo);
    DoCreateProcessHooks(inRemoteInfo);
    
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