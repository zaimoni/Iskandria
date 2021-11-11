#ifndef ZAIMONI_STL_COW_HPP
#define ZAIMONI_STL_COW_HPP 1

#include <memory>
#include <variant>
#include <optional>

// copy-on-write -- prioritizing RAM over speed
namespace zaimoni {

template<class T>
class COW final {
	std::shared_ptr<const T> _read; // std::variant here would break operator=
	std::unique_ptr<T> _write;

public:
	COW() = default;

	// deleting this constructor is exceptionally painful
	COW(const COW& src) : _read(src._read) {
		if (src._write) _write = src._w_clone();
	}

	COW(COW&& src) = default;
	COW(const std::shared_ptr<const T>& src) noexcept : _read(src) {}
	COW(std::unique_ptr<T>&& src)  noexcept : _write(std::move(src)) {}
	COW(T* src) noexcept : _write(src) {}
	~COW() = default;

	COW(COW& src) : _read(src._read) {
		if (src._write) {
			_read = std::shared_ptr<const T>(src._write.release());
			src = _read;
		}
	}

	COW& operator=(COW&& src) = default;

	COW& operator=(const std::shared_ptr<const T>& src) noexcept {
		_read = src;
		_write.reset();
		return *this;
	}

	COW& operator=(std::unique_ptr<T>&& src) noexcept {
		_write = std::move(src);
		_read.reset();
		return *this;
	}

	COW& operator=(COW& src) noexcept {
		if (this == &src) return *this;
		if (src._read) return *this = src._read;
		if (!src._write) {
			_read.reset();
			_write.reset();
			return *this;
		}
		_read = std::shared_ptr<const T>(src._write.release());
		src = _read;
		return *this;
	}

	// deleting this operator is exceptionally painful
	COW& operator=(const COW& src) {
		if (this == &src) return *this;
		if (src._write) return *this = src._w_clone();
		return *this = src._read;
	}

	explicit operator bool() const { return _read || _write; }

	const T* get_c() const {
		if (_read) return _read.get();
		if (_write) return _write.get();
		return nullptr;
	}

	const T* get() const { return get_c(); }

	T* get() { // multi-threaded: race condition against operoator=(COW& src)
		if (_read) _rw_clone();
		if (_write) return _write.get();
		return nullptr;
	}

	const T* operator->() const { return get_c(); }
	T* operator->() { return get(); }

	/// <returns>std::nullopt, or .second is non-null and .first is non-null if non-const operations are not logic errors</returns>
	template<class U> std::optional<std::pair<U*, const U*> > get_rw() {
		if (auto r = _get_rw<U>()) {
			std::pair<U*, const U*> ret;
			auto pre_exec = std::get_if<U*>(&(*r));
			ret.second = ret.first = pre_exec ? *pre_exec : nullptr;
			if (!ret.second) {
				if (auto pre_test = std::get_if<const U*>(&(*r))) ret.second = *pre_test;
				else return std::nullopt;
			}
			return ret;
		}
		return std::nullopt;
	}

/*
	operator const T* () const { return get_c(); }
	operator T* () { return get(); }
*/
	// value equality of contents
	friend bool operator==(const COW& lhs, const COW& rhs) {
		if (&lhs == &rhs) return true;
		if (lhs._read) {
			if (rhs._read) {
				return 0 == *lhs._read <=> *rhs._read;
			} else if (rhs._write) {
				return 0 == *lhs._read <=> *rhs._write;
			} else return false;
		} else if (lhs._write) {
			if (rhs._read) {
				return 0 == *lhs._write <=> *rhs._read;
			} else if (rhs._write) {
				return 0 == *lhs._write <=> *rhs._write;
			} else return false;
		} else return !rhs._read && !rhs._write;
	}

private:
	auto _w_clone() const requires requires() { _write->clone(); } {
		return std::unique_ptr<T>(_write->clone());
	}

	void _rw_clone() requires requires() { _read->clone(); } {
		_write = std::unique_ptr<T>(_read->clone());
		_read.reset();
	}

	template<class U> std::optional<std::variant<U*, const U*> > _get_rw() {
		if (_read) {
			if (auto test = dynamic_cast<const U*>(_read.get())) return test;
		};
		if (_write) {
			if (auto test = dynamic_cast<U*>(_write.get())) return test;
		};
		return std::nullopt;
	}
};

}	// namespace zaimoni

#endif
