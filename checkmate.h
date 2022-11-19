#include <iostream>  //i/o
#include <vector>    //for board
#include <chrono>
#include <thread>

//#include "functions.h"

using std::vector;
using std::cin;
using std::cout;

//FUNCTIONS ------------------------------------------------------------
bool checkmate(vector<vector<piece>> board, char color);
bool check(vector<vector<piece>> board, char color);
bool can_be_eaten(vector<vector<piece>>board, char color, int r, int c);
//----------------------------------------------------------------------

bool checkmate(vector<vector<piece>> board, char color){

    bool is_checkmate = false;

    char find_this_color = 'w';
    if(color == 'w') find_this_color = 'b';

    int r, c; //pos of king

    for(int i = 0; i<8; ++i){
        bool found = false;
        for(int j = 0; j<8; ++j){
            if(board[i][j].n == "king" && board[i][j].c == find_this_color){
                r = i;
                c = j;
                found = true;
                break;
            }
        }
        if(found) break;
    }

    int eatable_positions = 0;

    //check how many positions can be eaten
    if(r-1 > 0 && c-1 > 0){
        if(can_be_eaten(board, color, r-1, c-1)) ++eatable_positions;
    }
    if(r-1 > 0){
        if(can_be_eaten(board, color, r-1, c)) ++eatable_positions;
    }
    if(r-1>0 && c+1 < 8){
        if(can_be_eaten(board, color, r-1, c+1)) ++eatable_positions;
    }
    if(c-1 > 0){
        if(can_be_eaten(board, color, r,   c-1)) ++eatable_positions;
    }
    if(c+1 < 8){
        if(can_be_eaten(board, color, r,   c+1)) ++eatable_positions;
    }
    if(r+1<8 && c-1 > 0){
        if(can_be_eaten(board, color, r+1, c-1)) ++eatable_positions;
    }
    if(r+1<8){
        if(can_be_eaten(board, color, r+1, c  )) ++eatable_positions;
    }
    if(r+1<8 && c+1 < 8){
        if(can_be_eaten(board, color, r+1, c+1)) ++eatable_positions;
    }

    if(eatable_positions == 8){
        cout<<" ▄████▄   ██░ ██ ▓█████  ▄████▄   ██ ▄█▀ ███▄ ▄███▓ ▄▄▄     ▄▄▄█████▓▓█████     \n";
        cout<<"▒██▀ ▀█  ▓██░ ██▒▓█   ▀ ▒██▀ ▀█   ██▄█▒ ▓██▒▀█▀ ██▒▒████▄   ▓  ██▒ ▓▒▓█   ▀     \n";
        cout<<"▒▓█    ▄ ▒██▀▀██░▒███   ▒▓█    ▄ ▓███▄░ ▓██    ▓██░▒██  ▀█▄ ▒ ▓██░ ▒░▒███       \n";
        cout<<"▒▓▓▄ ▄██▒░▓█ ░██ ▒▓█  ▄ ▒▓▓▄ ▄██▒▓██ █▄ ▒██    ▒██ ░██▄▄▄▄██░ ▓██▓ ░ ▒▓█  ▄     \n";
        cout<<"▒ ▓███▀ ░░▓█▒░██▓░▒████▒▒ ▓███▀ ░▒██▒ █▄▒██▒   ░██▒ ▓█   ▓██▒ ▒██▒ ░ ░▒████▒    \n";
        cout<<"░ ░▒ ▒  ░ ▒ ░░▒░▒░░ ▒░ ░░ ░▒ ▒  ░▒ ▒▒ ▓▒░ ▒░   ░  ░ ▒▒   ▓▒█░ ▒ ░░   ░░ ▒░ ░    \n";
        cout<<"  ░  ▒    ▒ ░▒░ ░ ░ ░  ░  ░  ▒   ░ ░▒ ▒░░  ░      ░  ▒   ▒▒ ░   ░     ░ ░  ░    \n";
        cout<<"░         ░  ░░ ░   ░   ░        ░ ░░ ░ ░      ░     ░   ▒    ░         ░       \n";
        cout<<"░ ░       ░  ░  ░   ░  ░░ ░      ░  ░          ░         ░  ░           ░  ░    \n";
        cout<<"░                       ░                                                       \n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return true;
    }
    return false;
}

