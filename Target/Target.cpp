// Target.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <tchar.h>
#include <windows.h>
#include <strsafe.h>
#include <stdio.h>

int StartProc(LPTSTR procname) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    LPTSTR szCmdline = _tcsdup(procname);
    // Start the child process. 
    if (!CreateProcess(NULL,   // No module name (use command line)
        szCmdline,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return 0;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int WriteFile(LPTSTR filename) {
    HANDLE hFile = CreateFile(filename,                // name of the write
        GENERIC_WRITE,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        OPEN_EXISTING,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        std::wcout << "Err: CreateFile";
        return 1;
    }

    char str[] = "I'm writing to a file!";
    DWORD bytesWritten;
    bool result = WriteFile(hFile, str, strlen(str), &bytesWritten, NULL);
    if (!result) {
        std::wcout << "Err: WriteFile";
        CloseHandle(hFile);
        return 1;
    }
    CloseHandle(hFile);
    return 0;
}

int main()
{
    
    //std::wcout << "Press Enter to do the thing";
    //getchar();
    //WriteFile(_T("C:\\users\\david\\file.txt"));

    //std::wcout << "Done";

    std::wcout << "Press Enter to do the thing";
    getchar();

    StartProc(_T("C:\\windows\\system32\\calc.exe"));

    
}
