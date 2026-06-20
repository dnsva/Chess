// test.cpp — Comprehensive chess engine test suite
// No external dependencies; no I/O from engine (chess_engine.h is pure logic)

#include "chess_engine.h"

#include <iostream>
#include <string>
#include <cassert>

// ============================================================
// Simple test framework
// ============================================================
static int passed = 0, failed = 0;

#define ASSERT(cond, msg) \
    do { \
        if (cond) { ++passed; } \
        else { ++failed; std::cerr << "FAIL: " << msg \
                                    << " (" << __FILE__ << ":" << __LINE__ << ")\n"; } \
    } while(0)

#define ASSERT_EQ(a, b, msg) ASSERT((a) == (b), msg)
#define ASSERT_TRUE(cond, msg)  ASSERT(cond, msg)
#define ASSERT_FALSE(cond, msg) ASSERT(!(cond), msg)

// ============================================================
// Helper: empty board
// ============================================================
static Board empty_board() {
    return Board(8, std::vector<Piece>(8, Piece("space", 's')));
}

// ============================================================
// Helper: place piece
// ============================================================
static Board& place(Board& b, int r, int c, const std::string& name, char color) {
    b[r][c] = Piece(name, color);
    return b;
}

// ============================================================
// Helper: make a GameState from a board with given turn
// ============================================================
static GameState make_gs(Board b, char turn = 'w',
                          int ep_col = -1, int ep_row = -1,
                          CastlingRights cr = {true,true,true,true}) {
    GameState gs;
    gs.board   = std::move(b);
    gs.turn    = turn;
    gs.ep_col  = ep_col;
    gs.ep_row  = ep_row;
    gs.castling = cr;
    gs.halfmove_clock  = 0;
    gs.fullmove_number = 1;
    return gs;
}

// ============================================================
// SECTION 1: Board Initialization
// ============================================================
static void test_initial_state() {
    GameState gs = make_initial_state();

    // White back rank (row 7)
    ASSERT_EQ(gs.board[7][0].name, std::string("rook"),   "w rook a1");
    ASSERT_EQ(gs.board[7][0].color, 'w',                  "w rook a1 color");
    ASSERT_EQ(gs.board[7][1].name, std::string("knight"), "w knight b1");
    ASSERT_EQ(gs.board[7][2].name, std::string("bishop"), "w bishop c1");
    ASSERT_EQ(gs.board[7][3].name, std::string("queen"),  "w queen d1");
    ASSERT_EQ(gs.board[7][4].name, std::string("king"),   "w king e1");
    ASSERT_EQ(gs.board[7][5].name, std::string("bishop"), "w bishop f1");
    ASSERT_EQ(gs.board[7][6].name, std::string("knight"), "w knight g1");
    ASSERT_EQ(gs.board[7][7].name, std::string("rook"),   "w rook h1");

    // White pawns (row 6)
    for (int c = 0; c < 8; ++c) {
        ASSERT_EQ(gs.board[6][c].name,  std::string("pawn"), "w pawn row6");
        ASSERT_EQ(gs.board[6][c].color, 'w',                 "w pawn color");
    }

    // Black pawns (row 1)
    for (int c = 0; c < 8; ++c) {
        ASSERT_EQ(gs.board[1][c].name,  std::string("pawn"), "b pawn row1");
        ASSERT_EQ(gs.board[1][c].color, 'b',                 "b pawn color");
    }

    // Black back rank (row 0)
    ASSERT_EQ(gs.board[0][0].name,  std::string("rook"),   "b rook a8");
    ASSERT_EQ(gs.board[0][4].name,  std::string("king"),   "b king e8");
    ASSERT_EQ(gs.board[0][3].name,  std::string("queen"),  "b queen d8");

    // Empty middle rows
    for (int r = 2; r <= 5; ++r)
        for (int c = 0; c < 8; ++c)
            ASSERT_EQ(gs.board[r][c].color, 's', "empty middle");

    ASSERT_EQ(gs.turn, 'w', "white moves first");
    ASSERT_TRUE(gs.castling.w_k && gs.castling.w_q &&
                gs.castling.b_k && gs.castling.b_q, "all castling rights start true");
    ASSERT_EQ(gs.ep_col, -1, "no ep at start");
    ASSERT_EQ(gs.halfmove_clock, 0, "halfmove clock 0");
    ASSERT_EQ(gs.fullmove_number, 1, "fullmove 1");
}

// ============================================================
// SECTION 2: can_attack
// ============================================================
static void test_can_attack() {
    // --- Pawn attacks ---
    {
        Board b = empty_board();
        place(b, 5, 3, "pawn", 'w'); // white pawn at (5,3) attacks (4,2) and (4,4)
        place(b, 4, 4, "king", 'b');
        ASSERT_TRUE(can_attack(b, 'w', 4, 2), "w pawn attacks (4,2)");
        ASSERT_TRUE(can_attack(b, 'w', 4, 4), "w pawn attacks (4,4)");
        ASSERT_FALSE(can_attack(b, 'w', 5, 3), "w pawn doesn't attack own square");
        ASSERT_FALSE(can_attack(b, 'w', 4, 3), "w pawn doesn't attack straight ahead");
        ASSERT_FALSE(can_attack(b, 'w', 3, 2), "w pawn doesn't attack 2 rows up");
    }
    {
        Board b = empty_board();
        place(b, 2, 3, "pawn", 'b'); // black pawn at (2,3) attacks (3,2) and (3,4)
        ASSERT_TRUE(can_attack(b, 'b', 3, 2), "b pawn attacks (3,2)");
        ASSERT_TRUE(can_attack(b, 'b', 3, 4), "b pawn attacks (3,4)");
        ASSERT_FALSE(can_attack(b, 'b', 3, 3), "b pawn doesn't attack (3,3) forward");
        ASSERT_FALSE(can_attack(b, 'b', 1, 2), "b pawn doesn't attack backward");
    }
    // Pawn at edge (corner)
    {
        Board b = empty_board();
        place(b, 6, 0, "pawn", 'w'); // white pawn at a2 (6,0) - left edge
        ASSERT_TRUE(can_attack(b, 'w', 5, 1), "w pawn edge attacks (5,1)");
        ASSERT_FALSE(can_attack(b, 'w', 5, 0), "w pawn doesn't attack (5,0)");
        // col -1 would be off board - no crash
    }

    // --- Knight attacks ---
    {
        Board b = empty_board();
        place(b, 4, 4, "knight", 'w');
        // All 8 knight squares
        int targets[8][2] = {{2,3},{2,5},{3,2},{3,6},{5,2},{5,6},{6,3},{6,5}};
        for (auto& t : targets) {
            ASSERT_TRUE(can_attack(b, 'w', t[0], t[1]),
                        "knight attacks all 8 squares from center");
        }
        ASSERT_FALSE(can_attack(b, 'w', 4, 5), "knight doesn't attack adjacent");
        ASSERT_FALSE(can_attack(b, 'w', 4, 4), "knight doesn't attack own square");
    }
    // Knight at corner (0,0)
    {
        Board b = empty_board();
        place(b, 0, 0, "knight", 'b');
        ASSERT_TRUE(can_attack(b, 'b', 2, 1), "b corner knight attacks (2,1)");
        ASSERT_TRUE(can_attack(b, 'b', 1, 2), "b corner knight attacks (1,2)");
        ASSERT_FALSE(can_attack(b, 'b', 0, 1), "corner knight no side attack");
    }

    // --- Rook attacks ---
    {
        Board b = empty_board();
        place(b, 3, 3, "rook", 'w');
        ASSERT_TRUE(can_attack(b, 'w', 3, 0), "rook attacks same row left");
        ASSERT_TRUE(can_attack(b, 'w', 3, 7), "rook attacks same row right");
        ASSERT_TRUE(can_attack(b, 'w', 0, 3), "rook attacks same col up");
        ASSERT_TRUE(can_attack(b, 'w', 7, 3), "rook attacks same col down");
        ASSERT_FALSE(can_attack(b, 'w', 2, 2), "rook doesn't attack diagonal");
    }
    // Rook blocked
    {
        Board b = empty_board();
        place(b, 3, 3, "rook",  'w');
        place(b, 3, 5, "pawn",  'b'); // blocker
        ASSERT_TRUE(can_attack(b, 'w', 3, 5), "rook attacks blocker");
        ASSERT_FALSE(can_attack(b, 'w', 3, 7), "rook blocked after enemy piece");
    }
    {
        Board b = empty_board();
        place(b, 3, 3, "rook",  'w');
        place(b, 3, 5, "pawn",  'w'); // own piece blocks further squares
        // can_attack checks "does white control this square" — rook controls (3,5) even
        // though it couldn't CAPTURE its own pawn there. For move-legality we check
        // same-color captures separately. The key semantic: squares beyond own pawn = not attacked.
        ASSERT_FALSE(can_attack(b, 'w', 3, 7), "rook blocked by own piece beyond");
    }

    // --- Bishop attacks ---
    {
        Board b = empty_board();
        place(b, 4, 4, "bishop", 'w');
        ASSERT_TRUE(can_attack(b, 'w', 1, 1), "bishop diagonal up-left");
        ASSERT_TRUE(can_attack(b, 'w', 0, 0), "bishop diagonal up-left far");
        ASSERT_TRUE(can_attack(b, 'w', 1, 7), "bishop diagonal up-right");
        ASSERT_TRUE(can_attack(b, 'w', 7, 7), "bishop diagonal down-right");
        ASSERT_TRUE(can_attack(b, 'w', 7, 1), "bishop diagonal down-left");
        ASSERT_FALSE(can_attack(b, 'w', 4, 5), "bishop doesn't attack straight");
    }
    // Bishop blocked
    {
        Board b = empty_board();
        place(b, 4, 4, "bishop", 'w');
        place(b, 2, 2, "pawn", 'b'); // blocks further diagonal
        ASSERT_TRUE(can_attack(b, 'w', 2, 2), "bishop attacks blocking piece");
        ASSERT_FALSE(can_attack(b, 'w', 0, 0), "bishop blocked past enemy");
    }

    // --- Queen attacks ---
    {
        Board b = empty_board();
        place(b, 4, 4, "queen", 'w');
        ASSERT_TRUE(can_attack(b, 'w', 4, 0), "queen horizontal");
        ASSERT_TRUE(can_attack(b, 'w', 0, 4), "queen vertical");
        ASSERT_TRUE(can_attack(b, 'w', 7, 7), "queen diagonal");
        ASSERT_TRUE(can_attack(b, 'w', 0, 0), "queen diagonal 2");
        ASSERT_FALSE(can_attack(b, 'w', 2, 3), "queen non-straight/diag");
    }

    // --- King attacks (1 step only) ---
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        static const int kd[8][2] = {{3,3},{3,4},{3,5},{4,3},{4,5},{5,3},{5,4},{5,5}};
        for (auto& d : kd)
            ASSERT_TRUE(can_attack(b, 'w', d[0], d[1]), "king attacks adjacent");
        ASSERT_FALSE(can_attack(b, 'w', 2, 4), "king doesn't attack 2 squares");
    }

    // --- Edge cases: no piece ---
    {
        Board b = empty_board();
        ASSERT_FALSE(can_attack(b, 'w', 4, 4), "no attacker on empty board");
        ASSERT_FALSE(can_attack(b, 'b', 4, 4), "no attacker on empty board b");
    }
}

