#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <fstream>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <getopt.h>
#include <assert.h>
#include <map>
#include "state.h"
#include "node.h"
#include "tree.h"


using namespace std;

int row = 14;
int col = 7;

class AI{
private:
    bool gameover = false;
    vector< vector<int> > board = vector< vector<int> >(row, vector<int>(col, 0));;

    int turn = 0;
    int mypts = 0;
    int oppopts = 0;
    int step = 0;

    char player1, player2;

public:
    AI(){
        init_arguments();
    }

    void init_arguments(){
        gameover = false;
        mypts = 0;
        oppopts = 0;
        step = 0;
    }

    void read_board(int board_index){
        ifstream input_file;
        input_file.open("board/board_" + to_string(board_index));
        assert (!input_file.fail());

        int number;
        for(auto &r:board)
            for(auto &c:r){
                input_file>>number;
                c=(number);
            }
    }

    void init_board(){
        //init board, randomly generate
        vector<int> temp;
        for (int i = 0; i < row; ++i){
            temp.push_back(i / 2 + 1);
        }
        while(true){
            for(int i = 0; i < col; ++i){
                random_shuffle(temp.begin(), temp.end());
                for (int j = 0; j < row; ++j){
                    board[j][i] = temp[j];
                }
            }            
            if (checkstable()) break;
        }
    }
    
    ~AI(){
        for (auto v : board) v.clear();
        board.clear();
    }

    void exchange_turn(){
        turn = 1 - turn;
    }

    void set_player_mode(char player1, char player2){
        if (player1 == 'm'){
            this->player1 = player1;
            this->player2 = player2;
        }
        else if (player2 == 'm'){
            this->player1 = player2;
            this->player2 = player1;
        }
        else{
            this->player1 = player1;
            this->player2 = player2;
        }
    }

    bool checkstable(){
        for (int i=0 ; i<row;i++)
            for(int j=0 ; j<(col-2) ; j++)
                if (abs(board[i][j]) == abs(board[i][j+1]) && 
                    abs(board[i][j+1])==abs(board[i][j+2]) && abs(board[i][j+2]) != 0){
                    return false;                    
                }
        return true;
    }

    void drop(){
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

    bool checkgameover(){ //any of last row == 0
        for(auto v :board[row-1])
            if (v == 0){
                gameover = true;
                break;
            }
        return gameover;
    }

    int clean(){
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
            drop();     
        }
        checkgameover();
        return pts;
    }
    
    pair<int, int> make_decision(int *simulations_number, 
        int *total_simulation_milliseconds, char mode, int score){
        /*
        #######################################################
        ##### This is the main part you need to implement #####
        #######################################################   */

        pair<int, int> move;
        State *root_state = new State(board, score, move, 1);  
        MCTS_node *root_node = new MCTS_node(nullptr, root_state);

        if (mode == 'd')
            return rand_select();
        if (mode == 's'){
            MCTS_tree_serial *tree = new MCTS_tree_serial(root_node);

            chrono::milliseconds start = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());
            move = tree->best_action(simulations_number, total_simulation_milliseconds);
            chrono::milliseconds end = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());

            long long l_start = reinterpret_cast<long long&>(start);
            long long l_end = reinterpret_cast<long long&>(end);
            // cout << 100000.0 / (l_end - l_start) * 1000 << endl;

