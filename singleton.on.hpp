// singleton.on.hpp
// macro support for declaring a singleton class

#undef ISK_SINGLETON_HEADER
#undef ISK_SINGLETON_BODY

// C++99 version won't have the move constructor line

#define ISK_SINGLETON_HEADER(T)	\
private:	\
	T();	\
	T(const T& src) = delete;	\
	T(T&& src) = delete;	\
	void operator=(const T& src) = delete;	\
	void operator=(T&& src) = delete;	\
	~T();	\
public:	\
	static T& get()

#define ISK_SINGLETON_HEADER_DEFAULT_DESTRUCTOR(T)	\
private:	\
	T();	\
	T(const T& src) = delete;	\
	T(T&& src) = delete;	\
	void operator=(const T& src) = delete;	\
	void operator=(T&& src) = delete;	\
	~T() = default;	\
public:	\
	static T& get()

#define ISK_SINGLETON_BODY(T)	\
T& T::get() 	\
{	\
	static T oaoo;	/* "Once And Only Once" */	\
	return oaoo;	\
}


