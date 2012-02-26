#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

// Pretty cool macro, bro
#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr)+(DWORD)(addValue))

// Aw yeah definitions
#define DEBUG