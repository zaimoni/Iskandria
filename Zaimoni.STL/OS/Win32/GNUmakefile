# Makefile for OS/Win32 directory
# assumes GNU make

.SUFFIXES:
.SUFFIXES: .cpp .a .o

include ../../POSIX.inc

CFLAGS := $(CFLAGS_CORE) $(CFLAGS_MICROOPT)
CXXFLAGS := $(CFLAGS)

LIBS := libz_mutex.a libz_clock.a

# GNU targets we know about
all: $(LIBS)

clean:
	rm -f *.a *.o

host_install: $(addprefix ../../../$(host_libdir)/, $(LIBS))

# rules
.cpp.o:
	$(CXX) -c $< $(CXXFLAGS) $(ARCH_FLAGS) -o $*.o

libz_mutex.a: mutex.o
	$(AR) $(ARFLAGS) libz_mutex.a mutex.o

libz_clock.a: clock.o
	$(AR) $(ARFLAGS) libz_clock.a clock.o

../../../$(host_libdir)/libz_mutex.a: libz_mutex.a
	cp libz_mutex.a ../../../$(host_libdir)/libz_mutex.a

../../../$(host_libdir)/libz_clock.a: libz_clock.a
	cp libz_clock.a ../../../$(host_libdir)/libz_clock.a

