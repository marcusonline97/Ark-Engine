#include "DirectoryScanner.h"

#include <algorithm>
#include <system_error>

#include "Logger.h"

namespace Ark::Editor
{
	void DirectoryScanner::Start()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_running)
			return;

		m_stop = false;
		m_running = true;
		m_thread = std::thread([this]() { WorkerMain(); });
	}

	void DirectoryScanner::Stop()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_running)
				return;
			m_stop = true;
		}
		m_cv.notify_one();

		if (m_thread.joinable())
			m_thread.join();

		std::lock_guard<std::mutex> lock(m_mutex);
		m_running = false;
		m_stop = false;
		m_pending.clear();
	}

	std::string DirectoryScanner::KeyFor(const std::filesystem::path& p)
	{
		std::error_code ec;
		const auto abs = std::filesystem::absolute(p, ec);
		return ec ? p.lexically_normal().string() : abs.lexically_normal().string();
	}

	void DirectoryScanner::RequestScan(const std::filesystem::path& dir)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_running)
				return;
			m_pending.push_back(dir);
		}
		m_cv.notify_one();
	}

	bool DirectoryScanner::TryGetListing(const std::filesystem::path& dir, std::vector<DirectoryEntry>& out) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		const auto it = m_cache.find(KeyFor(dir));
		if (it == m_cache.end())
			return false;
		out = it->second;
		return true;
	}

	void DirectoryScanner::WorkerMain()
	{
		while (true)
		{
			std::filesystem::path dir;
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_cv.wait(lock, [this]() { return m_stop || !m_pending.empty(); });
				if (m_stop)
					return;
				dir = m_pending.back();
				m_pending.pop_back();
			}

			std::vector<DirectoryEntry> entries;

			try
			{
				std::error_code ec;

				std::filesystem::directory_iterator it(dir, ec);
				std::filesystem::directory_iterator end;
				if (ec)
				{
					Logging::Warning() << "DirectoryScanner: Failed to enumerate '" << dir.string()
						<< "': " << ec.message() << "\n";
				}
				else
				{
					for (; it != end; it.increment(ec))
					{
						if (ec)
						{
							Logging::Warning() << "DirectoryScanner: Enumeration error in '" << dir.string()
								<< "': " << ec.message() << "\n";
							break;
						}

						const auto& e = *it;

						DirectoryEntry de;
						de.path = e.path();
						de.name = e.path().filename().string();

						std::error_code typeEc;
						de.isDirectory = e.is_directory(typeEc);
						entries.push_back(std::move(de));
					}
				}
			}
			catch (const std::system_error& e)
			{
				Logging::Warning() << "DirectoryScanner: std::system_error while scanning '" << dir.string()
					<< "': " << e.what() << "\n";
				entries.clear();
			}
			catch (const std::exception& e)
			{
				Logging::Warning() << "DirectoryScanner: exception while scanning '" << dir.string()
					<< "': " << e.what() << "\n";
				entries.clear();
			}
			catch (...)
			{
				Logging::Warning() << "DirectoryScanner: unknown exception while scanning '" << dir.string() << "'\n";
				entries.clear();
			}

			std::sort(entries.begin(), entries.end(),
				[](const DirectoryEntry& a, const DirectoryEntry& b)
				{
					if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
					return a.name < b.name;
				});

			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_cache[KeyFor(dir)] = std::move(entries);
			}
		}
	}
}