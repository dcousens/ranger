CXX=clang++
CFLAGS=-std=c++1z -pedantic -W -Wall -Wcast-qual -Wconversion -Werror -Wextra -Wwrite-strings
IFLAGS=
LFLAGS=
OFLAGS=

SOURCES=test/ranger.cpp
OBJECTS=$(addsuffix .o, $(basename $(SOURCES)))
DEPENDENCIES=$(OBJECTS:.o=.d)

# COMMANDS
test: test/ranger
	./test/ranger

clean:
	$(RM) $(DEPENDENCIES) $(OBJECTS) test/ranger

# INFERENCES
%.o: %.cpp
	$(CXX) $(CFLAGS) $(OFLAGS) $(IFLAGS) -MMD -MP -c $< -o $@

test/ranger: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LFLAGS) $(OFLAGS) -o $@

-include $(DEPENDENCIES)
