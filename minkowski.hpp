// minkowski.hpp

#ifndef MINKOWSKI_HPP
#define MINKOWSKI_HPP

#include "constants.hpp"
#include "coord_chart.hpp"

namespace iskandria {

// This has to reach the hard drive: barycentric coordinates for a system, etc.
class minkowski final
{
public:
	typedef fundamental_constants::interval interval;
	typedef zaimoni::math::Cartesian_vector<interval,4> coord_type;
private:
	fundamental_constants::units _system;

public:
	minkowski() = default;
	minkowski(FILE* src);
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(minkowski);
	void save(FILE* dest) const;
};

}	// namespace iskandria

namespace zaimoni {
	template<>
	struct rw_mode<iskandria::minkowski>
	{
		enum {
			group_write = 3,
			group_read = 3
		};
	};
}

#endif
