
//-------------------------------------------------------------------------------

//Libraries 
#include <iostream>  //i/o
#include <vector>    //for board
#include <algorithm> //vector functions 

//-------------------------------------------------------------------------------

using std::vector;
using std::string;
using std::cin;
using std::cout;

//-------------------------------------------------------------------------------
/*
namespace pieces {
    char const* w_king   = "\u2654";
    char const* w_queen  = "\u2655";
    char const* w_rook   = "\u2656";
    char const* w_bishop = "\u2657";
    char const* w_knight = "\u2658";
    char const* w_pawn   = "\u2659";
    char const* b_king   = "\u265A";
    char const* b_queen  = "\u265B";
    char const* b_rook   = "\u265C";
    char const* b_bishop = "\u265D";
    char const* b_knight = "\u265E";
    char const* b_pawn   = "\u265F";
}
*/
//-------------------------------------------------------------------------------

// Struct for each piece on a chessboard 
struct piece {
    piece(string const& name, char color)
      : n{name}
      , c{color}
    {
        if(c == 'w'){ //Go through white pieces
            if(n == "king"       ) u = "\u2654";
            else if(n == "queen" ) u = "\u2655";
            else if(n == "rook"  ) u = "\u2656";
            else if(n == "bishop") u = "\u2657";
            else if(n == "knight") u = "\u2658";
            else if(n == "pawn"  ) u = "\u2659";

        }else if(c == 'b'){ //Go through black pieces 
            if(n == "king")        u = "\u265A";
            else if(n == "queen" ) u = "\u265B";
            else if(n == "rook"  ) u = "\u265C";
            else if(n == "bishop") u = "\u265D";
            else if(n == "knight") u = "\u265E";
            else if(n == "pawn"  ) u = "\u265F";

        }else{ //Space
            u = " ";
        }
    }

    string n;      //n = name
    char c;        //c = color
    char const* u; //u = unicode 
};

//-------------------------------------------------------------------------------

// All functions created: 
bool is_king_eaten();
void print(vector<vector<piece>> b);
void flip_board(vector<vector<piece>> b);
void get_start_piece(vector<vector<piece>> b, char color, int*r1, int*c1);
void get_end_piece(vector<vector<piece>> b, char color, int*r1, int*c1, int*r2, int*c2);
bool pawn_move(vector<vector<piece>>* b, int r1, int c1, int r2, int c2);
bool rook_move(vector<vector<piece>>* b, int r1, int c1, int r2, int c2);
bool bishop_move(vector<vector<piece>>* b, int r1, int c1, int r2, int c2);
bool knight_move(vector<vector<piece>>* b, int r1, int c1, int r2, int c2);
bool queen_move(vector<vector<piece>>* b, int r1, int c1, int r2, int c2);
bool king_move(vector<vector<piece>>* b, int r1, int c1, int r2, int c2);
bool youre_in_my_way(vector<vector<piece>>  b, char direction, int*r1, int* c1, int* r2, int* c2);

//-------------------------------------------------------------------------------

bool king_eaten = false;

//-------------------------------------------------------------------------------

//This function checks whether the king was eaten by a piece
//Special case - only happens after check if player doesnt "un-check" themselves
bool is_king_eaten(){
    if(king_eaten) return true;
    return false;
}

//-------------------------------------------------------------------------------