// ============================================================
// SECTION 3: is_in_check
// ============================================================
static void test_is_in_check() {
    // White king in check from black rook
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 4, 0, "rook", 'b');
        ASSERT_TRUE(is_in_check(b, 'w'), "w king in check from b rook");
        ASSERT_FALSE(is_in_check(b, 'b'), "b not in check");
    }
    // White king NOT in check (rook blocked)
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 4, 0, "rook", 'b');
        place(b, 4, 2, "pawn", 'w'); // blocks
        ASSERT_FALSE(is_in_check(b, 'w'), "w king NOT in check: rook blocked");
    }
    // King in check from knight
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 2, 3, "knight", 'b'); // attacks (4,4)? dr=2,dc=1 yes
        ASSERT_TRUE(is_in_check(b, 'w'), "w king in check from b knight");
    }
    // King in check from bishop
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 2, 2, "bishop", 'b');
        ASSERT_TRUE(is_in_check(b, 'w'), "w king in check from b bishop");
    }
    // King in check from pawn
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 3, 3, "pawn", 'b'); // black pawn at (3,3) attacks (4,2) and (4,4)
        ASSERT_TRUE(is_in_check(b, 'w'), "w king in check from b pawn");
    }
    // King in check from queen
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "queen", 'b');
        ASSERT_TRUE(is_in_check(b, 'w'), "w king in check from b queen vertical");
    }
    // Not in check
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 4, 0, "rook", 'w'); // own rook, no threat
        ASSERT_FALSE(is_in_check(b, 'w'), "own rook not a threat");
    }
    // Black king in check from white queen
    {
        Board b = empty_board();
        place(b, 0, 4, "king", 'b');
        place(b, 7, 4, "queen", 'w');
        ASSERT_TRUE(is_in_check(b, 'b'), "b king in check from w queen");
    }
}

// ============================================================
// SECTION 4: Pawn moves
// ============================================================
static void test_pawn_moves() {
    // White pawn forward 1
    {
        Board b = empty_board();
        place(b, 4, 3, "pawn", 'w');
        place(b, 7, 4, "king", 'w'); // need kings for legal check
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        CastlingRights nocr = {false,false,false,false};
        gs.castling = nocr;
        auto out = try_move(gs, 4, 3, 3, 3);
        ASSERT_TRUE(out.valid, "w pawn forward 1");
    }
    // White pawn forward 2 from row 6
    {
        Board b = empty_board();
        place(b, 6, 3, "pawn", 'w');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 6, 3, 4, 3);
        ASSERT_TRUE(out.valid, "w pawn forward 2 from start");
        ASSERT_EQ(gs.ep_col, 3, "ep col set after double push");
        ASSERT_EQ(gs.ep_row, 5, "ep row set after double push (skipped row=5)");
    }
    // White pawn can NOT move 2 from non-start row
    {
        Board b = empty_board();
        place(b, 4, 3, "pawn", 'w');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 3, 2, 3);
        ASSERT_FALSE(out.valid, "w pawn can't move 2 from non-start");
    }
    // White pawn can NOT move backwards
    {
        Board b = empty_board();
        place(b, 4, 3, "pawn", 'w');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 3, 5, 3);
        ASSERT_FALSE(out.valid, "w pawn can't move backwards");
    }
    // White pawn diagonal capture only when enemy present
    {
        Board b = empty_board();
        place(b, 4, 3, "pawn", 'w');
        place(b, 3, 4, "pawn", 'b'); // enemy to capture
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 3, 3, 4);
        ASSERT_TRUE(out.valid, "w pawn captures diagonally");
        ASSERT_TRUE(out.capture, "w pawn capture flagged");
    }
    // White pawn diagonal with no enemy — invalid
    {
        Board b = empty_board();
        place(b, 4, 3, "pawn", 'w');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 3, 3, 4);
        ASSERT_FALSE(out.valid, "w pawn can't capture empty diagonal");
    }
    // Black pawn moves (symmetric)
    {
        Board b = empty_board();
        place(b, 1, 3, "pawn", 'b');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'b');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 1, 3, 3, 3);
        ASSERT_TRUE(out.valid, "b pawn forward 2 from start");
        ASSERT_EQ(gs.ep_col, 3, "b ep col");
        ASSERT_EQ(gs.ep_row, 2, "b ep row (skipped row=2)");
    }
    // Pawn promotion at row 0 (white)
    {
        Board b = empty_board();
        place(b, 1, 3, "pawn", 'w');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 1, 3, 0, 3, 'q');
        ASSERT_TRUE(out.valid, "w pawn promotes at row 0");
        ASSERT_TRUE(out.promotion, "promotion flagged");
        ASSERT_EQ(gs.board[0][3].name, std::string("queen"), "promoted to queen");
    }
    // Pawn promotion to rook
    {
        Board b = empty_board();
        place(b, 1, 3, "pawn", 'w');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        // The king at (0,4) will be in check from queen at (0,3)... let's use bishop
        auto out = try_move(gs, 1, 3, 0, 3, 'b');
        ASSERT_TRUE(out.valid, "w pawn promotes to bishop");
        ASSERT_EQ(gs.board[0][3].name, std::string("bishop"), "promoted to bishop");
    }
    // Pawn promotion to knight
    {
        Board b = empty_board();
        place(b, 1, 0, "pawn", 'w');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 1, 0, 0, 0, 'n');
        ASSERT_TRUE(out.valid, "w pawn promotes to knight");
        ASSERT_EQ(gs.board[0][0].name, std::string("knight"), "promoted to knight");
    }
    // Black pawn promotion at row 7
    {
        Board b = empty_board();
        place(b, 6, 3, "pawn", 'b');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'b');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 6, 3, 7, 3, 'r');
        ASSERT_TRUE(out.valid, "b pawn promotes at row 7");
        ASSERT_EQ(gs.board[7][3].name, std::string("rook"), "b promoted to rook");
    }
}

