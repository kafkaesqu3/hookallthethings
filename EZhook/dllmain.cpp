// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

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

