// agent.hpp

#ifndef AGENT_HPP
#define AGENT_HPP 1

#include "Zaimoni.STL/rw.hpp"

#include <stddef.h>
#include <stdio.h>

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

class agent final
{
public:
	static const agent_species* species;		// external configuration would be strictly for modding support.
	static const size_t species_len;
private:
	size_t _species_index;

public:
	agent() = default;
	agent(FILE* src);
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(agent);
	void save(FILE* dest) const;

	const agent_species& type() const {return species[_species_index];}
};

}	// namespace iskandria

namespace zaimoni {
	template<>
	struct rw_mode<iskandria::agent>
	{
		enum {
			group_write = 3,
			group_read = 3
		};
	};
}

#endif