//This function prints the chess board sent to it 
void print(vector<vector<piece>> b ){

    //Numbers at the top
    printf("\x1b[1;48;5;110m   ");
    for(int i = 1; i<=8; ++i){
        cout<<" "<<i<<"  ";
    }

    printf("  \x1b[0m\n\x1b[1;48;5;110m  \x1b[48;5;231m");
    cout<<"┌───┬───┬───┬───┬───┬───┬───┬───┐";
    printf("\x1b[1;48;5;110m  \x1b[0m\n");

    for(int r = 0; r<8; ++r){
        
        //Numbers at side
        printf("\x1b[1;48;5;110m%d \x1b[0m\x1b[48;5;231m|\x1b[0m", r+1);

        for(int c = 0; c<8; ++c){

            if(b[0][0].c == 'b'){
                if((r+c) % 2 == 0){ //white cell
                    printf("\x1b[1;48;5;224m %s \x1b[0m\x1b[48;5;231m|\x1b[0m", b[r][c].u); 
                }else{ //black cell
                    printf("\x1b[1;48;5;39m %s \x1b[0m\x1b[48;5;231m|\x1b[0m", b[r][c].u); 
                }
            }else{
                if((r+c) % 2 == 0){ //black
                    printf("\x1b[1;48;5;39m %s \x1b[0m\x1b[48;5;231m|\x1b[0m", b[r][c].u); 
                }else{ //white cell
                    printf("\x1b[1;48;5;224m %s \x1b[0m\x1b[48;5;231m|\x1b[0m", b[r][c].u); 
                }
            }
            
            if(c == 7){ //if last col
                printf("\x1b[1;48;5;110m  \x1b[0m");
            }
        }

        if(r != 7){
            printf("\n\x1b[1;48;5;110m  \x1b[48;5;231m");
            cout<<"├───┼───┼───┼───┼───┼───┼───┼───┤";
            printf("\x1b[0m\x1b[1;48;5;110m  \x1b[0m\n");
        }else{
            printf("\n\x1b[1;48;5;110m  \x1b[48;5;231m");
            cout<<"└───┴───┴───┴───┴───┴───┴───┴───┘";
            printf("\x1b[0m\x1b[1;48;5;110m  \x1b[0m\n");
        }
    }
    //End
    printf("\x1b[1;48;5;110m                                     \x1b[0m\n");
}

//-------------------------------------------------------------------------------

//This function flips the board for the next player
void flip_board(vector<vector<piece>>*b){
    std::reverse((*b).begin(), (*b).end());
}

//-------------------------------------------------------------------------------

//selects piece to move and does error checks 
void get_start_piece(vector<vector<piece>>b, char color, int *r1, int* c1){

    bool ok = false; //is the input good or bad 

    while(!ok){
        cout<<"SELECT PIECE TO MOVE:\n";
        cout<<"Row: ";
        cin>>*r1;
        cout<<"Col: ";
        cin>>*c1;

        --*r1, --*c1; //since arr starts at 0

        //CHECK INPUT: 
        //Are positions negative?
        if(*r1 < 0 || *c1 < 0){
			cout<<"BAD INPUT - negative positions. Enter again: \n";
			continue;
		}
        //Entering a non existent cell? 
		if(*r1 > 7 || *c1 > 7){
			cout<<"BAD INPUT - out of board bounds. Enter again: \n";
			continue;
		}
        //Selecting an empty square instead of a piece? 
		if(b[*r1][*c1].c == 's'){
			cout<<"BAD INPUT - not a piece. Enter again: \n";
            continue;
		}
        //Selecting the opposite color?
        if(b[*r1][*c1].c != color){
            cout<<"BAD INPUT - you can only select a ";
            if(color == 'w') cout<<"white ";
            else cout<<"black ";
            cout<<"piece. Enter again: \n";
            continue;
        }

        //If passed all previous checks set this to true
        ok = true;
    }
}

//-------------------------------------------------------------------------------

//selects piece to move to and does error checks
void get_end_piece(vector<vector<piece>>b, char color, int* r1, int* c1, int *r2, int* c2){

    bool ok = false;

    while(!ok){

		cout<<"SELECT SQUARE TO MOVE TO:\n";
        cout<<"Row: ";
		cin>>*r2;
        cout<<"Col: ";
        cin>>*c2;

		--*r2; --*c2; //Since arr starts at 0

        //CHECKS:
        //Negative positions?
		if(*r2 < 0 || *c2 < 0){
			cout<<"BAD INPUT - negative positions. RE-ENTER YOUR MOVE COMPLETELY \n";
            get_start_piece(b, color, r1, c1);
			continue;
		}
        //Entering into a non existent cell?
		if(*r2> 7 || *c2 > 7){
			cout<<"BAD INPUT - out of board bounds. RE-ENTER YOUR MOVE COMPLETELY. \n";
            get_start_piece(b, color, r1, c1);
			continue;
		}
        //Cant select the same square
		if(*r1 == *r2 && *c1 == *c2){
			cout<<"BAD INPUT - has to be different from selected position. RE-ENTER YOUR MOVE COMPLETELY. \n";
            get_start_piece(b, color, r1, r1);
			continue;
		}
		//Check if youre trying to go to a pos where its your own piece (you cant eat it since its your own team)
        if( (b[*r1][*c1].c == 'w' && b[*r2][*c2].c == 'w') ||
            (b[*r1][*c1].c == 'b' && b[*r2][*c2].c == 'b') ){
            cout<<"BAD INPUT - you can't move on top of your own piece. RE-ENTER YOUR MOVE COMPLETELY\n";
            get_start_piece(b, color, r1, c1);
            continue;
        }

        //if passed all check
        ok = true;
    }
	
}

