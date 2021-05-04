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

}

#endif
