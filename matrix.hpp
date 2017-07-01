// matrix.hpp

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "Zaimoni.STL/Logging.h"
#include "Zaimoni.STL/rw.hpp"
#include "slice.hpp"
#include "Euclidean.hpp"
#include <array>
#include <algorithm>

namespace zaimoni {
namespace math {

namespace pointwise {

template<class IO_Iter,class I_Iter>
void in_place_sum(IO_Iter dest, I_Iter src, size_t n)
{
	assert(0<n);
	assert(dest);
	assert(src);
	while(0<n)
		{
		*(dest++) += *(src++);
		--n;
		};
}

template<class IO_Iter,class I_Iter>
void in_place_difference(IO_Iter dest, I_Iter src, size_t n)
{
	assert(0<n);
	assert(dest);
	assert(src);
	while(0<n)
		{
		*(dest++) -= *(src++);
		--n;
		};
}


template<class IO_Iter,class T>
void in_place_scalar_product(IO_Iter dest, const T src, size_t n)
{
	assert(0<n);
	assert(dest);
	while(0<n)
		{
		*(dest++) *= src;
		--n;
		};
}

template<class IO_Iter,class T>
void in_place_scalar_quotient(IO_Iter dest, const T src, size_t n)
{
	assert(0<n);
	assert(dest);
	while(0<n)
		{
		*(dest++) /= src;
		--n;
		};
}

}	// namespace pointwise

// follow Boost uBLAS here
template<class Derived, class T>
struct matrix_CRTP
{
};

template<class T,size_t N>
class vector : public matrix_CRTP<vector<T,N>, T>
{
private:
	std::array<T,N> _x;
	ZAIMONI_STATIC_ASSERT(0<N);
public:
	enum {
		rows = N,
		cols = 1
	};

	// decide: constructor destructor assignment operator
	vector() = default;
	vector(T* src) { assert(src); std::copy_n(src,N,_x.data()); };
	vector(const vector& src) = default;
	vector(vector&& src) = default;
	vector operator=(const vector& src) {
		std::copy_n(src._x.data(),N,_x.data());
		return *this;
	}
	vector operator=(const T* src) {
		assert(src);
		std::copy_n(src,N,_x.data());
		return *this;
	}

	// array deference
	T operator[](size_t n) const {
		assert(N>n);
		return _x.data()[n];
	};
	T& operator[](size_t n) {
		assert(N>n);
		return _x.data()[n];
	};
	size_t size() const {return N;}

	// numerically simple operations
	vector& operator+=(const vector& src) {
		zaimoni::math::pointwise::in_place_sum(_x.data(),src._x.data(),N);
		return *this;
	}
	vector& operator-=(const vector& src) {
		zaimoni::math::pointwise::in_place_difference(_x.data(),src._x.data(),N);
		return *this;
	}
	vector& operator*=(const T& src) {
		zaimoni::math::pointwise::in_place_scalar_product(_x.data(),src,N);
		return *this;
	}
	vector& operator/=(const T& src) {
		zaimoni::math::pointwise::in_place_scalar_quotient(_x.data(),src,N);
		return *this;
	}
};

template<class T,size_t N>
vector<T,N> operator+(vector<T,N> lhs, const vector<T,N>& rhs)
{
	lhs += rhs;
	return lhs;
}

template<class T,size_t N>
vector<T,N> operator-(vector<T,N> lhs, const vector<T,N>& rhs)
{
	lhs -= rhs;
	return lhs;
}

template<class T,size_t N>
vector<T,N> operator*(vector<T,N> lhs, const T& rhs)
{
	lhs *= rhs;
	return lhs;
}

template<class T,size_t N>
vector<T,N> operator/(vector<T,N> lhs, const T& rhs)
{
	lhs /= rhs;
	return lhs;
}

template<class T,size_t N>
class covector : public matrix_CRTP<covector<T,N>, T>
{
private:
	std::array<T,N> _x;
	ZAIMONI_STATIC_ASSERT(0<N);
public:
	enum {
		rows = 1,
		cols = N
	};

