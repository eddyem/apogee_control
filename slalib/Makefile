#  Makefile for SLALIB
#   for Pentium/Linux
#   by Scott M. Ransom

# OS type
OS = Linux
#OS = OSX

# Linux is the first choice
ifeq ($(OS),Linux)
	LIBSUFFIX = .so
	LIBCMD = -shared
	SYSDIR = /usr
	LOCDIR = /usr/local
# else assume Darwin (i.e. OSX)
else
	LIBSUFFIX = .dylib
	LIBCMD = -dynamiclib
	SYSDIR = /sw
	LOCDIR = /sw
endif

CC = gcc
FC = gfortran
#FC = g77
CFLAGS = -O2 -Wall -W -fPIC
CLINKFLAGS = $(CFLAGS)
FFLAGS = -O2 -fPIC
FLINKFLAGS = $(FFLAGS)

all: libsla

libsla:
	$(FC) $(FFLAGS) -fno-underscoring -c -I. *.f *.F
	$(FC) $(LIBCMD) -o libsla$(LIBSUFFIX) -fno-underscoring *.o

clean:
	rm -f *.o *~ *#
	rm -rf build

install:
	cp sla.pc /usr/share/pkgconfig
	cp libsla$(LIBSUFFIX) /usr/lib
	cp slamac.h /usr/include
