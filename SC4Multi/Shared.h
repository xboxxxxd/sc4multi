#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinDef.h>

// Cool macros, bro
#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr)+(DWORD)(addValue))
#define GetPtr(ptr, addValue) ( (DWORD)(ptr) + (DWORD)(addValue) )

// Forward shared functions
ULONG_PTR ReadPointer(ULONG_PTR* ulbase, int noffset);

// Aw yeah definitions
#define DEBUG
#define SNULL 0xDEADBEEF