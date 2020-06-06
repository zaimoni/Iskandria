// gridspace.hpp

#ifndef GRIDSPACE_HPP
#define GRIDSPACE_HPP

#include "matrix.hpp"
#include "gridtile.hpp"
#include "Zaimoni.STL/GDI/box.hpp"

namespace iskandria {
namespace grid {

// requirements:
// * we must be able to state the position of a 2-d grid in the co-rotating coordinate system of a celestial object
// * we must be able to state the position of an agent or craft in a 2-d grid
// * we must be able to state the position of an agent in a craft

template<size_t N>
class cartesian final
{
	static_assert(2 <= N);
public:
	using coord_type = zaimoni::math::vector<ptrdiff_t, N>;
	// encoding for the orientation will end up dimension-dependent.
	// 2020-05-31: Unclear whether we need to be reflection-aware (for now assume not).
	// * for each two dimensions, full generality would require one copy of a proxy for the unit circle S^1.
	// ** our proxy is the canonical XCOM-like direction set
	// * we also would want a proxy for the subgroup of even permutation matrices of the permutation group Perm(n)
	// ** this will "blow up" as Perm(n) has size n!
	using orientation = unsigned char;	// will end up dimension-dependent
private:
//	std::vector<std::weak_ptr<agent> > _agents;
//	std::vector<std::weak_ptr<craft> > _crafts;
	zaimoni::gdi::box<coord_type> _domain;
	size_t _size;	// cache
	std::vector<map_cell> _terrain;	// in canonical NW facing
public:
	cartesian() = default;
	cartesian(const coord_type& src) : _domain(coord_type(), src), _size(size(_domain)), _terrain(_size) {}
	cartesian(const zaimoni::gdi::box<coord_type>& src) : _domain(src), _size(size(_domain)), _terrain(_size) {}
	cartesian(FILE* src)
	{
		zaimoni::read(_domain.tl_c(), src);
		zaimoni::read(_domain.br_c(), src);
		_size = size(_domain);
		size_t n = _size;
		while (0 < n) {
			_terrain.push_back(map_cell(src));
			--n;
		}
	}

	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(cartesian);
	void save(FILE* dest) const
	{
		zaimoni::write(_domain.tl_c(), dest);
		zaimoni::write(_domain.br_c(), dest);
		for (const map_cell& t : _terrain) t.save(dest);
	}

	// thin-forwarders
	const zaimoni::gdi::box<coord_type>& domain() const { return _domain; }	// insecure
	bool contains(const coord_type& src) const { return _domain.contains(src); }

	// coordinates
	static bool is_representable(const zaimoni::gdi::box<coord_type>& src) {
		size_t ub = N;
		size_t ret = 1;
		size_t tolerate = (size_t)(-1);
		do {
			--ub;
			const auto upper = src.br_c()[ub];
			const auto lower = src.tl_c()[ub];
			// \todo convert following to Augment.STL/typetraits wrapper
			if (lower >= upper) return false;
			const auto test = zaimoni::pos_diff(upper, lower);
			if (test > tolerate) return false;
			ret *= test;
			tolerate /= test;
		} while (0 < ub);
		return true;
	}
	static size_t size(const zaimoni::gdi::box<coord_type>& src) {
		size_t ub = N;
		size_t ret = 1;
		size_t tolerate = (size_t)(-1);
		do {
			--ub;
			const auto upper = src.br_c()[ub];
			const auto lower = src.tl_c()[ub];
			if (lower >= upper) throw std::runtime_error("denormalized");
			const auto test = zaimoni::pos_diff(upper, lower);
			if (test > tolerate) throw std::runtime_error("coordinate range not representable");
			ret *= test;
			tolerate /= test;
		} while (0 < ub);
		return ret;
	}
	size_t size() const { return _size; }

