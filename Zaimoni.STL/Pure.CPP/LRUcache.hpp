#ifndef LRUCACHE_HPP
#define LRUCACHE_HPP 1

#include <list>
#include <map>

namespace zaimoni {

// textbook cache that uses last-recently-used deletion policy
template<class K, class V>
class LRUcache
{
private:
	std::list<std::pair<K, V> > _values;
	typedef typename std::list<std::pair<K, V> >::iterator iter;
	std::map<K, iter> _index;
public:
	LRUcache() = default;
	LRUcache(const LRUcache& src) = delete;
	LRUcache(LRUcache&& src) = delete;
	~LRUcache() = default;	// i.e., only RAII values
	LRUcache& operator=(const LRUcache& src) = delete;
	LRUcache& operator=(LRUcache&& src) = delete;

	// READ
	V* get(const K& key) const {
		if (auto working = _index.find(key); _index.end() != working) {
			auto ret = working->second;
			_values.splice(_values.begin(), _values, ret);
			return &(ret->second);
		}
		return 0;
	}
	const V* get_c(const K& key) const {
		if (auto working = _index.find(key); _index.end() != working) {
			auto ret = working->second;
			_values.splice(_values.begin(), _values, ret);
			return &(ret->second);
		}
		return 0;
	}

	// CREATE/UPDATE
	void set(const K& key, const V& val) {
		if (auto test = _index.find(key); _index.end() != test) {
			auto working = test->second;
			working->second = val;
			_values.splice(_values.begin(), _values, working);
			return;
		}
		_values.emplace_front(key, val);
		_index[key] = _values.begin();
	}

	// DELETE
	void unset(const K& key) {
		if (auto test = _index.find(key); _index.end() != test) {
			auto working = test->second;
			_index.erase(test);
			unlink(working);
		}
	}

	auto size() const { return _index.size(); }
	void expire(size_t n) {
		while (0 < n && !_values.empty()) {
			--n;
			auto working = _values.end() - 1;
			_index.erase(working->first);
			unlink(working);
		}
	}
private:
	void unlink(iter mortal) {
		_values.erase(mortal);
		if (_index.empty() && !_values.empty()) _values.clear();
		if (_values.empty() && !_index.empty()) _index.clear();
	}
};

}




#endif
