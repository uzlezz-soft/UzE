#include <iostream>
#include "uze/engine.h"
#include "uze/log.h"
#include "uze/core/job_system.h"

int main()
{
	uze::initLogging(std::cout);
	uze::EntryPoint();

	uze::job_system::deinit();

	return 0;
}