// ============================================================
// SECTION 5: Rook moves
// ============================================================
static void test_rook_moves() {
    {
        Board b = empty_board();
        place(b, 4, 4, "rook", 'w');
        place(b, 7, 0, "king", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};

        // Horizontal
        {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, 4, 0);
            ASSERT_TRUE(out.valid, "rook horizontal left");
        }
        // Vertical
        {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, 0, 4);
            ASSERT_TRUE(out.valid, "rook vertical up");
        }
        // Diagonal not allowed
        {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, 3, 5);
            ASSERT_FALSE(out.valid, "rook can't go diagonal");
        }
    }
    // Rook blocked by own piece
    {
        Board b = empty_board();
        place(b, 4, 4, "rook",  'w');
        place(b, 4, 2, "pawn",  'w'); // own piece at (4,2)
        place(b, 7, 0, "king", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 4, 4, 0);
        ASSERT_FALSE(out.valid, "rook blocked by own piece");
    }
    // Rook captures enemy, can't go through it
    {
        Board b = empty_board();
        place(b, 4, 4, "rook",  'w');
        place(b, 4, 2, "pawn",  'b'); // enemy at (4,2)
        place(b, 7, 0, "king", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, 4, 2);
            ASSERT_TRUE(out.valid, "rook captures enemy");
        }
        {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, 4, 0); // through enemy pawn
            ASSERT_FALSE(out.valid, "rook can't jump through enemy");
        }
    }
}

// ============================================================
// SECTION 6: Bishop moves
// ============================================================
static void test_bishop_moves() {
    {
        Board b = empty_board();
        place(b, 4, 4, "bishop", 'w');
        place(b, 7, 0, "king", 'w');
        place(b, 0, 7, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};

        // All 4 diagonals
        int targets[4][2] = {{1,1},{7,7},{1,7},{7,1}};
        for (auto& t : targets) {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, t[0], t[1]);
            ASSERT_TRUE(out.valid, "bishop diagonal");
        }
        // Non-diagonal rejected
        {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, 4, 5);
            ASSERT_FALSE(out.valid, "bishop can't go straight");
        }
    }
    // Bishop blocked
    {
        Board b = empty_board();
        place(b, 4, 4, "bishop", 'w');
        place(b, 2, 2, "pawn",   'w'); // own piece at (2,2) blocking (0,0)
        place(b, 7, 0, "king", 'w');
        place(b, 0, 7, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 4, 0, 0);
        ASSERT_FALSE(out.valid, "bishop blocked by own piece");
    }
}

// ============================================================
// SECTION 7: Knight moves
// ============================================================
static void test_knight_moves() {
    {
        Board b = empty_board();
        place(b, 4, 4, "knight", 'w');
        place(b, 7, 0, "king", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};

        int L_moves[8][2] = {{2,3},{2,5},{3,2},{3,6},{5,2},{5,6},{6,3},{6,5}};
        for (auto& m : L_moves) {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, m[0], m[1]);
            ASSERT_TRUE(out.valid, "knight L move");
        }

        // Knight jumps over pieces
        place(b, 3, 3, "pawn", 'b'); // piece in the "middle"
        place(b, 3, 5, "pawn", 'b');
        place(b, 5, 3, "pawn", 'b');
        place(b, 5, 5, "pawn", 'b');
        {
            GameState g2 = make_gs(b, 'w'); g2.castling = {false,false,false,false};
            auto out = try_move(g2, 4, 4, 2, 3);
            ASSERT_TRUE(out.valid, "knight jumps over pieces");
        }

        // Non-L move invalid
        {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, 4, 5);
            ASSERT_FALSE(out.valid, "knight can't go straight");
        }
    }
}

// ============================================================
// SECTION 8: Queen moves
// ============================================================
static void test_queen_moves() {
    {
        Board b = empty_board();
        place(b, 4, 4, "queen", 'w');
        place(b, 7, 0, "king", 'w');
        place(b, 0, 7, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};

        // Horizontal
        {GameState g2=gs; ASSERT_TRUE(try_move(g2,4,4,4,0).valid, "queen horiz left");}
        {GameState g2=gs; ASSERT_TRUE(try_move(g2,4,4,4,7).valid, "queen horiz right");}
        // Vertical
        {GameState g2=gs; ASSERT_TRUE(try_move(g2,4,4,0,4).valid, "queen vert up");}
        {GameState g2=gs; ASSERT_TRUE(try_move(g2,4,4,7,4).valid, "queen vert down");}
        // Diagonals
        {GameState g2=gs; ASSERT_TRUE(try_move(g2,4,4,1,1).valid, "queen diag");}
        {GameState g2=gs; ASSERT_TRUE(try_move(g2,4,4,7,7).valid, "queen diag 2");}
        // Invalid
        {GameState g2=gs; ASSERT_FALSE(try_move(g2,4,4,2,3).valid, "queen non-straight/diag");}
    }
}

// ============================================================
// SECTION 9: King moves
// ============================================================
static void test_king_moves() {
    // King can move 1 square in any direction
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};

        int dirs[8][2] = {{3,3},{3,4},{3,5},{4,3},{4,5},{5,3},{5,4},{5,5}};
        for (auto& d : dirs) {
            GameState g2 = gs;
            auto out = try_move(g2, 4, 4, d[0], d[1]);
            ASSERT_TRUE(out.valid, "king 1-step move");
        }
        // Can't move 2
        {
            GameState g2 = gs;
            ASSERT_FALSE(try_move(g2, 4, 4, 4, 6).valid, "king can't move 2 without castling");
        }
    }
    // King can't move into check
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 0, 3, "rook", 'b'); // controls col 3
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 4, 4, 3); // move into rook's file
        ASSERT_FALSE(out.valid, "king can't walk into check");
    }
}

// ============================================================
// SECTION 10: Castling
// ============================================================
static void test_castling() {
    // White kingside castling
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        auto out = try_move(gs, 7, 4, 7, 6);
        ASSERT_TRUE(out.valid, "w kingside castle");
        ASSERT_TRUE(out.castle, "castle flag");
        ASSERT_EQ(gs.board[7][6].name, std::string("king"), "king at g1");
        ASSERT_EQ(gs.board[7][5].name, std::string("rook"), "rook at f1");
        ASSERT_EQ(gs.board[7][7].color, 's', "h1 empty");
    }
    // White queenside castling
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 0, "rook", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        auto out = try_move(gs, 7, 4, 7, 2);
        ASSERT_TRUE(out.valid, "w queenside castle");
        ASSERT_EQ(gs.board[7][2].name, std::string("king"), "king at c1");
        ASSERT_EQ(gs.board[7][3].name, std::string("rook"), "rook at d1");
        ASSERT_EQ(gs.board[7][0].color, 's', "a1 empty");
    }
    // Black kingside castling
    {
        Board b = empty_board();
        place(b, 0, 4, "king", 'b');
        place(b, 0, 7, "rook", 'b');
        place(b, 7, 4, "king", 'w');
        GameState gs = make_gs(b, 'b');
        auto out = try_move(gs, 0, 4, 0, 6);
        ASSERT_TRUE(out.valid, "b kingside castle");
        ASSERT_EQ(gs.board[0][6].name, std::string("king"), "b king at g8");
        ASSERT_EQ(gs.board[0][5].name, std::string("rook"), "b rook at f8");
    }
    // Black queenside castling
    {
        Board b = empty_board();
        place(b, 0, 4, "king", 'b');
        place(b, 0, 0, "rook", 'b');
        place(b, 7, 4, "king", 'w');
        GameState gs = make_gs(b, 'b');
        auto out = try_move(gs, 0, 4, 0, 2);
        ASSERT_TRUE(out.valid, "b queenside castle");
        ASSERT_EQ(gs.board[0][2].name, std::string("king"), "b king at c8");
        ASSERT_EQ(gs.board[0][3].name, std::string("rook"), "b rook at d8");
    }
    // Can't castle if pieces in the way
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 7, 5, "bishop", 'w'); // blocking
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        auto out = try_move(gs, 7, 4, 7, 6);
        ASSERT_FALSE(out.valid, "can't castle with pieces in the way");
    }
    // Can't castle if king moved (rights lost)
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 0, 4, "king", 'b');
        CastlingRights cr = {false, false, false, false}; // rights already lost
        GameState gs = make_gs(b, 'w', -1, -1, cr);
        auto out = try_move(gs, 7, 4, 7, 6);
        ASSERT_FALSE(out.valid, "can't castle after king moved");
    }
    // Can't castle if in check
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 0, 4, "rook", 'b'); // gives check on col 4
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        auto out = try_move(gs, 7, 4, 7, 6);
        ASSERT_FALSE(out.valid, "can't castle while in check");
    }
    // Can't castle through check (f1 = (7,5) is attacked)
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 0, 5, "rook", 'b'); // attacks f1=(7,5)
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        auto out = try_move(gs, 7, 4, 7, 6);
        ASSERT_FALSE(out.valid, "can't castle through attacked square");
    }
    // Can't castle into check (g1 = (7,6) is attacked)
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 0, 6, "rook", 'b'); // attacks g1=(7,6)
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        auto out = try_move(gs, 7, 4, 7, 6);
        ASSERT_FALSE(out.valid, "can't castle into check");
    }
    // Castling rights revoked after king moves
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        // Move king one step
        try_move(gs, 7, 4, 7, 3); // king moves to (7,3)
        ASSERT_FALSE(gs.castling.w_k, "w_k rights revoked after king moves");
        ASSERT_FALSE(gs.castling.w_q, "w_q rights revoked after king moves");
    }
    // Castling rights revoked after rook moves
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 7, 0, "rook", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        try_move(gs, 7, 7, 7, 6); // move kingside rook
        ASSERT_FALSE(gs.castling.w_k, "w_k revoked after rook moves");
        ASSERT_TRUE(gs.castling.w_q, "w_q still valid");
    }
    // Castling rights revoked when rook captured
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 7, 0, "rook", 'w');
        place(b, 0, 4, "king", 'b');
        place(b, 5, 7, "rook", 'b'); // can capture white h1 rook
        // Let black capture white's kingside rook
        GameState gs = make_gs(b, 'b');
        try_move(gs, 5, 7, 7, 7); // black rook captures white rook at h1
        ASSERT_FALSE(gs.castling.w_k, "w_k revoked when rook captured");
        ASSERT_TRUE(gs.castling.w_q, "w_q unaffected");
    }
    // Rook not present — can't castle
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        // No rook at h1
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        auto out = try_move(gs, 7, 4, 7, 6);
        ASSERT_FALSE(out.valid, "can't castle without rook");
    }
}

