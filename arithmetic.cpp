// order matters: interval must be before var.hpp and eval.hpp to prevent compile errors
#include "interval_shim.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"

namespace zaimoni {
namespace math {

	template<class T>
	typename std::enable_if<std::is_base_of<fp_API, T>::value, int>::type rearrange_sum(std::shared_ptr<T>& lhs, std::shared_ptr<T>& rhs) { return 0; }

	template<class T>
	typename std::enable_if<std::is_base_of<fp_API, T>::value, int>::type rearrange_product(std::shared_ptr<T>& lhs, std::shared_ptr<T>& rhs) { return 0; }

	template<class T>
	typename std::enable_if<std::is_base_of<fp_API, T>::value, T*>::type eval_quotient(const std::shared_ptr<T>& n, const std::shared_ptr<T>& d) { return 0; }

	template int rearrange_sum< _type<_type_spec::_R_SHARP_, _type_spec::none> >(std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& lhs, std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& rhs);
	template int rearrange_product< _type<_type_spec::_R_SHARP_, _type_spec::none> >(std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& lhs, std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& rhs);
#if 0
	template _type<_type_spec::_R_SHARP_, _type_spec::none>* eval_quotient< _type<_type_spec::_R_SHARP_, _type_spec::none> >(const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& n, const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& d);
#else
	template<class T,class F>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value, T*>::type eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F>& d)
	{
		try {
			auto ret = n / d;
			if (ret.lower() == ret.upper()) return new var<typename ISK_INTERVAL<F>::base_type>(ret.upper());
			return new var<decltype(ret)>(ret);
		} catch (zaimoni::math::numeric_error& e) {
			// doesn't help w/Boost, but our internal interval type should like to throw on overflow, etc.
			return 0;
		}
		return 0;
	}

	template<class T,class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value && std::is_floating_point<F2>::value && (std::numeric_limits<F>::max_exponent<std::numeric_limits<F2>::max_exponent), T*>::type eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient<T>(ISK_INTERVAL<F2>(n), d);
	}

	template<class T, class F, class F2>
	typename std::enable_if<std::is_base_of<fp_API, T>::value&& std::is_floating_point<F>::value&& std::is_floating_point<F2>::value && (std::numeric_limits<F>::max_exponent > std::numeric_limits<F2>::max_exponent), T*>::type eval_quotient(const ISK_INTERVAL<F>& n, const ISK_INTERVAL<F2>& d)
	{
		return eval_quotient<T>(n, ISK_INTERVAL<F>(d));
	}

	template<class T,class F>
	typename std::enable_if<std::is_base_of<fp_API, T>::value && std::is_floating_point<F>::value, T*>::type eval_quotient(const std::shared_ptr<T>& n, const ISK_INTERVAL<F>& d)
	{
		if (d == F(0)) throw zaimoni::math::numeric_error("division by zero");	// expected to be caught earlier when called from the quotient class
		if (F(0) > d.lower() && F(0) < d.upper()) throw zaimoni::math::numeric_error("division should result in two disjoint intervals");	// not always, but requires exact zero numerator which should be caught by the quotient class

		auto n_src = n.get();
		if (auto l = dynamic_cast<_access<float>*>(n_src)) return eval_quotient<T>(ISK_INTERVAL<float>(l->value()), d);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<float> >*>(n_src)) return eval_quotient<T>(l->value(), d);
		else if (auto l = dynamic_cast<_access<double>*>(n_src)) return eval_quotient<T>(ISK_INTERVAL<double>(l->value()), d);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<double> >*>(n_src)) return eval_quotient<T>(l->value(), d);
		else if (auto l = dynamic_cast<_access<long double>*>(n_src)) return eval_quotient<T>(ISK_INTERVAL<long double>(l->value()), d);
		else if (auto l = dynamic_cast<_access<ISK_INTERVAL<long double> >*>(n_src)) return eval_quotient<T>(l->value(), d);

		return 0;
	}

	template<> _type<_type_spec::_R_SHARP_, _type_spec::none>* eval_quotient< _type<_type_spec::_R_SHARP_, _type_spec::none> >(const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& n, const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& d)
	{	// we currently honor floating point types.  Integral types would also make sense here, mostly
		auto d_src = d.get();
		if (auto r = dynamic_cast<_access<float>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<float>(r->value()));
		else if (auto r = dynamic_cast<_access<ISK_INTERVAL<float> >*>(d_src)) return eval_quotient(n, r->value());
		else if (auto r = dynamic_cast<_access<double>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<double>(r->value()));
		else if (auto r = dynamic_cast<_access<ISK_INTERVAL<double> >*>(d_src)) return eval_quotient(n, r->value());
		else if (auto r = dynamic_cast<_access<long double>*>(d_src)) return eval_quotient(n, ISK_INTERVAL<long double>(r->value()));
		else if (auto r = dynamic_cast<_access<ISK_INTERVAL<long double> >*>(d_src)) return eval_quotient(n, r->value());
		return 0;
	}
#endif
}
}

#ifdef TEST_APP
// fast compile test
// g++ -std=c++14 -otest.exe -Os -D__STDC_LIMIT_MACROS -DTEST_APP arithmetic.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util
int main(int argc, char* argv[])
{
	// compile-time checks
	return 0;
}
#endif
