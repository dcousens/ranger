CFLAGS=$(shell cat compile_flags.txt)

test: test.cpp ranger.hpp compile_flags.txt
	clang++ $(CFLAGS) -ggdb3 $< -o $@

clean:
	rm test