//-------------------------------------------------------------------------------

//Does everything for a pawn piece
bool pawn_move(vector<vector<piece>>*b, int r1, int c1, int r2, int c2){

    piece p = (*b)[r1][c1]; //p = piece
    piece d = (*b)[r2][c2]; //d = destinatiton
    piece s = {"space", 's'}; //s = space

    //Moving into new cell
    if(d.n == "space"){

        // a) Check if at starting pos (can move 1 or 2 forward)
        if(r1 == 6 && (r1 - r2 == 1 || r1 - r2 == 2) && c1 == c2){
            (*b)[r2][c2] = p;
            (*b)[r1][c1] = s;
            return true;

        // b) Check if not starting pos or starting pos but user error
        }else{   
            // i. Can't move backwards
            if(r2 > r1){
                cout<<"BAD INPUT - Pawns can't move backwards. RE-ENTER YOUR MOVE COMPLETELY\n"; //error msg
                return false;
            }
            // ii. Can't move anything other than up 1
            else if(r1 - r2 != 1 || c1 != c2){
                cout<<"BAD INPUT - Pawns cannot move like that. RE-ENTER YOUR MOVE COMPLETELY\n"; //error msg
                return false;
            }
            //iii. No errors, so move 
            else { 
                std::swap((*b)[r1][c1], (*b)[r2][c2]);
                //Check if pawn turns into queen by going to end of board - SPECIAL CASE
                if(r2 == 0){ 
                    if (p.c == 'w'){
                        (*b)[r2][c2] = {"queen", 'w'};
                    }else{
                        (*b)[r2][c2] = {"queen", 'b'};
                    }
                }

                return true;
            }
        }
    
    //Eating another piece
    }else{

        // Has to be diagonal and forward
        if(r1 == r2 + 1 && (c1 == c2 - 1 || c1 == c2 + 1)){

            if((*b)[r1][c1].c != (*b)[r2][c2].c){ //make sure you're eating a different color piece
                if((*b)[r2][c2].n == "king") king_eaten = true;
                (*b)[r1][c1] = s;
                (*b)[r2][c2] = p;
                return true;
            }else{
                cout<<"BAD INPUT - You can't eat your own piece. RE-ENTER YOUR MOVE COMPLETELY\n";
                return false;
            }
            
        }else{
            cout<<"BAD INPUT - You can't eat a piece like that. RE-ENTER YOUR MOVE COMPLETELY\n";
            return false;
        }

    }

}

//-------------------------------------------------------------------------------

//Does everything for a rook piece
bool rook_move(vector<vector<piece>>*b, int r1, int c1, int r2, int c2){

    piece p = (*b)[r1][c1]; //p = piece
    piece d = (*b)[r2][c2]; //d = destinatiton
    piece s = {"space", 's'}; //s = space

    //vertical or horizontal only 
    if(r1 != r2 && c1 != c2){
        cout<<"BAD INPUT - You can only go horizontally or vertically. RE-ENTER YOUR MOVE COMPLETELY\n";
        return false;
    }
    
    // 1) Check if stop earlier needed
    bool ok; 
    ok = youre_in_my_way(*b, 'v', &r1, &c1, &r2, &c2);
    if(!ok) return false;
    ok = youre_in_my_way(*b, 'h', &r1, &c1, &r2, &c2);
    if(!ok) return false;
    
    // 2) Not eating only moving
    if((*b)[r2][c2].c == 's'){
        std::swap((*b)[r1][c1], (*b)[r2][c2]);
        return true;
    }

    // 3) Eating
    else{		
        if((*b)[r1][c1].c != (*b)[r2][c2].c){ //make sure you're eating a different color piece
            if((*b)[r2][c2].n == "king") king_eaten = true;
            (*b)[r1][c1] = s;
            (*b)[r2][c2] = p;
            return true;
        }else{
            cout<<"BAD INPUT - You can't eat your own piece. RE-ENTER YOUR MOVE COMPLETELY\n";
            return false;
        }
    }
    
}

