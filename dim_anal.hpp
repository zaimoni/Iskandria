// dim_anal.hpp
// dimensional analysis support.  Unit conversions aren't our job.

#ifndef DIM_ANALYSIS_HPP
#define DIM_ANALYSIS_HPP

#include "interval_shim.hpp"

namespace dim_analysis {

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/> class unit;
typedef unit<0, 0, 0, 0, 0> dimensionless;
typedef unit<1, 0, 0, 0, 0> mass;
typedef unit<0, 1, 0, 0, 0> length;
typedef unit<0, 0, 1, 0, 0> time;
typedef unit<0, 0, 0, 1, 0> temperature;
typedef unit<0, 0, 0, 0, 1> charge;

template<class U1, class U2> struct mult;
template<class U1, class U2> struct div;
template<class U, int N> struct power;

template<int m1, int L1, int t1, int T1, int Q1, int m2, int L2, int t2, int T2, int Q2>
struct mult<unit<m1, L1, t1, T1, Q1>, unit<m2, L2, t2, T2, Q2> >
{
	typedef unit<m1 + m2, L1 + L2, t1 + t2, T1 + T2, Q1 + Q2> type;
};

template<int m1, int L1, int t1, int T1, int Q1, int m2, int L2, int t2, int T2, int Q2>
struct div<unit<m1, L1, t1, T1, Q1>, unit<m2, L2, t2, T2, Q2> >
{
	typedef unit<m1 - m2, L1 - L2, t1 - t2, T1 - T2, Q1 - Q2> type;
};

template<int N, int m1, int L1, int t1, int T1, int Q1>
struct power<unit<m1, L1, t1, T1, Q1>, N >
{
	typedef unit<N*m1, N*L1, N*t1, N*T1, N*Q1> type;
};

typedef div<length, time>::type speed;
typedef mult<speed, mass>::type momentum;
typedef mult<power<speed, 2>::type, mass>::type energy;

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
class unit
{
private:
	interval_shim::interval _x;
public:
	enum {
		mass = m,
		length = L,
		time = t,
		temperature = T,
		charge = Q
	};

	// move construction not useful
	unit() = default;
	unit(const interval_shim::interval& src) : _x(src) {}	// implicit conversion ok
	unit(const double& lb, const double& ub) : _x(interval_shim::interval(lb, ub)) {}
	unit(const unit& src) = default;
	~unit() = default;
	unit& operator=(const unit& src) = default;
	unit& operator=(const interval_shim::interval& src) {
		_x = src;
		return *this;
	};

	const interval_shim::interval& x() const { return _x; }	// explicit downcast ok
	double lower() const { return _x.lower(); }
	double upper() const { return _x.upper(); }

	unit& operator+=(const unit& src) {
		_x += src._x;
		return *this;
	}
	unit& operator-=(const unit& src) {
		_x -= src._x;
		return *this;
	}
	unit& operator*=(double src) {
		_x *= src;
		return *this;
	}
	unit& operator/=(double src) {
		_x /= src;
		return *this;
	}
	unit& operator*=(const interval_shim::interval& src) {
		_x *= src;
		return *this;
	}
	unit& operator/=(const interval_shim::interval& src) {
		_x /= src;
		return *this;
	}

#define ZAIMONI_DEFINE_SCALE(NAME,VAR)	\
	void mult_scale_##NAME(const interval_shim::interval& src)	\
	{	\
		if constexpr(0 != VAR) {	\
			if (1 == src) return;	\
			auto scale = pow(src, (0 < VAR) ? VAR : -VAR);	\
			if (0 < VAR) _x /= scale;	\
			else _x *= scale;	\
		}	\
	}	\
	\
	void div_scale_##NAME(const interval_shim::interval& src)	\
	{	\
		if constexpr(0 != VAR) { \
			if (1 == src) return;	\
			auto scale = pow(src, (0 < VAR) ? VAR : -VAR);	\
			if (0 < VAR) _x *= scale;	\
			else _x /= scale;	\
		}	\
	}

	ZAIMONI_DEFINE_SCALE(mass, m)
	ZAIMONI_DEFINE_SCALE(length, L)
	ZAIMONI_DEFINE_SCALE(time, t)
	ZAIMONI_DEFINE_SCALE(temperature, T)
	ZAIMONI_DEFINE_SCALE(charge, Q)

#undef ZAIMONI_DEFINE_SCALE
};

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator+(unit<m, L, t, T, Q> x, const unit<m, L, t, T, Q>& rhs) {
	x += rhs;
	return x;
}

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator-(unit<m, L, t, T, Q> x, const unit<m, L, t, T, Q>& rhs) {
	x -= rhs;
	return x;
}

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator*(unit<m, L, t, T, Q> x, double src) {
	x *= src;
	return x;
}

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator*(double src, unit<m, L, t, T, Q> x) {
	x *= src;
	return x;
}

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator/(unit<m, L, t, T, Q> x, double src) {
	x /= src;
	return x;
}

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator/(double src, unit<m, L, t, T, Q> x) {
	x /= src;
	return x;
}

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator*(unit<m, L, t, T, Q> x, const interval_shim::interval& src) {
	x *= src;
	return x;
}

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator*(const interval_shim::interval& src, unit<m, L, t, T, Q> x) {
	x *= src;
	return x;
}

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator/(unit<m, L, t, T, Q> x, const interval_shim::interval& src) {
	x /= src;
	return x;
}

template<int m/* ass*/, int L/*ength*/, int t/*ime*/, int T/*emperature*/, int Q/*charge*/>
auto operator/(const interval_shim::interval& src, unit<m, L, t, T, Q> x) {
	x /= src;
	return x;
}

template<int m1,int L1, int t1, int T1, int Q1, int m2, int L2, int t2, int T2, int Q2>
unit<m1+m2, L1+L2, t1+t2, T1+T2, Q1+Q2> operator*(const unit<m1, L1, t1, T1, Q1>& lhs, const unit<m2, L2, t2, T2, Q2>& rhs)
{
	return lhs.x() * rhs.x();
}

template<int m1, int L1, int t1, int T1, int Q1, int m2, int L2, int t2, int T2, int Q2>
unit<m1 - m2, L1 - L2, t1 - t2, T1 - T2, Q1 - Q2> operator/(const unit<m1, L1, t1, T1, Q1>& lhs, const unit<m2, L2, t2, T2, Q2>& rhs)
{
	return lhs.x() / rhs.x();
}

template<int N, int m1, int L1, int t1, int T1, int Q1>
unit<N*m1, N * L1, N * t1, N * T1, N * Q1> pow(const unit<m1, L1, t1, T1, Q1>& lhs) {
	return pow(lhs.x(), N);
}

template<int m1, int L1, int t1, int T1, int Q1>
unit<m1/2, L1/2, t1/2, T1/2, Q1/2> sqrt(const unit<m1, L1, t1, T1, Q1>& lhs) {
	static_assert(0 == m1 % 2);
	static_assert(0 == L1 % 2);
	static_assert(0 == t1 % 2);
	static_assert(0 == T1 % 2);
	static_assert(0 == Q1 % 2);
	return sqrt(lhs.x());
}

template<int m1, int L1, int t1, int T1, int Q1>
unit<m1, L1, t1, T1, Q1> intersect(const unit<m1, L1, t1, T1, Q1>& lhs, const unit<m1, L1, t1, T1, Q1>& rhs) {
	return ::intersect(lhs.x(), rhs.x());
}

inline auto scalar(const dimensionless& x) { return x.x(); }

}	// namespace dim_analysis


#endif