// ============================================================
// SECTION 11: En passant
// ============================================================
static void test_en_passant() {
    // White captures black's double-pushed pawn
    {
        Board b = empty_board();
        place(b, 4, 4, "pawn", 'w'); // white pawn at row 4, col 4 (e5)
        place(b, 4, 5, "pawn", 'b'); // black pawn just double-pushed to row 4, col 5 (f5)
        place(b, 7, 0, "king", 'w');
        place(b, 0, 0, "king", 'b');
        // ep target: black pawn came from row 1→row 3... actually simulate properly:
        // After black double push 1→3: ep_row=2, ep_col=5
        // But here black's pawn is at row 4 col 5 (already moved, white is at row 4)
        // Actually for en passant, after black pushes from row 1 to row 3:
        // - black pawn at (3,5), ep_row=2, ep_col=5
        // - white pawn at (3,4) can capture to (2,5)
        Board b2 = empty_board();
        place(b2, 3, 4, "pawn", 'w');
        place(b2, 3, 5, "pawn", 'b');
        place(b2, 7, 0, "king", 'w');
        place(b2, 0, 0, "king", 'b');
        GameState gs = make_gs(b2, 'w', 5, 2); // ep target at (2,5)
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 3, 4, 2, 5);
        ASSERT_TRUE(out.valid, "w en passant capture");
        ASSERT_TRUE(out.en_passant, "en passant flag");
        ASSERT_EQ(gs.board[2][5].name, std::string("pawn"), "w pawn at (2,5)");
        ASSERT_EQ(gs.board[2][5].color, 'w', "w pawn color");
        ASSERT_EQ(gs.board[3][5].color, 's', "captured b pawn gone");
        ASSERT_EQ(gs.board[3][4].color, 's', "original w pawn gone");
    }
    // Black captures white's double-pushed pawn
    {
        Board b = empty_board();
        place(b, 4, 3, "pawn", 'b'); // black pawn at row 4 col 3
        place(b, 4, 2, "pawn", 'w'); // white pawn at row 4 col 2 (just double-pushed)
        place(b, 7, 0, "king", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'b', 2, 5); // ep target at (5, 2)
        gs.ep_col = 2; gs.ep_row = 5;
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 3, 5, 2);
        ASSERT_TRUE(out.valid, "b en passant capture");
        ASSERT_TRUE(out.en_passant, "b en passant flag");
        ASSERT_EQ(gs.board[5][2].color, 'b', "b pawn at ep square");
        ASSERT_EQ(gs.board[4][2].color, 's', "captured w pawn gone");
    }
    // En passant not available after delay of 1 move
    {
        Board b = empty_board();
        place(b, 3, 4, "pawn", 'w');
        place(b, 3, 5, "pawn", 'b');
        place(b, 7, 0, "king", 'w');
        place(b, 0, 7, "king", 'b');
        GameState gs = make_gs(b, 'w', 5, 2);
        gs.castling = {false,false,false,false};
        // White makes a different move (not en passant)
        try_move(gs, 3, 4, 2, 4); // white pawn moves forward normally
        // Now it's black's turn; ep should be cleared
        ASSERT_EQ(gs.ep_col, -1, "ep cleared after delay");
        // Now try en passant (black turn) — should fail since ep is gone
        // Actually gs.turn is now 'b', let's check white can't retroactively use it
        ASSERT_EQ(gs.ep_col, -1, "ep not available on next move");
    }
    // En passant clears after use
    {
        Board b = empty_board();
        place(b, 3, 4, "pawn", 'w');
        place(b, 3, 5, "pawn", 'b');
        place(b, 7, 0, "king", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w', 5, 2);
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 3, 4, 2, 5); // en passant
        ASSERT_TRUE(out.valid, "ep move valid");
        ASSERT_EQ(gs.ep_col, -1, "ep cleared after use");
    }
}

// ============================================================
// SECTION 12: Check detection
// ============================================================
static void test_check_detection() {
    // Every piece type giving check
    // Rook gives check
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 3, 4, "rook", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 3, 4, 1, 4); // rook to (1,4) gives check on (0,4)
        ASSERT_TRUE(out.valid, "rook check move");
        ASSERT_TRUE(out.gives_check, "rook gives check");
    }
    // Bishop gives check
    {
        Board b = empty_board();
        place(b, 7, 0, "king", 'w');
        place(b, 5, 5, "bishop", 'w');
        place(b, 3, 3, "king", 'b'); // bishop on diagonal to b king
        place(b, 3, 7, "pawn", 'b'); // extra piece so b has somewhere to go
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 5, 5, 4, 4); // bishop moves to (4,4) on diagonal to (3,3)
        ASSERT_TRUE(out.valid, "bishop check move");
        ASSERT_TRUE(out.gives_check, "bishop gives check");
    }
    // Knight gives check
    {
        Board b = empty_board();
        place(b, 7, 0, "king", 'w');
        // Place knight at (4,3); move to (2,4): dr=2,dc=1 — valid L
        // Knight at (2,4) attacks (0,3),(0,5),(1,2),(1,6),(3,2),(3,6),(4,3),(4,5)
        // Black king at (0,5) would be attacked by knight at (2,4)? dr=2,dc=1 YES
        place(b, 4, 3, "knight", 'w');
        place(b, 0, 5, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 3, 2, 4); // knight to (2,4): attacks (0,5)
        ASSERT_TRUE(out.valid, "knight check move valid");
        ASSERT_TRUE(out.gives_check, "knight gives check");
    }
    // Pawn gives check
    {
        Board b = empty_board();
        place(b, 7, 0, "king", 'w');
        place(b, 5, 3, "pawn", 'w'); // white pawn at (5,3)
        place(b, 3, 4, "king", 'b'); // black king at (3,4)
        // pawn at (4,3) would attack (3,2) and (3,4) — yes!
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 5, 3, 4, 3); // move pawn to (4,3) — now attacks (3,4)
        ASSERT_TRUE(out.valid, "pawn check move");
        ASSERT_TRUE(out.gives_check, "pawn gives check");
    }
    // Queen gives check
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 5, 5, "queen", 'w');
        place(b, 0, 5, "king", 'b'); // black king at (0,5), queen on same col gives check
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        // Move queen from (5,5) to (1,5): straight vertical, attacks (0,5)
        auto out = try_move(gs, 5, 5, 1, 5);
        ASSERT_TRUE(out.valid, "queen check move");
        ASSERT_TRUE(out.gives_check, "queen gives check");
    }
    // Discovered check
    {
        // White rook at (5,4), white bishop at (6,4) blocking, black king at (0,4)
        // Move bishop away → rook checks king
        Board b = empty_board();
        place(b, 7, 0, "king", 'w');
        place(b, 5, 4, "rook", 'w');
        place(b, 6, 4, "bishop", 'w'); // blocking rook's view of black king
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 6, 4, 5, 3); // move bishop out of the way (diagonal)
        ASSERT_TRUE(out.valid, "discovered check move valid");
        ASSERT_TRUE(out.gives_check, "discovered check fires");
    }
}

