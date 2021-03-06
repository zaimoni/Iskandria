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

	// we don't want to use a normal macro here as this code is not really expected to be stable
#pragma start_copy interface_of
	template<class Derived>
	struct _interface_of<Derived, ISK_INTERVAL<double> > : public virtual fp_API
	{
	private:
		_fp_stats<double> _stats_l;
		_fp_stats<double> _stats_u;
	public:
		void invalidate_stats() {
			_stats_l.invalidate_stats();
			_stats_u.invalidate_stats();
		}

		bool is_scal_bn_identity() const override {
			force_valid_stats();
			return _stats_l.is_scal_bn_identity() && _stats_u.is_scal_bn_identity();
		}

		std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
			_stats_l.scal_bn_safe_range(ret.first, ret.second);
			_stats_u.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		intmax_t ideal_scal_bn() const override {
			if (is_scal_bn_identity()) return 0;	// inline this if/when micro-optimizing
			const auto rhs = _stats_u.ideal_scal_bn();
			if (_stats_l.is_scal_bn_identity()) return rhs;
			const auto lhs = _stats_l.ideal_scal_bn();
			if (_stats_u.is_scal_bn_identity()) return lhs;
			if (0 < lhs && 0 < rhs) return (lhs < rhs) ? lhs : rhs;
			if (0 > lhs && 0 > rhs) return (lhs < rhs) ? rhs : lhs;
			//			if (0 == rhs || 0 == lhs) return 0;
			return 0;
		}

		fp_API* clone() const override {
			auto& x = static_cast<const Derived*>(this)->value();
			const auto tmp = x.lower();
			if (tmp != x.upper()) return new Derived(*static_cast<const Derived*>(this));
			// singleton...optimize when cloning
			return new var<double, type_of_t<Derived> >(tmp);
		}
	private:
		void force_valid_stats() const {
			if (!_stats_l.valid()) {
				auto& x = static_cast<const Derived*>(this)->value();
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
			}
		}
		void _scal_bn(intmax_t scale) override {
			auto& x = static_cast<Derived*>(this)->value();
			x.assign(std::scalbn(x.lower(), scale), std::scalbn(x.upper(), scale));
		}	// power-of-two
		fp_API* _eval() const override {
			auto& x = static_cast<const Derived*>(this)->value();
			if (x.lower() == x.upper()) return new var<double>(x.lower());
			return 0;
		}
		bool _is_inf() const override { return isINF(static_cast<const Derived*>(this)->value()); }
		bool _is_finite() const override { return isFinite(static_cast<const Derived*>(this)->value()); }
	};
#pragma end_copy
#pragma for F in float,long double
#pragma substitute $F for double in interface_of
	template<class Derived>
	struct _interface_of<Derived, ISK_INTERVAL<float> > : public virtual fp_API
	{
	private:
		_fp_stats<float> _stats_l;
		_fp_stats<float> _stats_u;
	public:
		void invalidate_stats() {
			_stats_l.invalidate_stats();
			_stats_u.invalidate_stats();
		}

		bool is_scal_bn_identity() const override {
			force_valid_stats();
			return _stats_l.is_scal_bn_identity() && _stats_u.is_scal_bn_identity();
		}

		std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
			_stats_l.scal_bn_safe_range(ret.first, ret.second);
			_stats_u.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		intmax_t ideal_scal_bn() const override {
			if (is_scal_bn_identity()) return 0;	// inline this if/when micro-optimizing
			const auto rhs = _stats_u.ideal_scal_bn();
			if (_stats_l.is_scal_bn_identity()) return rhs;
			const auto lhs = _stats_l.ideal_scal_bn();
			if (_stats_u.is_scal_bn_identity()) return lhs;
			if (0 < lhs && 0 < rhs) return (lhs < rhs) ? lhs : rhs;
			if (0 > lhs && 0 > rhs) return (lhs < rhs) ? rhs : lhs;
			//			if (0 == rhs || 0 == lhs) return 0;
			return 0;
		}

		fp_API* clone() const override {
			auto& x = static_cast<const Derived*>(this)->value();
			const auto tmp = x.lower();
			if (tmp != x.upper()) return new Derived(*static_cast<const Derived*>(this));
			// singleton...optimize when cloning
			return new var<float, type_of_t<Derived> >(tmp);
		}
	private:
		void force_valid_stats() const {
			if (!_stats_l.valid()) {
				auto& x = static_cast<const Derived*>(this)->value();
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
			}
		}
		void _scal_bn(intmax_t scale) override {
			auto& x = static_cast<Derived*>(this)->value();
			x.assign(std::scalbn(x.lower(), scale), std::scalbn(x.upper(), scale));
		}	// power-of-two
		fp_API* _eval() const override {
			auto& x = static_cast<const Derived*>(this)->value();
			if (x.lower() == x.upper()) return new var<float>(x.lower());
			return 0;
		}
		bool _is_inf() const override { return isINF(static_cast<const Derived*>(this)->value()); }
		bool _is_finite() const override { return isFinite(static_cast<const Derived*>(this)->value()); }
	};

	template<class Derived>
	struct _interface_of<Derived, ISK_INTERVAL<long double> > : public virtual fp_API
	{
	private:
		_fp_stats<long double> _stats_l;
		_fp_stats<long double> _stats_u;
	public:
		void invalidate_stats() {
			_stats_l.invalidate_stats();
			_stats_u.invalidate_stats();
		}

		bool is_scal_bn_identity() const override {
			force_valid_stats();
			return _stats_l.is_scal_bn_identity() && _stats_u.is_scal_bn_identity();
		}

		std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
			_stats_l.scal_bn_safe_range(ret.first, ret.second);
			_stats_u.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		intmax_t ideal_scal_bn() const override {
			if (is_scal_bn_identity()) return 0;	// inline this if/when micro-optimizing
			const auto rhs = _stats_u.ideal_scal_bn();
			if (_stats_l.is_scal_bn_identity()) return rhs;
			const auto lhs = _stats_l.ideal_scal_bn();
			if (_stats_u.is_scal_bn_identity()) return lhs;
			if (0 < lhs && 0 < rhs) return (lhs < rhs) ? lhs : rhs;
			if (0 > lhs && 0 > rhs) return (lhs < rhs) ? rhs : lhs;
			//			if (0 == rhs || 0 == lhs) return 0;
			return 0;
		}

		fp_API* clone() const override {
			auto& x = static_cast<const Derived*>(this)->value();
			const auto tmp = x.lower();
			if (tmp != x.upper()) return new Derived(*static_cast<const Derived*>(this));
			// singleton...optimize when cloning
			return new var<long double, type_of_t<Derived> >(tmp);
		}
	private:
		void force_valid_stats() const {
			if (!_stats_l.valid()) {
				auto& x = static_cast<const Derived*>(this)->value();
				_stats_l.init_stats(x.lower());
				_stats_u.init_stats(x.upper());
			}
		}
		void _scal_bn(intmax_t scale) override {
			auto& x = static_cast<Derived*>(this)->value();
			x.assign(std::scalbn(x.lower(), scale), std::scalbn(x.upper(), scale));
		}	// power-of-two
		fp_API* _eval() const override {
			auto& x = static_cast<const Derived*>(this)->value();
			if (x.lower() == x.upper()) return new var<long double>(x.lower());
			return 0;
		}
		bool _is_inf() const override { return isINF(static_cast<const Derived*>(this)->value()); }
		bool _is_finite() const override { return isFinite(static_cast<const Derived*>(this)->value()); }
	};
#pragma end_substitute
#pragma done
}


#endif
