# Makefile for OS/Win32 directory
# assumes GNU make

.SUFFIXES:
.SUFFIXES: .cpp .a .o

include ../../POSIX.inc

CFLAGS := -Os -Wall -Wextra -finline -Winline -Wdisabled-optimization --param max-inline-insns-single=500 -c -pipe -DNDEBUG
CXXFLAGS := $(CFLAGS)

LIBS := libz_mutex.a

# GNU targets we know about
all: $(LIBS)

clean:
	rm -f *.a *.o

host_install: $(addprefix ../../../$(host_libdir)/, $(LIBS))

# rules
mutex.o: mutex.cpp
	$(CXX) -c mutex.cpp $(CXXFLAGS) $(ARCH_FLAGS) -o mutex.o

libz_mutex.a: mutex.o
	$(AR) -ru libz_mutex.a mutex.o

../../../$(host_libdir)/libz_mutex.a: libz_mutex.a
	cp libz_mutex.a ../../../$(host_libdir)/libz_mutex.a

