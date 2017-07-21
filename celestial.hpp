// celestial.hpp

#ifndef CELESTIAL_HPP
#define CELESTIAL_HPP

#include "constants.hpp"
#include "angle.hpp"
#include "conventions.hpp"
#include "object.hpp"

// A celestial body of some kind.
// Imposes a spherical coordinate system (r,theta,phi).  We actually get two systems: the sidereal one, and the anchored-surface one.
// North pole: 0 degrees phi
// South pole: 180 degrees phi
// rotation is west to east: -180 degrees theta is 180 degrees west, 180 degrees theta is 180 degrees east.
// local properties are:
// * sidereal rotation rate in the proper time of the center of mass -- the velocity of the prime meridian.  Gas giants/stars may vary by latitude/theta
// * The gravitational-mass constant GM (this is *often* much more precisely known than mass, since direct force measurements are easy and Newtonian G is only known to 4 significant digits)
// ** from above: local g as a function of radius r (theoretical)
// ** from above: local g as a function of r,theta (includes centripetal psuedoforce)
// * if total mass is known more precisely we use that instead
// * celestial bodies are not known for their volitional abilities.  The events they should generate are:
// ** gravity: Newtonian orbit approximation
// ** rotation: constant approximation

namespace iskandria {

class celestial_body final : public isk::Object
{
private:
	celestial_body(FILE* src);
public:
	celestial_body() = default;
	celestial_body(const celestial_body& src) = default;
	celestial_body(celestial_body&& src) = default;
	celestial_body& operator=(const celestial_body& src) = default;
	celestial_body& operator=(celestial_body&& src) = default;
	~celestial_body() = default;

	static std::weak_ptr<celestial_body> track(std::shared_ptr<celestial_body> src);

	// this has to integrate with the world manager	
	static void world_setup();
	static std::weak_ptr<celestial_body> read_synthetic_id(FILE* src);
	static void write_synthetic_id(const std::shared_ptr<celestial_body>& src,FILE* dest);
private:
	void save(FILE* dest);
	static void update_all();
	static void gc_all();
	static void load_all(FILE* src);;
	static void save_all(FILE* dest);

	static std::vector<std::shared_ptr<celestial_body> >& cache();
};

}	// namespace iskandria

#endif