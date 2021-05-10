CFLAGS = -g -fsanitize=address -fno-omit-frame-pointer

all: skew_test bwt_test

skew_test: skew_test.c skew.h skew.o misc.h misc.o
	$(CC) $(CFLAGS) $< skew.o misc.o -o $@

bwt_test: bwt_test.c skew.h skew.o misc.h misc.o bwt.h bwt.o
	$(CC) $(CFLAGS) $< misc.o skew.o bwt.o -o $@