// ============================================================
// SECTION 13: Checkmate
// ============================================================
static void test_checkmate() {
    // Fool's mate: 1. f3 e5 2. g4 Qh4#
    {
        GameState gs = make_initial_state();
        try_move(gs, 6, 5, 5, 5); // f3: white pawn f2→f3
        try_move(gs, 1, 4, 3, 4); // e5: black pawn e7→e5
        try_move(gs, 6, 6, 4, 6); // g4: white pawn g2→g4
        auto out = try_move(gs, 0, 3, 4, 7); // Qh4#: black queen d8→h4
        ASSERT_TRUE(out.valid, "fool's mate move valid");
        ASSERT_TRUE(out.is_checkmate, "fool's mate is checkmate");
    }
    // Back-rank mate
    {
        Board b = empty_board();
        place(b, 7, 4, "king",  'w');
        place(b, 6, 3, "pawn",  'w'); // pawns blocking escape
        place(b, 6, 4, "pawn",  'w');
        place(b, 6, 5, "pawn",  'w');
        place(b, 0, 0, "rook",  'b');
        place(b, 0, 4, "king",  'b');
        GameState gs = make_gs(b, 'b');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 0, 0, 7, 0); // rook to back rank
        ASSERT_TRUE(out.valid, "back-rank mate move");
        ASSERT_TRUE(out.is_checkmate, "back-rank mate");
    }
    // Not checkmate — false positive test
    // Black rook gives check to white king, but king can escape
    {
        Board b = empty_board();
        place(b, 7, 4, "king",  'w');  // white king e1
        place(b, 2, 0, "rook",  'b'); // black rook at a6, will move to a1 (7,0)
        place(b, 0, 0, "king",  'b'); // black king a8
        GameState gs = make_gs(b, 'b');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 2, 0, 7, 0); // rook to (7,0): gives check via row 7? No — king at (7,4) not at (7,0) col.
        // Rook at (7,0) attacks row 7: (7,1),(7,2)...(7,4) — YES, gives check on row 7!
        ASSERT_TRUE(out.valid, "check but not mate: move valid");
        ASSERT_TRUE(out.gives_check, "rook gives check on row");
        ASSERT_FALSE(out.is_checkmate, "not checkmate — king can escape to row 6");
    }
    // Smothered mate
    {
        // White king at h1 (7,7), boxed in by own pieces at g1=(7,6) and g2=(6,6) and h2=(6,7).
        // Black knight at e4=(4,4) moves to f6=(6,5): dr=2,dc=1 valid L.
        // Knight at (6,5) attacks (7,7): dr=1,dc=2 YES.
        // White king escape squares: (6,6)=own pawn, (6,7)=own pawn, (7,6)=own rook — all blocked.
        // Can any white piece capture knight at (6,5)?
        //   Pawn at (6,6): captures to (5,5) or (5,7) only — cannot reach (6,5).
        //   Pawn at (6,7): captures to (5,6) only — cannot reach (6,5).
        //   Rook at (7,6): row 7 or col 6; col 6 blocked by pawn at (6,6). Row 7 doesn't reach (6,5).
        //   King at (7,7): adjacent (6,6),(6,7),(7,6) all own pieces; (6,8) off-board. Can't capture (6,5).
        // => Checkmate!
        Board b = empty_board();
        place(b, 7, 7, "king",   'w');
        place(b, 7, 6, "rook",   'w'); // g1
        place(b, 6, 7, "pawn",   'w'); // h2
        place(b, 6, 6, "pawn",   'w'); // g2
        place(b, 0, 0, "king",   'b');
        place(b, 4, 4, "knight", 'b'); // knight at e4, moves to f6=(6,5)
        GameState gs = make_gs(b, 'b');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 4, 6, 5); // knight to (6,5): checks (7,7)
        ASSERT_TRUE(out.valid, "smothered mate move valid");
        ASSERT_TRUE(out.is_checkmate, "smothered mate");
    }
    // Scholar's mate: 1.e4 e5 2.Bc4 Nc6 3.Qh5 Nf6?? 4.Qxf7#
    {
        GameState gs = make_initial_state();
        try_move(gs, 6, 4, 4, 4); // 1. e4
        try_move(gs, 1, 4, 3, 4); // 1... e5
        try_move(gs, 7, 5, 4, 2); // 2. Bc4 (bishop f1 to c4=(4,2))
        try_move(gs, 0, 1, 2, 2); // 2... Nc6 (knight b8 to c6=(2,2))
        try_move(gs, 7, 3, 3, 7); // 3. Qh5 (queen d1 to h5=(3,7))
        try_move(gs, 0, 6, 2, 5); // 3... Nf6 (knight g8 to f6=(2,5))
        auto out = try_move(gs, 3, 7, 1, 5); // 4. Qxf7# (queen h5 to f7=(1,5))
        ASSERT_TRUE(out.valid, "scholar's mate move valid");
        ASSERT_TRUE(out.is_checkmate, "scholar's mate is checkmate");
    }
}

// ============================================================
// SECTION 14: Stalemate
// ============================================================
static void test_stalemate() {
    // Classic stalemate: black king at a8=(0,0), white queen at b6=(2,1), white king at c6=(2,2)
    {
        Board b = empty_board();
        place(b, 0, 0, "king",  'b');
        place(b, 2, 1, "queen", 'w'); // queen controls b7 and a7 etc.
        place(b, 2, 2, "king",  'w');
        // Actually queen at (2,1) attacks (0,0)? dr=2,dc=1 not rook/bishop. No.
        // Let's try queen at (1,2) and king at (2,3):
        // Queen at (1,2): attacks (0,1),(0,2),(0,3) horiz; (2,2),(3,2) vert; diagonals (0,1),(2,3),(2,1),(0,3)
        // Black king at (0,0) must have no moves:
        // (0,1) attacked by queen at (1,2) diag, (1,0) attacked by queen (2 steps diag from (1,2))?
        // Let me just set up a known stalemate position and move into it.
        // Known: white Qd6, white Ka6, black Ka8 → black stalemate
        // In absolute coords: Ka6=(2,0), Qd6=(2,3), Ka8=(0,0)
        Board b2 = empty_board();
        place(b2, 0, 0, "king",  'b'); // black king a8
        place(b2, 2, 0, "king",  'w'); // white king a6
        place(b2, 2, 3, "queen", 'w'); // white queen d6
        // White queen at (2,3) controls row 2, col 3, diagonals
        // Black king at (0,0): can go to (0,1),(1,0),(1,1)
        // (0,1) attacked? queen at (2,3): diagonal? (2,3)→(1,2)→(0,1) yes!
        // (1,0) attacked? queen vert col 0? No queen at col 0. king at (2,0) attacks (1,0)? Yes king adjacent.
        // (1,1) attacked? queen diag (2,2)→(1,1)? Queen at (2,3) not at (2,2). King at (2,0) attacks? (2,0)→(1,1) no (dr=1,dc=1 yes king attacks (1,1)!)
        // So black king at (0,0) has no legal moves and is not in check: stalemate!
        GameState gs = make_gs(b2, 'b');
        gs.castling = {false,false,false,false};
        // White needs to move INTO this position — let's arrange that the position IS already stalemate
        // We simulate: it's white's turn, white moves queen to (2,3) from somewhere
        place(b2, 2, 4, "space", 's'); // remove placeholder
        place(b2, 3, 3, "queen", 'w'); // queen was at (3,3)
        b2[2][3] = Piece("space", 's');
        GameState gs2 = make_gs(b2, 'w');
        gs2.castling = {false,false,false,false};
        auto out = try_move(gs2, 3, 3, 2, 3); // queen moves to (2,3) → stalemate
        ASSERT_TRUE(out.valid, "stalemate move valid");
        ASSERT_TRUE(out.is_stalemate, "stalemate detected");
        ASSERT_FALSE(out.is_checkmate, "stalemate is not checkmate");
    }
    // Stalemate vs Checkmate distinguished — verified position
    {
        // Black King at (0,0), White Queen at (1,2), White King at (2,1).
        // Verified: black king not in check, has NO legal moves → stalemate.
        //   (0,1): attacked by queen (1,2) diagonal (-1,-1)=(0,1) YES
        //   (1,0): attacked by queen (1,2) row 1 YES; also king (2,1) adjacent YES
        //   (1,1): attacked by queen (1,2) adjacent YES; also king (2,1) adjacent YES
        // White queen moves from (3,2) to (1,2) causing stalemate.
        Board b2 = empty_board();
        place(b2, 0, 0, "king",  'b');
        place(b2, 2, 1, "king",  'w');
        place(b2, 3, 2, "queen", 'w'); // will move to (1,2)
        GameState gs = make_gs(b2, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 3, 2, 1, 2);
        ASSERT_TRUE(out.valid, "stalemate setup move valid");
        ASSERT_TRUE(out.is_stalemate, "stalemate confirmed 2");
        ASSERT_FALSE(out.is_checkmate, "not checkmate");
    }
}

