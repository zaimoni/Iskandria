// Euclidean.hpp

#ifndef EUCLIDEAN_HPP
#define EUCLIDEAN_HPP

#include <vector>

namespace zaimoni {
namespace math {
namespace Euclidean {

// the dot product is very easy to botch numerically.  Put a default wrong version here and deal with accuracy later.

template<class T1, class T2>
typename std::enable_if<std::is_same<typename T1::value_type, typename T2::value_type>::value, typename T1::value_type>::type dot(const T1& lhs, const T2& rhs)
{
	assert(lhs.size()==rhs.size());
	assert(0<lhs.size());

	size_t i = lhs.size();
	std::vector<typename std::remove_cv<typename T1::value_type>::type> accumulator(i);
	typename T1::value_type zero(0);
	while(0<i)
		{
		--i;
		// XXX need to handle other exceptional conditions by type; e.g. double NaN or infinity
		if (zero == lhs[i] || zero == rhs[i]) continue;
		// XXX need to handle overflow by type
		accumulator.push_back(lhs[i]*rhs[i]);
		}

	if (accumulator.empty()) return 0;
	// XXX need to be overflow-aware when doing this sum, etc.
	while(2<=accumulator.size()) {
		accumulator[0] += accumulator.back();
		accumulator.pop_back();
	}
	return accumulator.front();
}


}
}
}

#endif

