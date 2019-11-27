// taylor.cpp

#include "taylor.hpp"
#include "cyclic_fn.hpp"


// want:
// function object x -> (-1)^x
// function object x -> f(x/2) if even, 0 if odd
// function object x -> x-1
// cos is then x -> -1^(x/2) when even, 0 if odd
// sin is then x -> -1^((x-1)/2) when odd, 0 when even
// so gateway function is a Kronecker delta on an x_0 mod 2
// if that survives, call f((x-x_0)/2): f: x -> (-1)^x o g: x -> (x - x_0)/2

namespace zaimoni {
namespace math {

namespace linear {

template<intmax_t a_1_numerator, intmax_t a_1_denominator, intmax_t a_0> struct map;

template<intmax_t a_1_divisor>
struct map<1, a_1_divisor, 0>
{
	template<class DomainRange> static DomainRange eval(const DomainRange& x) {return x/int_as<a_1_divisor,DomainRange>();}	// XXX want integer math for integer types, but overprecise overrides for boost::numeric::interval
};

template<intmax_t a_0>
struct map<1,1, a_0>
{
	template<class DomainRange> static DomainRange eval(const DomainRange& x) {return x+int_as<a_0,DomainRange>();}
};

// division by zero shall not compile
template<intmax_t a_1_numerator, intmax_t a_0> struct map<a_1_numerator, 0, a_0>;

// this probably wants overprecision support rather than just lossy arithmetic support
template<intmax_t a_1_numerator, intmax_t a_1_denominator, intmax_t a_0>
struct map
{
	template<class DomainRange> static DomainRange eval(const DomainRange& x) {return int_as<a_1_numerator,DomainRange>()*x/int_as<a_1_denominator,DomainRange>()+int_as<a_0,DomainRange>();}
};

}	// namespace linear

// figure out where this goes later
static const zaimoni::math::mod_n::cyclic_fn_enumerated<2,signed char>&  alternator()
{
	static const signed char tmp[2] = {1,-1};
	static zaimoni::math::mod_n::cyclic_fn_enumerated<2,signed char> ret(tmp);
	return ret;
}

// unsigned_fn<0>::template kronecker_delta<Z_<2> >(n) * alternator o linear::map<1,2,0>::template eval<uintmax_t>(n)
// unsigned_fn<1>::template kronecker_delta<Z_<2> >(n) * alternator o linear::map<1,2,0>::template eval<uintmax_t> o linear_map<1,1,-1>::template eval<uintmax_t>


const TaylorSeries<int>& cos()
{
	// it would be *VERY* nice to calculate the function properties from the function generating the coefficients
	static TaylorSeries<int> ret(product(std::function<int (uintmax_t)>(unsigned_fn<0>::template kronecker_delta<Z_<2> >),
	                             compose(std::function<int (uintmax_t)>(alternator()),
	                                     std::function<uintmax_t (uintmax_t)>(linear::map<1,2,0>::template eval<uintmax_t>))),
	                             fn_algebraic_properties::ALTERNATING | fn_algebraic_properties::EVEN);
	return ret;
}

const TaylorSeries<int>& sin()
{
	static TaylorSeries<int> ret(product(std::function<int (uintmax_t)>(unsigned_fn<1>::template kronecker_delta<Z_<2> >),
	                                     compose(compose(std::function<int (uintmax_t)>(alternator()),
	                                                     std::function<uintmax_t (uintmax_t)>(linear::map<1,2,0>::template eval<uintmax_t>)),
										         std::function<uintmax_t (uintmax_t)>(linear::map<1,1,-1>::template eval<uintmax_t>))),
	                             fn_algebraic_properties::ALTERNATING | fn_algebraic_properties::ODD);
	return ret;
}

const TaylorSeries<int>& exp()
{
	static TaylorSeries<int> ret(std::function<int(uintmax_t)>(unsigned_fn<1>::constant<uintmax_t>),
		fn_algebraic_properties::NONZERO | fn_algebraic_properties::NONNEGATIVE);
	return ret;
}

const TaylorSeries<int>& cosh()
{
	static TaylorSeries<int> ret(product(std::function<int(uintmax_t)>(unsigned_fn<0>::template kronecker_delta<Z_<2> >),
		compose(std::function<int(uintmax_t)>(unsigned_fn<1>::constant<uintmax_t>),
			std::function<uintmax_t(uintmax_t)>(linear::map<1, 2, 0>::template eval<uintmax_t>))),
		fn_algebraic_properties::NONZERO | fn_algebraic_properties::NONNEGATIVE | fn_algebraic_properties::EVEN);
	return ret;
}

const TaylorSeries<int>& sinh()
{
	static TaylorSeries<int> ret(product(std::function<int(uintmax_t)>(unsigned_fn<1>::template kronecker_delta<Z_<2> >),
		compose(compose(std::function<int(uintmax_t)>(unsigned_fn<1>::constant<uintmax_t>),
			std::function<uintmax_t(uintmax_t)>(linear::map<1, 2, 0>::template eval<uintmax_t>)),
			std::function<uintmax_t(uintmax_t)>(linear::map<1, 1, -1>::template eval<uintmax_t>))),
		fn_algebraic_properties::ODD);
	return ret;
}

}	// namespace math
}	// namespace zaimoni