	// decide: constructor destructor assignment operator
	covector() = default;
	covector(T* src) { assert(src); std::copy_n(src,N,_x.data()); };
	covector(const covector& src) = default;
	covector(covector&& src) = default;
	covector operator=(const covector& src) {
		std::copy_n(src._x.data(),N,_x.data());
		return *this;
	}

	// array deference
	T operator[](size_t n) const {
		assert(N>n);
		return _x.data()[n];
	};
	T& operator[](size_t n) {
		assert(N>n);
		return _x.data()[n];
	};
	size_t size() const {return N;}

	// numerically simple operations
	covector& operator+=(const covector& src) {
		zaimoni::math::pointwise::in_place_sum(_x.data(),src._x.data(),N);
		return *this;
	}
	covector& operator-=(const covector& src) {
		zaimoni::math::pointwise::in_place_difference(_x.data(),src._x.data(),N);
		return *this;
	}
	covector& operator*=(const T& src) {
		zaimoni::math::pointwise::in_place_scalar_product(_x.data(),src,N);
		return *this;
	}
	covector& operator/=(const T& src) {
		zaimoni::math::pointwise::in_place_scalar_quotient(_x.data(),src,N);
		return *this;
	}
};

template<class T,size_t N>
covector<T,N> operator+(covector<T,N> lhs, const covector<T,N>& rhs)
{
	lhs += rhs;
	return lhs;
}

template<class T,size_t N>
covector<T,N> operator-(covector<T,N> lhs, const covector<T,N>& rhs)
{
	lhs -= rhs;
	return lhs;
}

template<class T,size_t N>
covector<T,N> operator*(covector<T,N> lhs, const T& rhs)
{
	lhs *= rhs;
	return lhs;
}

template<class T,size_t N>
covector<T,N> operator/(covector<T,N> lhs, const T& rhs)
{
	lhs /= rhs;
	return lhs;
}

// matrix products
template<class T,size_t N>
T operator*(const covector<T,N>& lhs, const vector<T,N>& rhs)
{
	return zaimoni::math::Euclidean::dot(lhs,rhs);
}

template<class T, size_t N>
class matrix_square : public matrix_CRTP<matrix_square<T,N>, T>
{
private:
	ZAIMONI_STATIC_ASSERT(0<N);
	ZAIMONI_STATIC_ASSERT((size_t)(-1)/N>=N);
	std::array<T,N*N> _x;
public:
	typedef zaimoni::slice_array<T> row_type;
	typedef zaimoni::slice_array<T> col_type;
	typedef const zaimoni::slice_array<const T> const_row_type;
	typedef const zaimoni::slice_array<const T> const_col_type;

	enum {
		rows = N,
		cols = N
	};

	// decide: constructor destructor assignment operator
	matrix_square() = default;
	matrix_square(T* src) { assert(src); std::copy_n(src,N*N,_x.c_array()); };
	matrix_square(const matrix_square& src) = default;
	matrix_square(matrix_square&& src) = default;
	matrix_square operator=(const matrix_square& src) {
		std::copy_n(src._x.data(),N*N,_x.data());
		return *this;
	}

	// accessors for matrix elements
	T operator()(size_t r,size_t c) const {
		assert(rows>r);
		assert(cols>c);
		return _x.data()[r*cols+c];
	};
	T& operator()(size_t r,size_t c) {
		assert(rows>r);
		assert(cols>c);
		return _x.data()[r*cols+c];
	};

	// numerically simple operations
	matrix_square& operator+=(const matrix_square& src) {
		zaimoni::math::pointwise::in_place_sum(_x.data(),src._x.data(),N*N);
		return *this;
	}
	matrix_square& operator-=(const matrix_square& src) {
		zaimoni::math::pointwise::in_place_difference(_x.data(),src._x.data(),N*N);
		return *this;
	}
	matrix_square& operator*=(const T& src) {
		zaimoni::math::pointwise::in_place_scalar_product(_x.data(),src,N*N);
		return *this;
	}
	matrix_square& operator/=(const T& src) {
		zaimoni::math::pointwise::in_place_scalar_quotient(_x.data(),src,N*N);
		return *this;
	}

