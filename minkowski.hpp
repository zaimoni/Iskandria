// minkowski.hpp

#ifndef MINKOWSKI_HPP
#define MINKOWSKI_HPP

#include "object.hpp"
#include "constants.hpp"
#include "coord_chart.hpp"

namespace iskandria {

// This has to reach the hard drive: barycentric coordinates for a system, etc.
class minkowski final : public isk::Object
{
public:
	typedef fundamental_constants::interval interval;
	typedef zaimoni::math::Cartesian_vector<interval,4> coord_type;
private:
	minkowski(FILE* src);
public:
	minkowski() = default;
	minkowski(const minkowski& src) = default;
	minkowski(minkowski&& src) = default;
	minkowski& operator=(const minkowski& src) = default;
	minkowski& operator=(minkowski&& src) = default;
	~minkowski() = default;

	static std::weak_ptr<minkowski> track(std::shared_ptr<minkowski> src);

	// this has to integrate with the world manager
	static void world_setup();
	static std::weak_ptr<minkowski> read_synthetic_id(FILE* src);
	static void write_synthetic_id(const std::shared_ptr<minkowski>& src,FILE* dest);
private:
	void save(FILE* dest);
	static void update_all();
	static void gc_all();
	static void load_all(FILE* src);
	static void save_all(FILE* dest);

	static std::vector<std::shared_ptr<minkowski> >& cache();
};

}	// namespace iskandria

#endif
