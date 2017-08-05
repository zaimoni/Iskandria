// agent.hpp

#ifndef AGENT_HPP
#define AGENT_HPP 1

#include "object.hpp"

namespace iskandria
{

// per XCOM
// name
// rank
// assigned base
// assigned craft
// missions?
// kills?
// wound recovery
// value (possibly calculated rather than field)
// destination base during transfer (but redundant with transfer tracking)
// initial stats
// improved stats
// assigned armor? [shouldn't use a normal inventory slot]
// gender: M/F (biological: do you get a *natural* high testosterone level?  Androgen (ab)use is a separate issue.)
// race (includes species)

// The primary coordinate chart for an agent, is inventory.
// otherwise, it would be unusual to primary-define an object's location relative to an agent.
// first person views would use agent-anchored coordinates.
// the distance scales for gridspaces are often chosen so that one agent fits in one cell with no slack.

struct agent_species
{	
	unsigned char _civilization;
	const char* name;
	size_t name_len;
	const char* desc;
	size_t desc_len;
};

class agent final : public isk::Object
{
public:
	static const agent_species* species;		// external configuration would be strictly for modding support.
	static const size_t species_len;
private:
	size_t _species_index;

	agent(FILE* src);
public:
	agent() = default;
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(agent);

	const agent_species& type() const {return species[_species_index];}

	static std::weak_ptr<agent> track(std::shared_ptr<agent> src);

	// this has to integrate with the world manager	
	static void world_setup();
	static std::weak_ptr<agent> read_synthetic_id(FILE* src);
	static void write_synthetic_id(const std::shared_ptr<agent>& src,FILE* dest);
private:
	void save(FILE* dest);
	static void update_all();
	static void gc_all();
	static void load_all(FILE* src);
	static void save_all(FILE* dest);

	static std::vector<std::shared_ptr<agent> >& cache();
	};

}

#endif
