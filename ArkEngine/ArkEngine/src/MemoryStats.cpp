#include "MemoryStats.h"

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace Engine
{
	MemoryStats QueryMemoryStats()
	{
		MemoryStats result;
#if defined(_WIN32)
		PROCESS_MEMORY_COUNTERS_EX pmc = {};
		pmc.cb = sizeof(pmc);
		if (GetProcessMemoryInfo(GetCurrentProcess(),
			reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)))
		{
			result.workingSetBytes = pmc.WorkingSetSize;
			result.peakWorkingSetBytes = pmc.PeakWorkingSetSize;
			result.privateBytes = pmc.PrivateUsage;
		}
#endif
		return result;
	}

	float QueryWorkingSetMiB()
	{
		return static_cast<float>(QueryMemoryStats().workingSetBytes) / (1024.0f * 1024.0f);
	}
}
