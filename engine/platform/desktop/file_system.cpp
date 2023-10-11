#include "uze/platform.h"

#if UZE_PLATFORM != UZE_PLATFORM_WEB

#include "uze/core/file_system.h"
#include <fstream>

namespace uze::fs
{

	Buffer getFileContents(std::string_view file)
	{
		const std::ios::openmode flags = std::ios::in | std::ios::ate | std::ios::binary;

		std::fstream in(file.data(), flags);
		if (!in.is_open())
			return {};

		u64	end = in.tellg();
		in.seekg(0, std::ios::beg);
		const u64 size = end - in.tellg();

		Buffer data(size);
		in.read(reinterpret_cast<char*>(data.data), size);
		return data;
	}

}

#endif