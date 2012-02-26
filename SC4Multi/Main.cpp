#include "Shared.h"

// www.gamersoul.com/forums/showthread.php?15873-TUT-C-Form-in-DLL-read-pointers
__inline ULONG_PTR ReadPointer(ULONG_PTR* ulbase, int noffset)
{
	if (!IsBadReadPtr((void*)ulbase, sizeof(ULONG_PTR)))
	{
		if (!IsBadReadPtr((void*)((*(ULONG_PTR*)ulbase)+noffset), sizeof(ULONG_PTR)))
			return *(ULONG_PTR*)((*(ULONG_PTR*)ulbase)+noffset);
	}

	return 0;
}

void lal()
{
	int cash    = (int)ReadPointer( (ULONG_PTR*)0x00b4418c, 0x00000028 );
	char buf[35];

	sprintf( buf, "%i", cash );
	MessageBoxA( NULL, buf, "SC4Multi", NULL );
}

// www.infinityhacks.com/forum/showthread.php?7792-Most-basic-C-hack
DWORD WINAPI entryPoint( LPVOID )
{
	while( GetModuleHandleA( "SC4Multi.dll" ) == NULL )
		Sleep( 150 );

	for( ;; )
	{
		if( GetAsyncKeyState( 0x4C ) ) // L key
			lal();

		Sleep( 100 );
	}
}

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