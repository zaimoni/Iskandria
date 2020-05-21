// matrix.hpp

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "Zaimoni.STL/rw.hpp"
#include "slice.hpp"
#include "Euclidean.hpp"
#include "Zaimoni.STL/augment.STL/array"
#include <algorithm>
#include <initializer_list>

namespace zaimoni {

template<class T>
auto linear_encode(const T* lb, const T* ub, T* src, size_t n) -> unconditional_t<size_t, typename types<T>::norm>
{
	size_t ret = 0;
	size_t prior_span = 1;
	size_t i = n;
	while (0 < n) {
		assert(*lb < *ub);
		assert(*lb <= *src);
		assert(*src < *ub);
		auto max_delta = pos_diff(*ub, *lb);
		if (1 < max_delta) {
			auto delta = pos_diff(*src, *lb);
			ret *= prior_span;
			ret += delta;
			prior_span *= max_delta;
		}
		if (0 >= --n) break;
		++ub;
		++lb;
		++src;
	}
	return ret;
}

template<class T>
auto linear_decode(const T* lb, const T* ub, size_t code, T* dest, size_t n) -> unconditional_t<void, typename types<T>::norm>
{
	size_t ret = 0;
	size_t i = n;
	lb += (n - 1);
	lb += (n - 1);
	lb += (n - 1);
	while (0 < n) {
		assert(*lb < *ub);
		assert(*lb <= *src);
		assert(*src < *ub);
		auto max_delta = pos_diff(*ub, *lb);
		if (1 < max_delta) {
			auto delta = pos_diff(*src, *lb);
			*dest = code % max_delta;
			code /= max_delta;
		}
		if (0 >= --n) break;
		--ub;
		--lb;
		--dest;
	}
	return ret;
}

namespace math {

template<class T, class U>
void clamp_lb(T& dest, const U& lb) {
	if (lb > dest) dest = lb;
}

template<class T, class U>
void clamp_ub(T& dest, const U& ub) {
	if (ub < dest) dest = ub;
}

namespace pointwise {

template<class I_Iter, class OP>
bool all(I_Iter lhs, I_Iter rhs, size_t n, OP rel)
{
	assert(0 < n);
	assert(lhs);
	assert(rhs);
	while (0 < n) {
		if (!rel(*(lhs++), *(rhs++))) return false;
		--n;
	};
	return true;
}

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

template<class IO_Iter, class I_Iter>
void clamp_lb(IO_Iter dest, I_Iter src, size_t n)
{
	assert(0 < n);
	assert(dest);
	assert(src);
	while (0 < n) {
		zaimoni::math::clamp_lb(*(dest++), *(src++));
		--n;
	};
}

template<class IO_Iter, class I_Iter>
void clamp_ub(IO_Iter dest, I_Iter src, size_t n)
{
	assert(0 < n);
	assert(dest);
	assert(src);
	while (0 < n) {
		zaimoni::math::clamp_ub(*(dest++), *(src++));
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
	static_assert(0<N);
public:
	typedef T coord_type;
	enum {
		rows = N,
		cols = 1
	};

	constexpr vector() : _x({}) {
		if constexpr (std::is_trivially_constructible_v<T>) _x = zaimoni::array::fill<N>(int_as<0, T>());
	}
	constexpr vector(const std::initializer_list<T>& src) : _x(src) {}
	explicit vector(const T& src) { _x.fill(src); }
	vector(const T* src) { assert(src); std::copy_n(src, N, _x.data()); };
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(vector);

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
	auto data() { return _x.data(); }
	auto data() const { return _x.data(); }

	// numerically simple operations
	vector& operator+=(const vector& src) {
		zaimoni::math::pointwise::in_place_sum(_x.data(),src._x.data(),N);
		return *this;
	}
	template<class U>
	std::enable_if_t<sizeof(T)>=sizeof(U), vector&>
	operator+=(const vector<U, N>& src) {
		zaimoni::math::pointwise::in_place_sum(_x.data(), src.data(), N);
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

template<class T, class U, size_t N>
bool operator==(const vector<T, N>& lhs, const vector<U, N>& rhs)
{
	size_t ub = N;
	do {
		--ub;
		if (lhs[ub] != rhs[ub]) return false;
	} while(0 < ub);
	return true;
}

template<class T, class U, size_t N>
bool operator!=(const vector<T, N>& lhs, const vector<U, N>& rhs)
{
	return !(lhs == rhs);
}

template<class T,size_t N>
vector<T,N> operator+(vector<T,N> lhs, const vector<T,N>& rhs)
{
	lhs += rhs;
	return lhs;
}

template<class T, class U, size_t N>
vector<T, N> operator+(vector<T, N> lhs, const vector<U, N>& rhs)
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

template<class T, class U, size_t N>
void clamp_lb(vector<T, N>& dest, const vector<U, N>& lb)
{
	zaimoni::math::pointwise::clamp_lb(dest.data(), lb.data(), N);
}

template<class T, class U, size_t N>
void clamp_ub(vector<T, N>& dest, const vector<U, N>& ub)
{
	zaimoni::math::pointwise::clamp_ub(dest.data(), ub.data(), N);
}

template<class T, size_t N, class OP>
bool pointwise_test(const vector<T, N>& lhs, const vector<T, N>& rhs, OP rel)
{
	return zaimoni::math::pointwise::all(lhs.data(), rhs.data(), N, rel);
}

template<class T,size_t N>
class covector : public matrix_CRTP<covector<T,N>, T>
{
private:
	std::array<T,N> _x;
	static_assert(0<N);
public:
	typedef T coord_type;
	enum {
		rows = 1,
		cols = N
	};

	constexpr covector() : _x({}) {
		if constexpr (std::is_trivially_constructible_v<T>) _x = zaimoni::array::fill<N>(int_as<0, T>());
	}
	constexpr covector(const std::initializer_list<T>& src) : _x(src) {}
	explicit covector(const T& src) { _x.fill(src); }
	covector(const T* src) { assert(src); std::copy_n(src, N, _x.data()); };
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(covector);

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
	static_assert(0<N);
	static_assert((size_t)(-1)/N>=N);
	std::array<T,N*N> _x;
public:
	typedef T coord_type;
	typedef zaimoni::slice_array<T> row_type;
	typedef zaimoni::slice_array<T> col_type;
	typedef const zaimoni::slice_array<const T> const_row_type;
	typedef const zaimoni::slice_array<const T> const_col_type;

	enum {
		rows = N,
		cols = N
	};

	constexpr matrix_square() : _x({}) {
		if constexpr (std::is_trivially_constructible_v<T>) _x = zaimoni::array::fill<N*N>(int_as<0, T>());
	}
	constexpr matrix_square(const std::initializer_list<T>& src) : _x(src) {}
	explicit matrix_square(const T& src) {
		if constexpr(std::is_trivially_constructible<T>::value)
			_x.fill(int_as<0,T>());
		size_t i = N;
		do {
			--i;
			_x.data()[i * (cols + 1)] = src;
		} while (0 < i);
	}
	matrix_square(const T* src) { assert(src); std::copy_n(src,N*N,_x.c_array()); };
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(matrix_square);

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

template<class T, size_t N>
struct mult_inv<matrix_square<T, N> >
{
	typedef std::pair<typename zaimoni::types<T>::norm, size_t > norm_index;
	typedef std::array<bool, N> already_index;
	typedef std::vector<size_t> resolve_index;

	mult_inv() = default;
	~mult_inv() = default;

	static bool matrix_op_ok(const T& x)
	{
		return isFinite(x) && !causes_division_by_zero(x);
	}

	static void elementary_transpose(matrix_square<T, N>& src, matrix_square<T, N>& dest, const size_t r, const size_t r_2)
	{
		assert(r != r_2);
		size_t c = N;
		do {
			--c;
			std::swap(src(r, c), src(r_2, c));
			std::swap(dest(r, c), dest(r_2, c));
		} while(0 < c);
	}

	[[nodiscard]] static bool elementary_force_one(matrix_square<T, N>& src, matrix_square<T, N>& dest, const size_t r_0, const size_t c_0)
	{
		const auto& div_by = src(r_0, c_0);
		if (!matrix_op_ok(div_by)) return false;

		std::array<T, N> src_image;
		std::array<T, N> dest_image;
		size_t c = N;
		do {
			--c;
			const auto& dest_num = dest(r_0, c);
			if (is_zero(dest_num)) dest_image[c] = int_as<T>(0);
			else {
				dest_image[c] = dest_num / div_by;
				if (!matrix_op_ok(dest_image[c])) return false;
			}

			if (c_0 == c) src_image[c] = int_as<T>(1);
			else {
				const auto& src_num = src(r_0, c);
				if (is_zero(src_num)) src_image[c] = int_as<T>(0);
				else {
					src_image[c] = src_num / div_by;
					if (!matrix_op_ok(src_image[c])) return false;
				}
			}
		} while (0 < c)
		c = N;
		do {
			--c;
			std::swap(src(r_0, c), src_image[c]);
			std::swap(dest(r_0, c), dest_image[c]);
		} while (0 < c);
		return true;
	}

private:
	// may not actually need to be ACID
	[[nodiscard]] static bool elementary_force_zero(matrix_square<T, N>& src, matrix_square<T, N>& dest, const size_t r_0, const size_t c_0, const size_t r_lever)
	{
		const auto& target = src(r_0, c_0);
		if (is_zero(target)) return true;	// no-op
		const auto& lever = src(r_0, c_lever);
		if (!matrix_op_ok(lever)) return false;

		const T scale(src(r_0, c_0) / lever);

		std::array<T, N> src_stage;
		std::array<T, N> dest_stage;
		size_t c = N;
		do {
			--c;
			const auto& src_num = src(r_lever, c);
			if (is_zero(src_num)) src_stage[c] = int_as<T>(0);
			else if (c == c_0) src_stage[c] = target;
			else {
				src_stage[c] = src_num*scale;
				if (!matrix_op_ok(src_stage[c])) return false;
			}
			const auto& dest_num = dest(r_lever, c);
			if (is_zero(dest_num)) dest_stage[c] = int_as<T>(0);
			else if (c == c_0) dest_stage[c] = scale;
			else {
				dest_stage[c] = dest_num * scale;
				if (!matrix_op_ok(dest_stage[c])) return false;
			}
		} while (0 < c);
		std::array<T, N> src_image;
		std::array<T, N> dest_image;
		c = N;
		do {
			--c;
			if (c == c_0) src_image[c] = int_as<T>(0);
			else if (!is_zero(src_stage[c])) {
				src_image[c] = src(r_0, c) - src_stage[c];
				if (!matrix_op_ok(src_image[c])) return false;
			}
			if (!is_zero(dest_stage[c])) {
				dest_image[c] = dest(r_0, c) - dest_stage[c];
				if (!matrix_op_ok(dest_image[c])) return false;
			}
		} while (0 < c);
		c = N;
		do {
			--c;
			std::swap(src(r_0, c), src_image[c]);
			std::swap(dest(r_0, c), dest_image[c]);
		} while (0 < c);
		return true;
	}

public:
	// not ACID; relies on caller using working copies to be reasonably safe
	[[nodiscard]] static bool elementary_force_zero(matrix_square<T, N>& src, matrix_square<T, N>& dest, const size_t c_0, const size_t r_lever)
	{
		size_t r = N;
		do {
			if (r_lever == --r) continue;
			if (!elementary_force_zero(src, dest, r, c_0, r_lever)) return false;
		} while (0 < r);
		return true;
	}

	[[nodiscard]] static bool col_norms(const matrix_square<T, N>& src, resolve_index& resolve_cols, size_t& col, std::vector<norm_index>& ret)
	{
		assert(!resolve_cols.empty());

		std::vector<norm_index> working(N);	// XXX C realloc looks tempting here for intervals; allocator not an option for that
		size_t ub = 0;
		size_t dies = SIZE_MAX;
		for (auto c : resolve_cols) {
			++dies;
			size_t r = N;
			do {
				const auto& test = src(--r, c);
				if (!matrix_op_ok(test)) continue;
				working[ub++] = norm_index(norm(test), r)
			} while (0 < r);
		    if (0 < ub) {
				col = c;
				ret = std::move(std::vector<norm_index>(working.begin(), working.begin() + ub));
				resolve_cols.erase(resolve_cols.begin(), resolve_cols.begin() + dies);
				return true;
			}
		}
		return false;
	}

private:
	[[nodiscard]] static bool sort_norms(const norm_index& lhs, const norm_index&rhs) {
		if (rhs.first < lhs.first) return true;
		if (lhs.first < rhs.first) return false;
		return lhs.second < rhs.second;
	}

	// C error code convention
	[[nodiscard]] static int complete_inversion(matrix_square<T, N>& src, matrix_square<T, N>& dest, resolve_index resolve_cols, const size_t row, const size_t c_resolve) {
		assert(src);
		assert(dest);
		if (   !elementary_force_zero(src, dest, row, c_resolve)
			|| !elementary_force_one(src, dest, row, c_resolve)) {
			// something went wrong
			return -1;
		}
		if (row != c_resolve) elementary_transpose(src, dest, row, c_resolve);
		while (!resolve_cols.empty()) {
			resolve_index c_norms;
			if (!col_norms(src, resolve_cols, c_resolve, c_norms)) return INT_MIN;
			const size_t norm_options = c_norms.size();
			if (1 == norm_options) {
				if (   !elementary_force_zero(src, dest, c_norms.front().second, c_resolve)
					|| !elementary_force_one(src, dest, c_norms.front().second, c_resolve)) return -1;
				if (c_norms.front().second != c_resolve) elementary_transpose(src, dest, c_norms.front().second, c_resolve);
			} else {
				std::stable_sort(resolve_cols.begin(), resolve_cols.end(), sort_norms);
				std::unique_ptr<matrix_square<T, N> > working_src(new matrix_square<T, N>(src));
				std::unique_ptr<matrix_square<T, N> > working_dest(new matrix_square<T, N>(dest));

				// iterate over options, use first one that works
				for (const auto& x : resolve_cols) {
					int C_err_code = complete_inversion(*working_src, *working_dest, resolve_cols, x.second, c_resolve);
					if (0 <= C_err_code) {
						dest = std::move(*working_dest);
						return 0;
					}
					*working_src = src;
					*working_dest = dest;
				}
				return -1;
			}
		}
		return 0;
	}

public:
	matrix_square<T, N> operator()(matrix_square<T, N> src) {
		matrix_square<T, N> dest(T(1));

		size_t r = 0;
		resolve_index resolve_cols(N);
		do resolve_cols[r] = r;
		while (N > ++r);

		// our priority is to avoid/minimize numerical error without truly excessive time.  In general, this entails depth n for an nxn matrix
		size_t c_resolve = 0;	// column to resolve
		while (!resolve_cols.empty()) {
			resolve_index c_norms;
			if (!col_norms(src, resolve_cols, c_resolve, c_norms)) throw numeric_error("matrix inversion failed: lousy condition number?");
			const size_t norm_options = c_norms.size();
			if (1 == norm_options) {
				if (   !elementary_force_zero(src, dest, c_norms.front().second, c_resolve)
				    || !elementary_force_one(src, dest, c_norms.front().second, c_resolve)) throw numeric_error("matrix inversion failed");
				if (c_norms.front().second != c_resolve) elementary_transpose(src, dest, c_norms.front().second, c_resolve);
			} else {
				std::stable_sort(resolve_cols.begin(), resolve_cols.end(), sort_norms);
				{	// scoping brace
				std::unique_ptr<matrix_square<T, N> > working_src(new matrix_square<T, N>(src));
				std::unique_ptr<matrix_square<T, N> > working_dest(new matrix_square<T, N>(dest));

				// iterate over options, use first one that works
				for (const auto& x : resolve_cols) {
					int C_err_code = complete_inversion(*working_src, *working_dest, resolve_cols, x.second, c_resolve);
					if (0 <= C_err_code) {
						dest = std::move(*working_dest);
						goto done;
					}
					*working_src = src;
					*working_dest = dest;
				}
				}	// end scoping brace
				throw numeric_error("matrix inversion failed: lousy condition number?");
			}
		}
done:
		return dest;
	}
};

template<class T, size_t R, size_t C>
class matrix : public matrix_CRTP<matrix<T,R,C>, T>
{
private:
	static_assert(0<R);
	static_assert(0<C);
	static_assert((size_t)(-1)/R>=C);
	std::array<T,R*C> _x;
public:
	typedef T coord_type;
	typedef zaimoni::slice_array<T> row_type;
	typedef zaimoni::slice_array<T> col_type;
	typedef const zaimoni::slice_array<const T> const_row_type;
	typedef const zaimoni::slice_array<const T> const_col_type;

	enum {
		rows = R,
		cols = C
	};

	constexpr matrix() : _x({}) {
		if constexpr (std::is_trivially_constructible_v<T>) _x = zaimoni::array::fill<R*C>(int_as<0, T>());
	}
	constexpr matrix(const std::initializer_list<T>& src) : _x(src) {}
	matrix(const T* src) { assert(src); std::copy_n(src,R*C,_x.data()); };
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(matrix);

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

template<class T,size_t R1C2,size_t C1R2>
matrix_square<T,R1C2> operator*(const matrix<T,R1C2,C1R2>& lhs,const matrix<T,C1R2,R1C2>& rhs)
{
	matrix_square<T,R1C2> tmp;
	size_t r = 0;
	do	{
		auto lhs_row = lhs.row(r);
		size_t c = 0;
		do	tmp(r,c) = zaimoni::math::Euclidean::dot(lhs_row,rhs.col(c));
		while(R1C2> ++c);
		}
	while(R1C2> ++r);
	return tmp;
}

}	// namespace math

template<class T>
struct _make<zaimoni::math::vector<T,2> >
{
	zaimoni::math::vector<T, 2> operator()(typename const_param<T>::type x0, typename const_param<T>::type x1) {
		T src[2] = { x0, x1 };
		return zaimoni::math::vector<T, 2>(src);
	}
};

// savefile support
template<class T,size_t N>
struct rw_mode<zaimoni::math::vector<T,N> >
{
	enum {
		group_write = 2,
		group_read = 2
	};
};

template<class T,size_t N>
struct rw_mode<zaimoni::math::covector<T,N> >
{
	enum {
		group_write = 2,
		group_read = 2
	};
};

}	// namespace zaimoni

#endif