//-------------------------------------------------------------------------------

//Does everything for a bishop piece
bool bishop_move(vector<vector<piece>>*b, int r1, int c1, int r2, int c2){

    piece p = (*b)[r1][c1]; //p = piece
    piece d = (*b)[r2][c2]; //d = destinatiton
    piece s = {"space", 's'}; //s = space

    //Check that move is diagonal (Rise/Run has to be 1) 
    // m = (y2-y1)/(x2-x1) - it doesnt matter y over x or x over y since it has to be one
    double slope;
    if(c2-c1 == 0){ //cant divide by zero
        slope = 2; //something other than 1 or -1
    }else{
        slope = (r2 - r1)/(c2 - c1);
    }

    if(!(slope == 1 || slope == -1)){
        cout<<"BAD INPUT - Move is not diagonal. RE-ENTER YOUR MOVE COMPLETELY\n";
        return false;
    }

    // 1) Check if stop earlier needed
    bool ok; 
    ok = youre_in_my_way(*b, 'd', &r1, &c1, &r2, &c2);
    if(!ok) return false;

    // 2) Not eating just moving 
    if((*b)[r2][c2].c == 's'){
        std::swap((*b)[r1][c1], (*b)[r2][c2]); // Move
        return true;
    }

    // 3) Eating
    else{
        if((*b)[r1][c1].c != (*b)[r2][c2].c){ //make sure you're eating a different color piece
            if((*b)[r2][c2].n == "king") king_eaten = true;
            (*b)[r1][c1] = s;
            (*b)[r2][c2] = p;
            return true;
        }else{
            cout<<"BAD INPUT - You can't eat your own piece. RE-ENTER YOUR MOVE COMPLETELY\n";
            return false;
        }
    }
    
}

//-------------------------------------------------------------------------------

//Does everything for a knight piece
bool knight_move(vector<vector<piece>>*b, int r1, int c1, int r2, int c2){

    piece p = (*b)[r1][c1]; //p = piece
    piece d = (*b)[r2][c2]; //d = destinatiton
    piece s = {"space", 's'}; //s = space

    bool move_is_an_l = false;

    if(r2 == r1 - 1){
        if(c2 == c1 - 2 || c2 == c1 + 2){
            move_is_an_l = true;
        }
    }else if(r2 == r1 - 2){
        if(c2 == c1 - 1 || c2 == c1 + 1){
            move_is_an_l = true;
        }
    }else if(r2 == r1 + 1){
        if(c2 == c1 - 2 || c2 == c1 + 2){
            move_is_an_l = true;
        }
    }else if(r2 == r1 + 2){
        if(c2 == c1 - 1 || c2 == c1 + 1){
            move_is_an_l = true;
        }
    }

    if(!move_is_an_l){
        cout<<"BAD INPUT - Horses move in Ls. RE-ENTER YOUR MOVE COMPLETELY\n";
        return false;
    }

    // 1) Not eating 
    if((*b)[r2][c2].c == 's'){
        std::swap((*b)[r1][c1], (*b)[r2][c2]); 
        return true;
    }

    // 2) Eating
    else{	
        if((*b)[r1][c1].c != (*b)[r2][c2].c){ //make sure you're eating a different color piece
            if((*b)[r2][c2].n == "king") king_eaten = true;
            (*b)[r1][c1] = s;
            (*b)[r2][c2] = p;
            return true;
        }else{
            cout<<"BAD INPUT - You can't eat your own piece. RE-ENTER YOUR MOVE COMPLETELY\n";
            return false;
        }
    }
    
}

//-------------------------------------------------------------------------------

