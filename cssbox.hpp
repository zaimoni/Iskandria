#ifndef CSSBOX_HPP
#define CSSBOX_HPP 1
/* (C)2019 Kenneth Boyd, license: LICENSE_BOOST.txt */

#include <utility>
#include <memory>
#include <vector>

#include "Zaimoni.STL/c_bitmap.hpp"
#define POINT_IS_Z_VECTOR 1
#if POINT_IS_Z_VECTOR
#include "matrix.hpp"
#include "Zaimoni.STL/GDI/box.hpp"
#else
#include "Zaimoni.STL/Compiler.h"

// XXX These are not Rust-coherent
template<class T>
std::pair<T, T> operator+(std::pair<T, T> lhs, const std::pair<T, T>& rhs)
{
	lhs.first += rhs.first;
	lhs.second += rhs.second;
	return rhs;
}

template<class T>
std::pair<T, T> operator-(std::pair<T, T> lhs, const std::pair<T, T>& rhs)
{
	lhs.first -= rhs.first;
	lhs.second -= rhs.second;
	return rhs;
}
#endif

#define MULTITHREAD_DRAW 1

namespace css {

// following CSS keywords are wildcards
// initial: explicitly requests default value (does not require storage)
// inherit: explicitly requests using parent's value (does require storage)
// these two require being able to name any property, to route them correctly
namespace property {
	enum dir {
		LEFT = 0,
		TOP,
		RIGHT,
		BOTTOM	// highest index for margin, padding
	};
	enum {
		DIR_COUNT = BOTTOM+1
	};
	enum {
		MARGIN = 0,
		WIDTH = MARGIN+ DIR_COUNT,
		HEIGHT,	// last property for which auto is legal
		PADDING,
		MIN_WIDTH = PADDING + DIR_COUNT,
		MAX_WIDTH = MIN_WIDTH + DIR_COUNT,
		MIN_HEIGHT = MAX_WIDTH + DIR_COUNT,
		MAX_HEIGHT = MIN_HEIGHT + DIR_COUNT,
		CLEAR = PADDING+ DIR_COUNT,
		FLOAT,
		POSITION,
		Z_INDEX
	};
	enum {
		COUNT = Z_INDEX + 1
	};
}

class box {
#undef LEFT
#undef TOP
#undef RIGHT
#undef BOTTOM
#undef WIDTH
#undef HEIGHT
#undef REFLOW
public:
#if POINT_IS_Z_VECTOR
	typedef zaimoni::math::vector<int, 2> point;
	typedef zaimoni::gdi::box<point> rect;
#else
	typedef std::pair<int, int> point;
#endif
	enum auto_legal {
		LEFT = 0,
		TOP,
		RIGHT,
		BOTTOM,	// highest index for margin, padding
		WIDTH,
		HEIGHT	// highest index for auto flagging
	};
	static_assert(LEFT == css::property::MARGIN + css::property::LEFT);
	static_assert(TOP  == css::property::MARGIN + css::property::TOP);
	static_assert(RIGHT == css::property::MARGIN + css::property::RIGHT);
	static_assert(BOTTOM == css::property::MARGIN + css::property::BOTTOM);
	static_assert(WIDTH == css::property::WIDTH);
	static_assert(HEIGHT == css::property::HEIGHT);

	enum clear_float_legal {
		CF_NONE = 0,
		CF_LEFT,
		CF_RIGHT,
		CF_BOTH_INHERIT
	};
	static_assert(3 == CF_BOTH_INHERIT);
	enum position_legal {
		POS_STATIC = 0,
		POS_RELATIVE,
		POS_ABSOLUTE,
		POS_FIXED,
		POS_INHERIT	// for formal completeness, not obviously useful
	};
protected:
	unsigned char _auto;	// bitmap: margins, width, height
	unsigned char _clear_float;	// clear, float encodings; also position property encoding
	typename zaimoni::bitmap<css::property::COUNT>::type _inherited;
	typename zaimoni::bitmap<css::property::COUNT>::type _reflow;
	point _origin;	// (x,y) i.e. (left,top): relative to container
	point _screen;	// (x,y) i.e. (left,top): global
private:
	point _size;
	point _size_min;
	point _size_max;
	int _margin[css::property::DIR_COUNT];
	int _padding[css::property::DIR_COUNT];
	int _z_index;
	std::weak_ptr<box> _parent;
	std::shared_ptr<box> _self;
#if MULTITHREAD_DRAW
	static unsigned int _recalc_fakelock;
#endif
protected:
	box(bool bootstrap = false);
	ZAIMONI_DEFAULT_COPY_ASSIGN(box);
public:
	virtual ~box() = default;

	// auto values
	bool is_auto(auto_legal src) const { return _auto & (1ULL << src); }
	void set_auto(auto_legal src);

	// width/height
	auto size() const { return _size; }
#if POINT_IS_Z_VECTOR
	int width() const { return _size[0]; }
	int height() const { return _size[1]; }
	int min_width() const { return _size_min[0]; }
	int min_height() const { return _size_min[1]; }
	int max_width() const { return _size_max[0]; }
	int max_height() const { return _size_max[1]; }
#else
	int width() const { return _size.first; }
	int height() const { return _size.second; }
	int min_width() const { return _size_min.first; }
	int min_height() const { return _size_min.second; }
	int max_width() const { return _size_max.first; }
	int max_height() const { return _size_max.second; }
#endif