            delete tree;
            return move;
        }
        if (mode == 'l'){
            MCTS_tree_leaf *tree = new MCTS_tree_leaf(root_node);
            
            chrono::milliseconds start = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());
            move = tree->best_action(simulations_number, total_simulation_milliseconds);
            chrono::milliseconds end = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());

            long long l_start = reinterpret_cast<long long&>(start);
            long long l_end = reinterpret_cast<long long&>(end);
            // cout << 100000.0 / (l_end - l_start) * 1000 / 11664.5<< endl;

            delete tree;
            return move;
        }
        if (mode == 'r'){
            MCTS_tree_root *tree = new MCTS_tree_root(root_node);
            
             chrono::milliseconds start = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());
            move = tree->best_action(simulations_number, total_simulation_milliseconds);
            chrono::milliseconds end = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());

            long long l_start = reinterpret_cast<long long&>(start);
            long long l_end = reinterpret_cast<long long&>(end);
            // cout << 100000.0 / (l_end - l_start) * 1000 / 11664.5<< endl;

            delete tree;
            return move;
        }
        if (mode == 't'){
            MCTS_tree_tree *tree = new MCTS_tree_tree(root_node);
            
             chrono::milliseconds start = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());
            move = tree->best_action(simulations_number, total_simulation_milliseconds);
            chrono::milliseconds end = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch());

            long long l_start = reinterpret_cast<long long&>(start);
            long long l_end = reinterpret_cast<long long&>(end);
            // cout << 100000.0 / (l_end - l_start) * 1000 / 11664.5<< endl;

            delete tree;
            return move;
        }
        throw invalid_argument("player mode error");
    }

    pair<int, int> rand_select(){
        int p = 0;
        int x ;
        int y ;
        while (!p){
            x = rand()%row;
            y = rand()%col;
            p = board[x][y];
        }
        return { x , y };
    }    

    int make_move(int x, int y){
        int pts = board[x][y];
        board[x][y]= 0;
        drop();
        if (checkgameover())
            return pts;
        pts+= clean();
        return pts;
    }

    int start(int *simulations_number, int *total_simulation_milliseconds){
        int result;
        if (player1 == 'm'){
            cout<<"Game start!"<<endl;
            pair<int, int> xy;
            int pts;
            show_board();
            cout<<"Set the player's order(0:first, 1:second): ";
            cin>>turn;
            if (turn!=0 && turn!=1){
                cout<<"wrong order";
                exit(0);
            }
                

            while(!gameover){
                cout<<"-------------------------"<< endl;
                cout<<"Turn"<<step<<endl;
                if((step%2) == turn){
                    cout<<"It\'s your turn"<<endl;
                    xy = make_decision(simulations_number, total_simulation_milliseconds, player2, mypts - oppopts);
                    cout << "your move is :"<<xy.first<<","<<xy.second << endl;
                    if(board[xy.first][xy.second]==0){
                        cout<<"illegal move";
                        exit(0);
                    }
                    pts = make_move(xy.first,xy.second);
                    mypts += pts;
                    cout<<"You get "<<pts<<" points"<< endl;
                    show_board();
                } 
                else{
                    cout<<"It\'s opponent\'s turn"<<endl;
                    cout<<"Your opponent move is ";
                    cin>>xy.first>>xy.second; //open this line when you submit

                    //xy = rand_select(); //can use this while testing ,close this line when you submit

                    cout <<"oppo move is:"<<xy.first<<","<<xy.second << endl;
                    if(board[xy.first][xy.second]==0){
                        cout<<"illegal move";
                        exit(0);
                    }
                    pts = make_move(xy.first,xy.second);
                    oppopts += pts;
                    cout<<"Your opponent get "<<pts<<" points"<< endl;
                    show_board();
                }
            step++;
            }
            //gameover
            if(mypts > oppopts){
                cout<<"You win!"<<endl;
                result = 1;
            }
            else if(mypts < oppopts){
                cout<<"You lose!"<<endl;
                result = -1;
            }
            else{
                cout<<"Tie!"<<endl;
                result =  0;
            }
        }
		
        else{
            pair<int, int> xy;
            int pts;

            while(!gameover){
                if((step%2) == turn){
                    xy = make_decision(simulations_number, total_simulation_milliseconds, player1, mypts - oppopts);
                    if(board[xy.first][xy.second]==0){
                        cout<<"illegal move";
                        exit(0);
                    }
                    pts = make_move(xy.first,xy.second);
                    mypts += pts;
                } 
                else{
                    xy = make_decision(simulations_number, total_simulation_milliseconds, player2, oppopts - mypts);
                    if(board[xy.first][xy.second]==0){
                        cout<<"illegal move";
                        exit(0);
                    }
                    pts = make_move(xy.first,xy.second);
                    oppopts += pts;
                }
            step++;
            }
            //gameover
            if(mypts > oppopts){
                result = 1;
            }
            else if(mypts < oppopts){
                result = -1;
            }
            else{
                result = 0;
            }
        }
        return result;
	}

    void show_board(){
        cout<<"my points:"<< mypts << endl;
        cout<<"opponent\'s points: "<< oppopts <<endl;
        cout<<"The board is : "<<endl;
        for(int i = 0; i< row; i++){
            for(int j = 0;j < col; j++){
                cout<<board[i][j];
            }
            cout << endl;
        }
        cout<<"-------------------------"<< endl;
    }    
};


void usage(const char *progname)
{
    printf("Usage: %s [options]\n", progname);
    printf("Program Options:\n");
    printf("  -m  --man             set one player to manual mode\n");
    printf("  -d  --rand            set one player to random mode\n");
    printf("  -s  --serial          set one player to serial MCTS mode\n");
    printf("  -l  --leaf            set one player to tree parallelization MCTS mode\n");
    printf("  -r  --root            set one player to root parallelization MCTS mode\n");
    printf("  -t  --tree            set one player to tree parallelization MCTS mode\n");
    printf("  -n  --game_num <INT>  number of games(default:1000). If one player is in manual mode, this argument will be ignored\n");
    printf("  -i  --iter <INT>      number of iterations per MCTS. You have to provide -i or -x, but not both\n");
    printf("  -x  --ms <INT>        execution time (in millisecond) per MCTS. You have to provide -i or -x, but not both\n");
    printf("  -?  --help            This message\n");
}

