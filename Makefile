CFLAGS=$(shell cat compile_flags.txt)

test: test.cpp ranger.hpp
	clang++ $(CFLAGS) -ggdb3 $< -o $@
	./test

clean:
	rm test
