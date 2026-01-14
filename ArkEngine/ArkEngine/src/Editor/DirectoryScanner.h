#pragma once

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace Ark::Editor
{
	struct DirectoryEntry
	{
		std::filesystem::path path;
		std::string name;
		bool isDirectory = false;
	};

	// Background directory listing cache to keep the editor frame lightweight.
	// ImGui must remain on the main thread, but filesystem enumeration can be offloaded.
	class DirectoryScanner
	{
	public:
		DirectoryScanner() = default;
		~DirectoryScanner() { Stop(); }

		DirectoryScanner(const DirectoryScanner&) = delete;
		DirectoryScanner& operator=(const DirectoryScanner&) = delete;

		void Start();
		void Stop();

		// Request that a directory be scanned/refreshed. Non-blocking.
		void RequestScan(const std::filesystem::path& dir);

		// Returns true if we have a cached listing for `dir`.
		bool TryGetListing(const std::filesystem::path& dir, std::vector<DirectoryEntry>& out) const;

	private:
		void WorkerMain();
		static std::string KeyFor(const std::filesystem::path& p);

	private:
		mutable std::mutex m_mutex;
		std::condition_variable m_cv;
		std::thread m_thread;
		bool m_running = false;
		bool m_stop = false;

		std::vector<std::filesystem::path> m_pending; // simple queue
		std::unordered_map<std::string, std::vector<DirectoryEntry>> m_cache;
	};
}