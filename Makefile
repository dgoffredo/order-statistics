test: test.cpp kth-percentile.h
	$(CXX) --std=c++20 -Wall -Wextra -pedantic -Werror -g -Og -o $@ $<
