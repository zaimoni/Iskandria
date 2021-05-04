#ifndef ZAIMONI_STL_BITS_INTERVAL_VAR_HPP
#define ZAIMONI_STL_BITS_INTERVAL_VAR_HPP 1

#ifndef _INTERVAL_HPP
#error assumed _interval.hpp was included
#endif
#ifndef VAR_HPP
#error assumed var.hpp was included
#endif

#include <type_traits>

namespace zaimoni {

namespace bits {

	template<class T> struct _type_of<ISK_INTERVAL<T> > { typedef type_of_t<T> type; };

}

	// get something intelligible out if the C preprocessor is buggy
	static_assert(std::is_same_v<type_of_t<ISK_INTERVAL<float> >, _type<_type_spec::_R_SHARP_> >);
	static_assert(std::is_same_v<type_of_t<ISK_INTERVAL<double> >, _type<_type_spec::_R_SHARP_> >);
	static_assert(std::is_same_v<type_of_t<ISK_INTERVAL<long double> >, _type<_type_spec::_R_SHARP_> >);
}

// close/re-open

namespace zaimoni {
namespace detail {

	template<class T>
	struct var_fp_impl<ISK_INTERVAL<T> > {
		using param_type = ISK_INTERVAL<T>;
		using coord_type = T;

		static const math::type* domain(const param_type& x) {
			if (isNaN(x)) return nullptr;
			if (isINF(x)) return &zaimoni::math::get<_type<_type_spec::_R_SHARP_>>();
			return &zaimoni::math::get<_type<_type_spec::_R_>>();
		}
		static constexpr bool self_eval(const param_type& x) { return false; }
		static constexpr int sgn(const param_type& x) { return zaimoni::sgn(x); }
		static std::string to_s(const param_type& x) { return to_string(x); }
		static bool is_scal_bn_identity(const param_type& x) {
			return  _fp_stats<coord_type>(x.lower()).is_scal_bn_identity()
			     && _fp_stats<coord_type>(x.upper()).is_scal_bn_identity();
		}
		static std::pair<intmax_t, intmax_t> scal_bn_safe_range(const param_type& x) {
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
			_fp_stats<coord_type>(x.lower()).scal_bn_safe_range(ret.first, ret.second);
			_fp_stats<coord_type>(x.upper()).scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}
		static intmax_t ideal_scal_bn(const param_type& x) {
			if (_fp_stats<coord_type>(x.lower()).is_scal_bn_identity()) return _fp_stats<coord_type>(x.upper()).ideal_scal_bn();
			if (_fp_stats<coord_type>(x.upper()).is_scal_bn_identity()) return _fp_stats<coord_type>(x.lower()).ideal_scal_bn();
			intmax_t L_ideal = _fp_stats<coord_type>(x.lower()).ideal_scal_bn();
			intmax_t U_ideal = _fp_stats<coord_type>(x.upper()).ideal_scal_bn();
			if (L_ideal == U_ideal) return L_ideal;
			// \todo more complicated cases
			return 0;
		}
		static fp_API* clone(const param_type& x) {
			if (x.lower() == x.upper()) return new var_fp<coord_type>(x.lower());
			return nullptr;
		}
		static void _scal_bn(param_type& x, intmax_t scale) { x = scalBn(x, scale); }
		static constexpr fp_API* _eval(const param_type& x) { return nullptr; }
	};

}
}

#endif
