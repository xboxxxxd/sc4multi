#include "Shared.h"
#include "Injection.h"
#include "CGame.h"

DWORD WINAPI setTitle( LPVOID )
{
	HWND hHopefulWnd = NULL;
	while( !hHopefulWnd )
	{
		// Find the window instance of SC4
		HWND hWnd = FindWindow( NULL, L"SimCity 4" );

		// Change the name to SC4Multi because I'm badass like that
		SetWindowText( hWnd, L"SC4Multi 0.1pre" );

		// Find the window, if we can.
		hHopefulWnd = FindWindow( NULL, L"SC4Multi 0.1pre" );

		// Take a break.
		Sleep( 25 );
	}

	return 1;
}

// www.infinityhacks.com/forum/showthread.php?7792-Most-basic-C-hack
DWORD WINAPI entryPoint( LPVOID )
{
	while( GetModuleHandleA( "SC4Multi.dll" ) == NULL )
		Sleep( 150 );

	CreateThread( NULL, NULL, setTitle, NULL, NULL, NULL );
	CGame cGameInst = CGame();

	for( ;; )
	{
		if( GetAsyncKeyState( 0x4C ) && GetAsyncKeyState( VK_MENU ) ) // Alt + L. Alt is referred to as the Menu key
		{
			long cash = CGame::GetCash();

			char szCash[128];
			sprintf( szCash, "You have %i simoleons.\nYou have %i citizens.", cash );

			MessageBoxA( NULL, szCash, "SC4Multi -- City Funding", MB_OK | MB_ICONINFORMATION );
		}
		else if( GetAsyncKeyState( 0x4D ) && GetAsyncKeyState( VK_MENU ) ) // Alt + M.
		{
			CGame::SetCash( 133337 );
			MessageBoxA( NULL, "You should now have $133,337 simoleons next month.", "SC4Multi -- City Funding", MB_OK | MB_ICONEXCLAMATION );
		}

		Sleep( 100 );
	}

	return 1;
}