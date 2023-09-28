#include <iostream>
#include "uze/engine.h"
#include "uze/log.h"

int main()
{
	uze::initLogging(std::cout);
	uze::EntryPoint();

	return 0;
}
