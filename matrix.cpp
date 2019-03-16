// matrix.cpp
// pure test driver

#include <boost/numeric/interval.hpp>
#include "matrix.hpp"

// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -std=c++11 -omatrix.exe -D__STDC_LIMIT_MACROS matrix.cpp -Llib/host.isk -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -omatrix.exe -D__STDC_LIMIT_MACROS matrix.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

// Also test driver for Euclidean.hpp, overprecise.hpp

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	char buf[100];

	STRING_LITERAL_TO_STDOUT("starting main\n");

	// some basic compile tests
	zaimoni::math::vector<boost::numeric::interval<double>,4> tmp_vec;
	zaimoni::math::covector<boost::numeric::interval<double>,4> tmp_covec;
	zaimoni::math::matrix_square<boost::numeric::interval<double>,4> tmp_sq_mat;
	zaimoni::math::matrix<boost::numeric::interval<double>,4,4> tmp_mat;
	const zaimoni::math::vector<boost::numeric::interval<double>,4> c_tmp_vec;
	const zaimoni::math::covector<boost::numeric::interval<double>,4> c_tmp_covec;
	const zaimoni::math::matrix_square<boost::numeric::interval<double>,4> c_tmp_sq_mat;
	const zaimoni::math::matrix<boost::numeric::interval<double>,4,4> c_tmp_mat;

	zaimoni::math::static_cache<boost::numeric::interval<double> >::as<0>();
	zaimoni::math::int_as<0, boost::numeric::interval<double> >();

	// element accessors
	INTERVAL_TO_STDOUT(tmp_vec[0],"\n");
	INTERVAL_TO_STDOUT(tmp_covec[0],"\n");
	INTERVAL_TO_STDOUT(tmp_sq_mat(0,0),"\n");
	INTERVAL_TO_STDOUT(tmp_mat(0,0),"\n");
	INTERVAL_TO_STDOUT(c_tmp_vec[0],"\n");
	INTERVAL_TO_STDOUT(c_tmp_covec[0],"\n");
	INTERVAL_TO_STDOUT(c_tmp_sq_mat(0,0),"\n");
	INTERVAL_TO_STDOUT(c_tmp_mat(0,0),"\n");
	INTERVAL_TO_STDOUT(tmp_sq_mat.row(0)[0],"\n");
	INTERVAL_TO_STDOUT(tmp_mat.row(0)[0],"\n");
	INTERVAL_TO_STDOUT(c_tmp_sq_mat.row(0)[0],"\n");
	INTERVAL_TO_STDOUT(c_tmp_mat.row(0)[0],"\n");
	INTERVAL_TO_STDOUT(tmp_sq_mat.col(0)[0],"\n");
	INTERVAL_TO_STDOUT(tmp_mat.col(0)[0],"\n");
	INTERVAL_TO_STDOUT(c_tmp_sq_mat.col(0)[0],"\n");
	INTERVAL_TO_STDOUT(c_tmp_mat.col(0)[0],"\n");
	STRING_LITERAL_TO_STDOUT("accessors ok\n");

	// double with operator +=
	tmp_vec += tmp_vec;
	tmp_sq_mat += tmp_sq_mat;
	tmp_mat += tmp_mat;

	// double with operator *=
	tmp_vec *= 2;
	tmp_sq_mat *= 2;
	tmp_mat *= 2;

	tmp_sq_mat*tmp_sq_mat;
	tmp_mat*tmp_mat;

	// check rearrange arithmatic
	double lhs = 1.0;
	double rhs = 1.0;
	int ok = zaimoni::math::rearrange_sum(lhs,rhs);
	ZAIMONI_PASSTHROUGH_ASSERT(ok);
	ZAIMONI_PASSTHROUGH_ASSERT(2.0==lhs);
	STRING_LITERAL_TO_STDOUT("1,0+1.0 = 2.0\n");

	lhs = 2.0;
	rhs = 1.0;
	ok = zaimoni::math::rearrange_sum(lhs,rhs);	
	ZAIMONI_PASSTHROUGH_ASSERT(ok);
	ZAIMONI_PASSTHROUGH_ASSERT(3.0==lhs);
	STRING_LITERAL_TO_STDOUT("2,0+1.0 = 3.0\n");

	lhs = 1.0;
	rhs = 1.0;
	int ok2 = zaimoni::math::trivial_product(lhs,rhs);	
	ZAIMONI_PASSTHROUGH_ASSERT(1==ok2);
	ZAIMONI_PASSTHROUGH_ASSERT(1.0==lhs);
	STRING_LITERAL_TO_STDOUT("1,0*1.0 = 1.0\n");

	lhs = 3.0;
	rhs = 3.0;
	ok = zaimoni::math::rearrange_product(lhs,rhs);	
	ZAIMONI_PASSTHROUGH_ASSERT(ok);
	ZAIMONI_PASSTHROUGH_ASSERT(9.0==lhs);
	STRING_LITERAL_TO_STDOUT("3,0*3.0 = 9.0\n");

	zaimoni::math::power_term<double,intmax_t> x(3.0,0);
	ZAIMONI_PASSTHROUGH_ASSERT(1==x.base());
	ZAIMONI_PASSTHROUGH_ASSERT(1==x.power());
	STRING_LITERAL_TO_STDOUT("3.0^0 = 1\n");

	lhs = 0.0;
	rhs = 2.0;
	assert(-1==zaimoni::math::trivial_sum(lhs,rhs));

	lhs = 2.0;
	rhs = 0.0;
	assert(1==zaimoni::math::trivial_sum(lhs,rhs));

	lhs = std::numeric_limits<double>::infinity();
	rhs = 1.0;
	assert(1==zaimoni::math::trivial_sum(lhs,rhs));

	lhs = 1.0;
	rhs = std::numeric_limits<double>::infinity();
	assert(-1==zaimoni::math::trivial_sum(lhs,rhs));

	lhs = std::numeric_limits<double>::infinity();
	rhs = std::numeric_limits<double>::infinity();
	assert(0!=zaimoni::math::trivial_sum(lhs,rhs));

	lhs = 1.0;
	rhs = 2.0;
	assert(0==zaimoni::math::trivial_sum(lhs,rhs));
	INFORM("zaimoni::math::trivial_sum positive tests ok");

	lhs = std::numeric_limits<double>::infinity();
	rhs = std::numeric_limits<double>::infinity();
	zaimoni::math::set_signbit(rhs,true);
	try	{
		zaimoni::math::trivial_sum(lhs,rhs);
		}
	catch(std::runtime_error& x)
		{
		INFORM(x.what());
		}
	INFORM("zaimoni::math::trivial_sum negative tests ok");

	zaimoni::math::power_term<boost::numeric::interval<long double>, uintmax_t > x_n(3.0,3);
	zaimoni::math::int_range<uintmax_t> factorial_3(2,3);
//	INFORM(eval(x_n),"\n");
	auto ret(zaimoni::math::quotient_of_series_products(x_n,factorial_3));
	INC_INFORM("3^3/6 = 4.5: ");
	INFORM(ret);

	STRING_LITERAL_TO_STDOUT("floating point stats: digits\n");
	INFORM(std::numeric_limits<float>::digits);
	INFORM(std::numeric_limits<double>::digits);
	INFORM(std::numeric_limits<long double>::digits);

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}
