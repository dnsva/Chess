#pragma once

#include "chess_engine.h"
#include "ANSI Windows Terminal.h"

#include <cstdio>
#include <iostream>

// ============================================================
// display.h — Terminal display layer
// ============================================================

// Return the Unicode character for a piece
inline const char* piece_unicode(const Piece& p) {
    if (p.color == 'w') {
        if (p.name == "king")   return "♔";
        if (p.name == "queen")  return "♕";
        if (p.name == "rook")   return "♖";
        if (p.name == "bishop") return "♗";
        if (p.name == "knight") return "♘";
        if (p.name == "pawn")   return "♙";
    } else if (p.color == 'b') {
        if (p.name == "king")   return "♚";
        if (p.name == "queen")  return "♛";
        if (p.name == "rook")   return "♜";
        if (p.name == "bishop") return "♝";
        if (p.name == "knight") return "♞";
        if (p.name == "pawn")   return "♟";
    }
    return " ";
}

// Print the chess board.
// viewing_as='w': white at bottom (row 7 displayed at bottom, row 0 at top)
// viewing_as='b': black at bottom (row 0 displayed at bottom, row 7 at top)
inline void print_board(const Board& b, char viewing_as) {
    // Numbers at the top (column labels 1-8)
    printf("\x1b[1;48;5;110m   ");
    for (int i = 1; i <= 8; ++i) {
        printf(" %d  ", i);
    }
    printf("  \x1b[0m\n\x1b[1;48;5;110m  \x1b[48;5;231m");
    printf("┌───┬───┬───┬───┬───┬───┬───┬───┐");
    printf("\x1b[1;48;5;110m  \x1b[0m\n");

    for (int display_r = 0; display_r < 8; ++display_r) {
        // Map display row to board row
        int board_r;
        int row_label;
        if (viewing_as == 'w') {
            // White at bottom: display row 0 = board row 0 (black back rank, label 8 from white's perspective)
            // Actually: we want board row 7 at the bottom.
            // display_r=0 → board_r=0 (top), display_r=7 → board_r=7 (bottom)
            // Row label from white's view: top = row 8, bottom = row 1
            board_r   = display_r;
            row_label = 8 - display_r; // label: top=8, bottom=1
        } else {
            // Black at bottom: display row 0 = board row 7 (white back rank)
            board_r   = 7 - display_r;
            row_label = display_r + 1; // label: top=1 (black's row 1), bottom=8
        }

        printf("\x1b[1;48;5;110m%d \x1b[0m\x1b[48;5;231m|\x1b[0m", row_label);

        for (int c = 0; c < 8; ++c) {
            const Piece& piece = b[board_r][c];
            // Square color: same formula as original
            // When white at bottom: (display_r+c)%2 matches original (r+c)%2
            bool light_square = (display_r + c) % 2 != 0;
            const char* pu = piece_unicode(piece);

            if (light_square) {
                // light square
                printf("\x1b[1;48;5;224m %s \x1b[0m\x1b[48;5;231m|\x1b[0m", pu);
            } else {
                // dark square
                printf("\x1b[1;48;5;39m %s \x1b[0m\x1b[48;5;231m|\x1b[0m", pu);
            }

            if (c == 7) {
                printf("\x1b[1;48;5;110m  \x1b[0m");
            }
        }

        if (display_r != 7) {
            printf("\n\x1b[1;48;5;110m  \x1b[48;5;231m");
            printf("├───┼───┼───┼───┼───┼───┼───┼───┤");
            printf("\x1b[0m\x1b[1;48;5;110m  \x1b[0m\n");
        } else {
            printf("\n\x1b[1;48;5;110m  \x1b[48;5;231m");
            printf("└───┴───┴───┴───┴───┴───┴───┴───┘");
            printf("\x1b[0m\x1b[1;48;5;110m  \x1b[0m\n");
        }
    }
    printf("\x1b[1;48;5;110m                                     \x1b[0m\n");
}

inline void print_check() {
    std::cout << "▄█▄     ▄  █ ▄███▄   ▄█▄    █  █▀ \n";
    std::cout << "█▀ ▀▄  █   █ █▀   ▀  █▀ ▀▄  █▄█   \n";
    std::cout << "█   ▀  ██▀▀█ ██▄▄    █   ▀  █▀▄   \n";
    std::cout << "█▄  ▄▀ █   █ █▄   ▄▀ █▄  ▄▀ █  █  \n";
    std::cout << "▀███▀     █  ▀███▀   ▀███▀    █   \n";
    std::cout << "         ▀                   ▀    \n";
}

inline void print_checkmate() {
    std::cout << " ▄████▄   ██░ ██ ▓█████  ▄████▄   ██ ▄█▀ ███▄ ▄███▓ ▄▄▄     ▄▄▄█████▓▓█████     \n";
    std::cout << "▒██▀ ▀█  ▓██░ ██▒▓█   ▀ ▒██▀ ▀█   ██▄█▒ ▓██▒▀█▀ ██▒▒████▄   ▓  ██▒ ▓▒▓█   ▀     \n";
    std::cout << "▒▓█    ▄ ▒██▀▀██░▒███   ▒▓█    ▄ ▓███▄░ ▓██    ▓██░▒██  ▀█▄ ▒ ▓██░ ▒░▒███       \n";
    std::cout << "▒▓▓▄ ▄██▒░▓█ ░██ ▒▓█  ▄ ▒▓▓▄ ▄██▒▓██ █▄ ▒██    ▒██ ░██▄▄▄▄██░ ▓██▓ ░ ▒▓█  ▄     \n";
    std::cout << "▒ ▓███▀ ░░▓█▒░██▓░▒████▒▒ ▓███▀ ░▒██▒ █▄▒██▒   ░██▒ ▓█   ▓██▒ ▒██▒ ░ ░▒████▒    \n";
    std::cout << "░ ░▒ ▒  ░ ▒ ░░▒░▒░░ ▒░ ░░ ░▒ ▒  ░▒ ▒▒ ▓▒░ ▒░   ░  ░ ▒▒   ▓▒█░ ▒ ░░   ░░ ▒░ ░    \n";
    std::cout << "  ░  ▒    ▒ ░▒░ ░ ░ ░  ░  ░  ▒   ░ ░▒ ▒░░  ░      ░  ▒   ▒▒ ░   ░     ░ ░  ░    \n";
    std::cout << "░         ░  ░░ ░   ░   ░        ░ ░░ ░ ░      ░     ░   ▒    ░         ░       \n";
    std::cout << "░ ░       ░  ░  ░   ░  ░░ ░      ░  ░          ░         ░  ░           ░  ░    \n";
    std::cout << "░                       ░                                                       \n";
}
