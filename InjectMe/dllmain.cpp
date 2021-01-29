// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "detours.h"
#include <tchar.h>

// Address of the real WriteFile API
BOOL(WINAPI* True_WriteFile)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED) = WriteFile;

// Our intercept function
BOOL WINAPI HookedWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    const char* pBuf = "Hijacked";
    DWORD bufsize = sizeof(pBuf);
    return True_WriteFile(hFile, pBuf, bufsize, lpNumberOfBytesWritten, lpOverlapped);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        MessageBox(NULL, _T("Hello from DllMain"), _T("Msg title"), MB_OK | MB_ICONQUESTION);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)True_WriteFile, HookedWriteFile);

        LONG lError = DetourTransactionCommit();
        if (lError != NO_ERROR) {
            MessageBox(HWND_DESKTOP, L"Failed to detour", L"Error", MB_OK);
            return FALSE;
        }
    }
    break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
    {
        MessageBox(NULL, _T("Goodbye from DllMain"), _T("Msg title"), MB_OK | MB_ICONQUESTION);
        /*DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)True_WriteFile, HookedWriteFile);

        LONG lError = DetourTransactionCommit();
        if (lError != NO_ERROR) {
            MessageBox(HWND_DESKTOP, L"Failed to detour", L"Error", MB_OK);
            return FALSE;
        }*/
    }
    break;
    return TRUE;
    }
}