	// row/column accessors
	row_type row(size_t r) {assert(N>r); return zaimoni::slice_array<T>(_x.data()+N*r,zaimoni::slice(0,N,1));};
	col_type col(size_t c) {assert(N>c); return zaimoni::slice_array<T>(_x.data()+c,zaimoni::slice(0,N,N));};
	const_row_type row(size_t r) const {assert(N>r); return zaimoni::slice_array<const T>(_x.data()+N*r,zaimoni::slice(0,N,1));};
	const_col_type col(size_t c) const {assert(N>c); return zaimoni::slice_array<const T>(_x.data()+c,zaimoni::slice(0,N,N));};

/*
	// square matrix has a determinant; raw # of terms Factorial(n!)
	// thus, difficult to numerically evaluate accurately -- have to delegate
	// possibly should *NOT* be a member function
	T determinant() const {return zaimoni::math::determinant(_x.data(),N);}
	void prepare_for_determinant();
*/
};

template<class T,size_t N>
matrix_square<T,N> operator+(matrix_square<T,N> lhs, const matrix_square<T,N>& rhs)
{
	lhs += rhs;
	return lhs;
}

template<class T,size_t N>
matrix_square<T,N> operator-(matrix_square<T,N> lhs, const matrix_square<T,N>& rhs)
{
	lhs -= rhs;
	return lhs;
}

template<class T,size_t N>
matrix_square<T,N> operator*(matrix_square<T,N> lhs, const T& rhs)
{
	lhs *= rhs;
	return lhs;
}

template<class T,size_t N>
matrix_square<T,N> operator/(matrix_square<T,N> lhs, const T& rhs)
{
	lhs /= rhs;
	return lhs;
}

// matrix products
// STL slice may be relevant here as design issue, but we don't want to commit to std::valarray
template<class T,size_t N>
matrix_square<T,N> operator*(const matrix_square<T,N>& lhs,const matrix_square<T,N>& rhs)
{
	matrix_square<T,N> tmp;
	size_t r = 0;
	do	{
		typename matrix_square<T,N>::const_row_type lhs_row = lhs.row(r);
		size_t c = 0;
		do	tmp(r,c) = zaimoni::math::Euclidean::dot(lhs_row,rhs.col(c));	// this would depend on arithmetic
		while(N> ++c);
		}
	while(N> ++r);
	return tmp;
}

template<class T,size_t N>
vector<T,N> operator*(const matrix_square<T,N>& lhs,const vector<T,N>& rhs)
{
	vector<T,N> tmp;
	size_t r = 0;
	do	tmp[r] = zaimoni::math::Euclidean::dot(lhs.row(r),rhs);
	while(N> ++r);
	return tmp;
}

// STL slice may be relevant here as design issue, but we don't want to commit to std::valarray
template<class T,size_t N>
covector<T,N> operator*(const covector<T,N>& lhs,const matrix_square<T,N>& rhs)
{
	covector<T,N> tmp;
	size_t c = 0;
	do	tmp[c] = zaimoni::math::Euclidean::dot(lhs,rhs.col(c));
	while(N> ++c);
	return tmp;
}

template<class T,size_t N>
matrix_square<T,N> operator*(const vector<T,N>& lhs,const covector<T,N>& rhs)
{
	matrix_square<T,N> tmp;
	size_t r = 0;
	do	{
		size_t c = 0;
		do	tmp(r,c) = lhs[r]*rhs[c];
		while(N> ++c);
		}
	while(N> ++r);
	return tmp;
}

template<class T, size_t R, size_t C>
class matrix : public matrix_CRTP<matrix<T,R,C>, T>
{
private:
	ZAIMONI_STATIC_ASSERT(0<R);
	ZAIMONI_STATIC_ASSERT(0<C);
	ZAIMONI_STATIC_ASSERT((size_t)(-1)/R>=C);
	std::array<T,R*C> _x;
public:
	typedef zaimoni::slice_array<T> row_type;
	typedef zaimoni::slice_array<T> col_type;
	typedef const zaimoni::slice_array<const T> const_row_type;
	typedef const zaimoni::slice_array<const T> const_col_type;

