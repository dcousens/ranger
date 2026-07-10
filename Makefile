CFLAGS=$(shell cat compile_flags.txt)

test: test.cpp ranger.hpp
	clang++ $(CFLAGS) -ggdb3 $< -o $@

clean:
	rm test
