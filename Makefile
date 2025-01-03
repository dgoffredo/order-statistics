test: test.cpp kth-percentile.h Makefile
	$(CXX) --std=c++20 -Wall -Wextra -pedantic -Werror -fsanitize=undefined -fsanitize=address -g -Og -o $@ $<
