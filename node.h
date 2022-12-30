#ifndef NODE_H
#define NODE_H

#include "state.h"
#include <vector>
#include <omp.h>
#include <atomic> 

using namespace std;

double UCT(double p_n, double c_n, double win_minus_lose, 
           double c_param);

class MCTS_node {
public:
    State *state;
    MCTS_node *parent;
    vector<MCTS_node *> *children = new vector<MCTS_node *>();
    vector< pair<int, int>* > *_untried_actions = nullptr;
    atomic<int> n;
    atomic<int> win_minus_lose;
    omp_lock_t lock;

    MCTS_node(MCTS_node *parent, State *state);
    ~MCTS_node();
    void untried_actions();
    MCTS_node* expand();
    bool is_terminal_node() const;
    int rollout() const;
    void backpropagate(int result, int count);
    bool is_fully_expanded();
    MCTS_node* best_child(double c_param) const;
    pair<int, int> rollout_policy(vector< pair<int, int> *> *possible_moves) const;
};

#endif