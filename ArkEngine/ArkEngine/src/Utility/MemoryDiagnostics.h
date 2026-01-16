#pragma once

#include <atomic>
#include <cstdint>
#include <string>

namespace Ark::Memory
{
	struct DiagnosticsSnapshot
	{
		std::uint64_t allocatedBytes = 0;
		std::uint64_t freedBytes = 0;
		std::uint64_t liveBytes = 0;
		std::uint64_t peakLiveBytes = 0;
	};

	// Lightweight tracker. You can wire this into custom allocators or specific subsystems.
	void TrackAlloc(std::uint64_t bytes);
	void TrackFree(std::uint64_t bytes);

	DiagnosticsSnapshot GetSnapshot();

	std::string BytesToString(std::uint64_t bytes);
}