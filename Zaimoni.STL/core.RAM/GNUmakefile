# Makefile for core.RAM directory
# assumes GNU make

.SUFFIXES:
.SUFFIXES: .cpp .a .o

include ../POSIX.inc

CFLAGS := $(CFLAGS_CORE) $(CFLAGS_MICROOPT)
CXXFLAGS := $(CFLAGS)

LIBS := libz_memory.a libz_memory_c.a

# GNU targets we know about
all: $(LIBS)

clean:
	rm -f *.a *.o

host_install: $(addprefix ../../$(host_libdir)/, $(LIBS))

# rules
memory.o: memory.cpp ../z_memory.h ../OS/AIMutex.hpp
	$(CXX) -c memory.cpp $(CXXFLAGS) $(ARCH_FLAGS) -o memory.o

memory_c.o: memory.cpp ../z_memory.h
	$(CC) -c -xc memory.cpp $(CFLAGS) $(ARCH_FLAGS) -o memory_c.o

libz_memory.a: memory.o
	$(AR) -ru libz_memory.a memory.o

libz_memory_c.a: memory_c.o
	$(AR) -ru libz_memory_c.a memory_c.o

../../$(host_libdir)/libz_memory.a: libz_memory.a
	cp libz_memory.a ../../$(host_libdir)/libz_memory.a

../../$(host_libdir)/libz_memory_c.a: libz_memory_c.a
	cp libz_memory_c.a ../../$(host_libdir)/libz_memory_c.a

