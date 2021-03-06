#ifndef CSS_SFML_HPP
#define CSS_SFML_HPP 1

#include "cssbox.hpp"
#include "display_manager.hpp"

namespace css {

template<class T>
class wrap : public box
{
private:
	std::shared_ptr<T> _x;
	std::pair<float, float> _scale;
public:
	wrap() = default;
	wrap(T* src) : _x(src) { assign_bootstrap(); }
	wrap(const wrap& src) = default;
	wrap(wrap&& src) = default;
	~wrap() = default;
	wrap& operator=(const wrap& src) = default;
	wrap& operator=(wrap&& src) = default;

	wrap& operator=(const std::shared_ptr<T>& src) { _x = src; assign_bootstrap(); return *this; };
	wrap& operator=(std::shared_ptr<T>&& src) { _x = src; assign_bootstrap(); return *this; };
	wrap& operator=(T* const src) { _x = decltype(_x)(src); assign_bootstrap(); return *this; };

	auto natural_dimensions() const { return _x->getLocalBounds(); }
	auto screen_dimensions() const { return _x->getGlobalBounds(); }

	void disconnect() override {};
	void set_self(std::shared_ptr<box>& src) override {};
	void draw() const override {
		if (_x) isk::DisplayManager::get().getWindow()->draw(*_x);
	}

	void screen_coords(point logical_origin) override {
		box::screen_coords(logical_origin);
		const auto scale = isk::DisplayManager::get().inverseScale();
		_x->setScale(scale.first * _scale.first, scale.second * _scale.second);
		_x->setPosition(_screen[0] * scale.first, _screen[1] * scale.second);
	}
private:
	layout_op assign_bootstrap_code() const {
		if (_reflow & ((1ULL << css::property::HEIGHT) | (1ULL << css::property::WIDTH))) return layout_op(1,0);
		return layout_op(0,0);
	}
	void physical_width_height() {
		auto natural = _x->getLocalBounds();
		_scale.first = 1;
		_scale.second = 1;
		if (_auto & (1ULL << HEIGHT)) {
			if (!(_auto & (1ULL << WIDTH)) && natural.width != width()) {
				natural.height *= width();
				natural.height /= natural.width;
				_scale.first *= width();
				_scale.first /= natural.width;
				natural.width = width();
				_scale.second = _scale.first;
			}
		} else if ((_auto & (1ULL << WIDTH))) {
			if (natural.height != height()) {
				natural.width *= height();
				natural.width /= natural.height;
				_scale.second *= height();
				_scale.second /= natural.height;
				natural.height = height();
				_scale.first = _scale.second;
			}
		} else {
			if (natural.width != width()) {
				_scale.first *= width();
				_scale.first /= natural.width;
				natural.width = width();
			}
			if (natural.height != height()) {
				_scale.second *= height();
				_scale.second /= natural.height;
				natural.height = height();
			}
		}
		if (0 <= natural.width) _width((int)(natural.width + 0.5));
		if (0 <= natural.height) _height((int)(natural.height + 0.5));
		_reflow &= ~((1ULL << css::property::HEIGHT) | (1ULL << css::property::WIDTH));
	}
	void assign_bootstrap() {
		if (!_x) return;	// reject NULL;
		physical_width_height();
	}

	bool flush() override { return !_x; }

	layout_op need_recalc() const override {
		if (!_x) return layout_op(0,0);	// reject NULL;
		const auto ret = assign_bootstrap_code();
		if (0 < ret.first) return ret;
		return layout_op(0, 0);
	}
	void _recalc(layout_op& code) override {
		if (0 >= code.first) return;	// do not process errors or no-op
		switch (code.first)	// arguably should have a private enum for legibility.  Backfit it when it's clear what's going on.
		{
		case 1:
			physical_width_height();
			return;
		default: return;	// not handled is like error
		}
	};
};

}	// namespace css

#endif
