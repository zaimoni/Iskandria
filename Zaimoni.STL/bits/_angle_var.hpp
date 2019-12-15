#ifndef ZAIMONI_STL_BITS_INTERVAL_VAR_HPP
#define ZAIMONI_STL_BITS_INTERVAL_VAR_HPP

#ifndef ANGLE_HPP
#error assumed angle.hpp was included
#endif
#ifndef VAR_HPP
#error assumed var.hpp was included
#endif

namespace zaimoni {

namespace circle {

	void init(const angle& src, _fp_stats<double>& l_stat, _fp_stats<double>& u_stat)
	{
		l_stat.init_stats(src._theta.lower());
		u_stat.init_stats(src._theta.upper());
	}

}

	template<>
	struct _type_of<zaimoni::circle::angle>
	{
		typedef _type<_type_spec::_S1_> type;
	};

	template<class Derived>
	struct _interface_of<Derived, zaimoni::circle::angle, 0> : public virtual fp_API
	{
	private:
		_fp_stats<double> _stats_l;
		_fp_stats<double> _stats_u;
	public:
		void invalidate_stats() {
			_stats_l.invalidate_stats();
			_stats_u.invalidate_stats();
		}

		virtual bool is_scal_bn_identity() const {
			force_valid_stats();
			return _stats_l.is_scal_bn_identity() && _stats_u.is_scal_bn_identity();
		}

		virtual std::pair<intmax_t, intmax_t> scal_bn_safe_range() const {
			// frexp convention: mantissa is [0.5,1.0) and exponent of 1.0 is 1
			force_valid_stats();
			std::pair<intmax_t, intmax_t> ret(fp_API::max_scal_bn_safe_range());
			_stats_l.scal_bn_safe_range(ret.first, ret.second);
			_stats_u.scal_bn_safe_range(ret.first, ret.second);
			return ret;
		}

		virtual intmax_t ideal_scal_bn() const {
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

		typename _type_of<Derived>::type* clone() const override {
			return new var<zaimoni::circle::angle, typename _type_of<Derived>::type>(tmp);
		}
	private:
		void force_valid_stats() const {
			if (!_stats_l.valid()) {
				auto& x = static_cast<const Derived*>(this)->value();
				init(x, _stats_l, _stats_u);
			}
		}
		virtual void _scal_bn(intmax_t scale) {
			auto& x = static_cast<Derived*>(this)->value();
			x.assign(std::scalbn(x.lower(), scale), std::scalbn(x.upper(), scale));
		}	// power-of-two
		fp_API* _eval() const override { return 0; }
	};
}


#endif
