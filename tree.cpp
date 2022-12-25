#include "tree.h"
#include <random>
#include <cmath>
#include <assert.h>
// #include <ctime>
#include <chrono>
#include <iostream>
#include <omp.h>

using namespace std;

#define number_of_simulation_per_leaf 4 //for leaf parallelization

/*** MCTS TREE SERIAL ***/
MCTS_tree_serial::MCTS_tree_serial(MCTS_node *root){
    this->root = root;
}

MCTS_tree_serial::~MCTS_tree_serial(){
    delete root;
}

MCTS_node* MCTS_tree_serial::_tree_policy() const{
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

pair<int, int> MCTS_tree_serial::best_action(
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
            leaf_node->backpropagate(rollout_result, 1);
        }
    }
    else{
        assert(!total_simulation_milliseconds);
        for (int i = 0; i < *simulations_number; ++i){
            MCTS_node* leaf_node = _tree_policy();
            int rollout_result = leaf_node->rollout();
            leaf_node->backpropagate(rollout_result, 1);
        }
    }
    return root->best_child(0.0)->state->move;
}

/*** MCTS TREE LEAF ***/
// same as serial
MCTS_tree_leaf::MCTS_tree_leaf(MCTS_node *root){
    this->root = root;
}

// same as serial
MCTS_tree_leaf::~MCTS_tree_leaf(){
    delete root;
}

// same as serial
MCTS_node* MCTS_tree_leaf::_tree_policy() const{
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

// leaf parallelization implementation here
pair<int, int> MCTS_tree_leaf::best_action(
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
            
            int rollout_result = 0;
            /*** leaf parallelization implementation here ***/
            #pragma omp parallel for reduction(+:rollout_result)
            for (int j = 0; j < number_of_simulation_per_leaf; ++j)
                rollout_result += leaf_node->rollout();

            leaf_node->backpropagate(rollout_result, number_of_simulation_per_leaf);
        }
    }
    else{
        assert(!total_simulation_milliseconds);
        for (int i = 0; i < *simulations_number; i += number_of_simulation_per_leaf){
            MCTS_node* leaf_node = _tree_policy();
            
            int rollout_result = 0;
            /*** leaf parallelization implementation here ***/
            #pragma omp parallel for reduction(+:rollout_result)
            for (int j = 0; j < number_of_simulation_per_leaf; ++j)
                rollout_result += leaf_node->rollout();

            leaf_node->backpropagate(rollout_result, number_of_simulation_per_leaf);
        }
    }
    return root->best_child(0.0)->state->move;
}