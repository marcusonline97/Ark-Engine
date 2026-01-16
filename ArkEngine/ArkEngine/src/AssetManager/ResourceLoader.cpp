#include "ResourceLoader.h"

namespace Ark::Rendering
{
	void ResourceLoader::Enqueue(Job job)
	{
		if (!job)
			return;

		std::lock_guard<std::mutex> lock(m_mutex);
		m_jobs.push(std::move(job));
	}

	void ResourceLoader::Pump(int maxJobs)
	{
		for (int i = 0; i < maxJobs; ++i)
		{
			Job job;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (m_jobs.empty())
					return;

				job = std::move(m_jobs.front());
				m_jobs.pop();
			}

			job();
		}
	}

	std::size_t ResourceLoader::PendingCount() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_jobs.size();
	}
}