#include "tree.h"
#include <random>
#include <cmath>
#include <assert.h>
// #include <ctime>
#include <chrono>
#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

#define number_of_simulation_per_leaf 20 //for leaf parallelization
#define number_of_tree 20 //for root parallelization
#define number_of_thread 20 //for tree parallelization

/*** MCTS TREE SERIAL ***/
MCTS_tree_serial::MCTS_tree_serial(MCTS_node *root){
    this->root = root;
}

MCTS_tree_serial::~MCTS_tree_serial(){
    delete root;
}

MCTS_node* MCTS_tree_serial::_tree_policy() const{
    MCTS_node *current_node = root;
    while (!current_node->is_terminal_node()){
        if (!current_node->is_fully_expanded())
            return current_node->expand();
        else
            current_node = current_node->best_child(1.0);
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
    while (!current_node->is_terminal_node()){
        if (!current_node->is_fully_expanded())
            return current_node->expand();
        else
            current_node = current_node->best_child(1.0);
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
            #pragma omp parallel for reduction(+:rollout_result) num_threads(number_of_simulation_per_leaf) if (number_of_simulation_per_leaf > 1)
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
            #pragma omp parallel for reduction(+:rollout_result) num_threads(number_of_simulation_per_leaf) if (number_of_simulation_per_leaf > 1)
            for (int j = 0; j < number_of_simulation_per_leaf; ++j)
                rollout_result += leaf_node->rollout();

            leaf_node->backpropagate(rollout_result, number_of_simulation_per_leaf);
        }
    }
    return root->best_child(0.0)->state->move;
}

/*** MCTS TREE ROOT ***/
// same as serial
MCTS_tree_root::MCTS_tree_root(MCTS_node *root){
    this->root = root;
}
// same as serial
MCTS_tree_root::~MCTS_tree_root(){
    delete root;
    
    // detele tree_vector;
    for (int i = 0; i < tree_vector->size(); ++i){
        delete (*tree_vector)[i];
    }
    delete tree_vector;
}
// same as serial
MCTS_node* MCTS_tree_root::_tree_policy() const{
    MCTS_node *current_node = root;
    while (!current_node->is_terminal_node()){
        // 同一個node critical
        if (!current_node->is_fully_expanded()) 
            return current_node->expand();   
        else
            current_node = current_node->best_child(1.0);
    }
    return current_node;
}
// root parallelization implementation here
pair<int, int> MCTS_tree_root::best_action(
    int *simulations_number, 
    int *total_simulation_milliseconds)
{
    tree_vector = new vector< MCTS_tree_serial* >;
    for (int i = 0; i < number_of_tree - 1; ++i){
        State* copy_root_state = new State(this->root->state->board, this->root->state->score, 
            this->root->state->move, this->root->state->next_to_move);
        tree_vector->push_back(new MCTS_tree_serial(new MCTS_node(nullptr, copy_root_state)));
    }

    if (!simulations_number){
        assert(total_simulation_milliseconds);
        chrono::milliseconds now = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());
        chrono::milliseconds end = now + 
            chrono::milliseconds(*total_simulation_milliseconds);

        #pragma omp parallel for num_threads(number_of_tree) if (number_of_tree>1)
        for (int i = 0; i < number_of_tree; ++i){
            if (i == 0){
                do{
                    MCTS_node* leaf_node = _tree_policy();
                    int rollout_result = leaf_node->rollout();
                    leaf_node->backpropagate(rollout_result, 1);
                }while(chrono::duration_cast< chrono::milliseconds >(
                    chrono::system_clock::now().time_since_epoch()) < end);
            }
            else{
                do{
                    MCTS_node* leaf_node = (*tree_vector)[i - 1]->_tree_policy();
                    int rollout_result = leaf_node->rollout();
                    leaf_node->backpropagate(rollout_result, 1);
                }while (chrono::duration_cast< chrono::milliseconds >(
                    chrono::system_clock::now().time_since_epoch()) < end);
            }
        }
    }
    else{
        assert(!total_simulation_milliseconds);

        #pragma omp parallel for num_threads(number_of_tree) if (number_of_tree>1)
        for (int i = 0; i < number_of_tree; ++i){
            if (i == 0){
                int master_count = *simulations_number / number_of_tree == 0 ? 
                    1 : *simulations_number / number_of_tree;
                for (int j = 0; j < master_count; ++j){
                    MCTS_node* leaf_node = _tree_policy();
                    int rollout_result = leaf_node->rollout();
                    leaf_node->backpropagate(rollout_result, 1);
                }
            }
            else{
                for (int j = 0; j < *simulations_number / number_of_tree; ++j){
                    MCTS_node* leaf_node = (*tree_vector)[i - 1]->_tree_policy();
                    int rollout_result = leaf_node->rollout();
                    leaf_node->backpropagate(rollout_result, 1);
                }
            }
        }
    }
    
    // cout << "----------------------" << endl;
    // cout << this->root->n << endl;
    // for (int i = 0; i < number_of_tree - 1; ++i){
    //     cout << (*tree_vector)[i]->root->n << endl;
    // }

    //sum up result
    //find the tree with max # of children
    int max = this->root->children->size(), max_index = 0;
    for (int i = 0; i < number_of_tree - 1; ++i){
        if ((*tree_vector)[i]->root->children->size() > max){
            max = (*tree_vector)[i]->root->children->size();
            max_index = i + 1;
        }
    }

    //collect result to the tree with max # of children
    if (max_index == 0){
        for (int i = 0; i < number_of_tree - 1; ++i){
            this->root->n += (*tree_vector)[i]->root->n;
            for (int j = 0; j < (*tree_vector)[i]->root->children->size(); ++j){
                (*this->root->children)[j]->n += 
                    (*((*tree_vector)[i])->root->children)[j]->n;
                (*this->root->children)[j]->win_minus_lose += 
                    (*((*tree_vector)[i])->root->children)[j]->win_minus_lose;
            }
        }
        return root->best_child(0.0)->state->move;
    }
    else{
        (*tree_vector)[max_index - 1]->root->n += this->root->n;
        for (int i = 0; i < this->root->children->size(); ++i){
            (*((*tree_vector)[max_index - 1])->root->children)[i]->n +=
                (*this->root->children)[i]->n;
            (*((*tree_vector)[max_index - 1]->root->children))[i]->win_minus_lose +=
                (*this->root->children)[i]->win_minus_lose;
        }

        for (int i = 0; i < number_of_tree - 1; ++i){
            if (i + 1 == max_index)
                continue;
            (*tree_vector)[max_index - 1]->root->n += (*tree_vector)[i]->root->n;
            for (int j = 0; j < (*tree_vector)[i]->root->children->size(); ++j){
                (*((*tree_vector)[max_index - 1])->root->children)[j]->n +=
                    (*((*tree_vector)[i])->root->children)[j]->n;
                (*((*tree_vector)[max_index - 1])->root->children)[j]->win_minus_lose +=
                    (*((*tree_vector)[i])->root->children)[j]->win_minus_lose;
            }
        }

        return (*tree_vector)[max_index - 1]->root->best_child(0.0)->state->move;
    }
}

