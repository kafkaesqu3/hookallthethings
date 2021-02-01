// Target.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <tchar.h>

#include <strsafe.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
using namespace std;

DWORD MyThreadFunction(LPVOID lpParam);
int StartProc(LPWSTR procname);
int WriteFile(LPTSTR filename);
int WriteToFile(LPWSTR filename, const wchar_t* text);
void PrintError(LPTSTR lpszFunction);

int main()
{



    //start a thread to do nothing
    LPDWORD dwThreadId;
    //HANDLE hThread = CreateThread(NULL, 0, MyThreadFunction, NULL, 0, dwThreadId);
    
    
    /*std::wcout << "Press Enter to write to the file";
    getchar();
    WriteFile(_T("C:\\users\\david\\file.txt"));

    std::wcout << "Done";*/

    //WaitForSingleObject(hThread, INFINITE);
    std::wcout << "Press Enter to start the process";
    getchar();

    StartProc(L"C:\\windows\\system32\\calc.exe");

    std::wcout << "Done";
}


int WriteToFile(LPWSTR filename, const wchar_t* text) {
    HANDLE hFile = CreateFileW(filename,                // name of the write
        FILE_APPEND_DATA,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        FILE_APPEND_DATA,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        PrintError(TEXT("CreateFile"));
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


int StartProc(LPWSTR procname) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    LPTSTR szCmdline = _wcsdup(procname);

    // Start the child process. 
    if (!CreateProcessW(NULL,   // No module name (use command line)
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
        printf("CreateProcessW failed (%d).\n", GetLastError());
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

DWORD MyThreadFunction(LPVOID lpParam)
{
    while (true) {
        Sleep(5000);
    }

    return 0;
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