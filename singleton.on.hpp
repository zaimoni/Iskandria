// singleton.on.hpp
// macro support for declaring a singleton class

#undef ISK_SINGLETON_HEADER
#undef ISK_SINGLETON_BODY

// if we had move constructors they would be included below

#define ISK_SINGLETON_HEADER(T)	\
private:	\
	T();	\
	T(const T& src);	\
	void operator=(const T& src);	\
protected:	\
	~T();	\
public:	\
	static T& get()

#define ISK_SINGLETON_BODY(T)	\
T& T::get() 	\
{	\
	static T oaoo;	/* "Once And Only Once" */	\
	return oaoo;	\
}