//Does everything for a queen piece
bool queen_move(vector<vector<piece>>*b, int r1, int c1, int r2, int c2){

    piece p = (*b)[r1][c1]; //p = piece
    piece d = (*b)[r2][c2]; //d = destinatiton
    piece s = {"space", 's'}; //s = space

    double slope;
    if(c2-c1 == 0){ //cant divide by zero
        slope = 2; //something other than 1 or -1
    }else{
        slope = (r2 - r1)/(c2 - c1);
    }

    //vertical or horizontal or diagonal
    if(!( r1 == r2 || c1 == c2 || slope == 1 || slope == -1)){
        cout<<"BAD INPUT - Can only be horizontal, vertical and diagonal. RE-ENTER YOUR MOVE COMPLETELY\n";
        return false;
    }

    // 1) Check if stop earlier needed
    bool ok; 
    ok = youre_in_my_way(*b, 'v', &r1, &c1, &r2, &c2);
    if(!ok) return false;
    ok = youre_in_my_way(*b, 'h', &r1, &c1, &r2, &c2);
    if(!ok) return false;
    ok = youre_in_my_way(*b, 'd', &r1, &c1, &r2, &c2);
    if(!ok) return false;

    // 1) Not eating 
    if((*b)[r2][c2].c == 's'){
        std::swap((*b)[r1][c1], (*b)[r2][c2]); 
        return true;
    }

    // 2) Eating
    else{	
        if((*b)[r1][c1].c != (*b)[r2][c2].c){ //make sure you're eating a different color piece
            if((*b)[r2][c2].n == "king") king_eaten = true;
            (*b)[r1][c1] = s;
            (*b)[r2][c2] = p;
            return true;
        }else{
            cout<<"BAD INPUT - You can't eat your own piece. RE-ENTER YOUR MOVE COMPLETELY\n";
            return false;
        }
    }

}

//-------------------------------------------------------------------------------

//Does everything for a king piece
bool king_move(vector<vector<piece>>*b, int r1, int c1, int r2, int c2){

    piece p = (*b)[r1][c1]; //p = piece
    piece d = (*b)[r2][c2]; //d = destinatiton
    piece s = {"space", 's'}; //s = space

    bool one_space = false;

    //can only move one 
    if (((r2 == r1) && (c2 == c1-1 || c2 == c1+1))
    ||((r2 == r1-1) && (c2 == c1 || c2 == c1-1 || c2 == c1+1))
    ||((r2 == r1+1) && (c2 == c1 || c2 == c1-1 || c2 == c1+1))){
        one_space = true;
    }

    if(!one_space){
        cout<<"BAD INPUT - You can only move one cell with king. RE-ENTER YOUR MOVE COMPLETELY\n";
        return false;
    }

    // 1) Not eating 
    if((*b)[r2][c2].c == 's'){
        std::swap((*b)[r1][c1], (*b)[r2][c2]); 
        return true;
    }

    // 2) Eating
    else{	
        if((*b)[r1][c1].c != (*b)[r2][c2].c){ //make sure you're eating a different color piece
            if((*b)[r2][c2].n == "king") king_eaten = true;
            (*b)[r1][c1] = s;
            (*b)[r2][c2] = p;
            return true;
        }else{
            cout<<"BAD INPUT - You can't eat your own piece. RE-ENTER YOUR MOVE COMPLETELY\n";
            return false;
        }
    }

}

//-------------------------------------------------------------------------------

