#include "Shared.h"

ULONG_PTR ReadPointer(ULONG_PTR* ulbase, int noffset)
{
	if (!IsBadReadPtr((void*)ulbase, sizeof(ULONG_PTR)))
	{
		if (!IsBadReadPtr((void*)((*(ULONG_PTR*)ulbase)+noffset), sizeof(ULONG_PTR)))
			return *(ULONG_PTR*)((*(ULONG_PTR*)ulbase)+noffset);
	}

	return 0;
}