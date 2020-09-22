CFLAGS=$(shell cat compile_flags.txt)

.PHONY: test

test: test.cpp
	clang++ $(CFLAGS) -ggdb3 $< -o $@
	./test

clean:
	rm test
