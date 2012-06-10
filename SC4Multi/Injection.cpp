// Client stuffs
#include "Shared.h"
#include "Injection.h"
#include "CGame.h"
#include "COrdinance.h"

// Networking (finally!)
#include "netcode.h"

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
	// Wait for SC4 to acknowledge our DLL
	while( GetModuleHandleA( "SC4Multi.dll" ) == NULL )
		Sleep( 150 );

	// Attempts to set the title
	CreateThread( NULL, NULL, setTitle, NULL, NULL, NULL );

	// Handles networking events
	CreateThread( NULL, NULL, HandleNetworking, NULL, NULL, NULL );
	
	#ifdef DEBUG

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
			else if( GetAsyncKeyState( 0x4E ) && GetAsyncKeyState( VK_MENU ) ) // Alt + N
			{
				char szOrd[192];
				sprintf
				(
					szOrd,
					"CPR: %i\n"
						"Water: %i\n"
						"Paper: %i\n"
						"Clinics: %i\n"
						"Smoke Detectors: %i",

					COrdinance::CPR(),
					COrdinance::WaterConservation(),
					COrdinance::PaperReduction(),
					COrdinance::FreeClinics(),
					COrdinance::SmokeDetector()
				);

				MessageBoxA( NULL, szOrd, "SC4Multi -- Ordinances", MB_OK | MB_ICONINFORMATION );
			}

			Sleep( 100 );
		}

	#endif

	return 1;
}