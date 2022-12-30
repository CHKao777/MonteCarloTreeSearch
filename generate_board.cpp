#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <iterator>

using namespace std;

#define row  14
#define col  7
#define num_board 10000

bool checkstable(vector< vector<int> > &board){
        for (int i=0 ; i<row;i++)
            for(int j=0 ; j<(col-2) ; j++)
                if (abs(board[i][j]) == abs(board[i][j+1]) && 
                    abs(board[i][j+1])==abs(board[i][j+2]) && abs(board[i][j+2]) != 0){
                    return false;                    
                }
        return true;
}

int main(){
    string file_name("board/board_");

    vector< vector<int> > board = vector< vector<int> >(row, vector<int>(col, 0));;

    vector<int> temp;
    for (int i = 0; i < row; ++i){
        temp.push_back(i / 2 + 1);
    }


    for (int i = 0; i < num_board; ++i){

        while(true){
            for(int i = 0; i < col; ++i){
                random_shuffle(temp.begin(), temp.end());
                for (int j = 0; j < row; ++j){
                    board[j][i] = temp[j];
                }
            }            
            if (checkstable(board)) break;
        }

        ofstream output_file(file_name + to_string(i));
        ostream_iterator<int> output_iterator(output_file, " ");
        for (vector<int> &x:board){
            copy(x.begin(), x.end(), output_iterator);
            output_file << '\n';
        }
    }
}