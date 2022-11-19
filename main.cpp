
#include <iostream>  //i/o
#include <vector>    //for board
#include <algorithm> //vector functions
#include <chrono>    //time
#include <thread>    //sleep

#include "functions.h" //has all functions
#include "checkmate.h" //checks whether theres checkmate
#include "ANSI Windows Terminal.h" //cross platform 

#ifdef _WIN32
	bool ok = init_term();
#endif

using namespace std;

//stores the entire chess board
vector<vector<piece> > chess_board = {
    { {"rook",'b'}, {"knight",'b'}, {"bishop",'b'}, {"queen",'b'}, {"king",'b'}, {"bishop",'b'}, {"knight",'b'}, {"rook",'b'} },
    { {"pawn", 'b'}, {"pawn", 'b'}, {"pawn", 'b'}, {"pawn", 'b'}, {"pawn", 'b'}, {"pawn", 'b'}, {"pawn", 'b'}, {"pawn", 'b'} },
    { {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'} },
    { {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'} },
    { {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'} },
    { {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'}, {"space",'s'} },
    { {"pawn", 'w'}, {"pawn", 'w'}, {"pawn", 'w'}, {"pawn", 'w'}, {"pawn", 'w'}, {"pawn", 'w'}, {"pawn", 'w'}, {"pawn", 'w'} },
    { {"rook",'w'}, {"knight",'w'}, {"bishop",'w'}, {"queen",'w'}, {"king",'w'}, {"bishop",'w'}, {"knight",'w'}, {"rook",'w'} },
};

//Function Declarations
void move(char color);
void print_title();


//main
int main(){

    print_title();
    
    char current_turn = 'w'; //colours representing current turn 
    //'w' = white & 'b' == black

    while(true){ //RUN ONE TURN 
        
        printf("\033[2J"); //clear screen
        print(chess_board);
        move(current_turn);
        printf("\033[2J");
        print(chess_board);
   
        //check for special case
        if(is_king_eaten()) break;
            
        //check if checkmate after move
        bool is_checkmate = checkmate(chess_board, current_turn);
        if(is_checkmate)
            break;
        else //if not checkmate check if check
            bool is_check = check(chess_board, current_turn);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //change turn color 
        current_turn = (current_turn == 'w') ? 'b' : 'w';

        printf("\x1b[1m\n\n\nFLIPPING BOARD FOR NEXT PLAYER...\n\x1b[0m");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        flip_board(&chess_board);
    }

    cout<<"THE WINNER IS....\n\n";
    if(current_turn == 'w') cout<<"WHITE!\n";
    else cout<<"BLACK!\n";
    cout<<"THANKS FOR PLAYING\n";
    return 0;

}

//moves pieces and eats other pieces 
void move(char color){
    bool good_move = false;
    int r1, c1, r2, c2;
    while(!good_move){
        get_start_piece (chess_board, color, &r1, &c1);
        get_end_piece   (chess_board, color, &r1, &c1, &r2, &c2);
        piece selected = chess_board[r1][c1];
        if(selected.n == "pawn"){
            good_move = pawn_move(&chess_board, r1, c1, r2, c2);
        }else if(selected.n == "rook"){
            good_move = rook_move(&chess_board, r1, c1, r2, c2);
        }else if(selected.n == "bishop"){
            good_move = bishop_move(&chess_board, r1, c1, r2, c2);
        }else if(selected.n == "knight"){
            good_move = knight_move(&chess_board, r1, c1, r2, c2);
        }else if(selected.n == "queen"){
            good_move = queen_move(&chess_board, r1, c1, r2, c2);
        }else if(selected.n == "king"){
            good_move = king_move(&chess_board, r1, c1, r2, c2);
        }
    }
}

void print_title(){

    printf("\x1b[38;5;21m"); //make color blue

    cout<<" ██████╗██╗  ██╗███████╗███████╗███████╗    \n";
    cout<<"██╔════╝██║  ██║██╔════╝██╔════╝██╔════╝    \n";
    cout<<"██║     ███████║█████╗  ███████╗███████╗    \n";
    cout<<"██║     ██╔══██║██╔══╝  ╚════██║╚════██║    \n";
    cout<<"╚██████╗██║  ██║███████╗███████║███████║    \n";
    cout<<" ╚═════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝    \n";

    printf("\x1b[0m"); //reset color

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    cout<<".\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    cout<<"..\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    cout<<"...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    cout<<"....\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    cout<<".....\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    printf("\033[2J"); //clear screen

}