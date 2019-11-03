#include "cssbox.hpp"
#include <string.h>
#include <limits>
#include "Zaimoni.STL/Logging.h"
#if MULTITHREAD_DRAW
#include "Zaimoni.STL/ref_inc.hpp"
#endif

namespace css {

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
	int code;
	while (0 < (code = need_recalc())) _recalc(code);
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

int box_dynamic::need_recalc() const
{
	// check contents first;
	if (!_contents.empty()) {	// self-recalc check
		auto i = _contents.size();
		do {
			if (0 < _contents[--i]->need_recalc()) return i + 1;
		} while (0 < i);
	}
	// \todo: actually reposition contents
	// parallel: css_SFML.hpp
	if ((_auto & (1ULL << HEIGHT)) && (_auto & (1ULL << WIDTH))) {
		if (_reflow & ((1ULL << css::property::HEIGHT) | (1ULL << css::property::WIDTH))) return _contents.size()+1;
	}
	if (parent()) {	// can only resolve margins if we have a parent
		if ((_auto & (1ULL << LEFT)) && (_reflow & (1ULL << css::property::MARGIN + css::property::LEFT))) return _contents.size() + 2;
		if ((_auto & (1ULL << RIGHT)) && (_reflow & (1ULL << css::property::MARGIN + css::property::RIGHT))) return _contents.size() + 2;
		if ((_auto & (1ULL << TOP)) && (_reflow & (1ULL << css::property::MARGIN + css::property::TOP))) return _contents.size() + 3;
		if ((_auto & (1ULL << BOTTOM)) && (_reflow & (1ULL << css::property::MARGIN + css::property::BOTTOM))) return _contents.size() + 3;
	}
	return 0;
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

void box_dynamic::_recalc(int code)
{
	if (0 >= code) return;
	const auto c_size = _contents.size();
	if (c_size >= code) {
		// self-recalc entry
		_contents[code - 1]->recalc();
		if (!_redo_layout || _redo_layout > code) _redo_layout = code;	// \todo lock this update down to when it's needed (i.e. border-box actually changes)
		return;
	}
	code -= c_size;	// rescale the code
	// parallel: css_SFML.hpp
	switch (code)
	{
	case 1:	// size
		if (1 == c_size) {
			const auto _size = _contents[0]->size();
			// usual strcmp coding: -1 less than lower bound, 1 greater than lower bound, 0 ok
			const int width_bad = (min_width() <= __width) ? ((max_width() >= __width) ? 0 : 1) : -1;
			const int height_bad = (min_height() <= __height) ? ((max_height() >= __height) ? 0 : 1) : -1;
			if (!width_bad && !height_bad) {
				_width(__width);
				_height(__height);
				return;
			}
		}
		assert(0 && "unhandled resize request");
		_reflow &= ~((1ULL << css::property::WIDTH) | (1ULL << css::property::HEIGHT));	// do not infinite-loop
		break;
	case 2:	// horizontal auto-center
		horizontal_centering(parent()->width(), parent()->origin());
		break;
	case 3:	// vertical auto-center
		vertical_centering(parent()->height(), parent()->origin());
		break;
	}
}

void box_dynamic::draw() const
{
	if (_contents.empty()) return;
	// We will need to be *much* smarter about this; anything that is off-screen or completely hidden by higher z-index need not be done here
	for(auto& x : _contents) {
		if (!x) continue;
		x->draw();
	}
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
