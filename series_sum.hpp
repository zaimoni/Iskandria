// series_sum.hpp

#ifndef SERIES_SUM_HPP
#define SERIES_SUM_HPP 1

#include "overprecise.hpp"
#include <vector>
#include <algorithm>

namespace zaimoni {
namespace math {

template<class T>
class series_sum
{
private:
	std::vector<T> _x;	// XXX not appropriate for self-destructive evaluation
public:
	series_sum() = default;
	series_sum(const std::vector<T>& src) : _x(src) {};
	series_sum(std::vector<T>&& src) : _x(src) {};
	ZAIMONI_DEFAULT_COPY_DESTROY_ASSIGN(series_sum);

	void push_back(T src)
	{
		assert(!isNaN(src));
		// cf trivial_sum
		if (exact_equals(src,0)) return;	// XXX fails for intervals: std::terminate
#ifdef ZAIMONI_USING_STACKTRACE
		zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif
		const bool no_further_op = _x.empty();
		if (!no_further_op) {
			const bool was_finite = isFinite(_x.front());
			const int code = trivial_sum(_x.front(),src);
			if (-1==code) _x.front() = src;
			if (was_finite && !isFinite(_x.front()))
				{
				size_t i = _x.size();
				size_t ub = i-1;
				T* _raw = _x.data();
				while(1 <= --i)
					switch(trivial_sum(_raw[0],_raw[i]))
					{
					case -1: swap(_raw[0],_raw[i]);
					// intentional fall-through
					case 1: if (i<ub) swap(_raw[i],_raw[ub]);
						ub--;
						break;						
					default: break;
					}
				++ub;
				if (ub<_x.size()) _x.resize(ub);
				}
			if (code) return;
		}
		_x.push_back(src);
		if (!no_further_op) _rearrange_sum();
	};

	// also want: self-destructive version
	typename interval_type<T>::type eval()
	{
#ifdef ZAIMONI_USING_STACKTRACE
		zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif
		switch(_x.size())
		{
		case 0: return int_as<0,typename interval_type<T>::type >();
		case 1: return _x.front();
		case 2: break;
		default: std::stable_sort(_x.begin(),_x.end(), fp_compare<T>::good_sum_lt); break;	// sort in strictly increasing exponent order
		};
		typename interval_type<T>::type ret(int_as<0, typename interval_type<T>::type>());
		for (auto i : _x) ret += i;
		return ret;
	}

private:
	void _rearrange_sum()
	{
		assert(2 <= _x.size());
#ifdef ZAIMONI_USING_STACKTRACE
		zaimoni::ref_stack<zaimoni::stacktrace, const char*> log(zaimoni::stacktrace::get(), __PRETTY_FUNCTION__);
#endif
restart:
		size_t test_vertex = _x.size()-1;
retry:
		T* const _raw = _x.data();
		// we almost certainly have to improve the O(...) of this, eyeballs as cubic O(n^3)
		// should exploit that our only caller has just pushed the "unstable numeral"
		do {
			size_t i = test_vertex;
			do	{
				switch(trivial_sum(_raw[--i], _raw[test_vertex]))
				{
				case -1:	// lhs gone
					if (1 < test_vertex - i) swap(_raw[i], _raw[test_vertex - 1]);
					swap(_raw[test_vertex - 1], _raw[test_vertex]);
					if (test_vertex < _x.size() - 1) swap(_raw[test_vertex], _raw[_x.size() - 1]);
					_x.pop_back();
					if (1 >= _x.size()) return;
					--test_vertex;
					goto retry;
				case 1:		// rhs gone
					if (1 < test_vertex - i) swap(_raw[i], _raw[test_vertex - 1]);
					if (test_vertex < _x.size() - 1) swap(_raw[test_vertex], _raw[_x.size() - 1]);
					_x.pop_back();
					if (1 >= _x.size()) return;
					--test_vertex;
					goto retry;
				}
				switch(rearrange_sum(_raw[i], _raw[test_vertex]))
				{
				case 1:	// rhs deleted
					if (1<test_vertex-i) swap(_raw[i],_raw[test_vertex-1]);
					if (test_vertex<_x.size()-1) swap(_raw[test_vertex],_raw[_x.size()-1]);
					--test_vertex;
					if (exact_equals(_raw[test_vertex],0))
						{
						if (test_vertex<_x.size()-2) swap(_raw[test_vertex],_raw[_x.size()-2]);
						_x.pop_back();
						_x.pop_back();
						if (1 >= _x.size() || _x.size() <= test_vertex) return;
						goto restart;
						}
					_x.pop_back();
					if (1 >= _x.size()) return;
					goto retry;
				case 2:	// lhs, rhs altered:
					if (2 > test_vertex) break;
					if (1<test_vertex-i) swap(_raw[i],_raw[test_vertex-1]);
					test_vertex--;
					i = test_vertex;
					break;
				default: break;	// do not recognize the change code, assume zero/no-op
				}
			} while(0 < i);
			}
		while(++test_vertex < _x.size());
	};
};

template<class T>
typename interval_type<T>::type eval(const series_sum<T>& x)
{
	return x.eval();
}

}	// namespace math
}	// namespace zaimoni

#endif
