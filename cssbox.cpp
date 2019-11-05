#include "cssbox.hpp"
#include <string.h>
#include <limits>
#include <map>

#include "Zaimoni.STL/Logging.h"
#if MULTITHREAD_DRAW
#include "Zaimoni.STL/ref_inc.hpp"
#endif

namespace css {

using zaimoni::math::clamp_lb;
using zaimoni::math::clamp_ub;

#if MULTITHREAD_DRAW
unsigned int box::_recalc_fakelock = 0;
#endif

box::box()
: _auto((1ULL << HEIGHT) | (1ULL << WIDTH)), _clear_float(0), _inherited(0), _reflow((1ULL << css::property::HEIGHT) | (1ULL << css::property::WIDTH)),
#if POINT_IS_Z_VECTOR
  _origin(0), _screen(0), _size(0), _size_min(0), _size_max(std::numeric_limits<int>::max())
#else
  _origin(0, 0), _screen(0, 0), _size(0, 0), _size_min(0, 0), _size_max(std::numeric_limits<int>::max(), std::numeric_limits<int>::max())
#endif
{
	memset(_margin, 0, sizeof(_margin));
	memset(_padding, 0, sizeof(_padding));
}

void box::set_auto(auto_legal src) {
	if (!is_auto(src)) {
		_auto |= (1ULL << src);
		_inherited |= ~(1ULL << src);
		_reflow |= (1ULL << src);	// uses auto_legal::LEFT == css::property::MARGIN, etc.
	}
}

void box::width(int w) {
	if (0 > w) throw std::runtime_error("negative width");
	_auto &= ~(1ULL << WIDTH);
	_width(w);
}

// we intentionally choose implementation-reserved names
#if POINT_IS_Z_VECTOR
#define __width _size[0]
#define __height _size[1]
#define __min_width _size_min[0]
#define __min_height _size_min[1]
#define __max_width _size_max[0]
#define __max_height _size_max[1]
#else
#define __width _size.first
#define __height _size.second
#define __min_width _size_min.first
#define __min_height _size_min.second
#define __max_width _size_max.first
#define __max_height _size_max.second
#endif

void box::_width(int w) {
	_reflow &= ~(1ULL << css::property::WIDTH);
	if (__width != w) {
		__width = w;
		if ((_auto & (1ULL << LEFT))) _reflow |= (1ULL << css::property::MARGIN + css::property::LEFT);
		if ((_auto & (1ULL << RIGHT))) _reflow |= (1ULL << css::property::MARGIN + css::property::RIGHT);
		schedule_reflow();
		auto parent(_parent.lock());
		if (parent) parent->schedule_reflow();
	}
}

void box::height(int h) {
	if (0 > h) throw std::runtime_error("negative height");
	_auto &= ~(1ULL << HEIGHT);
	_height(h);
}

void box::_height(int h) {
	_reflow &= ~(1ULL << css::property::HEIGHT);
	if (__height != h) {
		__height = h;
		if ((_auto & (1ULL << TOP))) _reflow |= (1ULL << css::property::MARGIN + css::property::TOP);
		if ((_auto & (1ULL << BOTTOM))) _reflow |= (1ULL << css::property::MARGIN + css::property::BOTTOM);
		schedule_reflow();
		auto parent(_parent.lock());
		if (parent) parent->schedule_reflow();
	}
}

void box::min_width(int w) {
	if (0 > w) throw std::runtime_error("negative width");
	__min_width = w;
	if (w > __max_width) max_width(w);
	else if (w > __width) width(w);
}

void box::min_height(int h) {
	if (0 > h) throw std::runtime_error("negative height");
	__min_height = h;
	if (h > __max_height) max_height(h);
	else if (h > __height) height(h);
}

void box::max_width(int w) {
	if (0 > w) throw std::runtime_error("negative width");
	__max_width = w;
	if (w < __min_width) min_width(w);
	else if (w < __width) width(w);
}

void box::max_height(int h) {
	if (0 > h) throw std::runtime_error("negative height");
	__max_height = h;
	if (h < __min_height) min_height(h);
	else if (h < __height) height(h);
}

int box::effective_max_width() const
{
	auto ret = max_width();
	if (auto mother = parent()) {
		auto test = mother->width();
		if (0 < test && ret > test) ret = test;
	}
	return ret;
}

int box::effective_max_height() const
{
	auto ret = max_height();
	if (auto mother = parent()) {
		auto test = mother->height();
		if (0 < test && ret > test) ret = test;
	}
	return ret;
}


void box::set_origin(const point logical_origin) {
	_origin = logical_origin + margin<LEFT, TOP>() + padding<LEFT, TOP>();
}

void box::screen_coords(const point logical_origin) {
	_screen = _origin + logical_origin;
}

void box_dynamic::screen_coords(point logical_origin) {
	box::screen_coords(logical_origin);
	for (auto& x : _contents) x->screen_coords(_screen);
}

box::clear_float_legal box::CSS_float() const {
	return (clear_float_legal)((_clear_float >> 2) & 3U);
}

box::position_legal box::position() const {
	return (position_legal)((_clear_float >> 4) & 7U);
}

box_dynamic::box_dynamic(bool bootstrap) : _redo_layout(0) {
	if (bootstrap) _self = decltype(_self)(this);
}

box_dynamic::~box_dynamic()
{
	if (!_contents.empty()) for (auto& x : _contents) if (x) x->disconnect();
}

// content management
void box_dynamic::set_parent(std::shared_ptr<box>& src) {
	if (src) {
		src->set_self();
		src->set_parent(_self);
		if (_auto & (1ULL << WIDTH)) _reflow |= (1ULL << css::property::WIDTH);
		if (_auto & (1ULL << HEIGHT)) _reflow |= (1ULL << css::property::HEIGHT);
	}
}

void box::set_parent(std::shared_ptr<box_dynamic>& src)
{
	_parent = src;
	// trigger all inherited properties
	int i;
	if (_inherited) {
		i = css::property::COUNT;
		while (0 < i--) if (_inherited & (1ULL << i)) inherit(i, src);
	}
	while (0 < i--) if (_auto & (1ULL << i)) _reflow |= (1ULL << i);	// trigger auto properties here
}

bool box::has_no_box() const
{
	// \todo display:none has no box
	if (0 >= width() || 0 >= height()) return true;
	return false;
}

bool box::can_reflow() const
{
	if (has_no_box()) return false;	// might be able to pre-condition this test
	if (POS_MIN_NOFLOW <= position()) return false;
	return true;
}

void box::remove(std::shared_ptr<box> gone) {}
void box_dynamic::remove(std::shared_ptr<box> gone) {
	size_t ub = _contents.size();
	do {
		if (_contents[ub] == gone) _contents.erase(_contents.begin() + ub);
		else _contents[ub]->remove(gone);
	} while(0<ub);
	gone->disconnect();
}

void box_dynamic::append(std::shared_ptr<box> src) {
	if (src) {
		bool resize_ok = parent() ? true : false;
		// absolute positioning will prevent resizing; possibly other cases
		if (resize_ok) {
			if (_auto & (1ULL << WIDTH)) _reflow |= (1ULL << css::property::WIDTH);
			if (_auto & (1ULL << HEIGHT)) _reflow |= (1ULL << css::property::HEIGHT);
		}
		set_parent(src);
		_contents.push_back(src);
		size_t layout_calc = _contents.size();
		if (!_redo_layout) _redo_layout = _contents.size();
	}
}

void box::recalc() {
#if MULTITHREAD_DRAW
	zaimoni::ref_semaphore<unsigned int> lock(_recalc_fakelock);
	if (!lock.locked() && !_parent.lock()) return;
#endif

	flush();
	std::pair<int, size_t> code;
	while (0 < (code = need_recalc()).first) _recalc(code);
}

// used by the outermost dynamic box i.e. top window
void box_dynamic::force_size(int w, int h)
{
	if (0 > w) throw std::runtime_error("negative width");
	if (0 > h) throw std::runtime_error("negative height");
	// lock down both min and max dimensions
	const int w_delta = w - width();
	const int h_delta = h - height();
	if (0 == w_delta && 0 == h_delta) return;
	min_width(w);
	max_width(w);
	min_height(h);
	max_height(h);
	// scan contents for issues.  Non-const so can delete, but shouldn't do a deep check
	bool need_reflow = false;
	auto i = _contents.size();
	while (0 < i) {
		auto& x = _contents[--i];
		if (!x) {	// null
			_contents.erase(_contents.begin() + i);
			continue;
		}
		if (0 != w_delta && x->request_horz_margins()) need_reflow = true;
		if (0 != h_delta && x->request_vert_margins()) need_reflow = true;
		// other checks go here
	}
	if (need_reflow) recalc();
}

bool box_dynamic::flush() {
	auto i = _contents.size();
	while(0 < i) {
		if (   !_contents[--i] 	// null.
			||  _contents[  i]->flush()) {	// should remove
			_contents.erase(_contents.begin() + i);
//			continue;
		}
	}
	return _contents.empty();
}

css::box::layout_op box_dynamic::need_recalc() const
{
	// check contents first;
	if (!_contents.empty()) {	// self-recalc check
		auto i = _contents.size();
		do {
			if (0 < _contents[--i]->need_recalc().first) return layout_op(1,i);
		} while (0 < i);
		i = _contents.size();
		do {
			if (_contents[--i]->need_horz_margins()) return layout_op(2, i);
		} while (0 < i);
		i = _contents.size();
		do {
			if (_contents[--i]->need_vert_margins()) return layout_op(3, i);
		} while (0 < i);
	}
	// \todo: actually reposition contents
	// parallel: css_SFML.hpp
	if ((_auto & (1ULL << HEIGHT)) && (_auto & (1ULL << WIDTH))) {
		if (_reflow & ((1ULL << css::property::HEIGHT) | (1ULL << css::property::WIDTH))) return layout_op(4,0);
	}
	return layout_op(0,0);
}

void box::horizontal_centering(const int ub, const point local_origin)
{
	int span = full_width();
	int sides = 0;
	if (_auto & (1ULL << LEFT)) sides++;
	else span += margin<LEFT>();
	if (_auto & (1ULL << RIGHT)) sides++;
	else span += margin<RIGHT>();
	if (ub >= span) {
		const int delta = ub - span;
		if (1 == sides) {
			if (_auto & (1ULL << LEFT)) _set_margin<LEFT>(delta);
			else _set_margin<RIGHT>(delta);
		} else {
			int half = delta / sides;
			_set_margin<LEFT>(half);
			_set_margin<RIGHT>(delta - half);
		}
		set_origin(local_origin);
		return;
	}
	assert(0 && "unhandled horizontal center request");
	// fail sensibly, if non-conformantly
	_set_margin<LEFT>(0);
	_set_margin<RIGHT>(0);
	set_origin(local_origin);
}

void box::vertical_centering(const int ub, const point local_origin)
{
	int span = full_width();
	int sides = 0;
	if (_auto & (1ULL << TOP)) sides++;
	else span += margin<TOP>();
	if (_auto & (1ULL << BOTTOM)) sides++;
	else span += margin<BOTTOM>();
	if (ub >= span) {
		const int delta = ub - span;
		if (1 == sides) {
			if (_auto & (1ULL << TOP)) _set_margin<TOP>(delta);
			else _set_margin<BOTTOM>(delta);
		} else {
			int half = delta / sides;
			_set_margin<TOP>(half);
			_set_margin<BOTTOM>(delta - half);
		}
		set_origin(local_origin);
		return;
	}
	assert(0 && "unhandled vertical center request");
	// fail sensibly, if non-conformantly
	_set_margin<TOP>(0);
	_set_margin<BOTTOM>(0);
	set_origin(local_origin);
}

bool box::request_horz_margins()
{
	bool ret = false;
	if (_auto & (1ULL << LEFT)) {
		_reflow |= (1ULL << css::property::MARGIN + css::property::LEFT);
		ret = true;
	}
	if (_auto & (1ULL << RIGHT)) {
		_reflow |= (1ULL << css::property::MARGIN + css::property::RIGHT);
		ret = true;
	}
	return ret;
}

int box::need_horz_margins() const
{
	int ret = 0;
	if ((_auto & (1ULL << LEFT)) && (_reflow & (1ULL << css::property::MARGIN + css::property::LEFT))) ret += 1;
	if ((_auto & (1ULL << RIGHT)) && (_reflow & (1ULL << css::property::MARGIN + css::property::RIGHT))) ret += 2;
	return ret;
}

bool box::request_vert_margins()
{
	bool ret = false;
	if (_auto & (1ULL << TOP)) {
		_reflow |= (1ULL << css::property::MARGIN + css::property::TOP);
		ret = true;
	}
	if (_auto & (1ULL << BOTTOM)) {
		_reflow |= (1ULL << css::property::MARGIN + css::property::BOTTOM);
		ret = true;
	}
	return ret;
}

int box::need_vert_margins() const
{
	int ret = 0;
	if ((_auto & (1ULL << TOP)) && (_reflow & (1ULL << css::property::MARGIN + css::property::TOP))) ret += 1;
	if ((_auto & (1ULL << BOTTOM)) && (_reflow & (1ULL << css::property::MARGIN + css::property::BOTTOM))) ret += 2;
	return ret;
}

void box_dynamic::_recalc(layout_op& code)
{
	if (0 >= code.first) return;
	if (1 == code.first) {
		_contents[code.second]->recalc();
		if (!_redo_layout || _redo_layout > code.second) _redo_layout = code.second;	// \todo lock this update down to when it's needed (i.e. border-box actually changes)
		while (0 < code.second) {
			if (0 < _contents[--code.second]->need_recalc().first) {
				_contents[code.second]->recalc();
				if (!_redo_layout || _redo_layout > code.second) _redo_layout = code.second;	// \todo lock this update down to when it's needed (i.e. border-box actually changes)
			}
		}
		return;
	}
	if (2 == code.first) {
		_contents[code.second]->horizontal_centering(width(), origin());
		if (!_redo_layout || _redo_layout > code.second) _redo_layout = code.second;	// \todo lock this update down to when it's needed (i.e. border-box actually changes)
		while (0 < code.second) {
			if (_contents[--code.second]->need_horz_margins()) {
				_contents[code.second]->horizontal_centering(width(), origin());
				if (!_redo_layout || _redo_layout > code.second) _redo_layout = code.second;	// \todo lock this update down to when it's needed (i.e. border-box actually changes)
			}
		}
		return;
	}
	if (3 == code.first) {
		_contents[code.second]->vertical_centering(height(), origin());
		if (!_redo_layout || _redo_layout > code.second) _redo_layout = code.second;	// \todo lock this update down to when it's needed (i.e. border-box actually changes)
		while (0 < code.second) {
			if (_contents[--code.second]->need_vert_margins()) {
				_contents[code.second]->vertical_centering(height(), origin());
				if (!_redo_layout || _redo_layout > code.second) _redo_layout = code.second;	// \todo lock this update down to when it's needed (i.e. border-box actually changes)
			}
		}
		return;
	}
	const auto c_size = _contents.size();
	// parallel: css_SFML.hpp
	switch(code.first)
	{
	case 4:	// size
		if (reflow()) return;
		assert(0 && "unhandled resize request");
		_reflow &= ~((1ULL << css::property::WIDTH) | (1ULL << css::property::HEIGHT));	// do not infinite-loop
		break;
	}
}

void box_dynamic::draw() const
{
	if (_contents.empty()) return;
	// We will need to be *much* smarter about this; anything that is off-screen or completely hidden by higher z-index need not be done here
	for(auto& x : _contents) {
		x->draw();
	}
}

// following likely should be ported to Zaimoni.STL/GDI/box.hpp
static int _interval_compare(int x_lb, int x_ub, int y_lb, int y_ub)
{
	if (x_ub <= y_lb) return -3;	// disjoint
	if (y_ub <= x_lb) return 3;		// disjoint
	// at this point: x_ub > y_lb, y_ub > x_lb: we have some sort of intersection
	if (x_lb <= y_lb && x_ub < y_ub) return -2;	// clean difference: lhs < rhs
	if (y_lb <= x_lb && y_ub < x_ub) return  2;	// clean difference: lhs > rhs

	if (x_lb == y_lb && x_ub == y_ub) return 0;	// equal
	if (x_lb <= y_lb && y_ub <= x_ub) return -1;	// lhs contains rhs
	if (y_lb <= x_lb && x_ub <= y_ub) return 1;		// rhs contains lhs
	throw std::logic_error("non-exhaustive linear interval classification");
}

static bool _intersects(const css::box::rect& lhs, const css::box::rect& rhs, std::array<int,2>& codes)
{
	size_t ub = 2;
	do {
		--ub;
		codes[ub] = _interval_compare(lhs.tl_c()[ub], lhs.br_c()[ub], rhs.tl_c()[ub], rhs.br_c()[ub]);
		if (3 == codes[ub] || -3 == codes[ub]) return false;
	} while (0 < ub);
	return true;
}

static void _relative_geometry(const css::box::rect& lhs, const css::box::rect& rhs, std::array<int, 2>& codes)
{
	size_t ub = 2;
	do {
		--ub;
		codes[ub] = _interval_compare(lhs.tl_c()[ub], lhs.br_c()[ub], rhs.tl_c()[ub], rhs.br_c()[ub]);
	} while (0 < ub);
}

// comparison order for the codes is as follows:
// 0,1 are last (they are both complete-deletion on that coordinate)
// -1 is first (3-way split of which center is vulnerable to later deletion)
// 2,-2 are middle (2-way split of which center is vulnerable to later deletion)
// i.e.: -1; 2,-2; 0,1
static int set_difference_interval_priority(int x)
{
	if (-1 == x) return 0;
	if (-2 == x || 2 == x) return 1;
	return 2;
}

static std::vector<css::box::rect> _set_difference(const css::box::rect& lhs, const css::box::rect& rhs, std::array<int, 2>& codes)
{
	std::array<int, 2> test_order;
	size_t test_order_ub = 0;
	// Insertion sort.  Threshold for a more advanced sort needs testing, once ported
	size_t ub = 2;
	do {
		--ub;
		test_order[test_order_ub++] = ub;
		if (2 <= test_order_ub) {
			size_t back_scan = test_order_ub - 1;
			while (set_difference_interval_priority(test_order[back_scan-1]) > set_difference_interval_priority(test_order[back_scan])) {
				std::swap(test_order[back_scan-1], test_order[back_scan]);
				if (0 >= --back_scan) break;
			}
		}
	} while (0 < ub);

	std::vector<css::box::rect> ret;
	std::vector<css::box::rect> working;
	working.push_back(lhs);

	ub = 2;
	do {
		--ub;
		const auto i = test_order[ub];
		size_t outer_ub = working.size();
		do {
			--outer_ub;
			auto& test = working[outer_ub];
			const auto code = _interval_compare(test.tl_c()[i], test.br_c()[i], rhs.tl_c()[i], rhs.br_c()[i]);
			switch (code)
			{
			case -1:	// 3-way split
				{
				auto safe_1(test);
				auto safe_2(test);
				safe_1.tl_c()[i] = rhs.br_c()[i];
				safe_2.br_c()[i] = rhs.tl_c()[i];
				assert(safe_1.tl_c()[i] < safe_1.br_c()[i]);
				assert(safe_2.tl_c()[i] < safe_2.br_c()[i]);
				ret.push_back(safe_1);
				ret.push_back(safe_2);
				test.tl_c()[i] = rhs.tl_c()[i];
				test.br_c()[i] = rhs.br_c()[i];
				assert(test.tl_c()[i] < test.br_c()[i]);
				}
				break;
			case -2:	// 2-way split
				{
				auto safe(test);
				safe.br_c()[i] = rhs.tl_c()[i];
				assert(safe.tl_c()[i] < safe.br_c()[i]);
				ret.push_back(safe);
				test.tl_c()[i] = rhs.br_c()[i];
				assert(test.tl_c()[i] < test.br_c()[i]);
				}
				break;
			case  2:	// 2-way split
				{
				auto safe(test);
				safe.tl_c()[i] = rhs.br_c()[i];
				assert(safe.tl_c()[i] < safe.br_c()[i]);
				ret.push_back(safe);
				assert(test.tl_c()[i] < test.br_c()[i]);
				}
				break;
			case  1:
			case  0:
				working.erase(working.begin() + outer_ub);
				break;
			};
		} while (0 < outer_ub);
		if (working.empty()) break;
	}  while(0 < ub);
	return ret;
}

static void _self_union(std::vector<css::box::rect>& x, css::box::rect src)
{
	if (x.empty()) {
		x.push_back(src);
		return;
	}

restart:
	// This doesn't have to be "complete" for CSS processing -- the incoming rectangles are meant to be disjoint
	size_t ub = x.size();
	do {
		auto& r = x[--ub];
		std::array<int, 2> geometry;
		_relative_geometry(r, src, geometry);
		std::pair<std::array<int, 2>, std::pair<size_t,size_t> > r_contains_src;	// format: codes, count, missed coordinate
		std::pair<std::array<int, 2>, std::pair<size_t, size_t> > r_eq_src;
		std::pair<std::array<int, 2>, std::pair<size_t, size_t> > src_contains_r;
		r_contains_src.second.first = 0;
		r_eq_src.second.first = 0;
		src_contains_r.second.first = 0;
		bool disjoint = false;
		size_t scan = 2;
		do {
			--scan;
			if (-3 == geometry[scan] && r.br_c()[scan] < src.tl_c()[scan]) {
				disjoint = true;
				break;
			}
			if ( 3 == geometry[scan] && r.tl_c()[scan] > src.br_c()[scan]) {
				disjoint = true;
				break;
			}

			if (r_eq_src.first[scan] = (0 == geometry[scan])) r_eq_src.second.first++;
			else r_eq_src.second.second = scan;
			if (r_contains_src.first[scan] = (0 == geometry[scan] || 1 == geometry[scan])) r_contains_src.second.first++;
			else r_contains_src.second.second = scan;
			if (src_contains_r.first[scan] = (0 == geometry[scan] || -1 == geometry[scan])) src_contains_r.second.first++;
			else src_contains_r.second.second = scan;
		} while (0 < scan);
		if (disjoint) continue;
		if (2 == r_contains_src.second.first) return;	// consumed
		if (2 == src_contains_r.second.first) {	// consumed
			if (1 == x.size()) {
				r = src;
				return;
			}
			x.erase(x.begin() + ub);
			goto restart;
		}
		if (1 == r_eq_src.second.first) {
			if (-3 == geometry[r_eq_src.second.second] || -2 == geometry[r_eq_src.second.second]) {
				// extends on this coordinate
				if (1 == x.size()) {
					r.br_c()[r_eq_src.second.second] = src.br_c()[r_eq_src.second.second];
					return;
				}
				src.tl_c()[r_eq_src.second.second] = r.tl_c()[r_eq_src.second.second];
				x.erase(x.begin() + ub);
				goto restart;
			}
			if ( 3 == geometry[r_eq_src.second.second] || 2 == geometry[r_eq_src.second.second]) {
				// extends on this coordinate
				if (1 == x.size()) {
					r.tl_c()[r_eq_src.second.second] = src.tl_c()[r_eq_src.second.second];
					return;
				}
				src.br_c()[r_eq_src.second.second] = r.br_c()[r_eq_src.second.second];
				x.erase(x.begin() + ub);
				goto restart;
			}
		}
		// other cases possible, especially in higher dimensions
	} while (0 < ub);

	// failover
	x.push_back(src);
}

// end migration target

bool box_dynamic::reflow()
{
	if (_contents.empty()) return true;

	bool layout_changed = false;
	rect flowed_hull(point(0), point(0));
	rect last_flowed(point(0), point(0));

	const std::pair<int, int> reset_bounds(0, effective_max_width());
	const int reset_delta = reset_bounds.second - reset_bounds.first;
	const auto clean_rect = rect(point(0), max_size());
	// one key for each z-index
	std::map<int, std::vector<rect> > in_use;
	std::map<int, std::vector<rect> > available;

	std::map<int, std::pair<int,int> > work_bounds;			// bounding rectangles on where things could go

	for(auto& x : _contents) {
		if (x->has_no_box()) continue;
		const auto z_index = x->z_index();
		// handle z-index setup
		if (!work_bounds.count(z_index)) {
			work_bounds[z_index] = reset_bounds;
		}
		if (!x->can_reflow()) {
			auto test = x->clickable_box();
			clamp_ub(flowed_hull.tl_c(), test.tl_c());
			clamp_lb(flowed_hull.br_c(), test.br_c());
			continue;
		}
		if (!in_use.count(z_index)) in_use[z_index] = std::vector<rect>();
		if (!available.count(z_index)) {
			std::vector<rect> staging;
			staging.push_back(clean_rect);
			available[z_index] = std::move(staging);
		}

		auto& live = in_use[z_index];
		auto& vacuum = available[z_index];
		const auto x_clear = x->CSS_clear();
		const auto x_float = x->CSS_float();
		// we don't cleanly handle clear-left float-right or the inverse, just fail for now \todo non-debug mode should do something sensible
		if (CF_LEFT == x_float && (CF_RIGHT == x_clear || CF_BOTH == x_clear)) throw std::runtime_error("sorry, clear:right float:left not implemented");
		if (CF_RIGHT == x_float && (CF_LEFT == x_clear || CF_BOTH == x_clear)) throw std::runtime_error("sorry, clear:left float:right not implemented");

		const bool right_aligned = (CF_RIGHT == x_float);
		auto test = x->outer_box();
		const auto test_width = test.br_c()[0] - test.tl_c()[0];

		auto& bounds = work_bounds[z_index];
		point snap_to;
		snap_to[1] = last_flowed.tl_c()[1];
		snap_to[0] = right_aligned ? bounds.second : bounds.first;
		const int delta = bounds.second - bounds.first;
		if (reset_delta > delta && delta < test_width) {
			// need to sink down
			assert(0 && "unhandled operation");
		}
		if (CF_LEFT == x_clear || CF_BOTH == x_clear) {
			if (0 < bounds.first) {
				// need to sink down
				auto clear_for = x->border_box();
				css:box::rect candidate(point(INT_MAX), point(INT_MAX));
				for (const auto& r : vacuum) {
					if (0 < r.tl_c()[0]) continue;
					if (clear_for.tl_c()[1] <= r.tl_c()[1] && candidate.tl_c()[1] > r.tl_c()[1]) candidate = r;
				}
				if (INT_MAX > candidate.tl_c()[1]) {
					snap_to = candidate.tl_c();
					// but top margin doesn't really matter here
					snap_to[1] -= x->margin<TOP>();
					// things below us should not be flowed yet
					bounds = reset_bounds;
					point float_scan(candidate.tl_c());
restart_float_scan:
					float_scan[0] = bounds.second-1;
					for (const auto& r : live) {
						if (r.contains(float_scan)) {
							bounds.second = r.tl_c()[0];
							if (bounds.second >= test_width) break;	// \todo verify whether this is cause to go even lower?  Definitely request more width.
							goto restart_float_scan;
						}
					}
				}
			}
		}
		if (CF_RIGHT == x_clear || CF_BOTH == x_clear) {
			if (reset_bounds.second > bounds.second) {
				// need to sink down
				assert(0 && "unhandled operation");
			}
		}
		point other_corner(test.tl_c());
		// \todo float is actually supposed to trigger looking for the highest possible location
		if (right_aligned) {
			other_corner[0] = test.br_c()[0];
			other_corner[1] = test.tl_c()[1];
		}
		const auto shift = snap_to - other_corner;
		if (shift[0] || shift[1]) {
			x->set_origin(shift + x->origin());
			test = x->outer_box();
			layout_changed = true;
		}
		if (right_aligned) {
			bounds.second = test.tl_c()[0];
		} else {
			bounds.first = test.br_c()[0];
		}
		clamp_ub(flowed_hull.tl_c(), test.tl_c());
		clamp_lb(flowed_hull.br_c(), test.br_c());
		if (CF_RIGHT == x_clear || CF_BOTH == x_clear) {
			// need to block off entire area to right
			assert(0 && "unhandled operation");
		}
		last_flowed = test;
		_self_union(live, test);

		{	// update free space model
		std::array<int,2> codes;
		size_t ub = vacuum.size();
		do {
			auto& target = vacuum[--ub];
			if (_intersects(target, test, codes)) {
				auto replace = _set_difference(target, test, codes);
				const auto r_size = replace.size();
				if (1 == r_size) {
					target = replace[0];
				} else {
					vacuum.erase(vacuum.begin() + ub);
					vacuum.reserve(vacuum.size() + r_size);
					for (const auto& r : replace) _self_union(vacuum, r);
				}
			}
		} while(0 < ub);
		}
	}
	clamp_lb(flowed_hull.tl_c()[0], 0);
	clamp_lb(flowed_hull.tl_c()[1], 0);
	clamp_ub(flowed_hull.br_c()[0], reset_bounds.second);
	clamp_ub(flowed_hull.br_c()[1], effective_max_height());

	_width(flowed_hull.br_c()[0]);
	_height(flowed_hull.br_c()[1]);
	return true;
}


void box::schedule_reflow() {}
void box_dynamic::schedule_reflow() { _redo_layout = 1; }

}	// namespace css

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP2 cssbox.cpp
int main(int argc, char* argv[])
{
	css::box_dynamic test;
	return 0;
}
#endif
