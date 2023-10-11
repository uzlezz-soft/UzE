#include "uze/platform.h"

#if UZE_PLATFORM == UZE_PLATFORM_WEB

#include "uze/core/file_system.h"
#include <emscripten.h>

EM_JS(char*, js_loadFile, (const char* name, int* fileSize),
{
	let str = 'test';
	let len = lengthToBytesUTF8(str) + 1;
	let strPtr = _malloc(len);
	stringToUTF8(str, strPtr, len);
	fileSize = len - 1;
	Module.onOutParameterSet($0);
	return strPtr;
});

namespace uze::fs
{

	Buffer getFileContents(std::string_view file)
	{
		int size = 0;
		char* data = js_loadFile(file.data(), &size);
		Buffer buf(size);
		buf.data = data;
		return buf;
	}

}

#endif