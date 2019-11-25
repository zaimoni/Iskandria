#include "stacktrace.hpp"
#include "../Logging.h"

namespace zaimoni {

stacktrace& stacktrace::get()
{
	static stacktrace master;
	return master;
}

void stacktrace::_append(const char* src) const
{
	if (src && *src) INFORM(src);
}

void stacktrace::_summarize() const
{
	if (_x.empty()) return;
	const auto& fr = _x.back();
	INFORM("####");
	_append(fr.first);
	for (const auto& note : fr.second) _append(note.c_str());
}

}
