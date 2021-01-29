#include <stdio.h>
#include <tchar.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files to include
#pragma comment (lib, "Shlwapi.lib")

#include <windows.h>
#include <iostream>
#include <conio.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <strsafe.h>
#include <easyhook.h>
#include <string>
#include <iostream>
#include <sstream>



#if _WIN64
#pragma comment(lib, "EasyHook64.lib")
#else
#pragma comment(lib, "EasyHook32.lib")
#endif


DWORD WINAPI GetProcPID(__in LPCWSTR targetExecutable) {
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
                break;
            }
        }
    }
	return entry.th32ProcessID;
    
}

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD processId = GetProcPID(TEXT("notepad.exe"));
	
	wchar_t selfdir[MAX_PATH] = { 0 };

	//get our current working directory
	GetModuleFileName(NULL, selfdir, MAX_PATH);
	PathRemoveFileSpec(selfdir);



	//get DLL to inject and name of exe to inject into
	std::wstring dllToInject = std::wstring(selfdir) + TEXT("\\EZhook.dll");

	wprintf(L"Attempting to inject: %s\n\n", dllToInject);

	// Inject dllToInject into the target process Id, passing 
	// freqOffset as the pass through data.
	NTSTATUS nt = RhInjectLibrary(
		processId,   // The process to inject into
		0,           // ThreadId to wake up upon injection
		EASYHOOK_INJECT_DEFAULT,
		NULL, // 32-bit
		&dllToInject[0],		 // 64-bit not provided
		NULL, // data to send to injected DLL entry point
		NULL// size of data to send
	);

	if (nt != 0)
	{
		printf("RhInjectLibrary failed with error code = %d\n", nt);
		PWCHAR err = RtlGetLastErrorString();
		std::wcout << err << "\n";
	}
	else
	{
		std::wcout << L"Library injected successfully.\n";
	}

	std::wcout << "Press Enter to exit";
	std::wstring input;
	std::getline(std::wcin, input);
	return 0;
}