	// these are for canonical facing
	map_cell* grid(const coord_type& src) {
		size_t i = index(src);
		if ((size_t)(-1) == i) return 0;
		if (_terrain.size() <= i) return 0;	// error condition?
		return _terrain.data() + i;
	}
	const map_cell* grid(const coord_type& src) const {
		size_t i = index(src);
		if ((size_t)(-1) == i) return 0;
		if (_terrain.size() <= i) return 0;	// error condition?
		return _terrain.data() + i;
	}

	// caller handles opacity decisions/recursion, etc.
	std::vector<std::string> render_tiles_isometric(const coord_type& src, const orientation o) const {
		auto primary_facing = o % iskandria::compass::XCOM_STRICT_UB;
#ifndef NDEBUG
		switch (primary_facing)
		{
		case iskandria::compass::N:
		case iskandria::compass::E:
		case iskandria::compass::S:
		case iskandria::compass::W:
			assert(0 && "orientation out of range");
		}
#endif
		std::vector<std::string> ret;
		if (want_e_cell(primary_facing)) {
			decltype(auto) src_e(src);
			src_e[0] += 1;
			if (auto gr = grid(src_e)) {
				std::string img_key;
				if (map_cell::n_wall_ok_e(primary_facing)) {
					if (!(img_key = gr->n_wall_e()).empty()) ret.push_back(std::move(img_key));
				}
				if (map_cell::w_wall_ok_e(primary_facing)) {
					if (!(img_key = gr->w_wall_e()).empty()) ret.push_back(std::move(img_key));
				}
			}
		}
		if (want_s_cell(primary_facing)) {
			decltype(auto) src_s(src);
			src_s[1] += 1;
			if (auto gr = grid(src_s)) {
				std::string img_key;
				if (map_cell::n_wall_ok_s(primary_facing)) {
					if (!(img_key = gr->n_wall_s()).empty()) ret.push_back(std::move(img_key));
				}
				if (map_cell::w_wall_ok_s(primary_facing)) {
					if (!(img_key = gr->w_wall_s()).empty()) ret.push_back(std::move(img_key));
				}
			}
		}
		// actual view target
		if (auto gr = grid(src)) {
			std::string img_key;
			if (map_cell::n_wall_ok(primary_facing)) {
				if (!(img_key = gr->n_wall(primary_facing)).empty()) ret.push_back(std::move(img_key));
			}
			if (map_cell::w_wall_ok(primary_facing)) {
				if (!(img_key = gr->w_wall(primary_facing)).empty()) ret.push_back(std::move(img_key));
			}
			if (!(img_key = gr->floor(primary_facing)).empty()) ret.push_back(std::move(img_key));
			// \todo intercalate map objects, agents, etc. here
		}
		return ret;
	}

	size_t index(const coord_type& src) const {
		size_t ub = N;
		size_t ret = 0;
		do {
			--ub;
			const auto upper = _domain.br_c()[ub];
			const auto lower = _domain.tl_c()[ub];
			const auto test_coord = src[ub];
			if (lower > test_coord || upper <= test_coord) return (size_t)(-1);	// out of bounds
			ret *= zaimoni::pos_diff(upper, lower);
			ret += zaimoni::pos_diff(test_coord, lower);
			if ((size_t)(-1) == ret) return (size_t)(-1);	// predicted to denormalize
		} while (0 < ub);
		return ret;
	}
	coord_type coords(size_t src) const {
		coord_type ret(0);
		size_t i = 0;
		do {
			const auto upper = _domain.br_c()[i];
			const auto lower = _domain.tl_c()[i];
			const auto test = zaimoni::pos_diff(upper, lower);
			ret[i] = src % test;
			src /= test;
		} while (++i < N);
		return ret;
	}
};

}	// namespace grid
}	// namespace iskandria

namespace zaimoni {
	template<size_t N>
	struct rw_mode<iskandria::grid::cartesian<N> >
	{
		enum {
			group_write = 3,
			group_read = 3
		};
	};
}

#endif
