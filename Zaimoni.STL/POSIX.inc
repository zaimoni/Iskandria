# GNU compatibility core
# prefix = ...
# exec_prefix = $(prefix)
# bindir = $(exec_prefix)/bin
# includedir = $(exec_prefix)/include
# libdir = $(exec_prefix)/lib

# install host-built libraries here for Z.C++ here
# (stage 1 of three-stage bootstrap)
host_libdir = lib/host.isk

AR = ar
CC = gcc -std=gnu11
CXX = g++ -std=c++11

EXEEXT = exe
OBJEXT = o

ARCH_FLAGS = 

# other control points
# FORCE_ISO: ZAIMONI_FORCE_ISO disables the custom memory manager
FORCE_ISO =
# FORCE_ISO = -DZAIMONI_FORCE_ISO

# compiler options
CFLAGS_CORE =  -Os -Wall -finline
CFLAGS_STRICT =  -fargument-alias -Wno-long-long --pedantic
CFLAGS_MICROOPT = -Wextra -Winline -Wdisabled-optimization --param max-inline-insns-single=500 -c -pipe -DNDEBUG

# archiver options
ARFLAGS = -sru
