#include "Shared.h"
#include "Injection.h"

BOOL WINAPI DllMain (HMODULE hDll, DWORD dwReason, LPVOID lpReserved)
{
    DisableThreadLibraryCalls( hDll );
	if( dwReason == DLL_PROCESS_ATTACH )
	{
		#ifdef DEBUG
			MessageBoxA( NULL, "SC4Multi by stormeus, formerly simmaster07", "SC4Multi DLL", NULL );
		#endif

		CreateThread( NULL, NULL, entryPoint, NULL, NULL, NULL );
	}

    return TRUE;
}