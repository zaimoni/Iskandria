#ifndef WORLD_VIEW_HPP
#define WORLD_VIEW_HPP 1

#include "matrix.hpp"
#include "Zaimoni.STL/GDI/box.hpp"

// Subclasses of this don't directly coordinate drawing to the screen; instead they format 
// CSS appropriately.
class WorldView
{
protected:
	WorldView() = default;
	WorldView(const WorldView& src) = default;
	WorldView(WorldView&& src) = default;
	WorldView& operator=(const WorldView& src) = default;
	WorldView& operator=(WorldView&& src) = default;
public:
	virtual ~WorldView() = default;

	// returns true if and only if "modal" i.e. should not draw anything earlier in the stack
	virtual bool draw(const zaimoni::gdi::box<zaimoni::math::vector<int,2> >& view_logical) = 0;
};

#endif
