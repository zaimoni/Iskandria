#include "cssbox.hpp"
#include <string.h>
#include <limits>

namespace css {

box::box(bool bootstrap)
: _auto((1ULL << (HEIGHT + 1)) - 1),_auto_recalc(0),_origin(0,0),_size(0,0), _size_min(0, 0),
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
	if (_size.first != w) {
		_size.first = w;
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
	if (_size.second != h) {
		_size.second = h;
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

// content management
void box_dynamic::append(std::shared_ptr<box>& src) {
	if (src) {
		set_parent(src);
		_contents.push_back(src);
		_auto |= (1ULL << REFLOW);
	}
}

bool box::flush() { return false; }
int box::need_recalc() const { return 0; }
void box::recalc(int code) {}

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
	return 0;
}

void box_dynamic::recalc(int code)
{
	if (0 >= code) return;
	if (_contents.size() >= code) {
		// self-recalc entry
		_contents[code - 1]->recalc();
		return;
	}
}


void box::draw() const {}
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
