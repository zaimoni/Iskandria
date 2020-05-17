#include "gridtile.hpp"
#include "gridspace.hpp"
#include "Zaimoni.STL/Pure.CPP/json.hpp"
#include <stdexcept>

namespace iskandria {
namespace grid {

floor_model::floor_model(const zaimoni::JSON& src) : floor_model()
{
	if (decltype(auto) x = src.get("id"); x && x->is_scalar()) _id = x->scalar();
	else throw std::runtime_error("did not load floor tile");
	if (decltype(auto) x = src.get("name"); x && x->is_scalar()) _name = x->scalar();
	else throw std::runtime_error("did not load floor tile");
	if (decltype(auto) x = src.get("path"); x && zaimoni::JSON::object == x->mode()) {
		if (decltype(auto) facing_img = x->get("nw"); facing_img && facing_img->is_scalar()) _img_path_nw = facing_img->scalar();
		if (decltype(auto) facing_img = x->get("ne"); facing_img && facing_img->is_scalar()) _img_path_ne = facing_img->scalar();
		if (decltype(auto) facing_img = x->get("se"); facing_img && facing_img->is_scalar()) _img_path_se = facing_img->scalar();
		if (decltype(auto) facing_img = x->get("sw"); facing_img && facing_img->is_scalar()) _img_path_sw = facing_img->scalar();
	}
}

std::vector<std::weak_ptr<floor_model> >& floor_model::cache()
{
	static std::vector<std::weak_ptr<floor_model> > ooao;
	return ooao;
}

std::shared_ptr<floor_model> floor_model::get(const std::string& id) {
	decltype(auto) _models = cache();
	size_t ub = _models.size();
	while (0 < ub) {
		auto x = _models[--ub].lock();
		if (!x) {
			_models.erase(_models.begin() + ub);
			continue;
		}
		if (id == x->_id) return x;
	}
	return decltype(_models.front().lock())();
}

std::shared_ptr<floor_model> floor_model::get(const zaimoni::JSON& src)
{
	std::shared_ptr<floor_model> ret(new floor_model(src));
	cache().push_back(ret);	// \todo handle duplicate-id configs
	return ret;
}

std::shared_ptr<floor_model> floor_model::read_synthetic_id(FILE* src)
{
	decltype(auto) _cache = cache();
	uintmax_t new_id = read_uintmax(_cache.size(), src);
	return (_cache.size() >= new_id && 0 < new_id) ? _cache[new_id - 1].lock() : decltype(_cache[new_id - 1].lock())();
}

void floor_model::write_synthetic_id(const std::shared_ptr<floor_model>& src, FILE* dest)
{
	decltype(auto) _cache = cache();
	const size_t ub = _cache.size();
	size_t i = ub;
	do {
		decltype(auto) x = _cache[--i];
		if (x.lock() == src) {
			write_uintmax(ub, i + 1, dest);
			return;
		}
	} while (0 < ub);
	write_uintmax(ub, 0, dest);
}

wall_model::wall_model(const zaimoni::JSON& src) : wall_model()
{
	if (decltype(auto) x = src.get("id"); x && x->is_scalar()) _id = x->scalar();
	else throw std::runtime_error("did not load wall tile");
	if (decltype(auto) x = src.get("name"); x && x->is_scalar()) _name = x->scalar();
	else throw std::runtime_error("did not load wall tile");
	if (decltype(auto) x = src.get("path"); x && zaimoni::JSON::object == x->mode()) {
		if (decltype(auto) facing_img = x->get("out_n"); facing_img && facing_img->is_scalar()) _img_path_outside_n = facing_img->scalar();
		if (decltype(auto) facing_img = x->get("out_w"); facing_img && facing_img->is_scalar()) _img_path_outside_w = facing_img->scalar();
		if (decltype(auto) facing_img = x->get("in_e"); facing_img && facing_img->is_scalar()) _img_path_inside_e = facing_img->scalar();
		if (decltype(auto) facing_img = x->get("in_s"); facing_img && facing_img->is_scalar()) _img_path_inside_s = facing_img->scalar();
	}
}

std::vector<std::weak_ptr<wall_model> >& wall_model::cache()
{
	static std::vector<std::weak_ptr<wall_model> > ooao;
	return ooao;
}

std::shared_ptr<wall_model> wall_model::get(const std::string& id)
{
	decltype(auto) _models = cache();
	size_t ub = _models.size();
	while (0 < ub) {
		auto x = _models[--ub].lock();
		if (!x) {
			_models.erase(_models.begin() + ub);
			continue;
		}
		if (id == x->_id) return x;
	}
	return decltype(_models.front().lock())();
}

std::shared_ptr<wall_model> wall_model::get(const zaimoni::JSON& src)
{
	std::shared_ptr<wall_model> ret(new wall_model(src));
	cache().push_back(ret);	// \todo handle duplicate-id configs
	return ret;
}

std::shared_ptr<wall_model> wall_model::read_synthetic_id(FILE* src)
{
	decltype(auto) _cache = cache();
	uintmax_t new_id = read_uintmax(_cache.size(), src);
	return (_cache.size() >= new_id && 0 < new_id) ? _cache[new_id - 1].lock() : decltype(_cache[new_id - 1].lock())();
}

void wall_model::write_synthetic_id(const std::shared_ptr<wall_model>& src, FILE* dest)
{
	decltype(auto) _cache = cache();
	const size_t ub = _cache.size();
	size_t i = ub;
	do {
		decltype(auto) x = _cache[--i];
		if (x.lock() == src) {
			write_uintmax(ub, i + 1, dest);
			return;
		}
	} while (0 < ub);
	write_uintmax(ub, 0, dest);
}

bool operator==(const map_cell& lhs, const map_cell& rhs)
{
	// empty less than non-empty;
	if (lhs._floor) {
		if (!rhs._floor) return false;
		if (*lhs._floor != *rhs._floor) return false;
	} else if (rhs._floor) return false;

	if (lhs._wall_w) {
		if (!rhs._wall_w) return false;
		if (*lhs._wall_w != *rhs._wall_w) return false;
	} else if (rhs._wall_w) return false;

	if (lhs._wall_n) {
		if (!rhs._wall_n) return false;
		if (*lhs._wall_n != *rhs._wall_n) return false;
	} else if (rhs._wall_n) return false;

	return lhs._flags == rhs._flags;
}

bool operator<(const map_cell& lhs, const map_cell& rhs)
{
	// empty less than non-empty;
	if (lhs._floor) {
		if (!rhs._floor) return false;
		// \todo rewrite once <=> available
		if (*lhs._floor < *rhs._floor) return true;
		if (*rhs._floor < *lhs._floor) return false;
	} else if (rhs._floor) return true;

	if (lhs._wall_w) {
		if (!rhs._wall_w) return false;
		// \todo rewrite once <=> available
		if (*lhs._wall_w < *rhs._wall_w) return true;
		if (*rhs._wall_w < *lhs._wall_w) return false;
	} else if (rhs._wall_w) return true;

	if (lhs._wall_n) {
		if (!rhs._wall_n) return false;
		// \todo rewrite once <=> available
		if (*lhs._wall_n < *rhs._wall_n) return true;
		if (*rhs._wall_n < *lhs._wall_n) return false;
	} else if (rhs._wall_n) return true;

	return lhs._flags < rhs._flags;
}

}
}	// namespace iskandria

