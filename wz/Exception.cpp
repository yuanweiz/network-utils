#include "Exception.h"
#include <assert.h>
#include <execinfo.h>
void backTrace(std::string & callstack_)
{
	const int len = 200;
	void* buffer[len];
	int nptrs = ::backtrace(buffer, len);
	char** strings = ::backtrace_symbols(buffer, nptrs);
	if (strings)
	{
		for (int i = 0; i < nptrs; ++i)
		{
			// TODO demangle funcion name with abi::__cxa_demangle
			callstack_.push_back('\n');
			callstack_.append(strings[i]);
		}
		free(strings);
	}
}
Exception::Exception(const char * msg):callstack_(msg) {
	backTrace(callstack_);
}
Exception::Exception(const std::string & msg):callstack_(msg) {
	backTrace(callstack_);
}
const char * Exception::what()const noexcept{
	return callstack_.c_str();
}


LogicError::LogicError(const char*msg) :Exception(msg) {
}
LogicError::LogicError(const std::string&msg) :Exception(msg) {
}
const char*LogicError::what() const noexcept{
	return Exception::what();
}
