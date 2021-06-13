#ifndef ZAIMONI_STL_COW_HPP
#define ZAIMONI_STL_COW_HPP 1

#include <memory>

// copy-on-write -- prioritizing RAM over speed
// intentionally not using std::variant here -- behavior of operator= is not what we want
namespace zaimoni {

template<class T>
class COW final {
	std::shared_ptr<const T> _read;
	std::unique_ptr<T> _write;

public:
	COW() = default;
	COW(const COW& src) = delete;
	COW(COW&& src) = default;
	COW(const std::shared_ptr<const T>& src) noexcept : _read(src) {};
	COW(std::unique_ptr<T>&& src)  noexcept : _write(std::move(src)) {};
	~COW() = default;

	COW(COW& src) : _read(src._read) {
		if (src._write) {
			_read = std::shared_ptr<const T>(src._write.release());
			src = _read;
		}
	}

	COW& operator=(const COW& src) = delete;
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
		if (src._read) return *this = _read;
		if (!src._write) {
			_read.reset();
			_write.reset();
			return *this;
		}
		_read = std::shared_ptr<const T>(src._write.release());
		src = _read;
		return *this;
	}

	const T* get_c() const {
		if (_read) return _read.get();
		if (_write) return _write.get();
		return nullptr;
	}

	const T* get() const { return get_c(); }
	T* get() {
		if (_write) return _write.get();
		if (_read) {
			if constexpr (requires { _read->clone(); })
				_write = std::unique_ptr<T>(_read->clone());
			else
				_write = std::unique_ptr<T>(new T(*_read));

			_read.reset();
			return _write.get();
		}
		return nullptr;
	}

	operator const T* () const { return get_c(); }
	operator T* () { return get(); }
};

}

#endif