bool check(vector<vector<piece>> board, char color){

    bool is_check = false;

    char find_this_color = 'w';
    if(color == 'w') find_this_color = 'b';

    int r, c; //pos of king

    for(int i = 0; i<8; ++i){
        bool found = false;
        for(int j = 0; j<8; ++j){
            if(board[i][j].n == "king" && board[i][j].c == find_this_color){
                r = i;
                c = j;
                found = true;
                break;
            }
        }
        if(found) break;
    }

    is_check = can_be_eaten(board, color, r, c);

    if(is_check){
        cout<<"▄█▄     ▄  █ ▄███▄   ▄█▄    █  █▀ \n";
        cout<<"█▀ ▀▄  █   █ █▀   ▀  █▀ ▀▄  █▄█   \n";
        cout<<"█   ▀  ██▀▀█ ██▄▄    █   ▀  █▀▄   \n";
        cout<<"█▄  ▄▀ █   █ █▄   ▄▀ █▄  ▄▀ █  █  \n";
        cout<<"▀███▀     █  ▀███▀   ▀███▀    █   \n";
        cout<<"         ▀                   ▀    \n";
        std::this_thread::sleep_for(std::chrono::seconds(2));

        return true;
    }

    return false;
}
     
bool can_be_eaten(vector<vector<piece>>board, char color, int r, int c){

    // _____      _                       ______                   ___  
    //|  ___|    | |                      | ___ \                 |__ 
    //| |__  __ _| |_ ___ _ __    ______  | |_/ /_ ___      ___ __   ) |
    //|  __|/ _` | __/ _ \ '_ \  |______| |  __/ _` \ \ /\ / / '_ \ / / 
    //| |__| (_| | ||  __/ | | |          | | | (_| |\ V  V /| | | |_|  
    //\____/\__,_|\__\___|_| |_|          \_|  \__,_| \_/\_/ |_| |_(_) 
    //------------------------------------------------------------------------------------------
    // can be eaten by a pawn?
    if(!( (r+1 > 8 || r+1<0) || (c-1 > 8 || c-1 < 0) )){
        if(board[r+1][c-1].n == "pawn" && board[r+1][c-1].c == color){
            return true;
        }
    }
    if(!( (r+1 > 8 || r+1<0) || (c+1 > 8 || c+1 < 0) )){
        if (board[r+1][c+1].n == "pawn" && board[r+1][c+1].c == color){
            return true;
        }
    }

    // _____      _                        _   __      _       _     _  ___  
    //|  ___|    | |                      | | / /     (_)     | |   | ||__ 
    //| |__  __ _| |_ ___ _ __    ______  | |/ / _ __  _  __ _| |__ | |_  ) |
    //|  __|/ _` | __/ _ \ '_ \  |______| |    \| '_ \| |/ _` | '_ \| __|/ / 
    //| |__| (_| | ||  __/ | | |          | |\  \ | | | | (_| | | | | |_|_|  
    //\____/\__,_|\__\___|_| |_|          \_| \_/_| |_|_|\__, |_| |_|\__(_)  
    //                                                    __/ |              
    //                                                   |___/               
    //------------------------------------------------------------------------------------------
    // can be eaten by a knight?
    if(!( (r-2 > 8 || r-2<0) || (c-1 > 8 || c-1 < 0) )){
        if(board[r-2][c-1].n == "knight" && board[r-2][c-1].c == color){
            return true;
        }
    }
    if(!( (r-2 > 8 || r-2<0) || (c+1 > 8 || c+1 < 0) )){
        if(board[r-2][c+1].n == "knight" && board[r-2][c+1].c == color){
            return true;
        }
    }
    if(!( (r-1 > 8 || r-1<0) || (c-2 > 8 || c-2 < 0) )){
        if(board[r-1][c-2].n == "knight" && board[r-1][c-2].c == color){
            return true;
        }
    }
    if(!( (r-1 > 8 || r-1<0) || (c+2 > 8 || c+2 < 0) )){
        if(board[r-1][c+2].n == "knight" && board[r-1][c+2].c == color){
            return true;
        }
    }
    if(!( (r+1 > 8 || r+1<0) || (c-2 > 8 || c-2 < 0) )){
        if(board[r+1][c-2].n == "knight" && board[r+1][c-2].c == color){
            return true;
        }
    }
    if(!( (r+1 > 8 || r+1<0) || (c-1 > 8 || c-1 < 0) )){
        if(board[r+1][c-1].n == "knight" && board[r+1][c-1].c == color){
            return true;
        }
    }
    if(!( (r+2 > 8 || r+2<0) || (c-1 > 8 || c-1 < 0) )){
        if(board[r+2][c-1].n == "knight" && board[r+2][c-1].c == color){
            return true;
        }
    }
    if(!( (r+2 > 8 || r+2<0) || (c+1 > 8 || c+1 < 0) )){
        if(board[r+2][c+1].n == "knight" && board[r+2][c+1].c == color){
            return true;
        }
    }


    // _____      _                        _   _            _                _        _ _      ___  
    //|  ___|    | |                      | | | |          (_)              | |      | | |    |__ 
    //| |__  __ _| |_ ___ _ __    ______  | |_| | ___  _ __ _ _______  _ __ | |_ __ _| | |_   _  ) |
    //|  __|/ _` | __/ _ \ '_ \  |______| |  _  |/ _ \| '__| |_  / _ \| '_ \| __/ _` | | | | | |/ / 
    //| |__| (_| | ||  __/ | | |          | | | | (_) | |  | |/ / (_) | | | | || (_| | | | |_| |_|  
    //\____/\__,_|\__\___|_| |_|          \_| |_/\___/|_|  |_/___\___/|_| |_|\__\__,_|_|_|\__, (_)  
    //                                                                                     __/ |    
    //                                                                                    |___/    
    //------------------------------------------------------------------------------------------ 
    // can be eaten by a queen or rook? (horizontal)
    //horizontal left to right
    for(int j = c + 1; j <= 7; ++j){
        if(board[r][j].c == color && (board[r][j].n == "queen" || board[r][j].n == "rook")){
            return true;
        }else if(board[r][j].n != "space"){
            break;
        }
    }
    //horizontal right to left 
    for(int j = c - 1; j >= 0; --j){
        if(board[r][j].c == color && (board[r][j].n == "queen" || board[r][j].n == "rook")){
            return true;
        }else if(board[r][j].n != "space"){
            break;
        }
    }
	
    // _____      _                        _   _           _   _           _ _      ___  
    //|  ___|    | |                      | | | |         | | (_)         | | |    |__ 
    //| |__  __ _| |_ ___ _ __    ______  | | | | ___ _ __| |_ _  ___ __ _| | |_   _  ) |
    //|  __|/ _` | __/ _ \ '_ \  |______| | | | |/ _ \ '__| __| |/ __/ _` | | | | | |/ / 
    //| |__| (_| | ||  __/ | | |          \ \_/ /  __/ |  | |_| | (_| (_| | | | |_| |_|  
    //\____/\__,_|\__\___|_| |_|           \___/ \___|_|   \__|_|\___\__,_|_|_|\__, (_)  
    //                                                                          __/ |    
    //                                                                         |___/    
    //------------------------------------------------------------------------------------------ 
    // can be eaten by a queen or rook? (vertical)
    //vertical down to up
    for(int i = r - 1; i >= 0; --i){
        if(board[i][c].c == color && (board[i][c].n == "queen" || board[i][c].n == "rook")){
            return true;
        }else if(board[i][c].n != "space"){
            break;
        }
    }
    //vertical up to down
    for(int i = r + 1; i <= 8; ++i){
        if(board[i][c].c == color  && (board[i][c].n == "queen" || board[i][c].n == "rook")){
            return true;
        }else if(board[i][c].n != "space"){
            break;
        }
    }

    // _____      _                       ______ _                               _ _      ___  
    //|  ___|    | |                      |  _  (_)                             | | |    |__ 
    //| |__  __ _| |_ ___ _ __    ______  | | | |_  __ _  __ _  ___  _ __   __ _| | |_   _  ) |
    //|  __|/ _` | __/ _ \ '_ \  |______| | | | | |/ _` |/ _` |/ _ \| '_ \ / _` | | | | | |/ / 
    //| |__| (_| | ||  __/ | | |          | |/ /| | (_| | (_| | (_) | | | | (_| | | | |_| |_|  
    //\____/\__,_|\__\___|_| |_|          |___/ |_|\__,_|\__, |\___/|_| |_|\__,_|_|_|\__, (_)  
    //                                                    __/ |                       __/ |    
    //                                                   |___/                       |___/      
    //------------------------------------------------------------------------------------------
    // can be eaten by a queen or bishop? (diagonal)
    // m = 1/1, UP
    for(int i=r-1, j=c+1; i >= 0 && j <= 8; --i, ++j){
        if(board[i][j].c == color && (board[i][j].n == "queen" ||  board[i][j].n == "bishop")){
            return true;
        }else if(board[i][j].n != "space"){
            break;
        }
    }
    // m = -1/-1, DOWN
    for(int i=r+1, j=c-1; i <= 8 && j >= 0; ++i, --j){
        if(board[i][j].c == color && (board[i][j].n == "queen" ||  board[i][j].n == "bishop")){
            return true;
        }else if(board[i][j].n != "space"){
            break;
        }
    }
    // m = -1/1, UP
    for(int i=r-1, j=c-1; i >= 0 && j >= 0; --i, --j){
        if(board[i][j].c == color && (board[i][j].n == "queen" ||  board[i][j].n == "bishop")){
            return true;
        }else if(board[i][j].n != "space"){
            break;
        }
    }
    // m = 1/-1, DOWN
    for(int i=r+1, j=c+1; i <= 8 && j <= 8; ++i, ++j){
        if(board[i][j].c == color && (board[i][j].n == "queen" ||  board[i][j].n == "bishop")){
            return true;
        }else if(board[i][j].n != "space"){
            break;
        }
    }
    
    //------------------------------------------------------------------------------------------

    return false;

}