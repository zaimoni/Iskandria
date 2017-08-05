// craft.hpp

#ifndef CRAFT_HPP
#define CRAFT_HPP

#include "object.hpp"

namespace iskandria {

// per X-COM:
// need type enum
// need static array of model traits
// need ship weapon enum and slot specification
// need inventory enum and craft inventory specification
// need craft name one way or another

// the primary coordinate chart within a craft, when it matters, is often a gridspace
// craft often have agents as crew

enum drive_train {
	DRIVE_NONE = 0,				// default-initialize to none
	DRIVE_ALT_AZIMUTH_MOUNT,	// these two would be mortars, howitzers, etc.
	DRIVE_EQUATORIAL_MOUNT,
	DRIVE_RHO,			// spacecraft: telekinesis drive for Iskandrans, gravity drive for Tzarz
	DRIVE_REACTION,		// spacecraft: linear accelerator drive for Terra, maglev drive for Wylkrai.
						// Also subsumes "low-technology" rocket thrusters that all space-faring civilizations have.
	DRIVE_TRACKS,		// tanks, etc.
	DRIVE_WHEELS,
	DRIVE_HOVERFAN		// typical hovercraft.
	// XXX legs are cool but the engineering issues make it hard to know what is internally consistent.  Belongs in biological category; # of legs matters
	// XXX snake drive is obviously feasible, but probably belongs in the biological category
	// XXX hydrosphere drives should be important.  At least: Motorboat, sailboat, submarine, and hydrofoil at least.
	// XXX likewise aerocraft should be important.  At least: rotary airplane, jet, hangglider.
	// XXX the macroscopic biological ones are: land: legs, snake.  water: "fish", "squid".  air: "avian", "mosquito-like", "butterfly-like" (hangglider analog rather than thrust-based)
};

struct craft_model
{
	unsigned char _civilization;
	unsigned char _drive_train;
	const char* name;
	size_t name_len;
	const char* desc;
	size_t desc_len;
};

class craft final : public isk::Object
{
public:
	static const craft_model* models;	// may need to allow external configuration to manage RAM loading.
	static const size_t models_len;
private:
	size_t _model_index;

	craft(FILE* src);
public:
	craft() = default;
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(craft);

	const craft_model& type() const {return models[_model_index];}

	static std::weak_ptr<craft> track(std::shared_ptr<craft> src);

	// this has to integrate with the world manager
	static void world_setup();
	static std::weak_ptr<craft> read_synthetic_id(FILE* src);
	static void write_synthetic_id(const std::shared_ptr<craft>& src,FILE* dest);
private:
	void save(FILE* dest);
	static void update_all();
	static void gc_all();
	static void load_all(FILE* src);
	static void save_all(FILE* dest);

	static std::vector<std::shared_ptr<craft> >& cache();
};

}	// namespace iskandria

#endif