#ifdef TEST_APP
#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	STRING_LITERAL_TO_STDOUT("starting main\n");

	typename iskandria::grid::cartesian<2>::coord_type dest;
	const decltype(dest) origin(7);	// for offset code 3 this is the center of a 16x16 voxel square
	const decltype(dest) quadrant_1(8);	// this test series is meant to be on an exact diagonal
	const decltype(dest) quadrant_3(6);
	bool all_tests_ok = true;

	INC_INFORM("origin: ");
	INC_INFORM(origin[0]);
	INC_INFORM(",");
	INFORM(origin[1]);

	INC_INFORM("quad 1: ");
	INC_INFORM(quadrant_1[0]);
	INC_INFORM(",");
	INFORM(quadrant_1[1]);

	INC_INFORM("quad 3: ");
	INC_INFORM(quadrant_3[0]);
	INC_INFORM(",");
	INFORM(quadrant_3[1]);

	// test series
#pragma start_copy identity_rotation
	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
#pragma end_copy
#pragma substitute quadrant_3 for quadrant_1 in identity_rotation
	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
#pragma end_substitute

#pragma start_copy origin_rotation
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
#pragma end_copy
#pragma for F in iskandria::compass::E,iskandria::compass::SE,iskandria::compass::S,iskandria::compass::SW,iskandria::compass::W,iskandria::compass::NW
#pragma substitute $F for iskandria::compass::NE in origin_rotation
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
#pragma end_substitute
#pragma done

	// reject diagonal rotations
	dest = quadrant_1;
#pragma start_copy reject_diagonal
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
#pragma end_copy
#pragma for F in iskandria::compass::SE,iskandria::compass::SW,iskandria::compass::NW
#pragma substitute $F for iskandria::compass::NE in reject_diagonal
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
#pragma end_substitute
#pragma done

	// reject E/w for partial-offsets
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	// verify that S i.e 180 degrees is period 2
	const auto Q1_S0(zaimoni::make<decltype(dest)>(6, 6));
	const auto Q1_S1(zaimoni::make<decltype(dest)>(7, 6));
	const auto Q1_S2(zaimoni::make<decltype(dest)>(6, 7));
	const auto Q1_S3(zaimoni::make<decltype(dest)>(7, 7));
	const auto Q3_S0(zaimoni::make<decltype(dest)>(8, 8));
	const auto Q3_S1(zaimoni::make<decltype(dest)>(9, 8));
	const auto Q3_S2(zaimoni::make<decltype(dest)>(8, 9));
	const auto Q3_S3(zaimoni::make<decltype(dest)>(9, 9));

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S2);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S2);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	// \todo verify that E i.e 270 degrees is period 4
	// \todo verify that W i.e 90 degrees is period 4
	const auto Q1_E0(zaimoni::make<decltype(dest)>(8, 6));
	const auto Q1_E3(zaimoni::make<decltype(dest)>(8, 7));
	const auto Q1_W0(zaimoni::make<decltype(dest)>(6, 8));
	const auto Q1_W3(zaimoni::make<decltype(dest)>(7, 8));
	const auto Q3_E0(zaimoni::make<decltype(dest)>(6, 8));
	const auto Q3_E3(zaimoni::make<decltype(dest)>(6, 9));
	const auto Q3_W0(zaimoni::make<decltype(dest)>(8, 6));
	const auto Q3_W3(zaimoni::make<decltype(dest)>(9, 6));

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_E0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_W0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_W0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_E0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_E3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_W3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_W3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_E3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_E0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_W0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_W0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_E0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_E3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_W3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_W3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_E3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}
#endif