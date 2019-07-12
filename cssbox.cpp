#include "cssbox.hpp"
#include <string.h>
#include <limits>

namespace css {

box::box(bool bootstrap)
: _origin(0,0),_size(0,0), _size_min(0, 0), _size_max(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()) {
	memset(_margin, 0, sizeof(_margin));
	memset(_padding, 0, sizeof(_padding));
	_auto = (1ULL << (HEIGHT+1))-1;
	if (bootstrap) _self = std::shared_ptr<box>(this);
}

void box::set_auto(auto_legal src) {
	if (!is_auto(src)) _auto |= (1ULL << src) | (1ULL << REFLOW);
}

void box::width(int w) {
	if (0 > w) throw std::runtime_error("negative width");
	_size.first = w;
	_auto &= ~(1ULL << WIDTH);
	_auto |= (1ULL << REFLOW);
}

void box::height(int h) {
	if (0 > h) throw std::runtime_error("negative height");
	_size.second = h;
	_auto &= ~(1ULL << HEIGHT);
	_auto |= (1ULL << REFLOW);
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
void box::append(std::shared_ptr<box>& src) {
	if (src) {
		src->_parent = _self;
		src->_self = src;
		_contents.push_back(src);
		_auto |= (1ULL << REFLOW);
	}
}

}	// namespace css

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP2 cssbox.cpp
int main(int argc, char* argv[])
{
	css::box test;
	return 0;
}
#endif