// Checks whether or not there are pieces in the way mid move 
bool youre_in_my_way(vector<vector<piece>> b, char direction, int* r1, int* c1, int* r2, int* c2){

	//CHECKS IF WHEN GOING FROM [r1][c1] TO [r2][c2] YOU RUN INTO OTHER EATABLE PIECES
	//IF YES YOU HAVE TO STOP THERE INSTEAD SO r2 AND c2 CHANGES
	
    piece p = b[*r1][*c1];
    
    //cout<<"HORIZONTAL? "<<bool(direction == 'h' && *r1 == *r2)<<"\n";
    //cout<<"VERTICAL? "<<bool(direction == 'v' && *c1 == *c2)<<"\n";
    //cout<<"DIAGONAL? "<<bool(direction == 'd' && *r1 != *r2 && *c1 != *c2)<<"\n";

	// HORIZONTAL
	if(direction == 'h' && *r1 == *r2){

		//a) Left to right
		if(*c1 < *c2){
			for(int c = *c1 + 1; c - 1 < *c2; ++c){
				if(b[*r1][c].c != 's' && b[*r1][c].c != p.c){
					*c2 = c;
					return true;
				}else if(b[*r1][c].c != 's' && b[*r1][c].c == p.c){
                    cout<<"BAD INPUT - You are running into yourself. RE-ENTER YOUR MOVE COMPLETELY\n";
					return false;
				}
			}

            return true;
		}
		//b) right to left 
		else if(*c2 < *c1){
			for(int c = *c1 - 1; c + 1 > *c2; --c){
				if(b[*r1][c].c != 's' && b[*r1][c].c != p.c){
					*c2 = c;
					return true;
				}else if(b[*r1][c].c != 's' && b[*r1][c].c == p.c){
					cout<<"BAD INPUT - You are running into yourself. RE-ENTER YOUR MOVE COMPLETELY\n";
					return false;
				}
			}
            return true;
		}
	}
	
	//--------------------------------------------------------------------------------------------
	
	// VERTICAL 
	if(direction == 'v' && *c1 == *c2){
		//a) down to up
		if(*r2 < *r1){
			for(int r = *r1 - 1; r + 1 > *r2; --r){
				if(b[r][*c1].c != 's' && b[r][*c1].c != p.c){
					*r2 = r;
					return true;
				}else if(b[r][*c1].c != 's' && b[r][*c1].c == p.c){
					cout<<"BAD INPUT - You are running into yourself. RE-ENTER YOUR MOVE COMPLETELY\n";
					return false;
				}
			}

            return true;
		}
		//b) up to down 
		else if(*r1 < *r2){
			for(int r = *r1 + 1; r - 1 < *r2; ++r){
				if(b[r][*c1].c != 's' && b[r][*c1].c != p.c){
					*r2 = r;
					return true;
				}else if(b[r][*c1].c != 's' && b[r][*c1].c == p.c){
					cout<<"BAD INPUT - You are running into yourself. RE-ENTER YOUR MOVE COMPLETELY\n";
					return false;
				}
			}
            return true;
		}
	}

	//--------------------------------------------------------------------------------------------
	// DIAGONAL 
	if(direction == 'd' && *r1 != *r2 && *c1 != *c2){

		// m = 1/1
		if(*r1 > *r2 && *c2 > *c1){ //up
			for(int r=*r1-1, c=*c1+1; r>*r2 && c<*c2; --r, ++c){
				if(b[r][c].c != 's' && b[r][c].c != p.c){
					*r2 = r;
					*c2 = c;
					return true;
				}else if(b[r][c].c != 's' && b[r][c].c == p.c){
					cout<<"BAD INPUT - You are running into yourself. RE-ENTER YOUR MOVE COMPLETELY\n";
					return false;
				}
			}

            return true;
		}
		// m = -1/-1
		if(*r2 > *r1 && *c1 > *c2){ //down
			for(int r=*r1+1, c=*c1-1; r<*r2 && c>*c2; ++r, --c){
				if(b[r][c].c != 's' && b[r][c].c != p.c){
					*r2 = r;
					*c2 = c;
					return true;
				}else if(b[r][c].c != 's' && b[r][c].c == p.c){
					cout<<"BAD INPUT - You are running into yourself. RE-ENTER YOUR MOVE COMPLETELY\n";
					return false;
				}
			}
            return true;
		}
		// m = -1/1
		if(*r1 > *r2 && *c1 > *c2){ //up 
			for(int r = *r1 - 1, c = *c1 - 1; r > *r2 && c > *c2; --r, --c){
				if(b[r][c].c != 's' && b[r][c].c != p.c){
					*r2 = r;
					*c2 = c;
					return true;
				}else if(b[r][c].c != 's' && b[r][c].c == p.c){
					cout<<"BAD INPUT - You are running into yourself. RE-ENTER YOUR MOVE COMPLETELY\n";
					return false;
				}
			}
            return true;
		}
		// m = 1/-1
		if(r2 > r1 && c2 > c1){ //down
			for(int r = *r1 + 1, c = *c1 + 1; r < *r2 && c < *c2; ++r, ++c){
				if(b[r][c].c != 's' && b[r][c].c != p.c){
					*r2 = r;
					*c2 = c;
					return true;
				}else if(b[r][c].c != 's' && b[r][c].c == p.c){
					cout<<"BAD INPUT - You are running into yourself. RE-ENTER YOUR MOVE COMPLETELY\n";
					return false;
				}	
			}
            return true;
		}	
	}
    
    return true;

}

//-------------------------------------------------------------------------------