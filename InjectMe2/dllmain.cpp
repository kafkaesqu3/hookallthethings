// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <tchar.h>

#include <sstream> //for std::stringstream 
#include <string>  //for std::string
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBox(NULL, _T("Hello from DllMain"), _T("Msg title"), MB_OK | MB_ICONQUESTION);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        const void* address = static_cast<const void*>(lpReserved);
        std::stringstream ss;
        ss << address;
        std::string name = ss.str();
        std::wstring stemp = std::wstring(name.begin(), name.end());
        LPCWSTR sw = stemp.c_str();
        MessageBox(NULL, _T("Goodbye from DllMain"), sw, MB_OK | MB_ICONQUESTION);
        break;
    }
    return TRUE;
}

