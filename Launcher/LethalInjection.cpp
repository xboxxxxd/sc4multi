#include "LethalInjection.h"
#include <stdio.h>
#include <tlhelp32.h>
#include <shlwapi.h>

#define DLL_NAME				"SC4Multi.dll"
#define CREATE_THREAD_ACCESS	(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ)

bool IsWindowsNT()
{
	// check current version of Windows
	DWORD version = GetVersion();

	// parse return
	DWORD majorVersion = (DWORD)(LOBYTE(LOWORD(version)));
	DWORD minorVersion = (DWORD)(HIBYTE(LOWORD(version)));

	// Return value
	return (version < 0x80000000);
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	if(IsWindowsNT())
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Confirmed Windows NT machine.", L"SC4Multi Loader", 0 );
		#endif

		LoadDll(DLL_NAME);
	}
	else
		MessageBox( NULL, L"ERROR: Stuck in the '90s. Upgrade to Windows 2000 or higher.", L"SC4Multi Loader", 0 );

	return 0;
}


BOOL LoadDll(char *dllName)
{
	if(!InjectDLL(dllName))
		MessageBox(NULL, L"Injection failed due to previous messages.\nIf no previous messages came up, something is srsly screwed.", L"SC4Multi Loader", NULL);

	return true;
}

BOOL InjectDLL(char *dllName)
{
	LPVOID RemoteString, LoadLibAddy;
	STARTUPINFOW startupInfo;
	PROCESS_INFORMATION procInfo;

	memset( &startupInfo, 0, sizeof(startupInfo) );
	memset( &procInfo, 0, sizeof(procInfo) );
	LPWSTR cmdLine;

	#ifdef DEBUG
		cmdLine = L"-w -r800x600x32 -CustomResolution:enable -intro:off -CPUPriority:low -IgnoreMissingModelDataBugs:on";
	#else
		cmdLine = L"-intro:off -IgnoreMissingModelDataBugs:on";
	#endif

	startupInfo.cb = sizeof( startupInfo );
	if( !CreateProcessW( const_cast<LPCWSTR>( L"SimCity 4.exe" ),
		cmdLine, 0, 0, false,
		CREATE_DEFAULT_ERROR_MODE, 0, 0,
		&startupInfo, &procInfo )
	)
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Failed to launch an instance of SimCity 4.", L"SC4Multi Launcher", NULL );
		#endif

		return false;
	}

	std::fstream fcheck;
	fcheck.open( "SC4Multi.dll", std::ios::in );
	if( !fcheck.is_open() )
	{
		#ifdef DEBUG
			MessageBox( NULL, L"DLL is missing. What the hell are you doing?", L"SC4Multi Loader", NULL );
		#endif

		fcheck.close();
		return false;
	}

	fcheck.close();
	LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

	if( !LoadLibAddy )
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Failed to retrieve the address for library loading from kernel32.dll.\nYou may be stuck in the '90s.", L"SC4Multi Loader", NULL );
		#endif

		return false;
	}

	RemoteString = (LPVOID)VirtualAllocEx(procInfo.hProcess, NULL, strlen(DLL_NAME), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	if( !RemoteString )
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Failed to allocate space for a remote thread.", L"SC4Multi Loader", NULL );
		#endif

		return false;
	}

	BOOL couldWrite = WriteProcessMemory(procInfo.hProcess, (LPVOID)RemoteString, dllName, strlen(dllName), NULL);
	if( !couldWrite )
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Failed to write process memory. Everything up to this point worked.", L"SC4Multi Loader", NULL );
		#endif

		return false;
	}

	HANDLE remoteHandle = CreateRemoteThread(procInfo.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);
	if( !remoteHandle )
	{
		#ifdef DEBUG
			MessageBox( NULL, L"The remote thread was dead before it even started.", L"SC4Multi Loader", NULL );
		#endif

		return false;
	}

	#ifdef DEBUG
		MessageBoxA( NULL, "DLL injection was a smashing success... I hope.", "SC4Multi Loader", NULL );
	#endif

	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);

	return true;
}