#ifdef TEST_APP
// example build line
// If doing INFORM-based debugging
// g++ -std=c++11 -otaylor.exe -DTEST_APP -D__STDC_LIMIT_MACROS taylor.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	INFORM(zaimoni::math::cos().a(0));
	INFORM(zaimoni::math::cos().a(1));
	INFORM(zaimoni::math::cos().a(2));
	INFORM(zaimoni::math::cos().a(3));
	INFORM(zaimoni::math::cos().a(4));

	STRING_LITERAL_TO_STDOUT("cos coefficients a_0..4\n");

	INFORM(zaimoni::math::sin().a(0));
	INFORM(zaimoni::math::sin().a(1));
	INFORM(zaimoni::math::sin().a(2));
	INFORM(zaimoni::math::sin().a(3));
	INFORM(zaimoni::math::sin().a(4));

	STRING_LITERAL_TO_STDOUT("sin coefficients a_0..4\n");

	INC_INFORM("sin(0): ");
	INFORM(zaimoni::math::sin().template eval(zaimoni::math::int_as<0,ISK_INTERVAL<long double> >()));
	INC_INFORM("cos(0): ");
	INFORM(zaimoni::math::cos().template eval(zaimoni::math::int_as<0,ISK_INTERVAL<long double> >()));

	INC_INFORM("sin(1): ");
	INFORM(zaimoni::math::sin().template eval(zaimoni::math::int_as<1,ISK_INTERVAL<long double> >()));
	INC_INFORM("cos(1): ");
	INFORM(zaimoni::math::cos().template eval(zaimoni::math::int_as<1,ISK_INTERVAL<long double> >()));

	INC_INFORM("exp(0): ");
	INFORM(zaimoni::math::exp().template eval(zaimoni::math::int_as<0, ISK_INTERVAL<long double> >()));
	INC_INFORM("sinh(0): ");
	INFORM(zaimoni::math::sinh().template eval(zaimoni::math::int_as<0, ISK_INTERVAL<long double> >()));
	INC_INFORM("cosh(0): ");
	INFORM(zaimoni::math::cosh().template eval(zaimoni::math::int_as<0, ISK_INTERVAL<long double> >()));

	INC_INFORM("exp(1): ");
	INFORM(zaimoni::math::exp().template eval(zaimoni::math::int_as<1, ISK_INTERVAL<long double> >()));
	INC_INFORM("sinh(1): ");
	INFORM(zaimoni::math::sinh().template eval(zaimoni::math::int_as<1, ISK_INTERVAL<long double> >()));
	INC_INFORM("cosh(1): ");
	INFORM(zaimoni::math::cosh().template eval(zaimoni::math::int_as<1, ISK_INTERVAL<long double> >()));

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}

#endif

