#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <detours.h>

#define DIRECT3D_VERSION 0x0800
#define DIRECTINPUT_VERSION 0x0800 
#include <d3d8.h>
#include <ddraw.h>
#include <d3dx8tex.h>

#include "cegui/include/CEGUI.h"

// Cool macros, bro
#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr)+(DWORD)(addValue))
#define GetPtr(ptr, addValue) ( (DWORD)(ptr) + (DWORD)(addValue) )

// Forward shared functions
ULONG_PTR ReadPointer(ULONG_PTR* ulbase, int noffset);

// Aw yeah definitions
#define DEBUG
#define SNULL 0xDEADFAB