// ============================================================
// SECTION 15: 50-move rule
// ============================================================
static void test_fifty_move_rule() {
    // Build a state with halfmove clock at 99; next non-pawn non-capture move fires it
    {
        Board b = empty_board();
        place(b, 7, 0, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        gs.halfmove_clock = 99;
        auto out = try_move(gs, 7, 7, 6, 7); // rook move, no pawn, no capture
        ASSERT_TRUE(out.valid, "50-move rule trigger move valid");
        ASSERT_TRUE(out.is_draw_fifty, "50-move rule fires at clock=100");
        ASSERT_EQ(gs.halfmove_clock, 100, "clock incremented to 100");
    }
    // Halfmove clock resets on pawn move
    {
        Board b = empty_board();
        place(b, 6, 0, "pawn", 'w');
        place(b, 7, 4, "king", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        gs.halfmove_clock = 50;
        auto out = try_move(gs, 6, 0, 5, 0); // pawn move
        ASSERT_EQ(gs.halfmove_clock, 0, "halfmove clock resets on pawn move");
    }
    // Halfmove clock resets on capture
    {
        Board b = empty_board();
        place(b, 7, 0, "rook", 'w');
        place(b, 7, 4, "king", 'w');
        place(b, 3, 0, "pawn", 'b'); // something to capture
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        gs.halfmove_clock = 30;
        try_move(gs, 7, 0, 3, 0); // rook captures
        ASSERT_EQ(gs.halfmove_clock, 0, "halfmove clock resets on capture");
    }
    // Clock does NOT fire at 99
    {
        Board b = empty_board();
        place(b, 7, 0, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        gs.halfmove_clock = 98;
        auto out = try_move(gs, 7, 7, 6, 7);
        ASSERT_FALSE(out.is_draw_fifty, "50-move not yet at clock=99");
    }
}

// ============================================================
// SECTION 16: Threefold repetition
// ============================================================
static void test_threefold_repetition() {
    // Move pieces back and forth to reach same position 3 times
    {
        GameState gs = make_initial_state();
        // Move white knight out and back (3 times) without pawn moves
        // Nd2-f3-d2-f3-d2-f3 — but we need same position 3 times.
        // Position includes turn, castling rights, ep.
        // Start: position stored once. After returning to same pos (white's turn), 2nd.
        // White Ng1→f3, black Ng8→f6, white Nf3→g1, black Nf6→g8: back to start (pos#2).
        // Repeat: pos#3. One more: pos 3 triggers on 3rd occurrence in history.

        // 1. Ng1-f3
        try_move(gs, 7, 6, 5, 5); // white knight g1→f3
        // 1... Ng8-f6
        try_move(gs, 0, 6, 2, 5); // black knight g8→f6
        // 2. Nf3-g1
        try_move(gs, 5, 5, 7, 6); // white knight f3→g1
        // 2... Nf6-g8
        try_move(gs, 2, 5, 0, 6); // black knight f6→g8
        // Position is identical to start now (move 2 done, back to white's turn).
        // The initial hash was stored once in make_initial_state.
        // After round trip, hash stored again = 2 occurrences.

        // 3. Ng1-f3
        try_move(gs, 7, 6, 5, 5);
        // 3... Ng8-f6
        try_move(gs, 0, 6, 2, 5);
        // 4. Nf3-g1
        try_move(gs, 5, 5, 7, 6);
        // 4... Nf6-g8
        auto out = try_move(gs, 2, 5, 0, 6); // back to start position for 3rd time
        ASSERT_TRUE(out.valid, "repetition move valid");
        ASSERT_TRUE(out.is_draw_repetition, "threefold repetition detected");
    }
    // Different castling rights = different position
    {
        // Use a simple custom board where king can move
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 7, 0, "rook", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        ASSERT_TRUE(gs.castling.w_k, "w_k initially true");
        ASSERT_TRUE(gs.castling.w_q, "w_q initially true");
        // Move king one step right (empty squares since we set up a simple board)
        try_move(gs, 7, 4, 7, 3); // king moves — loses both castling rights
        ASSERT_FALSE(gs.castling.w_k, "w_k lost after king moves");
        ASSERT_FALSE(gs.castling.w_q, "w_q lost after king moves");
        // Repetition won't fire for these positions
    }
    // Different en passant = different position
    {
        GameState gs = make_initial_state();
        try_move(gs, 6, 4, 4, 4); // e4 (double push, sets ep)
        ASSERT_EQ(gs.ep_col, 4, "ep set");
        // Next position has ep; if we returned to same board but no ep, different hash
        ASSERT_EQ(gs.ep_row, 5, "ep row set");
    }
}

// ============================================================
// SECTION 17: Insufficient material
// ============================================================
static void test_insufficient_material() {
    // K vs K
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        ASSERT_TRUE(is_insufficient_material(b), "K vs K insufficient");
    }
    // K+B vs K
    {
        Board b = empty_board();
        place(b, 7, 4, "king",   'w');
        place(b, 4, 4, "bishop", 'w');
        place(b, 0, 4, "king",   'b');
        ASSERT_TRUE(is_insufficient_material(b), "K+B vs K insufficient");
    }
    // K+N vs K
    {
        Board b = empty_board();
        place(b, 7, 4, "king",   'w');
        place(b, 4, 4, "knight", 'w');
        place(b, 0, 4, "king",   'b');
        ASSERT_TRUE(is_insufficient_material(b), "K+N vs K insufficient");
    }
    // K+B vs K+B (same square color)
    {
        Board b = empty_board();
        place(b, 7, 4, "king",   'w');
        place(b, 4, 0, "bishop", 'w'); // (4+0)%2 = 0 → dark
        place(b, 0, 4, "king",   'b');
        place(b, 3, 1, "bishop", 'b'); // (3+1)%2 = 0 → dark
        ASSERT_TRUE(is_insufficient_material(b), "K+B vs K+B same color insufficient");
    }
    // K+B vs K+B (different square color)
    {
        Board b = empty_board();
        place(b, 7, 4, "king",   'w');
        place(b, 4, 0, "bishop", 'w'); // (4+0)%2 = 0 → dark
        place(b, 0, 4, "king",   'b');
        place(b, 3, 0, "bishop", 'b'); // (3+0)%2 = 1 → light
        ASSERT_FALSE(is_insufficient_material(b), "K+B vs K+B diff color NOT insufficient");
    }
    // K+Q vs K is NOT insufficient
    {
        Board b = empty_board();
        place(b, 7, 4, "king",  'w');
        place(b, 4, 4, "queen", 'w');
        place(b, 0, 4, "king",  'b');
        ASSERT_FALSE(is_insufficient_material(b), "K+Q vs K NOT insufficient");
    }
    // K+R vs K is NOT insufficient
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 4, 4, "rook", 'w');
        place(b, 0, 4, "king", 'b');
        ASSERT_FALSE(is_insufficient_material(b), "K+R vs K NOT insufficient");
    }
    // K+P vs K is NOT insufficient
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 4, 4, "pawn", 'w');
        place(b, 0, 4, "king", 'b');
        ASSERT_FALSE(is_insufficient_material(b), "K+P vs K NOT insufficient");
    }
    // After draw by material detected in game
    {
        Board b = empty_board();
        place(b, 7, 0, "king",   'w');
        place(b, 5, 5, "bishop", 'w');
        place(b, 0, 0, "king",   'b');
        place(b, 2, 4, "bishop", 'b'); // (2+4)%2=0 same color as (5+5)%2=0
        // Black captures something leaving K+B vs K+B same color
        // Actually just test the direct function
        ASSERT_TRUE(is_insufficient_material(b), "K+B vs K+B same color via game position");
    }
}

