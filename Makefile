.PHONY: all
all: main.cpp state.cpp node.cpp tree.cpp
	g++ -o MCTS -fopenmp -O3 main.cpp state.cpp node.cpp tree.cpp

.PHONY: clean
clean:
	rm -f *.o MCTS
