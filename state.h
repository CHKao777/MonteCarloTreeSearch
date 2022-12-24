#ifndef STATE_H
#define STATE_H

#include <vector>

using namespace std;

int clean(vector< vector<int> > &board);  // clean the tile , return points
void drop(vector< vector<int> > &board);  // drop the tile

class State {
public:
    vector< vector<int> > board;
    pair<int, int> move;
    int next_to_move;
    int score;

    ~State() = default;
    State(vector< vector<int> > &board, int score, pair<int, int> &move, int next_to_move);  
    int game_result() const;
    bool is_game_over() const;
    void is_move_legal(pair<int, int> &move) const;
    State* move_to_next_state(pair<int, int> &move);
    void get_legal_actions(vector< pair<int, int>* >* legal_actions) const;
};


#endif