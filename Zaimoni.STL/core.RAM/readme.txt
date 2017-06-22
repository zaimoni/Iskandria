The file memory.cpp is not a header file.

It is the custom C and C++ memory manager (written in C++) that enforces the memory management assumptions of the Zaimoni STL.
* exact allocation of requested bytes
* working _msize extension
* catches invalid free and realloc calls (required for mathematically reliable programming)
* runs guard byte checks in many cases (required for mathematically reliable programming)
* trailing 4 guard bytes are 0 (useful for automatic null termination of strings and arrays of pointers)

The C++ build libz_memory.a also provides definitions of the throwing new and delete operators, and is multi-thread safe.
The C build libz_memory_c.a is not multi-thread safe at this time.

Verify that makeconf.inc is correctly set, then (with GNU make)
* make
* make install
====
Let the number of pointers already allocated be n.  Note that the error checks required for acceptable behavior 
in mathematical programming have order O(n).  The usual design speed for most of the C memory management functions is log(n).
====
At the moment, it only builds in WindowsNT and higher.  Ports to other operating systems would be appreciated, as long as they maintain 
the original design goals.

