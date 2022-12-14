#ifndef MCTS_H
#define MCTS_H

#include "state.h"
#include <vector>

using namespace std;

class MCTS_node {
    State *state;
    MCTS_node *parent;
    vector<MCTS_node *> children;
    int n = 0, win = 0, lose = 0;
public:
    vector< pair<int, int> *> *_untried_actions = nullptr;

    MCTS_node(MCTS_node *parent, State *state);
    ~MCTS_node();
    void untried_actions();
    MCTS_node expand();
    bool is_terminal_node() const;
    int rollout() const;
    void backpropagate(int reuslt);
    bool is_fully_expanded() const;
    MCTS_node best_child(double c_param = 1.) const;
    pair<int, int> rollout_policy(vector< pair<int, int> *> *possible_moves) const;
};



class MCTS_tree {
    MCTS_node *root;
public:
    MCTS_tree(MCTS_state *starting_state);
    ~MCTS_tree();
    MCTS_node *select(double c=1.41);        // select child node to expand according to tree policy (UCT)
    MCTS_node *select_best_child();          // select the most promising child of the root node
    void grow_tree(int max_iter, double max_time_in_seconds);
    void advance_tree(const MCTS_move *move);      // if the move is applicable advance the tree, else start over
    unsigned int get_size() const;
    const MCTS_state *get_current_state() const;
    void print_stats() const;
};


class MCTS_agent {                           // example of an agent based on the MCTS_tree. One can also use the tree directly.
    MCTS_tree *tree;
    int max_iter, max_seconds;
public:
    MCTS_agent(MCTS_state *starting_state, int max_iter = 100000, int max_seconds = 30);
    ~MCTS_agent() = default;
    const MCTS_move *genmove(const MCTS_move *enemy_move);
    const MCTS_state *get_current_state() const;
    void feedback() const { tree->print_stats(); }
};
