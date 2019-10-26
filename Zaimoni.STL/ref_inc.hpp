#ifndef ZAIMONI_STL_REF_INC_HPP
#define ZAIMONI_STL_REF_INC_HPP 1

namespace zaimoni {

	template<class T>
	class ref_semaphore
	{
	private:
		T& _x;
		bool _locked;
	public:
		ref_semaphore(T& x) : _x(x), _locked(1 >= ++_x) {};
		~ref_semaphore() { --_x; }
		ref_semaphore(const ref_semaphore& src) = delete;
		ref_semaphore(ref_semaphore&& src) = delete;
		ref_semaphore& operator=(const ref_semaphore& src) = delete;
		ref_semaphore& operator=(ref_semaphore&& src) = delete;

		bool locked() const { return _locked; }
	};

}

#endif
