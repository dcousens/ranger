CFLAGS=-std=c++17 -pedantic -Wall -Wextra -Werror -Wunused -Wcast-qual -Wconversion
OFLAGS=-ggdb3

.PHONY: test

test: test.cpp ranger.hpp serial.hpp
	clang++ test.cpp $(CFLAGS) $(OFLAGS) -o $@
	./test

clean:
	rm test
