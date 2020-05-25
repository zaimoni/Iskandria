#include "input_manager.hpp"
#include "display_manager.hpp"
#include "telephoto_grid.hpp"
#include "Zaimoni.STL/Pure.CPP/json.hpp"

// VAPORWARE: isometric view test driver, with a boardable AA gun
static bool AA_chessboard()
{
	isk::InputManager::close_menu();	// request closing our invoking menu only

	// \todo load required grid tiles
	zaimoni::JSON paths;
	zaimoni::JSON tile_config;
	tile_config.set("id", "lg_floor");
	tile_config.set("name", "light gray floor");
	paths.set("nw", "cgi:floor:#CCC");
	tile_config.set("path", paths);

	auto w_floor = iskandria::grid::floor_model::get(tile_config);

	tile_config.set("id", "g_floor");
	tile_config.set("name", "gray floor");
	paths.set("nw", "cgi:floor:#999999");
	tile_config.set("path", paths);

	auto g_floor = iskandria::grid::floor_model::get(tile_config);

	tile_config.set("id", "by_wall");
	tile_config.set("name", "blue-yellow wall");
	paths.reset();
	paths.set("out_n", "cgi:rwall:#0000FF");
	paths.set("in_s", "cgi:lwall:#FFFF00");
	paths.set("out_w", "cgi:lwall:#0000FF");	// in theory computable from out_n
	paths.set("in_e", "cgi:rwall:#FFFF00");		// in theory computable from in_s
	tile_config.set("path", paths);

	auto by_wall = iskandria::grid::wall_model::get(tile_config);

	tile_config.set("id", "rg_wall");
	tile_config.set("name", "red-green wall");
	paths.reset();
	paths.set("out_n", "cgi:rwall:#F00");
	paths.set("in_s", "cgi:lwall:#00FF00");
	paths.set("out_w", "cgi:lwall:#F00");	// in theory computable from out_n
	paths.set("in_e", "cgi:rwall:#00FF00");		// in theory computable from in_s
	tile_config.set("path", paths);

	auto rg_wall = iskandria::grid::wall_model::get(tile_config);

	// leave z-level 1 (mostly) blank
	iskandria::grid::cartesian<3> staging({10, 10, 2});	// suspected assertion failure

	// baseline map cells
	iskandria::grid::map_cell w_square;
	w_square.set_floor("lg_floor");

	iskandria::grid::map_cell g_square;
	g_square.set_floor("g_floor");

	decltype(auto) dims = staging.domain().br_c();
	auto scan(dims);
	scan[2] = 0;

	while (0 <= --scan[0]) {
		scan[1] = dims[1];
		while (0 <= --scan[1]) {
			decltype(auto) cell = staging.grid(scan);
			assert(cell);
			*cell = (0 == (scan[0] + scan[1]) % 2) ? w_square : g_square;
		}
	}

	std::shared_ptr< isk::Wrap<iskandria::grid::cartesian<3> > > test_map(new isk::Wrap<iskandria::grid::cartesian<3> >(std::move(staging)));
	isk::Wrap<iskandria::grid::cartesian<3> >::track(test_map);

	// \todo set up map data mockup and camera viewpoint
	// \todo install command processing menu
	auto terminate_menu = [=]() mutable {	// must capture by value so that std::weak_ptrs don't wink out prematurely
		test_map.reset();

		// tile data
		w_floor.reset();
		g_floor.reset();
		by_wall.reset();
		rg_wall.reset();
		return isk::InputManager::close_menus();	// close everything at/above our menu
	};

	isk::textmenu AA_map_controls;
	sf::Event::KeyEvent hotkey = { sf::Keyboard::Key::Escape, false, false, false, false };
	AA_map_controls.add_entry("Esc)ape", hotkey, terminate_menu);	// remove this once we have real content
	AA_map_controls.add_entry(hotkey, terminate_menu);
	isk::InputManager::get().install(AA_map_controls);
	return true;
}

std::function<bool(void)> test_drivers()	// VAPORWARE referenced in main.cpp
{	// VAPORWARE: installs a menu indexing test driver options when run.
	return AA_chessboard;
}

