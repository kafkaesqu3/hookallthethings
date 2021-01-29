// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include <easyhook.h>
#include <tchar.h>

#if _WIN64
#pragma comment(lib, "EasyHook64.lib")
#else
#pragma comment(lib, "EasyHook32.lib")
#endif

using namespace std;

BOOL WINAPI WriteFileHook(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);

BOOL WINAPI WriteFileHook(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
    const char* pBuf = "Hooked";
    return WriteFile(hFile, pBuf, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

BOOL DoHooks() {
    HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook
    cout << "\n";
    cout << GetProcAddress(GetModuleHandle(TEXT("kernel32")), "WriteFile");

    // Install the hook
    NTSTATUS result = LhInstallHook(
        GetProcAddress(GetModuleHandle(TEXT("kernel32")), "WriteFile"),
        WriteFileHook,
        NULL,
        &hHook);
    if (FAILED(result))
    {
        wstring s(RtlGetLastErrorString());
        wcout << "Failed to install hook: ";
        wcout << s;
        cout << "\n\nPress any key to exit.";
        cin.get();
        return -1;
    }

    cout << "Activating hook for current thread only.\n";
    // If the threadId in the ACL is set to 0, 
    // then internally EasyHook uses GetCurrentThreadId()
    ULONG ACLEntries[1] = { 0 };
    LhSetInclusiveACL(ACLEntries, 1, &hHook);

    cout << "Press any key to exit.";
    cin.get();

    return 0;
}

extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO * inRemoteInfo);

void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo)
{
    

    std::cout << "Injected by process Id: " << inRemoteInfo->HostPID << "\n";

    // Perform hooking
    HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook
    LPVOID RealWriteFile = GetProcAddress(GetModuleHandle(TEXT("kernel32")), "WriteFile");
    std::cout << "WriteFile found at address: " << RealWriteFile << "\n";

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

    return;
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        ///MessageBox(NULL, _T("Hello from DllMain"), _T("MsgBox"), MB_OK | MB_ICONQUESTION);
        //DoHooks();
    }
        
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH: {
        //MessageBox(NULL, _T("Goodbye from DllMain"), _T("MsgBox"), MB_OK | MB_ICONQUESTION);
    }
        break;
    }
    return TRUE;
}

