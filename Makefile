.PHONY: all
all: src/main.cpp src/state.cpp src/node.cpp src/tree.cpp
	g++ -o MCTS -fopenmp -O3 src/main.cpp src/state.cpp src/node.cpp src/tree.cpp

.PHONY: clean
clean:
	rm -f *.o MCTS
