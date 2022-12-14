#include "mcts.h"

using namespace std;


/*** MCTS NODE ***/
MCTS_node::MCTS_node(MCTS_node *parent, State *state){
    this->state = state;
    this->parent = parent;
}

// MCTS_node::~MCTS_node() {
//     delete state;
//     for (auto *child : children) {
//         delete child;
//     }
//     delete children;
//     while (!untried_actions->empty()) {
//         delete untried_actions->front();    // if a move is here then it is not a part of a child node and needs to be deleted here
//         untried_actions->pop();
//     }
//     delete untried_actions;
// }

void MCTS_node::untried_actions(){
    if (!_untried_actions) _untried_actions = &(state->get_legal_actions());
}

MCTS_node MCTS_node::expand(){
    untried_actions();
    pair<int, int> *move = _untried_actions->back();
    _untried_actions->pop_back();
    State *next_state = new State;
    *next_state = state->move_to_next_state(move);
    MCTS_node *child_node = new MCTS_node(this, next_state);
    children.push_back(child_node);
    return *child_node;
}

bool MCTS_node::is_terminal_node(){
    return state->is_game_over();
}

pair<int, int> MCTS_node::rollout_policy(vector< pair<int, int> *> *possible_moves){
    return (*possible_moves)[]
}

int MCTS_node::rollout(){
    State *current_rollout_state = state;
    while (!current_rollout_state->is_game_over()){
        vector< pair<int, int> *> possible_moves = current_rollout_state->get_legal_actions();

    }
}


/*** MCTS TREE ***/
MCTS_node *MCTS_tree::select(double c) {
    MCTS_node *node = root;
    while (!node->is_terminal()) {
        if (!node->is_fully_expanded()) {
            return node;
        } else {
            node = node->select_best_child(c);
        }
    }
    return node;
}

MCTS_tree::MCTS_tree(MCTS_state *starting_state) {
    assert(starting_state != NULL);
    root = new MCTS_node(NULL, starting_state, NULL);
}

MCTS_tree::~MCTS_tree() {
    delete root;
}

void MCTS_tree::grow_tree(int max_iter, double max_time_in_seconds) {
    MCTS_node *node;
    double dt;
    #ifdef DEBUG
    cout << "Growing tree..." << endl;
    #endif
    time_t start_t, now_t;
    time(&start_t);
    for (int i = 0 ; i < max_iter ; i++){
        // select node to expand according to tree policy
        node = select();
        // expand it (this will perform a rollout and backpropagate the results)
        node->expand();
        // check if we need to stop
        time(&now_t);
        dt = difftime(now_t, start_t);
        if (dt > max_time_in_seconds) {
            #ifdef DEBUG
            cout << "Early stopping: Made " << (i + 1) << " iterations in " << dt << " seconds." << endl;
            #endif
            break;
        }
    }
    #ifdef DEBUG
    time(&now_t);
    dt = difftime(now_t, start_t);
    cout << "Finished in " << dt << " seconds." << endl;
    #endif
}

unsigned int MCTS_tree::get_size() const {
    return root->get_size();
}

const MCTS_move *MCTS_node::get_move() const {
    return move;
}

const MCTS_state *MCTS_node::get_current_state() const { return state; }

void MCTS_node::print_stats() const {
    #define TOPK 10
    if (number_of_simulations == 0) {
        cout << "Tree not expanded yet" << endl;
        return;
    }
    cout << "___ INFO _______________________" << endl
         << "Tree size: " << size << endl
         << "Number of simulations: " << number_of_simulations << endl
         << "Branching factor at root: " << children->size() << endl
         << "Chances of P1 winning: " << setprecision(4) << 100.0 * (score / number_of_simulations) << "%" << endl;
    // sort children based on winrate of player's turn for this node (!)
    if (state->player1_turn()) {
        std::sort(children->begin(), children->end(), [](const MCTS_node *n1, const MCTS_node *n2){
            return n1->calculate_winrate(true) > n2->calculate_winrate(true);
        });
    } else {
        std::sort(children->begin(), children->end(), [](const MCTS_node *n1, const MCTS_node *n2){
            return n1->calculate_winrate(false) > n2->calculate_winrate(false);
        });
    }
    // print TOPK of them along with their winrates
    cout << "Best moves:" << endl;
    for (int i = 0 ; i < children->size() && i < TOPK ; i++) {
        cout << "  " << i + 1 << ". " << children->at(i)->move->sprint() << "  -->  "
             << setprecision(4) << 100.0 * children->at(i)->calculate_winrate(state->player1_turn()) << "%" << endl;
    }
    cout << "________________________________" << endl;
}

double MCTS_node::calculate_winrate(bool player1turn) const {
    if (player1turn) {
        return score / number_of_simulations;
    } else {
        return 1.0 - score / number_of_simulations;
    }
}

void MCTS_tree::advance_tree(const MCTS_move *move) {
    MCTS_node *old_root = root;
    root = root->advance_tree(move);
    delete old_root;       // this won't delete the new root since we have emptied old_root's children
}

const MCTS_state *MCTS_tree::get_current_state() const { return root->get_current_state(); }

MCTS_node *MCTS_tree::select_best_child() {
    return root->select_best_child(0.0);
}

void MCTS_tree::print_stats() const { root->print_stats(); }


/*** MCTS agent ***/
MCTS_agent::MCTS_agent(MCTS_state *starting_state, int max_iter, int max_seconds)
: max_iter(max_iter), max_seconds(max_seconds) {
    tree = new MCTS_tree(starting_state);
}

const MCTS_move *MCTS_agent::genmove(const MCTS_move *enemy_move) {
    if (enemy_move != NULL) {
        tree->advance_tree(enemy_move);
    }
    // If game ended from opponent move, we can't do anything
    if (tree->get_current_state()->is_terminal()) {
        return NULL;
    }
    #ifdef DEBUG
    cout << "___ DEBUG ______________________" << endl
         << "Growing tree..." << endl;
    #endif
    tree->grow_tree(max_iter, max_seconds);
    #ifdef DEBUG
    cout << "Tree size: " << tree->get_size() << endl
         << "________________________________" << endl;
    #endif
    MCTS_node *best_child = tree->select_best_child();
    if (best_child == NULL) {
        cerr << "Warning: Tree root has no children! Possibly terminal node!" << endl;
        return NULL;
    }
    const MCTS_move *best_move = best_child->get_move();
    tree->advance_tree(best_move);
    return best_move;
}

MCTS_agent::~MCTS_agent() {
    delete tree;
}

const MCTS_state *MCTS_agent::get_current_state() const { return tree->get_current_state(); }