	enum {
		rows = R,
		cols = C
	};

	// decide: constructor destructor assignment operator
	matrix() = default;
	matrix(T* src) { assert(src); std::copy_n(src,R*C,_x.data()); };
	matrix(const matrix& src) = default;
	matrix(matrix&& src) = default;
	matrix operator=(const matrix& src) {
		std::copy_n(src._x.data(),R*C,_x.data());
		return *this;
	}

	// accessors for matrix elements
	T operator()(size_t r,size_t c) const {
		assert(rows>r);
		assert(cols>c);
		return _x.data()[r*cols+c];
	};
	T& operator()(size_t r,size_t c) {
		assert(rows>r);
		assert(cols>c);
		return _x.data()[r*cols+c];
	};

	// numerically simple operations
	matrix& operator+=(const matrix& src) {
		zaimoni::math::pointwise::in_place_sum(_x.data(),src._x.data(),R*C);
		return *this;
	}
	matrix& operator-=(const matrix& src) {
		zaimoni::math::pointwise::in_place_difference(_x.data(),src._x.data(),R*C);
		return *this;
	}
	matrix& operator*=(const T& src) {
		zaimoni::math::pointwise::in_place_scalar_product(_x.data(),src,R*C);
		return *this;
	}
	matrix& operator/=(const T& src) {
		zaimoni::math::pointwise::in_place_scalar_quotient(_x.data(),src,R*C);
		return *this;
	}

