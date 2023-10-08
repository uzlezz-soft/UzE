#pragma once

#include "uze/common.h"
#include <atomic>
#include <functional>

namespace uze
{

	enum class UZE JobStatus
	{
		Prepairing, Submitted, InProgress, Finished
	};

	enum class UZE JobResult
	{
		NotDeterminedYet, Success, Failure
	};

	struct UZE Job
	{
		std::atomic<JobStatus> status { JobStatus::Prepairing };
		std::atomic<JobResult> result { JobResult::NotDeterminedYet };
		std::function<JobResult()> func;

		Job(std::function<JobResult()>&& f) : func(f) {}

		Job& operator=(std::function<JobResult()>&& f);

		void wait();
	};

	namespace job_system
	{

		UZE void init();
		UZE void submit(Job& job);
		UZE void deinit();

		UZE void waitForAllJobs();

		UZE u64 getNumWorkerThreads();
		UZE u64 getNumBusyWorkerThreads();

	}

}