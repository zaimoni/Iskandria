// taylor.hpp

#ifndef TAYLOR_HPP
#define TAYLOR_HPP 1

#include "Zaimoni.STL/augment.STL/functional"
#include "series_sum.hpp"

// strictly speaking could also handle Laurent series here, but the poles *probably* require special handling.

namespace zaimoni {
namespace math {

// while we allow for arbitrary coefficient types, the important one will be intmax_t

template<class COEFF>
class TaylorSeries
{
private:
	// serious data design issues here
	std::function<COEFF (uintmax_t)> term_numerator;	// alternative is "unbounded" integer class for domain of indexes
	// we also need a "tracker" for important fn traits
	// normal polynomial is merely finite upper bound for non-zero conefficient indexes
	// key optimizations:at this level
	// * upper bound on term size
	// ** upper bound on absolute value of a_n coming out
	// * non-strictly alternating series

	typename fn_algebraic_properties::bitmap_type _bitmap;
public:
	TaylorSeries(const std::function<COEFF (uintmax_t)>& src,typename fn_algebraic_properties::bitmap_type src_bitmap=0) : term_numerator(src),_bitmap(src_bitmap) {};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(TaylorSeries);

	COEFF a(uintmax_t n) const {return term_numerator(n);};

	// XXX need two kinds of overflow-aware integer arithmetic
	// bool return
	// throws std::overflow
	bool next_nonzero_term(uintmax_t& n, COEFF& a_n) const
	{
		while(UINTMAX_MAX>n)
			{
			a_n = term_numerator(++n);
			if (int_as<0,COEFF>()!=a_n) return true;
			}
		return false;
	}

	template<class DomainRange> DomainRange term(const DomainRange& x, uintmax_t n) const
	{
		if (0==n) return int_as<1,DomainRange>();	// a*1/0!
		if (1==n) return x;// x^1/1!
		// general case: x^n/n!

		return quotient_of_series_products(power_term<DomainRange,uintmax_t>(x,n)/* x^n */,int_range<uintmax_t>(2,n) /* n! */);
	}

	template<class DomainRange> DomainRange scale_term(const DomainRange& x, uintmax_t n1, uintmax_t n0) const
	{
		assert(n0<n1);
		if (1==n1-n0) return quotient(x,uint_as<DomainRange>(n1));	// XXX
		// general case: go from x^n0/n0! to x^n1/n1!
		return quotient_of_series_products(power_term<DomainRange,uintmax_t>(x,n1-n0)/* x^n */,int_range<uintmax_t>(n0+1,n1) /* n! */);
	}

	template<class DomainRange> DomainRange eval(const DomainRange& x) const
	{
		assert(isFinite(x));
		COEFF a_n = term_numerator(0);
		if (0 == (int_as<0, DomainRange>() <=> x)) return DomainRange(a_n);

		uintmax_t n = 0;
		if (int_as<0,COEFF>()==a_n && !next_nonzero_term(n,a_n)) return int_as<0,DomainRange>();	// give up if all visible terms non-zero
		assert(0!=a_n);
		
		// bootstrapping;  We need the initial x^n/n! as an intermediate stage, and the actual term.
		series_sum<DomainRange> accumulator;	// possibly should be an outright series summation type
		{	// scoping brace
		DomainRange core_term = term(x,n);
		auto full_term = DomainRange(a_n)*core_term;	// XXX

		accumulator.push_back(full_term);


		// update: if next term is available, construct it as an incremental product
		// evaluate whether error appears to be going up or down in the most intuitive norm
		// if numerical error is swamping the term, stop evaluation and return
		// otherwise accumulate

		do	{
			uintmax_t n_1 = n;
			if (!next_nonzero_term(n_1,a_n)) break;
			DomainRange scale = scale_term(x,n_1,n);
			core_term *= scale;	// XXX
			full_term = DomainRange(a_n)*core_term;	// XXX
			// this is where error estimation would go
			// if we think adding the term will *increase* the numerical error, abort

			fp_stats<DomainRange> stop_stats(accumulator.eval());
			fp_stats<DomainRange> term_stats(full_term);

			if (stop_stats.exponent()-std::numeric_limits<typename numerical<DomainRange>::exact_type>::digits > term_stats.exponent()) break;	// rounding error excessive			

			accumulator.push_back(full_term);
			n = n_1;
		} while(true);
		}	// end scoping brace
		return accumulator.eval();
	}
};

// important examples
const TaylorSeries<int>& cos();
const TaylorSeries<int>& sin();

const TaylorSeries<int>& exp();
const TaylorSeries<int>& cosh();
const TaylorSeries<int>& sinh();

}
}	// namespace zaimoni

#endif
