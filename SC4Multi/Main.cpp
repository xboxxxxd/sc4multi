#include "Shared.h"

BOOL WINAPI DllMain (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
			MessageBoxA( NULL, "This is a throwdown.", "Injected DLL", NULL );
            break;

        case DLL_PROCESS_DETACH:
			MessageBoxA( NULL, "Detached -- I'm free!", "Injected DLL", NULL );
            break;
    }
    return TRUE;
}