// ============================================================
// SECTION 18: Illegal moves
// ============================================================
static void test_illegal_moves() {
    // Out of bounds
    {
        GameState gs = make_initial_state();
        auto out = try_move(gs, -1, 0, 0, 0);
        ASSERT_FALSE(out.valid, "out of bounds r1<0");
    }
    {
        GameState gs = make_initial_state();
        auto out = try_move(gs, 0, 0, 8, 0);
        ASSERT_FALSE(out.valid, "out of bounds r2>7");
    }
    // Own piece capture
    {
        Board b = empty_board();
        place(b, 7, 4, "king",  'w');
        place(b, 5, 4, "rook",  'w');
        place(b, 5, 0, "pawn",  'w'); // own piece at dest
        place(b, 0, 4, "king",  'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 5, 4, 5, 0);
        ASSERT_FALSE(out.valid, "can't capture own piece");
    }
    // King into check
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 4, 0, "rook", 'b'); // controls row 4
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 4, 4, 3); // still on row 4 = still in check
        ASSERT_FALSE(out.valid, "king can't move into check");
    }
    // Move leaves king in check (discovered attack)
    {
        Board b = empty_board();
        place(b, 7, 4, "king",  'w');
        place(b, 7, 0, "rook",  'b'); // attacks row 7
        place(b, 7, 2, "pawn",  'w'); // pawn shields king; can't move
        place(b, 0, 4, "king",  'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 7, 2, 6, 2); // pawn moves, exposes king
        ASSERT_FALSE(out.valid, "pinned pawn can't move");
    }
    // Jumping over pieces (rook)
    {
        Board b = empty_board();
        place(b, 4, 4, "rook",  'w');
        place(b, 4, 2, "pawn",  'b'); // in the way
        place(b, 7, 4, "king",  'w');
        place(b, 0, 4, "king",  'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 4, 4, 0); // try to jump over pawn
        ASSERT_FALSE(out.valid, "rook can't jump pieces");
    }
    // Pawn can't capture forward
    {
        Board b = empty_board();
        place(b, 4, 3, "pawn", 'w');
        place(b, 3, 3, "pawn", 'b'); // blocking square
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 3, 3, 3); // pawn blocked
        ASSERT_FALSE(out.valid, "pawn blocked by piece directly ahead");
    }
    // Pawn double push blocked
    {
        Board b = empty_board();
        place(b, 6, 3, "pawn", 'w');
        place(b, 5, 3, "pawn", 'b'); // blocks row 5
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 6, 3, 4, 3); // double push through blocker
        ASSERT_FALSE(out.valid, "pawn double push blocked");
    }
}

// ============================================================
// SECTION 19: Edge cases
// ============================================================
static void test_edge_cases() {
    // King at corner
    {
        Board b = empty_board();
        place(b, 0, 0, "king",  'w'); // white king at corner
        place(b, 7, 7, "king",  'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        // Can only move to (0,1),(1,0),(1,1)
        {GameState g2=gs; ASSERT_TRUE(try_move(g2,0,0,0,1).valid, "corner king move right");}
        {GameState g2=gs; ASSERT_TRUE(try_move(g2,0,0,1,0).valid, "corner king move down");}
        {GameState g2=gs; ASSERT_TRUE(try_move(g2,0,0,1,1).valid, "corner king move diag");}
        {GameState g2=gs; ASSERT_FALSE(try_move(g2,0,0,0,-1).valid, "corner king off-board");}
    }
    // Promotion with capture
    {
        Board b = empty_board();
        place(b, 1, 3, "pawn",   'w');
        place(b, 0, 4, "bishop", 'b'); // can be captured diagonally by pawn
        place(b, 0, 3, "king",   'b'); // king in the way but pawn captures bishop
        place(b, 7, 4, "king",   'w');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 1, 3, 0, 4, 'q'); // pawn captures bishop and promotes
        ASSERT_TRUE(out.valid, "promotion with capture");
        ASSERT_TRUE(out.promotion, "promotion flag");
        ASSERT_TRUE(out.capture, "capture flag");
        ASSERT_EQ(gs.board[0][4].name, std::string("queen"), "promoted to queen after capture");
    }
    // En passant + check scenario: ep capture gives check
    {
        Board b = empty_board();
        place(b, 3, 4, "pawn", 'w'); // white pawn
        place(b, 3, 5, "pawn", 'b'); // black pawn (just double-pushed)
        place(b, 0, 3, "king", 'b'); // black king at (0,3)
        place(b, 7, 4, "king", 'w');
        // After ep capture, white pawn at (2,5) attacks (1,4) and (1,6), not (0,3).
        // But removing black pawn from (3,5) reveals white rook...
        // Let's add a white rook behind:
        place(b, 5, 5, "rook", 'w'); // rook behind, would give check if pawn removed
        // Actually rook at (5,5) not in line with (0,3). Let me try:
        // rook at (3,3): after ep removes (3,5) pawn from col 5, rook at (3,3) attacks along row 3
        // but nothing at (3,3) now attacks (0,3) — different row.
        // Simple: ep capture + resulting position gives check via white rook on col 5
        place(b, 5, 5, "space", 's');
        place(b, 5, 3, "rook", 'w'); // rook attacks col 3; after ep, pawn at (2,5)
        // Hmm this is getting complex. Just test ep + pawn gives check:
        // After ep: white pawn at (2,5), attacks (1,4) and (1,6). Put b king at (1,4).
        b[0][3] = Piece("space",'s');
        b[5][3] = Piece("space",'s');
        place(b, 1, 4, "king", 'b'); // black king at (1,4)
        GameState gs = make_gs(b, 'w', 5, 2); // ep_col=5, ep_row=2
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 3, 4, 2, 5); // en passant
        ASSERT_TRUE(out.valid, "ep+check move valid");
        ASSERT_TRUE(out.en_passant, "ep flag");
        ASSERT_TRUE(out.gives_check, "ep gives check");
    }
    // Castling after rook captured
    {
        Board b = empty_board();
        place(b, 7, 4, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 7, 0, "rook", 'w');
        place(b, 0, 4, "king", 'b');
        place(b, 5, 7, "rook", 'b');
        // Black captures white h1 rook
        GameState gs = make_gs(b, 'b');
        try_move(gs, 5, 7, 7, 7);
        // Now white tries to castle kingside — should fail
        ASSERT_FALSE(gs.castling.w_k, "castling rights revoked");
        auto out = try_move(gs, 7, 4, 7, 6);
        ASSERT_FALSE(out.valid, "can't castle after rook captured");
    }
}

