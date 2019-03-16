# POSIX makefile

.SUFFIXES:
.SUFFIXES: .cpp .o .exe

include POSIX.inc

# GNU targets we know about
all: Iskandria.exe

clean:
	rm -f *.o *.exe lib/host.isk/*.a
	cd Zaimoni.STL; make clean

# dependencies
include POSIX.dep

make_Zaimoni_STL:
	cd Zaimoni.STL; make host_install

Iskandria.exe : make_Zaimoni_STL $(OBJECTS_ISKANDRIA_LINK_PRIORITY)
	g++ -oIskandria.exe $(LINK_FLAGS) $(ARCH_FLAGS) $(OBJECTS_ISKANDRIA) -lz_logging -lz_format_util -lz_stdio_c -lz_stdio_log -lz_clock  -lsfml-graphics -lsfml-window -lsfml-system -lwinmm
	strip --preserve-dates --strip-unneeded Iskandria.exe

# inference rules
# processing details
.cpp.o:
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(ARCH_FLAGS) $(OTHER_INCLUDEDIR) $(C_MACROS) $(CXX_MACROS) \
	 -o $*.o -c -xc++ -pipe $<
	strip --preserve-dates --strip-unneeded $*.o

include POSIX2.inc
