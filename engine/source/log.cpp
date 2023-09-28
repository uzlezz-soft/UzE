#include "uze/log.h"

#include <filesystem>
#include <iostream>

namespace uze
{

	static std::ostream* out_stream = &std::cout;

	void initLogging(std::ostream& out)
	{
		out_stream = &out;
	}

	static const std::string_view log_format_str = "[{}] {}";

	void uzLog_ImplDebug(std::string_view log, std::string_view file, u64 line)
	{
		getOutputStream() << fmt::format(log_format_str, "DEBUG", fmt::format("`{}`, line {}", file, line)) << "\n";
		getOutputStream() << fmt::format(log_format_str, "DEBUG", log) << "\n";
	}

	void uzLog_ImplInfo(std::string_view log, std::string_view, u64)
	{
		getOutputStream() << fmt::format(log_format_str, "INFO ", log) << "\n";
	}

	void uzLog_ImplWarn(std::string_view log, std::string_view, u64)
	{
		getOutputStream() << fmt::format(log_format_str, "WARN ", log) << "\n";
	}

	void uzLog_ImplError(std::string_view log, std::string_view, u64)
	{
		getOutputStream() << fmt::format(log_format_str, "ERROR", log) << "\n";
	}

	void uzLog_ImplUnknown(std::string_view log, std::string_view file, u64 line)
	{
		uzLog_ImplWarn(fmt::format("Unknown log level at `{}`, line {}", file, line), file, line);
		getOutputStream() << fmt::format(log_format_str, "UNKNOWN", log) << "\n";
	}

	std::ostream& getOutputStream()
	{
		return *out_stream;
	}

}