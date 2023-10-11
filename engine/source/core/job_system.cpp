#include "uze/core/job_system.h"
#include <vector>
#include <thread>
#include <queue>
#include <mutex>

namespace uze
{

	static constexpr LogCategory log_job_system { "JobSystem" };

	Job& Job::operator=(std::function<JobResult()>&& f)
	{
		if (status != JobStatus::Prepairing && status != JobStatus::Finished)
		{
			uzLog(log_job_system, Warn, "Tried to set job function while job is submitted or in-progress");
			return *this;
		}

		func = f;
		return *this;
	}

	void Job::wait()
	{
		if (status == JobStatus::Prepairing)
			return;

		while (status != JobStatus::Finished)
			std::this_thread::yield();
	}

	static bool s_initialized = false;

	struct Worker
	{
		std::thread thread;
		std::atomic<bool> busy{ false };
	};

	static std::atomic<bool> s_executing;
#if UZE_PLATFORM != UZE_PLATFORM_WEB
	static std::vector<std::unique_ptr<Worker>> s_workers;
	static std::queue<Job*> s_jobs;
	static std::mutex s_job_mutex;

	void workerThread();
#endif

	void job_system::init()
	{
		if (s_initialized)
		{
			uzLog(log_job_system, Warn, "Tried to init second time");
			return;
		}

#if UZE_PLATFORM != UZE_PLATFORM_WEB
		const auto max_threads = std::min(std::thread::hardware_concurrency(), 8u);
		const auto num_threads = max_threads == 0 ? 1 : std::max(max_threads - 2u, 1u);
		uzLog(log_job_system, Info, "Creating {} worker threads", num_threads);
		s_workers.reserve(num_threads);

		for (u64 i = 0; i < num_threads; ++i)
		{
			auto worker = std::make_unique<Worker>();
			worker->thread = std::thread(workerThread);
			s_workers.push_back(std::move(worker));
		}
#else
		uzLog(log_job_system, Info, "Running on web, jobs will be executed on thread which submitted them");
#endif

		s_executing = true;
		s_initialized = true;
	}

	void job_system::submit(Job& job)
	{
		if (!s_executing)
		{
			uzLog(log_job_system, Error, "Tried to submit a job while job system isn't working");
			return;
		}

		if (job.status != JobStatus::Prepairing && job.status != JobStatus::Finished)
		{
			uzLog(log_job_system, Warn, "Tried to submit a job which already submitted or is in-progress");
			return;
		}

		job.status = JobStatus::Submitted;
#if UZE_PLATFORM != UZE_PLATFORM_WEB
		{
			std::scoped_lock lock(s_job_mutex);
			s_jobs.push(&job);
		}
#else
		job.status = JobStatus::InProgress;
		job.result = job.func();
		job.status = JobStatus::Finished;
#endif
	}

	void job_system::deinit()
	{
		s_executing = false;

#if UZE_PLATFORM != UZE_PLATFORM_WEB
		uzLog(log_job_system, Info, "Waiting for {} worker threads to stop", s_workers.size());

		for (u64 i = 0; i < s_workers.size(); ++i)
		{
			s_workers[i]->thread.join();
		}
#endif

		uzLog(log_job_system, Info, "Shutdown");
	}

	void job_system::waitForAllJobs()
	{
#if UZE_PLATFORM != UZE_PLATFORM_WEB
		if (!s_executing) return;

		while (!s_jobs.empty())
			std::this_thread::yield();
#endif
	}

	u64 job_system::getNumWorkerThreads()
	{
#if UZE_PLATFORM != UZE_PLATFORM_WEB
		return s_workers.size();
#else
		return 0;
#endif
	}

	u64 job_system::getNumBusyWorkerThreads()
	{
		u64 num_busy = 0;
#if UZE_PLATFORM != UZE_PLATFORM_WEB
		for (const auto& worker : s_workers)
		{
			num_busy += static_cast<u64>(worker->busy);
		}
#endif
		return num_busy;
	}

#if UZE_PLATFORM != UZE_PLATFORM_WEB
	void workerThread()
	{
		while (s_executing)
		{
			s_job_mutex.lock();

			if (s_jobs.empty())
			{
				s_job_mutex.unlock();
				std::this_thread::yield();
				continue;
			}

			auto job = s_jobs.front();
			s_jobs.pop();
			s_job_mutex.unlock();

			job->status = JobStatus::InProgress;
			job->result = job->func();
			job->status = JobStatus::Finished;
		}
	}
#endif
	
}
