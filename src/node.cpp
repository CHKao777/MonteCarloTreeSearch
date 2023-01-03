#include "node.h"
#include <random>
#include <cmath>
#include <assert.h>
#include <chrono>
#include <iostream>

using namespace std;


double UCT(double p_n, double c_n, double win_minus_lose, 
           double c_param){
    return win_minus_lose / c_n + 
            c_param * sqrt((2 * log(p_n) / c_n));
}

/*** MCTS NODE ***/
MCTS_node::MCTS_node(MCTS_node *parent, State *state){
    this->state = state;
    this->parent = parent;
    n = 0;
    win_minus_lose = 0;
    eng = new default_random_engine{random_device{}()};
    omp_init_lock(&lock);
}

MCTS_node::~MCTS_node() {
    // delete state
    delete state;

    // delete children
    for (int i = 0; i < children->size(); ++i){
        delete (*children)[i];
    }
    delete children;

    // delete _untried_actions
    if (_untried_actions){
        for (int i = 0; i < _untried_actions->size(); ++i){
            delete (*_untried_actions)[i];
        }
        delete _untried_actions;
    }

    //delete default_random_engine
    delete eng;
}


void MCTS_node::untried_actions(){
    if (!_untried_actions) {
        _untried_actions = new vector< pair<int, int>* >();
        state->get_legal_actions(_untried_actions);
    }
}

MCTS_node* MCTS_node::expand(){
    untried_actions();
    pair<int, int> *move = _untried_actions->back();
    _untried_actions->pop_back();
    State *next_state = state->move_to_next_state(*move);
    delete move;
    MCTS_node *child_node = new MCTS_node(this, next_state);
    children->push_back(child_node);
    return child_node;
}

bool MCTS_node::is_terminal_node() const{
    return state->is_game_over();
}

pair<int, int> MCTS_node::rollout_policy(vector< pair<int, int> *> *possible_moves) const{
    uniform_int_distribution<int> distr(0, possible_moves->size() - 1);
    return *((*possible_moves)[distr(*eng)]);
}

int MCTS_node::rollout() const{
    State *current_rollout_state = state;
    State *state_to_delete;
    vector< pair<int, int>* > *possible_moves = new vector< pair<int, int>* >();

    int count = 0;
    while (!current_rollout_state->is_game_over()){
        current_rollout_state->get_legal_actions(possible_moves);
        pair<int, int> move = rollout_policy(possible_moves);
        state_to_delete = current_rollout_state;
        current_rollout_state = current_rollout_state->move_to_next_state(move);

        //delete new generated state
        if (state_to_delete != state) delete state_to_delete;
    }

    int game_result = current_rollout_state->game_result();

    //delete new generated state
    if (current_rollout_state != state) delete current_rollout_state;
    //delete possible_moves
    for (int i = 0; i < possible_moves->size(); ++i){
        delete (*possible_moves)[i];
    }
    delete possible_moves;

    return game_result;
}

void MCTS_node::backpropagate(int result, int count){
    n += count;

    if (parent){
        if (parent->state->next_to_move == 1)
            win_minus_lose += result;
        else if (parent->state->next_to_move == 2) 
            win_minus_lose -= result;
    }

    if (parent)
        parent->backpropagate(result, count);
}

bool MCTS_node::is_fully_expanded(){
    untried_actions();
    return _untried_actions->size() == 0;
}

MCTS_node* MCTS_node::best_child(double c_param) const{
    int best_child_index = 0;
    double best_UCT_value = UCT(this->n, ((*children)[0])->n, 
        ((*children)[0])->win_minus_lose, c_param);

    for (int i = 1; i < children->size(); ++i){
        double UCT_value = UCT(this->n, ((*children)[i])->n, 
            ((*children)[i])->win_minus_lose, c_param);
        if (((*children)[i])->n == 0) cout << 123 << endl;
        if (UCT_value > best_UCT_value){
            best_child_index = i;
            best_UCT_value = UCT_value;
        }
    }
    return (*children)[best_child_index];
}