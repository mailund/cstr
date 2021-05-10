CFLAGS = -fsanitize=address -fno-omit-frame-pointer


skew_test: skew_test.c skew.h skew.o
	$(CC) $(CFLAGS) $< skew.o -o $@
