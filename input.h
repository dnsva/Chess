#pragma once

#include "chess_engine.h"
#include <iostream>
#include <string>

// ============================================================
// input.h — User input layer
// ============================================================

struct UserMove {
    int r1, c1, r2, c2;   // 0-indexed ABSOLUTE board coords
    char promotion;         // 'q','r','b','n'
};

// Get promotion piece choice from user
inline char get_promotion_choice() {
    std::cout << "Promote pawn to? (q=Queen, r=Rook, b=Bishop, n=Knight): ";
    char ch;
    while (true) {
        std::cin >> ch;
        ch = (char)std::tolower((unsigned char)ch);
        if (ch == 'q' || ch == 'r' || ch == 'b' || ch == 'n') return ch;
        std::cout << "Invalid choice. Enter q, r, b, or n: ";
    }
}

// Transform user-entered row/col to absolute board coordinates.

inline int user_row_to_board(int R) {
    // Both orientations use the same formula.
    // Display labels the row at the BOTTOM as 1 for white (board row 7) and 8 for black
    // (board row 0). In both cases: board_row = 8 - R.
    return 8 - R;
}

inline int user_col_to_board(int C) {
    return C - 1;
}

// Prompt the user for a move and return absolute board coordinates.
inline UserMove get_user_move(const GameState& gs, char viewing_as) {
    UserMove um;
    um.promotion = 'q';

    while (true) {
        // --- Select piece ---
        int R1, C1;
        std::cout << "SELECT PIECE TO MOVE:\n";
        bool piece_ok = false;
        while (!piece_ok) {
            std::cout << "Row: "; std::cin >> R1;
            std::cout << "Col: "; std::cin >> C1;
            um.r1 = user_row_to_board(R1, viewing_as);
            um.c1 = user_col_to_board(C1);
            if (um.r1 < 0 || um.r1 > 7 || um.c1 < 0 || um.c1 > 7) {
                std::cout << "BAD INPUT - out of board bounds. Enter again:\n"; continue;
            }
            if (gs.board[um.r1][um.c1].is_space()) {
                std::cout << "BAD INPUT - not a piece. Enter again:\n"; continue;
            }
            if (gs.board[um.r1][um.c1].color != gs.turn) {
                std::cout << "BAD INPUT - not your piece. Enter again:\n"; continue;
            }
            piece_ok = true;
        }

        // --- Select destination ---
        int R2, C2;
        std::cout << "SELECT SQUARE TO MOVE TO:\n";
        bool dest_ok = false;
        while (!dest_ok) {
            std::cout << "Row: "; std::cin >> R2;
            std::cout << "Col: "; std::cin >> C2;
            um.r2 = user_row_to_board(R2, viewing_as);
            um.c2 = user_col_to_board(C2);
            if (um.r2 < 0 || um.r2 > 7 || um.c2 < 0 || um.c2 > 7) {
                std::cout << "BAD INPUT - out of board bounds. RE-ENTER YOUR MOVE COMPLETELY.\n";
                break; // restart piece selection
            }
            if (um.r1 == um.r2 && um.c1 == um.c2) {
                std::cout << "BAD INPUT - must be different from selected position. RE-ENTER YOUR MOVE COMPLETELY.\n";
                break;
            }
            if (gs.board[um.r2][um.c2].color == gs.turn) {
                std::cout << "BAD INPUT - can't move on top of your own piece. RE-ENTER YOUR MOVE COMPLETELY.\n";
                break;
            }
            dest_ok = true;
        }
        if (!dest_ok) continue;

        // Check if this would be a promotion
        const Piece& p = gs.board[um.r1][um.c1];
        bool is_prom = (p.name == "pawn") &&
                       ((p.color == 'w' && um.r2 == 0) || (p.color == 'b' && um.r2 == 7));
        if (is_prom) {
            um.promotion = get_promotion_choice();
        }

        return um;
    }
}
