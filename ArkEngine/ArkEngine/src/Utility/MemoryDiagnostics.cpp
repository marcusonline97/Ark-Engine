#include "MemoryDiagnostics.h"

#include <cstdio>

namespace Ark::Memory
{
	static std::atomic<std::uint64_t> g_allocated{ 0 };
	static std::atomic<std::uint64_t> g_freed{ 0 };
	static std::atomic<std::uint64_t> g_peakLive{ 0 };

	void TrackAlloc(std::uint64_t bytes)
	{
		if (bytes == 0)
			return;

		const auto a = g_allocated.fetch_add(bytes, std::memory_order_relaxed) + bytes;
		const auto f = g_freed.load(std::memory_order_relaxed);
		const auto live = (a >= f) ? (a - f) : 0;

		auto peak = g_peakLive.load(std::memory_order_relaxed);
		while (live > peak && !g_peakLive.compare_exchange_weak(peak, live, std::memory_order_relaxed))
		{
		}
	}

	void TrackFree(std::uint64_t bytes)
	{
		if (bytes == 0)
			return;

		g_freed.fetch_add(bytes, std::memory_order_relaxed);
	}

	DiagnosticsSnapshot GetSnapshot()
	{
		DiagnosticsSnapshot s{};
		s.allocatedBytes = g_allocated.load(std::memory_order_relaxed);
		s.freedBytes = g_freed.load(std::memory_order_relaxed);
		s.liveBytes = (s.allocatedBytes >= s.freedBytes) ? (s.allocatedBytes - s.freedBytes) : 0;
		s.peakLiveBytes = g_peakLive.load(std::memory_order_relaxed);
		return s;
	}

	std::string BytesToString(std::uint64_t bytes)
	{
		const char* suffix = "B";
		double v = static_cast<double>(bytes);

		if (v >= 1024.0) { v /= 1024.0; suffix = "KiB"; }
		if (v >= 1024.0) { v /= 1024.0; suffix = "MiB"; }
		if (v >= 1024.0) { v /= 1024.0; suffix = "GiB"; }

		char buf[64]{};
		std::snprintf(buf, sizeof(buf), "%.2f %s", v, suffix);
		return std::string(buf);
	}
}