/*** MCTS TREE TREE ***/
// same as serial
MCTS_tree_tree::MCTS_tree_tree(MCTS_node *root){
    this->root = root;
}

// same as serial
MCTS_tree_tree::~MCTS_tree_tree(){
    delete root;
}

MCTS_node* MCTS_tree_tree::_tree_policy() const{
    MCTS_node *current_node = root;
    while (!current_node->is_terminal_node()){
        omp_set_lock(&current_node->lock);
        if (!current_node->is_fully_expanded()){
            MCTS_node* return_value = current_node->expand();
            omp_unset_lock(&current_node->lock);
            return return_value;
        }

        omp_unset_lock(&current_node->lock);

        int max_index = -1;
        int max_UCT_value = -1000000;
        for (int i = 0; i < current_node->children->size(); ++i){
            if (((*current_node->children)[i])->n == 0) continue;
            double UCT_value = UCT(current_node->n, ((*current_node->children)[i])->n, 
            ((*current_node->children)[i])->win_minus_lose, 1.0);
            if (UCT_value > max_UCT_value){
                max_index = i;
                max_UCT_value = UCT_value;
            }
        }
        if (max_index != -1){
            current_node = (*current_node->children)[max_index];
        }
        else{
            random_device rd;
            default_random_engine eng(rd());
            uniform_int_distribution<int> distr(0, current_node->children->size() - 1);
            return (*current_node->children)[distr(eng)];
        }
    }
    return current_node;
}

pair<int, int> MCTS_tree_tree::best_action(
    int *simulations_number, 
    int *total_simulation_milliseconds)
{
    if (!simulations_number){
        assert(total_simulation_milliseconds);
        chrono::milliseconds now = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());
        chrono::milliseconds end = now + 
            chrono::milliseconds(*total_simulation_milliseconds);

        #pragma omp parallel num_threads(number_of_thread) if (number_of_thread>1)
        while (chrono::duration_cast< chrono::milliseconds >(
            
            chrono::system_clock::now().time_since_epoch()) < end){
            MCTS_node* leaf_node = _tree_policy();
            int rollout_result = leaf_node->rollout();
            leaf_node->backpropagate(rollout_result, 1);
        }
    }
    else{
        assert(!total_simulation_milliseconds);
        #pragma omp parallel for num_threads(number_of_thread) if (number_of_thread>1)
        for (int i = 0; i < *simulations_number; ++i){
            MCTS_node* leaf_node = _tree_policy();
            int rollout_result = leaf_node->rollout();
            leaf_node->backpropagate(rollout_result, 1);
        }
    }
    return root->best_child(0.0)->state->move;
}