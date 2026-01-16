#pragma once

#include <functional>
#include <mutex>
#include <queue>

namespace Ark::Rendering
{
	// Pump-driven loader. Submit jobs from any thread; execute a limited amount per frame.
	class ResourceLoader
	{
	public:
		using Job = std::function<void()>;

		void Enqueue(Job job);

		// Runs up to `maxJobs` jobs. Call once per frame (main thread or render thread).
		void Pump(int maxJobs);

		std::size_t PendingCount() const;

	private:
		mutable std::mutex m_mutex;
		std::queue<Job> m_jobs;
	};
}