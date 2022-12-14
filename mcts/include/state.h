#ifndef MCTS_STATE_H
#define MCTS_STATE_H

// #include <stdexcept>
// #include <queue>

#include <vector>


using namespace std;


/** Implement all pure virtual methods. Notes:
 * - rollout() must return something in [0, 1] for UCT to work as intended and specifically
 * the winning chance of player1.
 * - player1 is determined by player1_turn()
 */
class MCTS_state {

public:
    // Implement these:
    virtual ~MCTS_state() = default;
    virtual int game_result() const = 0;
    virtual bool is_game_over() const = 0;
    virtual void is_move_legal(const pair<int, int> *move) const = 0;
    virtual MCTS_state move(const pair<int, int> *move) const = 0;
    virtual vector< pair<int, int> > get_legal_actions() const = 0;
};


#endif