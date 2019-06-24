#include "arithmetic.hpp"
#include "Zaimoni.STL/var.hpp"
#include "interval_shim.hpp"

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
	template _type<_type_spec::_R_SHARP_, _type_spec::none>* eval_quotient< _type<_type_spec::_R_SHARP_, _type_spec::none> >(const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& n, const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& d);
#if 0
	template<> _type<_type_spec::_R_SHARP_, _type_spec::none>* eval_quotient< _type<_type_spec::_R_SHARP_, _type_spec::none> >(const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& n, const std::shared_ptr<_type<_type_spec::_R_SHARP_, _type_spec::none> >& d)
	{	// we currently honor floating point types.  Integral types would also make sense here, mostly
		auto d_src = d.get();
		auto n_src = n.get();
		if (auto r = dynamic_cast<_access<float>*>(d_src)) {
		} else if (auto r = dynamic_cast<_access<ISK_INTERVAL<float> >*>(d_src)) {
		} else if (auto r = dynamic_cast<_access<double>*>(d_src)) {
		} else if (auto r = dynamic_cast<_access<ISK_INTERVAL<double> >*>(d_src)) {
		} else if (auto r = dynamic_cast<_access<long double>*>(d_src)) {
		} else if (auto r = dynamic_cast<_access<ISK_INTERVAL<long double> >*>(d_src)) {
		}

		if (auto l = dynamic_cast<_access<float>*>(n_src)) {
		} else if (auto l = dynamic_cast<_access<ISK_INTERVAL<float> >*>(n_src)) {
		} else if (auto l = dynamic_cast<_access<double>*>(n_src)) {
		} else if (auto l = dynamic_cast<_access<ISK_INTERVAL<double> >*>(n_src)) {
		} else if (auto l = dynamic_cast<_access<long double>*>(n_src)) {
		} else if (auto l = dynamic_cast<_access<ISK_INTERVAL<long double> >*>(n_src)) {
		}
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