	// row/column accessors
	row_type row(size_t r) {assert(R>r); return zaimoni::slice_array<T>(_x.data()+C*r,zaimoni::slice(0,R,1));};
	col_type col(size_t c) {assert(C>c); return zaimoni::slice_array<T>(_x.data()+c,zaimoni::slice(0,C,R));};
	const_row_type row(size_t r) const {assert(R>r); return zaimoni::slice_array<const T>(_x.data()+C*r,zaimoni::slice(0,R,1));};
	const_col_type col(size_t c) const {assert(C>c); return zaimoni::slice_array<const T>(_x.data()+c,zaimoni::slice(0,C,R));};
};

template<class T, size_t R, size_t C>
matrix<T,R,C> operator+(matrix<T,R,C> lhs, const matrix<T,R,C>& rhs)
{
	lhs += rhs;
	return lhs;
}

template<class T, size_t R, size_t C>
matrix<T,R,C> operator-(matrix<T,R,C> lhs, const matrix<T,R,C>& rhs)
{
	lhs -= rhs;
	return lhs;
}

template<class T, size_t R, size_t C>
matrix<T,R,C> operator*(matrix<T,R,C> lhs, const T& rhs)
{
	lhs *= rhs;
	return lhs;
}

template<class T, size_t R, size_t C>
matrix<T,R,C> operator/(matrix<T,R,C> lhs, const T& rhs)
{
	lhs /= rhs;
	return lhs;
}

// STL slice may be relevant here as design issue, but we don't want to commit to std::valarray
// matrix products
template<class T, size_t R, size_t C>
vector<T,R> operator*(const matrix<T,R,C>& lhs,const vector<T,C>& rhs)
{
	vector<T,R> tmp;
	size_t r = 0;
	do	tmp[r] = zaimoni::math::Euclidean::dot(lhs.row(r),rhs);
	while(R> ++r);
	return tmp;
}

template<class T, size_t R, size_t C>
covector<T,C> operator*(const covector<T,R>& lhs,const matrix<T,R,C>& rhs)
{
	covector<T,C> tmp;
	size_t c = 0;
	do	tmp[c] = zaimoni::math::Euclidean::dot(lhs,rhs.col(c));
	while(C> ++c);
	return tmp;
}

template<class T, size_t R, size_t C>
matrix<T,R,C> operator*(const vector<T,R>& lhs,const covector<T,C>& rhs)
{
	matrix<T,R,C> tmp;
	size_t r = 0;
	do	{
		size_t c = 0;
		do	tmp(r,c) = lhs[r]*rhs[c];
		while(C> ++c);
		}
	while(R> ++r);
	return tmp;
}

template<class T,size_t R1,size_t C1R2,size_t C2>
matrix<T,R1,C2> operator*(const matrix<T,R1,C1R2>& lhs,const matrix<T,C1R2,C2>& rhs)
{
	matrix<T,R1,C2> tmp;
	size_t r = 0;
	do	{
		typename matrix<T,R1,C1R2>::const_row_type lhs_row = lhs.row(r);
		size_t c = 0;
		do	tmp(r,c) = zaimoni::math::Euclidean::dot(lhs_row,rhs.col(c));
		while(C2> ++c);
		}
	while(R1> ++r);
	return tmp;
}

template<class T,size_t R1C2,size_t C1R2,size_t C2>
matrix_square<T,R1C2> operator*(const matrix<T,R1C2,C1R2>& lhs,const matrix<T,C1R2,R1C2>& rhs)
{
	matrix_square<T,R1C2> tmp;
	size_t r = 0;
	do	{
		typename matrix<T,R1C2,C1R2>::const_row_type lhs_row = lhs.row(r);
		size_t c = 0;
		do	tmp(r,c) = zaimoni::math::Euclidean::dot(lhs_row,rhs.col(c));
		while(R1C2> ++c);
		}
	while(R1C2> ++r);
	return tmp;
}

}	// namespace math

// savefile support
template<>
template<class T,size_t N>
struct rw_mode<zaimoni::math::vector<T,N> >
{
	enum {
		group_write = 2,
		group_read = 2
	};
};

template<>
template<class T,size_t N>
struct rw_mode<zaimoni::math::covector<T,N> >
{
	enum {
		group_write = 2,
		group_read = 2
	};
};

}	// namespace zaimoni

#ifdef TEST_APP
// example build line (have to copy from *.hpp to *.cpp or else main not seen
// g++ -omatrix.exe -Llib/host -DTEST_APP matrix.cpp -lz_stdio_log
// If doing INFORM-based debugging
// g++ -std=c++11 -omatrix.exe -DTEST_APP matrix.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util

#include <stdlib.h>
#include <stdio.h>
#include <boost/numeric/interval.hpp>

// console-mode application
#define STRING_LITERAL_TO_STDOUT(A) fwrite(A,sizeof(A)-1,1,stdout)
#define C_STRING_TO_STDOUT(A) fwrite(A,strlen(A),1,stdout)
#define STL_STRING_TO_STDOUT(A) fwrite((A).data(),(A).size(),1,stdout)
#define STL_PTR_STRING_TO_STDOUT(A) fwrite((A)->data(),(A)->size(),1,stdout)

#define INTERVAL_TO_STDOUT(A,UNIT)	\
	if (A.lower()==A.upper()) {	\
		sprintf(buf,"%.16g",A.lower());	\
		C_STRING_TO_STDOUT(buf);	\
		STRING_LITERAL_TO_STDOUT(UNIT);	\
	} else {	\
		STRING_LITERAL_TO_STDOUT("[");	\
		sprintf(buf,"%.16g",A.lower());	\
		C_STRING_TO_STDOUT(buf);	\
		STRING_LITERAL_TO_STDOUT(", ");	\
		sprintf(buf,"%.16g",A.upper());	\
		C_STRING_TO_STDOUT(buf);	\
		STRING_LITERAL_TO_STDOUT("]");	\
		STRING_LITERAL_TO_STDOUT(UNIT);	\
	}

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

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}

#endif

#endif
