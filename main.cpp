#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <random>
#include <algorithm>
#include <unistd.h>
#include "state.h"
#include "mcts.h"


using namespace std;


class AI{
private:
    int row = 10;
    int col = 5;
    bool gameover = false;
    vector< vector<int> > board = vector< vector<int> >(row, vector<int>(col, 0));;

    int turn = 0;
    int mypts = 0;
    int oppopts = 0;
    int step = 0;
    int mode = 0; //0:手動模式(manual_mode), 1:自動模擬(auto_mode)

public:
    AI(){
        init();
    }

    void init(){
        //init board, randomly generate
        vector<int> temp{1, 1, 2, 2, 3, 3, 4, 4, 5, 5};
        while(true){
            for(int i = 0; i < col; ++i){
                random_shuffle(temp.begin(), temp.end());
                for (int j = 0; j < row; ++j){
                    board[j][i] = temp[j];
                }
            }            
            if (checkstable()) break;
        }
            
        //init arguments
        gameover = false;
        turn = 0;
        mypts = 0;
        oppopts = 0;
        step = 0;
    }

    ~AI(){
        for (auto v : board) v.clear();
        board.clear();
    }

    void set_manual_mode(){
        mode = 0;
    }

    void set_auto_mode(){
        mode = 1;
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
    
    pair<int, int> make_decision(int simulations_number){
        /*
        #######################################################
        ##### This is the main part you need to implement #####
        #######################################################   */

        pair<int, int> move;
        State *root_state = new State(board, mypts - oppopts, move, 1);  
        MCTS_node *root_node = new MCTS_node(nullptr, root_state);
        MCTS_tree *tree = new MCTS_tree(root_node);

        int total_simulation_milliseconds = 10000;
        move = tree->best_action(&simulations_number, nullptr);
        // move = tree->best_action(nullptr, &total_simulation_milliseconds);

        delete tree;

        return move;

        // return rand_select();
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

    int start(int simulations_number){
        int result;
        if (mode == 0){
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
                    xy = make_decision(simulations_number);
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
		
        if (mode == 1){
            pair<int, int> xy;
            int pts;

            random_device rd;
            default_random_engine eng(rd());
            uniform_int_distribution<int> distr(0, 1);
            turn = distr(eng);

            while(!gameover){
                if((step%2) == turn){
                    xy = make_decision(simulations_number);
                    if(board[xy.first][xy.second]==0){
                        cout<<"illegal move";
                        exit(0);
                    }
                    pts = make_move(xy.first,xy.second);
                    mypts += pts;
                } 
                else{
                    xy = rand_select();
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
    printf("  -m        set mode(0 or 1, default:0), 0 for manual mode, 1 for auto mode\n");
    printf("  -n        number of games(default:1000). If mode is set to 0, -n will be ignored\n");
    printf("  -i        number of iterations per MCTS(default:250)\n");
    printf("  -?        This message\n");
}

int main(int argc, char **argv){

    srand( time(NULL));

    AI game;
    game.set_manual_mode();
    int mode = 0, game_n = 1000, simulations_number = 250;

    int opt;
    while ((opt = getopt(argc, argv, "m:n:i:?")) != EOF)
    {
        switch (opt)
        {
        case 'm':
        {
            mode = atoi(optarg);
            if (mode == 0)  game.set_manual_mode();
            else if (mode == 1) game.set_auto_mode();
            else {
                fprintf(stderr, "Invalid mode index\n");
                return 1;
            }
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
            simulations_number = atoi(optarg);
            if (simulations_number < 1){
                fprintf(stderr, "-i must >= 1\n");
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

    if (mode == 0){
        game.start(simulations_number);
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
        result = game.start(simulations_number);
        game.init();
        if (result == 1) ++win;
        else if (result == -1) ++lose;
        else ++tie;
    }

    cout << "\n-------result-------" << endl;
    cout << "win rate:" << (double)win / (win + lose + tie) << endl;
    cout << "tie rate:" << (double)tie / (win + lose + tie) << endl;
    cout << "lose rate:" << (double)lose / (win + lose + tie) << endl;

    return 0;
}