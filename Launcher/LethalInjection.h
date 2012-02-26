#define DEBUG
#include <Windows.h>
#include <iostream>
#include <fstream>

BOOL WriteProcessBYTES(HANDLE hProcess,LPVOID lpBaseAddress,LPCVOID lpBuffer,SIZE_T nSize);
BOOL LoadDll(char *dllName);
BOOL InjectDLL(char *dllName);