#include <iostream>
#include <chrono>
#include <thread>

#include "chess_engine.h"
#include "display.h"
#include "input.h"

#ifdef _WIN32
    bool _win_init = init_term();
#endif

using std::cout;

static void print_title() {
    printf("\x1b[38;5;21m");
    cout << " ██████╗██╗  ██╗███████╗███████╗███████╗    \n";
    cout << "██╔════╝██║  ██║██╔════╝██╔════╝██╔════╝    \n";
    cout << "██║     ███████║█████╗  ███████╗███████╗    \n";
    cout << "██║     ██╔══██║██╔══╝  ╚════██║╚════██║    \n";
    cout << "╚██████╗██║  ██║███████╗███████║███████║    \n";
    cout << " ╚═════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝    \n";
    printf("\x1b[0m");

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    cout << ".\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    cout << "..\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    cout << "...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    cout << "....\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    cout << ".....\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    printf("\033[2J");
}

int main() {
    print_title();

    GameState gs = make_initial_state();
    std::string end_reason;
    char winner = 's'; // 's' = no winner (draw)

    while (true) {
        // Clear screen and show board
        printf("\033[2J");

        // Show board from current player's perspective
        char viewing_as = gs.turn;
        print_board(gs.board, viewing_as);

        // Show CHECK if current player is in check
        if (is_in_check(gs.board, gs.turn)) {
            print_check();
        }

        cout << "\n";
        if (gs.turn == 'w') cout << "WHITE's turn\n";
        else                 cout << "BLACK's turn\n";

        // Get and apply move
        bool move_made = false;
        while (!move_made) {
            UserMove um = get_user_move(gs, viewing_as);
            MoveOutcome out = try_move(gs, um.r1, um.c1, um.r2, um.c2, um.promotion);

            if (!out.valid) {
                cout << "INVALID: " << out.error << "\n";
                cout << "Try again.\n";
                continue;
            }

            move_made = true;

            // Show board after move
            printf("\033[2J");
            // gs.turn has already been switched in try_move; show board from the new player's view
            // but for the post-move display, show from the PREVIOUS player's perspective briefly
            print_board(gs.board, (gs.turn == 'w') ? 'b' : 'w');

            // Handle game endings
            if (out.is_checkmate) {
                print_checkmate();
                // The player who just moved wins; gs.turn now points to the LOSING side
                winner = (gs.turn == 'w') ? 'b' : 'w';
                end_reason = "checkmate";
                goto game_over;
            }
            if (out.is_stalemate) {
                end_reason = "stalemate";
                goto game_over;
            }
            if (out.is_draw_fifty) {
                end_reason = "50-move rule";
                goto game_over;
            }
            if (out.is_draw_repetition) {
                end_reason = "threefold repetition";
                goto game_over;
            }
            if (out.is_draw_material) {
                end_reason = "insufficient material";
                goto game_over;
            }

            if (out.gives_check) {
                print_check();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        cout << "\x1b[1m\n\n\nFLIPPING BOARD FOR NEXT PLAYER...\n\x1b[0m";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

game_over:
    cout << "\n\nGAME OVER — " << end_reason << "\n\n";
    if (winner == 'w')      cout << "WHITE WINS!\n";
    else if (winner == 'b') cout << "BLACK WINS!\n";
    else                    cout << "DRAW!\n";
    cout << "THANKS FOR PLAYING\n";
    return 0;
}
