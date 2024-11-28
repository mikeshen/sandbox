# Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++2a -Wall -Wextra -O2

# Source iles
SRCS = main.cpp stream.cpp reconciler.cpp test_runner_fib.cpp djikstra.cpp disjoint_intervals.cpp order_engine.cpp market_data.cpp test.cpp

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
	rm -f $(TARGETS) $(OBJS) *.out

# Phony targets
.PHONY: all clean
OBJS = $(SRCS:.cpp=.o)
