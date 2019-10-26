#include "cssbox.hpp"
#include <string.h>
#include <limits>
#include "Zaimoni.STL/Logging.h"

namespace css {

box::box(bool bootstrap)
: _auto((1ULL << (HEIGHT)) | (1ULL << (WIDTH))),_auto_recalc(0),_origin(0,0), _screen(0, 0),_size(0,0), _size_min(0, 0),
  _size_max(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()) {
	memset(_margin, 0, sizeof(_margin));
	memset(_padding, 0, sizeof(_padding));
	if (bootstrap) _self = std::shared_ptr<box>(this);
}

void box::set_auto(auto_legal src) {
	if (!is_auto(src)) {
		_auto |= (1ULL << src);
		_auto_recalc |= (1ULL << src);
	}
}

void box::width(int w) {
	if (0 > w) throw std::runtime_error("negative width");
	_auto &= ~(1ULL << WIDTH);
	_width(w);
}

void box::_width(int w) {
	_auto_recalc &= ~(1ULL << WIDTH);
	if (_size.first != w) {
		_size.first = w;
		if ((_auto & (1ULL << LEFT))) _auto_recalc |= (1ULL << LEFT);
		if ((_auto & (1ULL << RIGHT))) _auto_recalc |= (1ULL << RIGHT);
		schedule_reflow();
		auto parent(_parent.lock());
		if (parent) parent->_auto |= (1ULL << REFLOW);
	}
}

void box::height(int h) {
	if (0 > h) throw std::runtime_error("negative height");
	_auto &= ~(1ULL << HEIGHT);
	_height(h);
}

void box::_height(int h) {
	_auto_recalc &= ~(1ULL << HEIGHT);
	if (_size.second != h) {
		_size.second = h;
		if ((_auto & (1ULL << TOP))) _auto_recalc |= (1ULL << TOP);
		if ((_auto & (1ULL << BOTTOM))) _auto_recalc |= (1ULL << BOTTOM);
		schedule_reflow();
		auto parent(_parent.lock());
		if (parent) parent->_auto |= (1ULL << REFLOW);
	}
}

void box::min_width(int w) {
	if (0 > w) throw std::runtime_error("negative width");
	_size_min.first = w;
	if (w > _size_max.first) max_width(w);
	else if (w > _size.first) width(w);
}

void box::min_height(int h) {
	if (0 > h) throw std::runtime_error("negative height");
	_size_min.second = h;
	if (h > _size_max.second) max_height(h);
	else if (h > _size.second) height(h);
}

void box::max_width(int w) {
	if (0 > w) throw std::runtime_error("negative width");
	_size_max.first = w;
	if (w < _size_min.first) min_width(w);
	else if (w < _size.first) width(w);
}

void box::max_height(int h) {
	if (0 > h) throw std::runtime_error("negative height");
	_size_max.second = h;
	if (h < _size_min.second) min_height(h);
	else if (h < _size.second) height(h);
}

void box::set_origin(const std::pair<int, int> logical_origin) {
	_origin.first = logical_origin.first + margin<LEFT>() + padding<LEFT>();
	_origin.second = logical_origin.second + margin<TOP>() + padding<TOP>();
}

void box::screen_coords(const std::pair<int, int> logical_origin) {
	_screen.first = _origin.first + logical_origin.first;
	_screen.second = _origin.second + logical_origin.second;
}

void box_dynamic::screen_coords(std::pair<int, int> logical_origin) {
	box::screen_coords(logical_origin);
	for (auto& x : _contents) x->screen_coords(_screen);
}

// content management
void box::set_parent(std::shared_ptr<box>& src) {
	if (src) {
		src->_parent = _self;
		src->_self = src;
		if (_auto & (1ULL << WIDTH)) _auto_recalc |= (1ULL << WIDTH);
		if (_auto & (1ULL << HEIGHT)) _auto_recalc |= (1ULL << HEIGHT);
	}
}

void box::remove(std::shared_ptr<box> gone) {}
void box_dynamic::remove(std::shared_ptr<box> gone) {
	size_t ub = _contents.size();
	do {
		if (_contents[ub] == gone) _contents.erase(_contents.begin() + ub);
		else _contents[ub]->remove(gone);
	} while(0<ub);
}

void box_dynamic::append(std::shared_ptr<box> src) {
	if (src) {
		bool resize_ok = parent() ? true : false;
		// absolute positioning will prevent resizing; possibly other cases
		if (resize_ok) {
			if (_auto & (1ULL << WIDTH)) _auto_recalc |= (1ULL << WIDTH);
			if (_auto & (1ULL << HEIGHT)) _auto_recalc |= (1ULL << HEIGHT);
		}
		set_parent(src);
		_contents.push_back(src);
		_auto |= (1ULL << REFLOW);
	}
}

void box::recalc() {
	flush();
	int code;
	while (0 < (code = need_recalc())) _recalc(code);
	_auto &= ~(1ULL << REFLOW);	// cancel reflow, we're stable
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
		if ((_auto_recalc & (1ULL << HEIGHT)) || (_auto_recalc & (1ULL << WIDTH))) return _contents.size()+1;
	}
	if (parent()) {	// can only resolve margins if we have a parent
		if ((_auto & (1ULL << LEFT)) && (_auto_recalc & (1ULL << LEFT))) return _contents.size() + 2;
		if ((_auto & (1ULL << RIGHT)) && (_auto_recalc & (1ULL << RIGHT))) return _contents.size() + 2;
		if ((_auto & (1ULL << TOP)) && (_auto_recalc & (1ULL << TOP))) return _contents.size() + 3;
		if ((_auto & (1ULL << BOTTOM)) && (_auto_recalc & (1ULL << BOTTOM))) return _contents.size() + 3;
	}
	return 0;
}

void box::horizontal_centering(const int ub, const std::pair<int, int> local_origin)
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

void box::vertical_centering(const int ub, const std::pair<int, int> local_origin)
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
		_auto_recalc |= (1ULL << LEFT);
		ret = true;
	}
	if (_auto & (1ULL << RIGHT)) {
		_auto_recalc |= (1ULL << RIGHT);
		ret = true;
	}
	return ret;
}

bool box::request_vert_margins()
{
	bool ret = false;
	if (_auto & (1ULL << TOP)) {
		_auto_recalc |= (1ULL << TOP);
		ret = true;
	}
	if (_auto & (1ULL << BOTTOM)) {
		_auto_recalc |= (1ULL << BOTTOM);
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
			const int width_bad = (min_width() <= _size.first) ? ((max_width() >= _size.first) ? 0 : 1) : -1;
			const int height_bad = (min_height() <= _size.second) ? ((max_height() >= _size.second) ? 0 : 1) : -1;
			if (!width_bad && !height_bad) {
				_width(_size.first);
				_height(_size.second);
				return;
			}
		}
		assert(0 && "unhandled resize request");
		_auto_recalc &= ~((1ULL << WIDTH) | (1ULL << HEIGHT));	// do not infinite-loop
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
void box_dynamic::schedule_reflow() { _auto |= (1ULL << REFLOW); }

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
