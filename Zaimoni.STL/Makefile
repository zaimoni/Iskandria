# POSIX makefile for Zaimoni.STL
# POSIX requires make sets $(MAKE)
# pure.C must be done first: Pure.C/auto_int.h, Pure.C/comptest.h
#  are generated files
# then repair.STL as it also has generated files

# GNU targets we know about
all:
	cd pure.C; $(MAKE) all; cd stdio.log; $(MAKE) all
	cd repair.STL; $(MAKE) all
	cd core.RAM; $(MAKE) all
	cd OS; $(MAKE) all
	cd OS; cd Win32; $(MAKE) all

clean:
	cd pure.C; $(MAKE) clean; cd stdio.log; $(MAKE) clean
	cd repair.STL; $(MAKE) clean
	cd core.RAM; $(MAKE) clean
	cd OS; $(MAKE) clean
	cd OS; cd Win32; $(MAKE) clean

# use this to avoid polluting host compiler with our libraries
host_install:
	cd pure.C; $(MAKE) host_install; cd stdio.log; $(MAKE) host_install
	cd repair.STL; $(MAKE) all
	cd core.RAM; $(MAKE) host_install
	cd OS; $(MAKE) host_install
	cd OS; cd Win32; $(MAKE) host_install

