#ifndef JSON_HPP
#define JSON_HPP 1

#include "enum_json.h"
#include <string.h>
#include <vector>
#include <map>
#include <iosfwd>

// Cf. www.json.org

// THe intent here is not a full JSON parser, but rather a loose parser that accepts more than the strict standard.

// fromJSON only has to work correctly on default-constructed targets when called from JSON::decode
namespace zaimoni {

class JSON;

}

bool fromJSON(const zaimoni::JSON& src, std::string& dest);
bool fromJSON(const zaimoni::JSON& src, int& dest);
bool fromJSON(const zaimoni::JSON& src, unsigned int& dest);
bool fromJSON(const zaimoni::JSON& src, char& dest);
bool fromJSON(const zaimoni::JSON& src, bool& dest);

zaimoni::JSON toJSON(int src);

namespace zaimoni {

class JSON
{
public:
	static std::map<const std::string, JSON> cache;

	enum mode : unsigned char {
		none = 0,
		object,
		array,
		string,
		literal
	};

private:
	typedef std::map<const std::string, JSON> _object_JSON;
	static const std::string discard_s;

	union {
		std::string* _scalar;
		std::vector<JSON>* _array;
		_object_JSON* _object;
	};
	unsigned char _mode;
public:
	JSON() : _mode(none), _scalar(0) {}
	JSON(mode src) : _mode(src), _scalar(0) {}
	JSON(const JSON& src);
	JSON(JSON&& src);
	JSON(std::istream& src);
	JSON(const std::string& src) : _mode(literal), _scalar(new std::string(src)) {}
	JSON(std::string&& src) : _mode(literal), _scalar(new std::string(src)) {}
	JSON(const char* src,bool is_literal = true) : _mode(is_literal ? literal : string), _scalar(new std::string(src)) {}
	JSON(std::string*& src, bool is_literal = true) : _mode(is_literal ? literal : string), _scalar(src) { src = 0; }
	~JSON() { reset(); };
	friend std::ostream& operator<<(std::ostream& os, const JSON& src);

	JSON& operator=(const JSON& src);
	JSON& operator=(JSON&& src);

	unsigned char mode() const { return _mode; };
	void reset();
	bool empty() const;
	size_t size() const;

	// reserved literal values in strict JSON
	static bool is_null(const JSON& src) { return literal == src._mode && src._scalar && !strcmp("null", src._scalar->c_str()); };

	// scalar evaluation
	bool is_scalar() const { return string <= _mode; }
	const std::string& scalar() const { return string <= _mode ? *_scalar : discard_s; }
	// array evaluation
	JSON& operator[](const size_t key) { return (*_array)[key]; };
	const JSON& operator[](const size_t key) const { return (*_array)[key]; };
	void push(const JSON& src);
	void push(JSON&& src);
	// object evaluation
	JSON* get(const std::string& key) {
		if (object != _mode || !_object || !_object->count(key)) return 0;
		return &((*_object)[key]);
	}
	const JSON* get(const std::string& key) const {
		if (object != _mode || !_object || !_object->count(key)) return 0;
		return &((*_object)[key]);
	}
	bool become_key(const std::string& key) {
		if (decltype(auto) x = get(key)) {
			JSON tmp(std::move(*x));
			*this = std::move(tmp);
			return true;
		}
		return false;
	}
	bool extract_key(const std::string& key, JSON& dest) {
		if (decltype(auto) x = get(key)) {
			dest = std::move(*x);
			unset(key);
			return true;
		}
		return false;
	}

	// use for testing values of array or object.
	// 2018-07-21: not only do not need to allow for function objects, they converted compile-time errors to run-time errors.
	JSON grep(bool (ok)(const JSON&)) const;	// Cf. Perl4+; evaluated in order for arrays
	bool destructive_grep(bool (ok)(const JSON&));	// evaluated in reverse order for arrays
	// key-value pair that is ok is not changed; a not-ok key-value pair that fails post-processing is deleted
	bool destructive_grep(bool (ok)(const std::string& key,const JSON&),bool (postprocess)(const std::string& key, JSON&));	// use for testing object
	bool destructive_merge(JSON& src);	// keys of src end up in ourselves, the destination.  Cf PHP3+
	bool destructive_merge(JSON& src, bool (ok)(const JSON&));	// keys of src end up in ourselves, the destination.  Cf PHP3+
	std::vector<std::string> keys() const;	// Cf. Perl
	void unset(const std::vector<std::string>& src);	// cf PHP 3+ -- clears keys from object
	void unset(const std::string& src);
	// workarounds for the array deference operator not auto-vivifying
	static bool is_legal_JS_literal(const char* src);
	void set(const std::string& src, const JSON& val);
	void set(const std::string& src, JSON&& val);
	void set(const std::string& src, const char* const val) { if (val) set(src, JSON(val, is_legal_JS_literal(val))); }

	bool syntax_ok() const;

	template<class T> static JSON encode(const std::vector<T>& src) {
		JSON ret;
		ret._mode = array;
		if (!src.empty()) {
			ret._array = new std::vector<JSON>();
			for (const auto& x : src) ret._array->push_back(toJSON(x));
		}
		return ret;
	}

