#include "state.h"
#include "cstdlib"
#include <assert.h>

using namespace std;

#define row 10
#define col 5

int clean(vector< vector<int> > &board){
    bool unstable = true;
    int pts = 0;
    while(unstable){
        unstable = false;
        for(int i=0; i<row; i++){
            for(int j=0; j<col-2; j++){
                if (abs(board[i][j]) == abs(board[i][j+1]) && abs(board[i][j+1])==abs(board[i][j+2]) && abs(board[i][j+2]) != 0){
                    board[i][j] = board[i][j+1] = board[i][j+2] = -abs(board[i][j]);
                    unstable = true;
                }
            }
        }
        for(int i = 0 ; i <row; i++){
            for(int j = 0; j<col; j++){
                int p = board[i][j];
                if (p<0){
                    pts-=p;
                    board[i][j] = 0;
                }                        
            }
        }
        drop(board);     
    }
    return pts;
}

void drop(vector< vector<int> > &board){
    for(int j=0 ; j<col ; j++){
        int wp = row-1;
        for(int i= row-1; i>=0; i--){
            if (board[i][j]>0){
                board[wp][j] = board[i][j];
                wp-=1;
            }
        }
        for(int k=wp; k>=0; k--)
            board[k][j] = 0;
    }
}

State::State(vector< vector<int> > &board, int score, pair<int, int> &move, int next_to_move){
    this->board = board;
    this->score = score;
    this->move = move;
    this->next_to_move = next_to_move;
} 

bool State::is_game_over() const {
    bool gameover = false;
    for(auto v :board[row-1])
        if (v == 0){
            gameover = true;
            break;
        }
    return gameover;
}

int State::game_result() const {
    if (score > 0) return 1;
    else if (score < 0) return -1;
    else return 0;
}

void State::is_move_legal(pair<int, int> &move) const {
    int x = move.first, y = move.second;
    assert(0 <= x && x <= row - 1 && 0 <= y && y <= col - 1);
    assert(board[x][y] > 0);
}

State* State::move_to_next_state(pair<int, int> &move){
    is_move_legal(move);

    int x = move.first, y = move.second;
    int pts = board[x][y];
    State *next_state = new State(board, score, move, 3 - next_to_move);
    next_state->board[x][y] = 0;
    drop(next_state->board);
    if (!next_state->is_game_over())
        pts += clean(next_state->board);
    if (next_to_move == 1) next_state->score += pts;
    else next_state->score -= pts;
    return next_state;
}

void State::get_legal_actions(vector< pair<int, int>* >* legal_actions) const {
    // vector< pair<int, int>* >* legal_actions_vector = new vector< pair<int, int>* >;

    //release legal_actions
    for (int i = 0; i < legal_actions->size(); ++i){
        delete (*legal_actions)[i];
    }
    //empty legal_actions
    legal_actions->clear();

    for (int i = 0; i < row; ++i){
        for (int j = 0; j < col; ++j){
            if (board[i][j] != 0) legal_actions->push_back(new pair<int, int>(i, j));
        }
    }
}