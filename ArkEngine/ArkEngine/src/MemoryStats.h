#pragma once
#include <cstddef>

namespace Engine
{
	struct MemoryStats
	{
		std::size_t workingSetBytes = 0;      // current physical RAM used by process
		std::size_t peakWorkingSetBytes = 0;  // high-water mark
		std::size_t privateBytes = 0;         // private committed bytes (virtual)
	};

	// Returns current process memory stats.
	// On non-Windows platforms, all fields return 0.
	MemoryStats QueryMemoryStats();

	// Convenience: returns workingSetBytes in MiB as a float.
	float QueryWorkingSetMiB();
}