	template<class T> static JSON encode(const T* src, size_t n) {
		JSON ret;
		ret._mode = array;
		if (src && 0 < n) {
			ret._array = new std::vector<JSON>();
			size_t i = 0;
			do ret._array->push_back(toJSON(src[i]));
			while (++i < n);
		}
		return ret;
	}

	template<class Key, class T> static JSON encode(const T* dest, size_t n) {
		JSON ret;
		ret._mode = object;
		if (dest && 0 < n) {
			size_t i = 0;
			do {
				if (dest[i]) {
					if (auto json = JSON_key((Key)(i))) ret.set(json,toJSON(dest[i]));
				}
			} while (++i < n);
		}
		return ret;
	}

	template<class Key> static JSON encode(const bool* dest, size_t n) {
		JSON ret;
		ret._mode = array;
		if (dest && 0 < n) {
			size_t i = 0;
			do {
				if (dest[i]) {
					if (auto json = JSON_key((Key)(i))) {
						if (!ret._array) ret._array = new std::vector<JSON>();
						ret._array->push_back(json);
					}
				}
			} while (++i < n);
		}
		return ret;
	}

	template<class T> bool decode(std::vector<T>& dest) const {
		if (array != _mode) return false;
		if (!_array || _array->empty()) {
			dest.clear();
			return true;
		}
		bool ok = true;
		std::vector<T> working;
		for (const auto& x : *_array) {
			T tmp;
			if (fromJSON(x, tmp)) working.push_back(tmp);
			else ok = false;
		}
		if (!working.empty()) dest = std::move(working);
		return ok;
	}

	template<class T> bool decode(std::vector<std::vector<T> >& dest) const {
		if (array != _mode) return false;
		if (!_array || _array->empty()) {
			dest.clear();
			return true;
		}
		bool ok = true;
		std::vector<std::vector<T> > working;
		for (const auto& x : *_array) {
			std::vector<T> tmp;
			if (x.decode(tmp)) working.push_back(tmp);
			else ok = false;
		}
		if (!working.empty()) dest = std::move(working);
		return ok;
	}

	template<class T> bool decode(T* dest,size_t n) const {
		if (array != _mode || !dest) return false;
		if (!_array || _array->empty()) return 0 == n;
		if (_array->size() != n) return false;
		bool ok = true;
		size_t i = 0;
		do {
			if (!fromJSON((*_array)[i], dest[i])) ok = false;
		} while(++i < n);
		return ok;
	}

	template<class Key, class T> bool decode(T* dest, size_t n) const {
		if (object != _mode || !dest) return false;
		if (!_object || _object->empty()) return true;
		zaimoni::JSON_parse<Key> parse;
		bool ok = true;
		for (auto& x : *_object) {
			auto key = parse(x.first);
			if (zaimoni::JSON_parse<Key>::origin > key) {
				ok = false;
				continue;
			}
			if (!fromJSON(x.second, dest[key])) ok = false;
		}
		return ok;
	}

	template<class Key> bool decode(bool* dest, size_t n) const {
		if (array != _mode || !dest) return false;
		if (!_array || _array->empty()) return true;
		zaimoni::JSON_parse<Key> parse;
		bool ok = true;
		for (auto& x : *_array) {
			if (!x.is_scalar()) {
				ok = false;
				continue;
			}
			auto key = parse(x.scalar());
			if (zaimoni::JSON_parse<Key>::origin > key) {
				ok = false;
				continue;
			}
			dest[key] = true;
		}
		return ok;
	}

protected:
	JSON(std::istream& src,unsigned long& line, char& first, bool must_be_scalar = false);
private:
	void finish_reading_object(std::istream& src, unsigned long& line);
	void finish_reading_array(std::istream& src, unsigned long& line);
	void finish_reading_string(std::istream& src, unsigned long& line, char& first);
	void finish_reading_literal(std::istream& src, unsigned long& line, char& first);

	static std::ostream& write_array(std::ostream& os, const std::vector<JSON>& src, int indent = 1);
	static std::ostream& write_object(std::ostream& os, const _object_JSON& src, int indent = 1);
	std::ostream& write(std::ostream& os, int indent) const;
};

template<> inline JSON JSON::encode<const char*>(const std::vector<const char*>& src) {
	JSON ret;
	ret._mode = array;
	if (0 < src.size()) {
		ret._array = new std::vector<JSON>();
		for (const auto& x : src) {
			if (x) ret._array->push_back(x);
		}
	}
	return ret;
}

// in this case, std::vector's inaction on destruction vs raw pointers is fine
template<> inline bool JSON::decode<const char*>(std::vector<const char*>& dest) const {
	if (array != _mode) return false;
	if (!_array || _array->empty()) {
		dest.clear();
		return true;
	}
	bool ok = true;
	std::vector<const char*> working;
	for (const auto& x : *_array) {
		if (!x.is_scalar() || !x._scalar || x._scalar->empty()) ok = false;
		else working.push_back(x._scalar->c_str());
	}
	if (!working.empty()) swap(working, dest);
	return ok;
}

}	// namespace zaimoni

#endif