test: test.cpp test.h kth-percentile.h tree.h Makefile
	$(CXX) --std=c++20 -Wall -Wextra -pedantic -Werror -fsanitize=undefined -fsanitize=address -g -Og -o $@ $<
