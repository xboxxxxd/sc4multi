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

	// Parse the returned version
	DWORD majorVersion = (DWORD)(LOBYTE(LOWORD(version)));
	DWORD minorVersion = (DWORD)(HIBYTE(LOWORD(version)));

	// Return value
	return (version < 0x80000000);
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	// If this is not a Windows NT machine or better, we cannot use CreateRemoteThread.
	// Because of this, reject non-NT machines, meaning Windows 98 and ME.
	// Anyone who still uses Windows 98/ME deserves to be thrown off a cliff.
	if(IsWindowsNT())
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Confirmed Windows NT machine.", L"SC4Multi Loader", 0 );
		#endif

		// Inject the DLL via remote thread and forced library loading.
		LoadDll(DLL_NAME);
	}
	else
	{
		// PUNNY
		MessageBox( NULL, L"ERROR: Stuck in the '90s. Upgrade to Windows 2000 or higher.", L"SC4Multi Loader", 0 );
	}

	return 0;
}


BOOL LoadDll(char *dllName)
{
	// Check for DLL loading errors
	if(!InjectDLL(dllName))
		MessageBox(NULL, L"Injection failed.", L"SC4Multi Loader", NULL);

	return true;
}

BOOL InjectDLL(char *dllName)
{
	// Check if the DLL exists.
	{
		// Open a file stream
		std::fstream fcheck;

		// Attempt to open the DLL file, read-only
		fcheck.open( "SC4Multi.dll", std::ios::in );

		// If a handle is not open, the file was not found
		if( !fcheck.is_open() )
		{
			#ifdef DEBUG
				MessageBox( NULL, L"DLL is missing or in use. What the hell are you doing?", L"SC4Multi Loader", NULL );
			#endif

			// Close our file stream to free any remaining resources
			fcheck.close();

			// FAIL'T
			return false;
		}

		// Successful stream check, file exists and is locked/not in use
		fcheck.close();
	}

	// Allocate some variables to be injected as remote memory
	LPVOID RemoteString, LoadLibAddy;

	// Allocate some variables for the startup and process information when
	// spawning our instance of SimCity 4
	STARTUPINFOW startupInfo;
	PROCESS_INFORMATION procInfo;

	// Allocate memory for the startup and process information
	memset( &startupInfo, 0, sizeof(startupInfo) );
	memset( &procInfo, 0, sizeof(procInfo) );
	startupInfo.cb = sizeof( startupInfo );

	// Allocate a variable for the command line
	LPWSTR cmdLine;

	#ifdef DEBUG
		cmdLine = L"-w -r800x600x32 -CustomResolution:enable -intro:off -CPUPriority:low -IgnoreMissingModelDataBugs:on";
	#else
		cmdLine = L"-intro:off -IgnoreMissingModelDataBugs:on";
	#endif

	// See if we successfully spawned a process
	// The process is created suspended to prevent further code from loading while
	//     our DLL is injected into the SC4 process.
	if( !CreateProcessW( const_cast<LPCWSTR>( L"SimCity 4.exe" ),
		cmdLine, 0, 0, false,
		CREATE_SUSPENDED, 0, 0,
		&startupInfo, &procInfo )
	)
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Failed to launch an instance of SimCity 4.", L"SC4Multi Launcher", NULL );
		#endif

		// Nope, we're done here.
		return false;
	}

	// Retrieve the address for the LoadLibraryA method
	LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

	// Was the address retrieved? Is the address valid?
	if( !LoadLibAddy )
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Failed to retrieve the address for library loading from kernel32.dll.\nYou may be stuck in the '90s.", L"SC4Multi Loader", NULL );
		#endif

		// Stop right there criminal scum.
		return false;
	}

	// Allocate memory in our remote process for the string with the name of our DLL
	RemoteString = (LPVOID)VirtualAllocEx(procInfo.hProcess, NULL, strlen(DLL_NAME), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	// If the address to the string does not exist, something went wrong.
	if( !RemoteString )
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Failed to allocate space for a remote thread.", L"SC4Multi Loader", NULL );
		#endif

		// Close any handles we have on SC4
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);

		// ABORT MISSION
		return false;
	}

	// See if we could write the string into the process' memory
	BOOL couldWrite = WriteProcessMemory(procInfo.hProcess, (LPVOID)RemoteString, dllName, strlen(dllName), NULL);

	// Nope.
	if( !couldWrite )
	{
		#ifdef DEBUG
			MessageBox( NULL, L"Failed to write process memory. Everything up to this point worked.", L"SC4Multi Loader", NULL );
		#endif

		// Close our handles
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);

		// Finito
		return false;
	}

	// See if a remote handle could be created
	HANDLE remoteHandle = CreateRemoteThread(procInfo.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);

	// Nuh-uh.
	if( !remoteHandle )
	{
		#ifdef DEBUG
			MessageBox( NULL, L"The remote thread was dead before it even started.", L"SC4Multi Loader", NULL );
		#endif

		// Destroy our handles
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);

		// Fin.
		return false;
	}

	// Resume the game's main thread to continue loading
	ResumeThread( procInfo.hThread );

	#ifdef DEBUG
		MessageBoxA( NULL, "DLL injection was a smashing success... I hope.", "SC4Multi Loader", NULL );
	#endif

	// Close the injector's handles on SC4 to free resources
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
	CloseHandle(remoteHandle);

	// Done
	return true;
}