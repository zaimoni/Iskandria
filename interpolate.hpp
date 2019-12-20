#ifndef INTERPOLATE_HPP
#define INTERPOLATE_HPP 1

#include "matrix.hpp"

namespace zaimoni {
namespace math {
namespace interpolate_1d {

// this actually can be used to define a linear function by example
template<class DOM, class RANGE>
class linear
{
public:
	typedef std::pair<DOM, RANGE> datum;
private:
	datum _x0;
	datum _x1;
	datum _span;
public:
	linear() = default;
	linear(const datum& x0, const datum& x1) : _x0(x0), _x1(x1), _span(x1.first - x0.first, x1.second - x0.second) {};
	linear(const linear& src) = default;
	linear(linear&& src) = default;
	~linear() = default;
	linear& operator=(const linear& src) = default;
	linear& operator=(linear && src) = default;

	RANGE operator()(const DOM& src) const {
		if (src == _x0.first) return _x0.second;
		if (src == _x1.first) return _x1.second;

		DOM from_x0(src - _x0.first);
		// if the domain is 1-real-dimensional we'd like to use the "nearer endpoint"
		if constexpr (1 == R_coords<DOM>::value) {
			DOM from_x1(_x1.first - src);
			if (zaimoni::norm(from_x1) < zaimoni::norm(from_x0)) return _x1.second - (from_x1 / span.first) * span.second;
		}
		return _x0.second + (from_x0 / span.first) * span.second;
	}

	auto slope() const { return _span.second / _span.first; }
};

// more generally: for n data points (x1,y1)...(xn,yn) on a linear domain
// * identify the pair that are "most distant" in domain; re-order so that (x1,y1) and (xn,yn) are that pair
// * the domain-span is then xn-x1; midpoint (xn+x1)/2 formally, if we subtract (xn-x1)/2 from all points we would land on a closed, symmetric interval
// * on that closed symmetric interval, (yn+y1)/2 and (yn-y1)/2 would correspond to the odd and even parts of the function
// * the kinds of transformations assume the range is compatible with the domain
#if 0
template<class DOM>
class quadratic
{
public:
	static_assert(1 == R_coords<DOM>::value);
	typedef std::pair<DOM, DOM> datum;
private:
	std::array<datum, 3> _data;	// the fitting points
	std::shared_ptr<std::array<DOM, 3> > _xn_coeff;	// if we have coefficients in our native domain
	std::shared_ptr<std::pair<DOM, quadratic> > _forward;	// if we forwarded: linear transform first.first + x, then the target quadratic
	unsigned char _bitmap;
public:
	quadratic() : _bitmap(0) {};
	quadratic(const quadratic& src) = default;
	quadratic(quadratic&& src) = default;
	~quadratic() = default;
	quadratic& operator=(const quadratic& src) = default;
	quadratic& operator=(quadratic&& src) = default;

	void init(size_t n, const datum& src) {
		assert(0 <= n && 3 > n);
		if (!(_bitmap & (1ULL << n)) || src != _data[n]) {
			_data[n] = src;
			_bitmap &= 7ULL;	// erase all heuristic stages
			_bitmap |= (1ULL << n);
			_xn_coeff.reset(0);
			if (src.first != _data[n].first) _forward.reset(0);
			else if (_forward) _forward->init(n, std::pair(_forward->second._data[n].first, src.second));
		}
	}

	bool can_eval() const { return (_bitmap & 7ULL) == 7ULL && (_xn_coeff || _forward); }
	bool can_solve() const { return (_bitmap & 7ULL) == 7ULL; }

	DOM operator()(const DOM& x) {
		assert(can_solve());
		if (!can_eval()) {
			solve();
			assert(can_eval());
		}
restart:
		if (_xn_coeff) {
			if (is_zero(x)) return (*_xn_coeff)[0];
			if (is_zero((*_xn_coeff)[2])) {
				if (is_zero((*_xn_coeff)[1]) return (*_xn_coeff)[0];
				DOM ret((*_xn_coeff)[1]);
				ret *= x;
				if (!is_zero((*_xn_coeff)[0])) ret += (*_xn_coeff)[0]);
				return ret;
			} else {
				DOM ret((*_xn_coeff)[2]);
				if (is_zero((*_xn_coeff)[1])) {
					ret *= square(x);
				} else {
					ret *= x;
					ret += (*_xn_coeff)[1];
					ret *= x;
				}
				if (!is_zero((*_xn_coeff)[0])) ret += (*_xn_coeff)[0]);
				return ret;
			}
		} else if (_forward) return _forward->second(x - _forward->first);
		else _fatal("quadratic solver did not do its job");
	}
private:
	void solve() {
		assert(can_solve());
		// 1) safe translation
		// Our proper interpolation domain is x1...xn.  We want to minimize the significant digits of our interpolation points, while
		// not using moving much more than our "nearest to zero" element; that is, we do not want to lose precision just by repositioning
		// the domain.
		_forward = std::shared_ptr(solve_shift());
		if (_forward) return;
		// 2) matrix representation
		// Treat the coefficients as a matrix arithmetic problem
		_xn_coeff = std::shared_ptr(solve_coeff);
		if (_xn_coeff) return;
		_fatal("quadratic solver did not do its job");
	}
	std::pair<DOM, quadratic>* solve_shift() {
		return 0;
	}
	std::array<DOM, 3>* solve_coeff() {
		return 0;
		// matrix-based solver is core of this
		// do have some very special-case shortcuts available
		vector<DOM, 3> target;
		matrix_square<DOM, 3> x;
		int c = -1;
		for (const auto& src : _data) {
			c++;
			target[c] = src.second;
			size_t r = 3;
			do {
				--r;
				x(r, c) = pow(src.first, r);
			}
			while(0 < r);
		}
//		auto res = /* matrix inverse */*target;
	}
};

template<class DOM>
class cubic
{
public:
	static_assert(1 == R_coords<DOM>::value);
	typedef std::pair<DOM, DOM> datum;
private:
public:
	cubic() = default;
	cubic(const cubic& src) = default;
	cubic(cubic&& src) = default;
	~cubic() = default;
	cubic& operator=(const cubic& src) = default;
	cubic& operator=(cubic && src) = default;
};

template<class DOM>
class quartic
{
public:
	static_assert(1 == R_coords<DOM>::value);
	typedef std::pair<DOM, DOM> datum;
private:
public:
	quartic() = default;
	quartic(const quartic& src) = default;
	quartic(quartic&& src) = default;
	~quartic() = default;
	quartic& operator=(const quartic& src) = default;
	quartic& operator=(quartic&& src) = default;
};
#endif

}
}
}

#endif