// ============================================================
// SECTION 20: Adversarial edge cases
// ============================================================
static void test_adversarial() {
    // En passant SELF-CHECK: capture removes the black pawn, exposing own king to rook
    {
        // White pawn at (4,3), black pawn at (4,4) (just pushed), white king at (4,0),
        // black rook at (4,7). En passant would remove (4,4), clearing the row and
        // exposing (4,0) king to the rook.
        Board b = empty_board();
        place(b, 4, 0, "king",  'w');
        place(b, 4, 3, "pawn",  'w');
        place(b, 4, 4, "pawn",  'b');
        place(b, 4, 7, "rook",  'b');
        place(b, 0, 7, "king",  'b');
        GameState gs = make_gs(b, 'w', 4, 3); // ep_col=4, ep_row=3
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 3, 3, 4); // try en passant
        ASSERT_FALSE(out.valid, "ep self-check: reveals rook attack on own king");
    }

    // Pinned piece: piece can't move because it shields own king
    {
        Board b = empty_board();
        place(b, 7, 4, "king",  'w');
        place(b, 7, 2, "rook",  'w'); // pinned by black rook on same row
        place(b, 7, 0, "rook",  'b'); // pins white rook to king
        place(b, 0, 0, "king",  'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        // White rook at (7,2) is pinned on row 7; moving it off the row exposes king
        auto out = try_move(gs, 7, 2, 5, 2); // try moving rook off the pin line
        ASSERT_FALSE(out.valid, "pinned rook cannot move off pin line");
        // But white rook CAN move ALONG the pin line (captures pinner or moves toward king)
        GameState gs2 = gs;
        auto out2 = try_move(gs2, 7, 2, 7, 0); // rook captures black rook (along row 7)
        ASSERT_TRUE(out2.valid, "pinned rook CAN capture along pin line");
    }

    // Must escape check: player in check must make a move that resolves it
    {
        Board b = empty_board();
        place(b, 7, 4, "king",  'w');
        place(b, 5, 4, "rook",  'b'); // gives check on col 4
        place(b, 0, 0, "king",  'b');
        place(b, 6, 0, "rook",  'w'); // white rook on row 6, can slide to (6,4) to block
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        // White rook moving away from col 4 to (4,0) doesn't resolve check
        auto out = try_move(gs, 6, 0, 4, 0); // white rook moves off blocking column
        ASSERT_FALSE(out.valid, "can't make non-resolving move while in check");
        // White CAN block by sliding to (6,4) — interposes between (5,4) and (7,4)
        GameState gs2 = gs;
        auto out2 = try_move(gs2, 6, 0, 6, 4); // rook slides right to (6,4), interposing
        ASSERT_TRUE(out2.valid, "blocking check is legal");
    }

    // Two kings can never be adjacent (king can't move next to other king)
    {
        Board b = empty_board();
        place(b, 4, 4, "king", 'w');
        place(b, 4, 6, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 4, 4, 5); // white king moves adjacent to black king
        ASSERT_FALSE(out.valid, "king can't move adjacent to enemy king");
    }

    // Knight gives check to king at edge (ensures no OOB)
    {
        Board b = empty_board();
        place(b, 7, 0, "king",   'w');
        place(b, 4, 4, "knight", 'b');
        place(b, 0, 7, "king",   'b');
        GameState gs = make_gs(b, 'b');
        gs.castling = {false,false,false,false};
        // Knight at (4,4) → (6,3): attacks (7,1) and (7,5) and (5,2) etc.
        // Move knight to (5,1): dr=1,dc=3? No. Let's move knight to (5,2): dr=1,dc=2: yes L.
        // Knight at (5,2) attacks (7,1) and (7,3) and (3,1) and (3,3) and (4,0) and (4,4) and (6,0) and (6,4)
        // White king at (7,0): attacked by (5,1) [dr=2,dc=1] or (6,2) [dr=1,dc=2]
        // Knight move (4,4)→(5,2): L? dr=1,dc=2 YES. Knight at (5,2) attacks (7,1) not (7,0).
        // Let me try: knight at (5,1) attacks (7,0)? dr=2,dc=1 YES! So move (4,4)→(5,2): does NOT attack king.
        // Move (4,4)→(6,1): dr=2,dc=3? NO. Try (4,4)→(6,3): dr=2,dc=1 YES.
        // Knight at (6,3) attacks (7,1),(7,5),(4,2),(4,4),(5,1),(5,5)... not (7,0).
        // Move (4,4)→(5,2) and then we need knight at (5,2) to check (7,0)? (7,0)→(5,2): dr=2,dc=2? No.
        // Hmm. Let me just put the knight at a position that directly attacks the corner king:
        Board b2 = empty_board();
        place(b2, 7, 0, "king",   'w');
        place(b2, 5, 1, "knight", 'b'); // knight at (5,1) attacks (7,0): dr=2,dc=1 YES
        place(b2, 0, 7, "king",   'b');
        ASSERT_TRUE(is_in_check(b2, 'w'), "knight at (5,1) attacks corner king (7,0)");
        // And no OOB crash
        ASSERT_FALSE(is_in_check(b2, 'b'), "black not in check");
    }

    // Stalemate position: king at corner, queen controls all escape squares
    {
        // Black king at (0,7) (h8), white queen at (1,5) (f7), white king at (2,7)
        // Black king moves: (0,6)? Queen at (1,5) attacks (0,6) diag? (1,5)→(0,6): dr=1,dc=1 YES.
        // (1,6)? King at (2,7) attacks (1,6)? dr=1,dc=1 YES. Queen also? (1,5)→(1,6) YES.
        // (1,7)? King at (2,7) attacks (1,7) YES.
        // So black has no legal moves and is not in check → stalemate.
        Board b = empty_board();
        place(b, 0, 7, "king",  'b');
        place(b, 2, 7, "king",  'w');
        place(b, 3, 5, "queen", 'w'); // will move to (1,5)
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 3, 5, 1, 5); // queen moves to (1,5) → stalemate
        ASSERT_TRUE(out.valid, "stalemate move valid (corner)");
        ASSERT_TRUE(out.is_stalemate, "corner stalemate detected");
        ASSERT_FALSE(out.is_checkmate, "corner stalemate is not checkmate");
    }

    // Pawn can't capture diagonally if the target is own piece (even if pseudo-diagonal)
    {
        Board b = empty_board();
        place(b, 4, 3, "pawn", 'w');
        place(b, 3, 4, "pawn", 'w'); // own pawn on diagonal
        place(b, 7, 4, "king", 'w');
        place(b, 0, 4, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        auto out = try_move(gs, 4, 3, 3, 4); // try to capture own piece diagonally
        ASSERT_FALSE(out.valid, "pawn can't capture own piece diagonally");
    }

    // Promotion gives check: white pawn promotes to queen, giving check to black king
    {
        Board b = empty_board();
        place(b, 1, 3, "pawn", 'w');
        place(b, 7, 0, "king", 'w');
        place(b, 0, 0, "king", 'b'); // black king on same col as promotion? No.
        // Put pawn at (1,0), king at (0,7): after pawn promotes to rook, attacks row 0 giving check
        Board b2 = empty_board();
        place(b2, 1, 0, "pawn", 'w');
        place(b2, 7, 7, "king", 'w');
        place(b2, 0, 7, "king", 'b');
        GameState gs2 = make_gs(b2, 'w');
        gs2.castling = {false,false,false,false};
        auto out2 = try_move(gs2, 1, 0, 0, 0, 'r'); // promote to rook at (0,0), rook attacks row 0
        ASSERT_TRUE(out2.valid, "promotion+check valid");
        ASSERT_TRUE(out2.promotion, "promotion flag");
        ASSERT_TRUE(out2.gives_check, "promotion gives check via rook on row 0");
    }

    // Halfmove clock doesn't reset on normal piece move
    {
        Board b = empty_board();
        place(b, 7, 0, "king", 'w');
        place(b, 7, 7, "rook", 'w');
        place(b, 0, 0, "king", 'b');
        GameState gs = make_gs(b, 'w');
        gs.castling = {false,false,false,false};
        gs.halfmove_clock = 10;
        try_move(gs, 7, 7, 6, 7); // rook move, no pawn, no capture
        ASSERT_EQ(gs.halfmove_clock, 11, "halfmove clock increments on rook move");
    }

    // Queenside castling: b-file must be clear too (col 1)
    {
        Board b = empty_board();
        place(b, 7, 4, "king",   'w');
        place(b, 7, 0, "rook",   'w');
        place(b, 7, 1, "knight", 'w'); // blocking b1
        place(b, 0, 4, "king",   'b');
        GameState gs = make_gs(b, 'w');
        auto out = try_move(gs, 7, 4, 7, 2); // queenside castle attempt
        ASSERT_FALSE(out.valid, "can't queenside castle with piece on b1");
    }

    // Checkmate: verify winner is correctly identified
    {
        // Fool's mate from test_checkmate — just verify gives_check and is_checkmate together
        GameState gs = make_initial_state();
        try_move(gs, 6, 5, 5, 5); // f3
        try_move(gs, 1, 4, 3, 4); // e5
        try_move(gs, 6, 6, 4, 6); // g4
        auto out = try_move(gs, 0, 3, 4, 7); // Qh4#
        ASSERT_TRUE(out.is_checkmate, "fool's mate checkmate");
        ASSERT_TRUE(out.gives_check, "checkmate implies gives_check");
        ASSERT_FALSE(out.is_stalemate, "checkmate is not stalemate");
    }
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cerr << "Running chess engine tests...\n\n";

    test_initial_state();
    std::cerr << "  Board initialization: OK\n";

    test_can_attack();
    std::cerr << "  can_attack: OK\n";

    test_is_in_check();
    std::cerr << "  is_in_check: OK\n";

    test_pawn_moves();
    std::cerr << "  Pawn moves: OK\n";

    test_rook_moves();
    std::cerr << "  Rook moves: OK\n";

    test_bishop_moves();
    std::cerr << "  Bishop moves: OK\n";

    test_knight_moves();
    std::cerr << "  Knight moves: OK\n";

    test_queen_moves();
    std::cerr << "  Queen moves: OK\n";

    test_king_moves();
    std::cerr << "  King moves: OK\n";

    test_castling();
    std::cerr << "  Castling: OK\n";

    test_en_passant();
    std::cerr << "  En passant: OK\n";

    test_check_detection();
    std::cerr << "  Check detection: OK\n";

    test_checkmate();
    std::cerr << "  Checkmate: OK\n";

    test_stalemate();
    std::cerr << "  Stalemate: OK\n";

    test_fifty_move_rule();
    std::cerr << "  50-move rule: OK\n";

    test_threefold_repetition();
    std::cerr << "  Threefold repetition: OK\n";

    test_insufficient_material();
    std::cerr << "  Insufficient material: OK\n";

    test_illegal_moves();
    std::cerr << "  Illegal moves: OK\n";

    test_edge_cases();
    std::cerr << "  Edge cases: OK\n";

    test_adversarial();
    std::cerr << "  Adversarial edge cases: OK\n";

    std::cerr << "\n========================================\n";
    std::cerr << "Results: " << passed << " passed, " << failed << " failed\n";
    std::cerr << "========================================\n";

    return (failed > 0) ? 1 : 0;
}