	void width(int w);
	void height(int h);
	void min_width(int w);
	void min_height(int h);
	void max_width(int w);
	void max_height(int h);

	// padding, margin
	template<int src> int padding() const {
		static_assert(0 <= src && WIDTH > src);
		return _padding[src];
	}

	template<int src, int src2> auto padding() const {
		static_assert(0 <= src && WIDTH > src2 && src+1==src2);
#if POINT_IS_Z_VECTOR
		return point(_padding + src);
#else
		return point(_padding[src], _padding[src2]);
#endif
	}

	template<int src> int margin() const {
		static_assert(0 <= src && WIDTH > src);
		return _margin[src];
	}
	template<int src, int src2> auto margin() const {
		static_assert(0 <= src && WIDTH > src2&& src + 1 == src2);
#if POINT_IS_Z_VECTOR
		return point(_margin + src);
#else
		return point(_margin[src], _margin[src2]);
#endif
	}
	template<int src> void _set_margin(int x) {
		static_assert(0 <= src && WIDTH > src);
		_reflow &= ~(1ULL << src);	// uses css::property::MARGIN == auto_legal::LEFT, etc.
		_margin[src] = x;
	}
	template<int src> void set_margin(int x) {
		static_assert(0 <= src && WIDTH > src);
		_auto &= ~(1ULL << src);
		_set_margin<src>(x);
	}

	// will include border once we're tracking it
	int full_width() const { return width() + padding<LEFT>() + padding<RIGHT>(); }
	int full_height() const { return height() + padding<TOP>() + padding<BOTTOM>(); }
	int outer_width() const { return width() + padding<LEFT>() + padding<RIGHT>() + margin<LEFT>() + margin<RIGHT>(); }
	int outer_height() const { return height() + padding<TOP>() + padding<BOTTOM>() + margin<TOP>() + margin<BOTTOM>(); }

	auto full_anchor() const { return _origin - padding<LEFT, TOP>(); }
//	auto border_anchor() const {....}
	auto outer_anchor() const { return _origin - padding<LEFT, TOP>() - margin<LEFT, TOP>(); }

	// layout boxes
#if POINT_IS_Z_VECTOR
	auto inner_box() const { return rect(_origin, _origin + _size); }
	auto outer_box() const { return rect(_origin - padding<LEFT,TOP>() - margin<LEFT,TOP>(), _origin + _size+ padding<RIGHT, BOTTOM>() + margin<RIGHT, BOTTOM>()); }
#endif

	// clear/float
	clear_float_legal CSS_clear() const { return (clear_float_legal)(_clear_float & 3U); }
	void set_clear(clear_float_legal src) { _clear_float &= ~3U; _clear_float |= src; }
	clear_float_legal CSS_float() const;
	void set_float(clear_float_legal src) { _clear_float &= ~(3U << 2); _clear_float |= (src << 2); }

	// position attribute
	position_legal position() const;
	void set_position(position_legal src) { _clear_float &= ~(7U << 4); _clear_float |= (src << 4); }

	// z-index
	int z_index() const { return _z_index; }
	void z_index(int src) { _z_index = src; }

	// layout recalculation (unsure about access control here)
	std::shared_ptr<box> parent() const { return _parent.lock(); }
	auto origin() const { return _origin; }
	virtual void remove(std::shared_ptr<box> gone);
	void disconnect() { _self.reset(); };

	// these should be abstract, but bringing up a new implementation may need waiving that
#ifdef PROTOTYPING_CSS
	virtual bool flush() { return false; };
	virtual int need_recalc() const { return 0; };	// return value is C error code convention; 0 no-action, negative error, positive action code
	virtual void draw() const {};
#else
	virtual bool flush() = 0;
	virtual int need_recalc() const = 0;	// return value is C error code convention; 0 no-action, negative error, positive action code
	virtual void draw() const = 0;
#endif
	virtual void force_size(int w, int h) {};	// should be abstract

	void recalc();
	virtual void set_origin(point logical_origin);
	virtual void screen_coords(point logical_origin);

	void horizontal_centering(int ub, point origin);
	void vertical_centering(int ub, point origin);
	bool request_horz_margins();
	bool request_vert_margins();
protected:
	virtual void schedule_reflow();
	void set_parent(std::shared_ptr<box>& src);
	void _width(int w);
	void _height(int h);

private:
#ifdef PROTOTYPING_CSS
	virtual void _recalc(int code) {};
#else
	virtual void _recalc(int code) = 0;
#endif
};

class box_dynamic : public box
{
private:
	std::vector<std::shared_ptr<box> > _contents;
	size_t _redo_layout;	// the lowest index of a box that needs its position finalized
public:
	// have to allow public default-construction because the top-level box isn't tied to content
	box_dynamic(bool bootstrap = false) : box(bootstrap),_redo_layout(0) {}
	ZAIMONI_DEFAULT_COPY_ASSIGN(box_dynamic);
	~box_dynamic();

	// content management
	void append(std::shared_ptr<box> src);
	virtual void remove(std::shared_ptr<box> gone);

	virtual void draw() const;
	virtual void screen_coords(point logical_origin);
	virtual void force_size(int w, int h);
protected:
	virtual void schedule_reflow();
private:
	virtual bool flush();
	virtual int need_recalc() const;
	virtual void _recalc(int code);
};

}	// namespace css

#endif
