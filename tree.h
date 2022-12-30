#ifndef TREE_H
#define TREE_H

#include "state.h"
#include "node.h"
#include <omp.h>

using namespace std;

class MCTS_tree_serial {
public:
    MCTS_node *root;

    MCTS_tree_serial(MCTS_node *root);
    ~MCTS_tree_serial();
    MCTS_node* _tree_policy() const; //selects node to run rollout for
    pair<int, int> best_action(
        int *simulations_number, 
        int *total_simulation_milliseconds);//one of the two must be nullptr
};

class MCTS_tree_leaf {
public:
    MCTS_node *root;

    MCTS_tree_leaf(MCTS_node *root);
    ~MCTS_tree_leaf();
    MCTS_node* _tree_policy() const; //selects node to run rollout for
    pair<int, int> best_action(
        int *simulations_number, 
        int *total_simulation_milliseconds);//one of the two must be nullptr
};

class MCTS_tree_root {
public:
    MCTS_node *root;
    vector< MCTS_tree_serial* > *tree_vector;

    MCTS_tree_root(MCTS_node *root);
    ~MCTS_tree_root();
    MCTS_node* _tree_policy() const; //selects node to run rollout for
    pair<int, int> best_action(
        int *simulations_number, 
        int *total_simulation_milliseconds);//one of the two must be nullptr
};

class MCTS_tree_tree {
public:
    MCTS_node *root;

    MCTS_tree_tree(MCTS_node *root);
    ~MCTS_tree_tree();
    MCTS_node* _tree_policy() const; //selects node to run rollout for
    pair<int, int> best_action(
        int *simulations_number, 
        int *total_simulation_milliseconds);//one of the two must be nullptr
};


#endif