# GNU compatibility core
prefix = /MingW32
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
includedir = $(exec_prefix)/include
libdir = $(exec_prefix)/lib

# install host-built libraries here for Z.C++ here
# (stage 1 of three-stage bootstrap)
host_libdir = lib/host.isk

AR = ar
CC = gcc
CXX = g++

EXEEXT = exe
OBJEXT = o

ARCH_FLAGS = 

# other control points
# FORCE_ISO: ZAIMONI_FORCE_ISO disables the custom memory manager
FORCE_ISO =
# FORCE_ISO = -DZAIMONI_FORCE_ISO