int main(int argc, char **argv){
    char player1 = 'z', player2 = 'z';
    int game_n = 1000;
    int *simulations_number = nullptr;
    int *total_simulation_milliseconds = nullptr;

    static struct option long_options[] = {
    {"man", 0, 0, 'm'},
    {"rand", 0, 0, 'd'},
    {"serial", 0, 0, 's'},
    {"leaf", 0, 0, 'l'},
    {"root", 0, 0, 'r'},
    {"tree", 0, 0, 't'},
    {"game_num", 1, 0, 'n'},
    {"ms", 1, 0, 'x'},
    {"iter", 1, 0, 'i'},
    {"help", 0, 0, '?'},
    {0, 0, 0, 0}};

    map<char, string> map1 = {{'d', "random",},
                                {'s', "serial",},
                                {'l', "leaf",},
                                {'r', "root",},
                                {'t', "tree",}};

    int opt, temp1, temp2;
    while ((opt = getopt_long(argc, argv, "mslrtn:i:x:?", long_options, NULL)) != EOF)
    {
        switch (opt)
        {
        case 'm':
        {
            if (player1 != 'z' && player2 != 'z'){
                fprintf(stderr, "player setting more than 2 times\n");
                return -1;
            }
            if (player1 == 'm' || player2 == 'm'){
                fprintf(stderr, "# of manual players must <= 1\n");
                return -1;
            }
            if (player1 == 'z')
                player1 = 'm';
            else
                player2 = 'm';
            break;
        }
        case 'd':
        {
            if (player1 != 'z' && player2 != 'z'){
                fprintf(stderr, "player setting more than 2 times\n");
                return -1;
            }
            if (player1 == 'z')
                player1 = 'd';
            else
                player2 = 'd';
            break;
        }   
        case 's':
        {
            if (player1 != 'z' && player2 != 'z'){
                fprintf(stderr, "player setting more than 2 times\n");
                return -1;
            }
            if (player1 == 'z')
                player1 = 's';
            else
                player2 = 's';
            break;
        }   
        case 'l':
        {
            if (player1 != 'z' && player2 != 'z'){
                fprintf(stderr, "player setting more than 2 times\n");
                return -1;
            }
            if (player1 == 'z')
                player1 = 'l';
            else
                player2 = 'l';
            break;
        }
        case 'r':
        {
            if (player1 != 'z' && player2 != 'z'){
                fprintf(stderr, "player setting more than 2 times\n");
                return -1;
            }
            if (player1 == 'z')
                player1 = 'r';
            else
                player2 = 'r';
            break;
        } 
        case 't':
        {
            if (player1 != 'z' && player2 != 'z'){
                fprintf(stderr, "player setting more than 2 times\n");
                return -1;
            }
            if (player1 == 'z')
                player1 = 't';
            else
                player2 = 't';
            break;
        }  

        case 'n':
        {
            game_n = atoi(optarg);
            if (game_n < 1){
                fprintf(stderr, "-n must >= 1\n");
                return 1;
            }
            break;
        }
        case 'i':
        {
            if (total_simulation_milliseconds){
                fprintf(stderr, "-i and -x can't be set simultaneously\n");
                return -1;
            }
            temp1 = atoi(optarg);
            simulations_number = &temp1;
            if (*simulations_number < 1){
                fprintf(stderr, "-i must >= 1\n");
                return 1;
            }
            break;
        }
        case 'x':
        {
            if (simulations_number){
                fprintf(stderr, "-i and -x can't be set simultaneously\n");
                return -1;
            }
            temp2 = atoi(optarg);
            total_simulation_milliseconds = &temp2;
            if (*total_simulation_milliseconds < 1){
                fprintf(stderr, "-x must >= 1\n");
                return 1;
            }
            break;
        }
        case '?':
        default:
            usage(argv[0]);
            return 1;
        }
    }

    if (player1 == 'z' || player2 == 'z'){
        fprintf(stderr, "players are set incorrectly\n");
        return -1;
    }

    if (!simulations_number && !total_simulation_milliseconds){
        fprintf(stderr, "You have to provide -i or -x\n");
        return -1;
    }

    srand( time(NULL));

    AI game;

    game.set_player_mode(player1, player2);

    if (player1 == 'm' || player2 == 'm'){
        game.start(simulations_number, total_simulation_milliseconds);
        return 0;
    }

    int win = 0, lose = 0, tie = 0, result;
    int x = game_n / 100;
    if (x == 0) ++x;
    for (int i = 0; i < game_n; ++i){
        if (i % x == 0) {
            cout << "\rprocessing:" << ((double) i) / game_n * 100 << "%";
            cout.flush();
        }

        game.read_board(i / 2);
        game.init_arguments();

        result = game.start(simulations_number, total_simulation_milliseconds);
        game.exchange_turn();

        if (result == 1) ++win;
        else if (result == -1) ++lose;
        else ++tie;
    }

    cout << "\n-------result-------" << endl;
    cout << map1[player1] << " vs. " << map1[player2] << endl;
    cout << "win rate:" << (double)win / (win + lose + tie) << endl;
    cout << "tie rate:" << (double)tie / (win + lose + tie) << endl;
    cout << "lose rate:" << (double)lose / (win + lose + tie) << endl;

    // game.read_board(10);

    // game.init_arguments();

    // game.make_decision(simulations_number, total_simulation_milliseconds, player1, 0);



    return 0;
}