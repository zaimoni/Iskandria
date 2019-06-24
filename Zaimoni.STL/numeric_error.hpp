#ifndef NUMERIC_ERROR_HPP
#define NUMERIC_ERROR_HPP

#include <stdexcept>

namespace zaimoni {
namespace math {

	// subclassing so we can distinguish our runtime errors from others
	class numeric_error : public std::runtime_error {
	public:
		explicit numeric_error(std::string e) : std::runtime_error(e) {}
		explicit numeric_error(const char* const e) : std::runtime_error(e) {}
	};

}
}

#endif