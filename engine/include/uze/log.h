#pragma once

#include <ios>
#include <fmt/format.h>

namespace uze
{

	using i8 = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;
	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;

	std::ostream& getOutputStream();
	void initLogging(std::ostream& out);

	enum LogLevel
	{
		LL_Debug, LL_Info, LL_Warn, LL_Error
	};

	struct LogCategory
	{
		const char* name;
		LogLevel min_level;

		constexpr LogCategory(const char* name_, LogLevel min_level_ = LL_Debug)
			: name(name_), min_level(min_level_) {}
	};

	static LogCategory log_debug { "Debug" };

#define LOG_FN_ARGS std::string_view, std::string_view, u64
	using LogFunction = void(*)(LOG_FN_ARGS);

	void uzLog_ImplDebug(LOG_FN_ARGS);
	void uzLog_ImplInfo(LOG_FN_ARGS);
	void uzLog_ImplWarn(LOG_FN_ARGS);
	void uzLog_ImplError(LOG_FN_ARGS);
	void uzLog_ImplUnknown(LOG_FN_ARGS);

	constexpr auto uzLog_getFunction(LogLevel level) -> LogFunction
	{
		switch (level)
		{
		case LL_Debug:
			return uzLog_ImplDebug;
		case LL_Info:
			return uzLog_ImplInfo;
		case LL_Warn:
			return uzLog_ImplWarn;
		case LL_Error:
			return uzLog_ImplError;
		default:
			return uzLog_ImplUnknown;
		}
	}

	constexpr void uzLog_Impl(const LogCategory& category, LogLevel level, std::string_view log,
		std::string_view file, u64 line)
	{
		uzLog_getFunction(level)(::fmt::format("{}: {}", category.name, log), file, line);
	}

#define UZ_EXPAND(x) x

#define uzLog(category, level, ...) \
	if constexpr (category.min_level <= LL_##level) \
		uzLog_Impl(category, LL_##level, UZ_EXPAND(::fmt::format(__VA_ARGS__)), __FILE__, __LINE__)

}