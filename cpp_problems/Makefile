# Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++2a -Wall -Wextra -O2

# Source files
SRCS = reconciler.cpp test_runner_fib.cpp djikstra.cpp palindrome.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Build targets for each source file
TARGETS = $(SRCS:.cpp=)

# Default target
all: $(TARGETS)

# Rule to build the targets
$(TARGETS): %: %.o
	$(CXX) $(CXXFLAGS) -o $@.out $<

# Rule to build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(TARGETS) $(OBJS)

# Phony targets
.PHONY: all clean
OBJS = $(SRCS:.cpp=.o)
