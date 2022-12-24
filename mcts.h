#ifndef MCTS_H
#define MCTS_H

#include "state.h"
#include <vector>

using namespace std;

double UCT(double p_n, double c_n, double win, 
           double lose, double c_param);

class MCTS_node {
public:
    State *state;
    MCTS_node *parent;
    vector<MCTS_node *> *children = new vector<MCTS_node *>();
    vector< pair<int, int>* > *_untried_actions = nullptr;
    int n = 0, win = 0, lose = 0;

    MCTS_node(MCTS_node *parent, State *state);
    ~MCTS_node();
    void untried_actions();
    MCTS_node* expand();
    bool is_terminal_node() const;
    int rollout() const;
    void backpropagate(int result);
    bool is_fully_expanded();
    MCTS_node* best_child(double c_param) const;
    pair<int, int> rollout_policy(vector< pair<int, int> *> *possible_moves) const;
};

class MCTS_tree {
public:
    MCTS_node *root;

    MCTS_tree(MCTS_node *root);
    ~MCTS_tree();
    MCTS_node* _tree_policy() const; //selects node to run rollout for
    pair<int, int> best_action(
        int *simulations_number, 
        int *total_simulation_milliseconds);//one of the two must be nullptr
};

#endif