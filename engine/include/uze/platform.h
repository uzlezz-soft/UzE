#pragma once

#define UZE_PLATFORM_WINDOWS 0
#define UZE_PLATFORM_LINUX   1
#define UZE_PLATFORM_MACOS   2
#define UZE_PLATFORM_WEB     3


#if defined(_WIN32)
#define UZE_PLATFORM UZE_PLATFORM_WINDOWS
#define UZE_PLATFORM_STRING "Windows"
#elif defined(__linux__)
#define UZE_PLATFORM UZE_PLATFORM_LINUX
#define UZE_PLATFORM_STRING "Linux"
#elif defined(__APPLE__)
#define UZE_PLATFORM UZE_PLATFORM_MACOS
#define UZE_PLATFORM_STRING "Mac Os"
#elif defined(__EMSCRIPTEN__)
#define UZE_PLATFORM UZE_PLATFORM_WEB
#define UZE_PLATFORM_STRING "Web"
#else
#error "Unknows platform"
#endif


#if defined(UZE_DEBUG)
#define UZE_CONFIG_STRING "Debug"
#elif defined(UZE_RELEASE)
#define UZE_CONFIG_STRING "Release"
#elif defined(UZE_SHIPPING)
#define UZE_CONFIG_STRING "Shipping"
#else
#define UZE_CONFIG_STRING "Unknown configuration"
#endif