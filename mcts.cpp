#include "mcts.h"
#include <random>
#include <cmath>
#include <assert.h>
// #include <ctime>
#include <chrono>
#include <iostream>

using namespace std;

double UCT(double p_n, double c_n, double win, 
           double lose, double c_param){
    return (win - lose) / c_n + 
            c_param * sqrt((2 * log(p_n) / c_n));
}

/*** MCTS NODE ***/
MCTS_node::MCTS_node(MCTS_node *parent, State *state){
    this->state = state;
    this->parent = parent;
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
    random_device rd;
    default_random_engine eng(rd());
    uniform_int_distribution<int> distr(0, possible_moves->size() - 1);
    return *((*possible_moves)[distr(eng)]);
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

void MCTS_node::backpropagate(int result){
    ++n;

    if (parent){
        if (parent->state->next_to_move == result) 
            ++win;
        else if (result != 0) 
            ++lose;
    }

    if (parent)
        parent->backpropagate(result);
}

bool MCTS_node::is_fully_expanded(){
    untried_actions();
    return _untried_actions->size() == 0;
}

MCTS_node* MCTS_node::best_child(double c_param) const{
    int best_child_index = 0;
    double best_UCT_value = UCT(this->n, ((*children)[0])->n, 
        ((*children)[0])->win, ((*children)[0])->lose, c_param);
    for (int i = 1; i < children->size(); ++i){
        double UCT_value = UCT(this->n, ((*children)[i])->n, 
            ((*children)[i])->win, ((*children)[i])->lose, c_param);
        if (UCT_value > best_UCT_value){
            best_child_index = i;
            best_UCT_value = UCT_value;
        }
    }
    return (*children)[best_child_index];
}

/*** MCTS TREE ***/
MCTS_tree::MCTS_tree(MCTS_node *root){
    this->root = root;
}

MCTS_tree::~MCTS_tree(){
    delete root;
}

MCTS_node* MCTS_tree::_tree_policy() const{
    MCTS_node *current_node = root;
    int count = 0;
    while (!current_node->is_terminal_node()){
        if (!current_node->is_fully_expanded())
            return current_node->expand();
        else
            current_node = current_node->best_child(1.0);
        count++;
    }
    return current_node;
}

pair<int, int> MCTS_tree::best_action(
    int *simulations_number, 
    int *total_simulation_milliseconds)
{
    if (!simulations_number){
        assert(total_simulation_milliseconds);
        chrono::milliseconds now = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());
        chrono::milliseconds end = now + 
            chrono::milliseconds(*total_simulation_milliseconds);

        while (chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch()) < end){
            MCTS_node* leaf_node = _tree_policy();
            int rollout_result = leaf_node->rollout();
            leaf_node->backpropagate(rollout_result);
        }
    }
    else{
        assert(!total_simulation_milliseconds);
        for (int i = 0; i < *simulations_number; ++i){
            MCTS_node* leaf_node = _tree_policy();
            int rollout_result = leaf_node->rollout();
            leaf_node->backpropagate(rollout_result);
        }
    }
    return root->best_child(0.0)->state->move;
}