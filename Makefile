#
# File          : Makefile
# Description   : Build file for CMPSC473 project 4


# Environment Setup
LIBDIRS=-L.
INCLUDES=-I. -I/usr/include/
CC=gcc
CFLAGS=-c $(INCLUDES) -g -m32 -Wall
# CFLAGS=-c $(INCLUDES) -g -Wall
LINK=gcc -g -m32
LDFLAGS=$(LIBDIRS)
AR=ar rc
RANLIB=ranlib

# Suffix rules
.c.o :
	${CC} ${CFLAGS} $< -o $@

#
# Setup builds

PT-TARGETS=cmpsc473-p4

# pthreads
LIBS=-l$(UTILLIB)

#
# Project Protections

p4 : $(PT-TARGETS)

cmpsc473-p4 : cmpsc473-p4.o cmpsc473-util.o cmpsc473-list.o cmpsc473-filesys.o cmpsc473-disk.o
	$(LINK) $(LDFLAGS) cmpsc473-util.o cmpsc473-list.o cmpsc473-p4.o cmpsc473-filesys.o \
	cmpsc473-disk.o -lm -o $@

clean:
	rm -f *.o *~ $(PT-TARGETS) $(LIBOBJS) lib$(UTILLIB).a

test:
	@chmod +x p4-grade.py
	python p4-grade.py

BASENAME=p4-xattr
tar:
	tar cvfz $(BASENAME).tgz -C ..\
	    $(BASENAME)/Makefile \
	    $(BASENAME)/cmpsc473-p4.c \
	    $(BASENAME)/cmpsc473-util.c \
	    $(BASENAME)/cmpsc473-util.h \
	    $(BASENAME)/cmpsc473-list.c \
	    $(BASENAME)/cmpsc473-list.h \
	    $(BASENAME)/cmpsc473-disk.c \
	    $(BASENAME)/cmpsc473-disk.h \
	    $(BASENAME)/cmpsc473-filesys.c \
	    $(BASENAME)/cmpsc473-filesys.h \
	    $(BASENAME)/cmd1 \
	    $(BASENAME)/cmd2 \
	    $(BASENAME)/cmd3 \
	    $(BASENAME)/cmd4 \
	    $(BASENAME)/cmd5 \
	    $(BASENAME)/p4-output \
	    $(BASENAME)/data1 \
	    $(BASENAME)/data2
