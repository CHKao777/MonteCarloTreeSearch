#ifndef MCTS_CANTRIS_H
#define MCTS_CANTRIS_H

#include "../mcts/include/state.h"


int clean(vector< vector<int> > &board);  // clean the tile , return points
void drop(vector< vector<int> > &board);  // drop the tile

class CANTRIS_state : public MCTS_state {
    vector< vector<int> > board;
public:
    pair<int, int> *move;
    int next_to_move;
    int score;
    
    CANTRIS_state(vector< vector<int> > &board, int score, pair<int, int> *move, int next_to_move);    
    virtual int game_result() const override;
    virtual bool is_game_over() const override;
    virtual void is_move_legal(const pair<int, int> *move) const override;
    virtual MCTS_state move(const pair<int, int> *move) const override;
    virtual vector< pair<int, int> > get_legal_actions() const override;
};


#endif