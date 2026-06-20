#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

// ============================================================
// chess_engine.h  — Pure game logic, zero I/O
//
// Coordinate system (ABSOLUTE):
//   Row 0 = black back rank   Row 7 = white back rank
//   Col 0 = a-file            Col 7 = h-file
//   White pawns: row 6 → row 0 (decreasing), promote at row 0
//   Black pawns: row 1 → row 7 (increasing), promote at row 7
// ============================================================

// ---- Core types ----

struct Piece {
    std::string name;   // "king","queen","rook","bishop","knight","pawn","space"
    char color;         // 'w','b','s'(space)

    Piece() : name("space"), color('s') {}
    Piece(std::string n, char c) : name(std::move(n)), color(c) {}

    bool is_space() const { return color == 's'; }
};

using Board = std::vector<std::vector<Piece>>;

struct CastlingRights {
    bool w_k = true;   // white kingside
    bool w_q = true;   // white queenside
    bool b_k = true;   // black kingside
    bool b_q = true;   // black queenside
};

struct GameState {
    Board board;
    char turn = 'w';
    CastlingRights castling;
    int ep_col  = -1;   // en-passant target column (-1 = none)
    int ep_row  = -1;   // en-passant target row
    int halfmove_clock   = 0;
    int fullmove_number  = 1;
    std::vector<std::string> position_history;
};

struct MoveOutcome {
    bool valid               = false;
    std::string error;
    bool capture             = false;
    bool promotion           = false;
    bool castle              = false;
    bool en_passant          = false;
    bool gives_check         = false;
    bool is_checkmate        = false;
    bool is_stalemate        = false;
    bool is_draw_fifty       = false;
    bool is_draw_repetition  = false;
    bool is_draw_material    = false;
};

// ============================================================
// Forward declarations
// ============================================================
static bool  can_attack(const Board&, char attacker, int r, int c);
static bool  is_in_check(const Board&, char color);
static bool  is_insufficient_material(const Board&);
static bool  has_legal_moves(const GameState&);
static MoveOutcome try_move(GameState&, int r1, int c1, int r2, int c2, char promotion = 'q');
static GameState   make_initial_state();

// ============================================================
// Internal helpers
// ============================================================
namespace detail {

inline bool in_bounds(int r, int c) {
    return r >= 0 && r <= 7 && c >= 0 && c <= 7;
}

// Serialize board state for repetition detection.
// Includes: pieces, active color, castling rights, en-passant target.
//
// Arguments:
//   b      — the board to fingerprint
//   turn   — whose turn it is after the move ('w' or 'b'); part of the
//             position identity under FIDE rules
//   cr     — current castling rights; two positions with identical pieces
//             but different castling availability are NOT the same position
//   ep_col — column of the en-passant target pawn (-1 if none)
//   ep_row — row the capturing pawn would land on (-1 if none)
inline std::string position_hash(const Board& b, char turn,
                                  const CastlingRights& cr,
                                  int ep_col, int ep_row) {
    std::string h;
    h.reserve(200);
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            h += b[r][c].name;
            h += b[r][c].color;
            h += '|';
        }
    h += turn;
    h += (cr.w_k ? '1' : '0');
    h += (cr.w_q ? '1' : '0');
    h += (cr.b_k ? '1' : '0');
    h += (cr.b_q ? '1' : '0');
    if (ep_col >= 0) {
        h += (char)('a' + ep_col);
        h += (char)('0' + ep_row);
    } else {
        h += '-';
    }
    return h;
}

// Find the king of a given color; returns false if not found.
//
// Arguments:
//   b     — board to search
//   color — which king to find ('w' or 'b')
//   kr    — output: row of the king once found
//   kc    — output: column of the king once found
inline bool find_king(const Board& b, char color, int& kr, int& kc) {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            if (b[r][c].name == "king" && b[r][c].color == color) {
                kr = r; kc = c; return true;
            }
    return false;
}

// Ray scan: determines whether square (r,c) is attacked along a single ray.
//
// Instead of asking "what can this rook reach?", we reverse the question:
// we shoot a ray outward FROM the target square (r,c) and ask "is there
// something threatening from that direction?"
//
// Parameters:
//   b        — current board
//   attacker — color of the potential attacker ('w' or 'b')
//   r, c     — the square we are testing (e.g. the king's square)
//   dr, dc   — direction to walk each step, e.g. (0,1) = rightward,
//               (-1,-1) = diagonally up-left. One of {-1,0,1} each.
//   names    — piece types that are dangerous along this particular ray
//               e.g. {"rook","queen"} for horizontal/vertical,
//                    {"bishop","queen"} for diagonals
//
// Walk step by step outward from (r,c):
//   - Empty square → keep walking (the ray continues through empty space)
//   - Any piece of WRONG color → it blocks the ray; nothing behind it can
//     attack (r,c) along this direction. Return false immediately.
//   - Piece of CORRECT color (attacker) → check if it's in the 'names' list.
//     If yes, it attacks (r,c). If no (e.g. a king one step away on a rook
//     ray), it still blocks — return false.
//   - Off the board → ray ended with no attacker found. Return false.
inline bool ray_attacked(const Board& b, char attacker,
                          int r, int c, int dr, int dc,
                          const std::vector<std::string>& names) {
    int rr = r + dr, cc = c + dc;
    while (in_bounds(rr, cc)) {
        if (!b[rr][cc].is_space()) {
            // Something is here. Does it attack (r,c)?
            if (b[rr][cc].color == attacker) {
                for (auto& n : names)
                    if (b[rr][cc].name == n) return true;
            }
            // Anything here — friendly or wrong-type enemy — blocks the ray.
            return false;
        }
        rr += dr; cc += dc;   // step further along the ray
    }
    return false;   // walked off the board without finding an attacker
}

} // namespace detail

// ============================================================
// can_attack: is square (r,c) attacked by any piece of color 'attacker'?
//
// Arguments:
//   b        — current board
//   attacker — color of the attacking side ('w' or 'b')
//   r, c     — the square being tested (e.g. the king's square)
//
// Checks every attack vector: pawns, knights, rook/queen rays,
// bishop/queen rays, and adjacent king squares.
// ============================================================
static bool can_attack(const Board& b, char attacker, int r, int c) {
    using namespace detail;

    // Pawn attacks:
    // White pawn at (pr, pc) attacks (pr-1, pc±1).
    // So if attacker==white, a white pawn attacking (r,c) is at (r+1, c±1).
    // Black pawn at (pr, pc) attacks (pr+1, pc±1).
    // So if attacker==black, a black pawn attacking (r,c) is at (r-1, c±1).
    if (attacker == 'w') {
        // white pawn would be one row below (r+1)
        for (int dc : {-1, 1}) {
            int pr = r + 1, pc = c + dc;
            if (in_bounds(pr, pc) && b[pr][pc].name == "pawn" && b[pr][pc].color == 'w')
                return true;
        }
    } else {
        // black pawn would be one row above (r-1)
        for (int dc : {-1, 1}) {
            int pr = r - 1, pc = c + dc;
            if (in_bounds(pr, pc) && b[pr][pc].name == "pawn" && b[pr][pc].color == 'b')
                return true;
        }
    }

    // Knight attacks
    static const int kd[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (auto& d : kd) {
        int nr = r + d[0], nc = c + d[1];
        if (in_bounds(nr, nc) && b[nr][nc].name == "knight" && b[nr][nc].color == attacker)
            return true;
    }

    // Rook / Queen (horizontal & vertical rays)
    static const int rook_dirs[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
    for (auto& d : rook_dirs) {
        if (ray_attacked(b, attacker, r, c, d[0], d[1], {"rook", "queen"}))
            return true;
    }

    // Bishop / Queen (diagonal rays)
    static const int diag_dirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto& d : diag_dirs) {
        if (ray_attacked(b, attacker, r, c, d[0], d[1], {"bishop", "queen"}))
            return true;
    }

    // King attacks (only 1 step)
    static const int king_dirs[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    for (auto& d : king_dirs) {
        int nr = r + d[0], nc = c + d[1];
        if (in_bounds(nr, nc) && b[nr][nc].name == "king" && b[nr][nc].color == attacker)
            return true;
    }

    return false;
}

// ============================================================
// is_in_check: is 'color's king currently in check?
//
// Arguments:
//   b     — board to examine
//   color — the side whose king we are testing ('w' or 'b')
//
// Locates the king, then delegates to can_attack with the opponent color.
// ============================================================
static bool is_in_check(const Board& b, char color) {
    int kr = -1, kc = -1;
    if (!detail::find_king(b, color, kr, kc)) return false;
    char opp = (color == 'w') ? 'b' : 'w';
    return can_attack(b, opp, kr, kc);
}

// ============================================================
// is_insufficient_material: can either side force checkmate?
//
// Arguments:
//   b — board to examine (checks the full piece set of both sides)
//
// Returns true for the four dead-draw configurations under FIDE rules:
//   K vs K, K+B vs K, K+N vs K, K+B vs K+B (bishops on same square color).
// ============================================================
static bool is_insufficient_material(const Board& b) {
    // Count pieces
    std::vector<Piece> whites, blacks;
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            if (b[r][c].color == 'w' && b[r][c].name != "king") whites.push_back(b[r][c]);
            if (b[r][c].color == 'b' && b[r][c].name != "king") blacks.push_back(b[r][c]);
        }

    // K vs K
    if (whites.empty() && blacks.empty()) return true;

    // K+B vs K or K+N vs K
    if (whites.empty() && blacks.size() == 1 &&
        (blacks[0].name == "bishop" || blacks[0].name == "knight")) return true;
    if (blacks.empty() && whites.size() == 1 &&
        (whites[0].name == "bishop" || whites[0].name == "knight")) return true;

    // K+B vs K+B (same color bishop)
    if (whites.size() == 1 && blacks.size() == 1 &&
        whites[0].name == "bishop" && blacks[0].name == "bishop") {
        // Find the bishops on the board and check square color
        int wb_r = -1, wb_c = -1, bb_r = -1, bb_c = -1;
        for (int r = 0; r < 8; ++r)
            for (int c = 0; c < 8; ++c) {
                if (b[r][c].name == "bishop" && b[r][c].color == 'w') { wb_r = r; wb_c = c; }
                if (b[r][c].name == "bishop" && b[r][c].color == 'b') { bb_r = r; bb_c = c; }
            }
        if (wb_r >= 0 && bb_r >= 0) {
            int w_sq_color = (wb_r + wb_c) % 2;
            int b_sq_color = (bb_r + bb_c) % 2;
            if (w_sq_color == b_sq_color) return true;
        }
    }

    return false;
}

// ============================================================
// Apply a pseudo-move on a board copy (no legality checks, no
// en-passant/castling side effects beyond the basic piece moves).
// Used internally to test whether a move leaves the king in check.
// ============================================================
namespace detail {

struct PseudoMoveResult {
    bool ok;
    Board board;
};

// Apply a move on a copy of the board without any legality checking.
// Handles en passant captures and the rook half of a castling move.
// Returns the new board state after the move.
//
// Arguments:
//   b         — board to copy and modify
//   r1, c1    — source square (0-indexed absolute coords)
//   r2, c2    — destination square
//   promotion — piece to promote to if a pawn reaches the back rank
//               ('q','r','b','n'); ignored for non-promotion moves
//   ep_col    — en-passant target column from GameState (-1 if none)
//   ep_row    — en-passant target row from GameState (-1 if none)
//   cr        — castling rights (unused here; rook presence checked in is_pseudo_legal)
//   is_castle — output: set to true if the move was a castling move
//   is_ep     — output: set to true if the move was an en passant capture
inline Board apply_pseudo_move(const Board& b, int r1, int c1, int r2, int c2,
                                char promotion,
                                int ep_col, int ep_row,
                                const CastlingRights& /*cr*/,
                                bool& is_castle, bool& is_ep) {
    Board nb = b;
    is_castle = false;
    is_ep = false;

    Piece p = nb[r1][c1];
    Piece space("space", 's');

    // En passant
    if (p.name == "pawn" && c2 == ep_col && r2 == ep_row && ep_col >= 0) {
        is_ep = true;
        // Remove the captured pawn
        int cap_row = (p.color == 'w') ? r2 + 1 : r2 - 1;
        nb[cap_row][c2] = space;
    }

    // Castling
    if (p.name == "king") {
        int dc = c2 - c1;
        if (dc == 2) {
            // Kingside castle: move rook from col 7 to col 5
            is_castle = true;
            nb[r1][5] = nb[r1][7];
            nb[r1][7] = space;
        } else if (dc == -2) {
            // Queenside castle: move rook from col 0 to col 3
            is_castle = true;
            nb[r1][3] = nb[r1][0];
            nb[r1][0] = space;
        }
    }

    // Move piece
    nb[r2][c2] = p;
    nb[r1][c1] = space;

    // Promotion
    if (p.name == "pawn") {
        if ((p.color == 'w' && r2 == 0) || (p.color == 'b' && r2 == 7)) {
            std::string pname;
            switch (promotion) {
                case 'r': pname = "rook";   break;
                case 'b': pname = "bishop"; break;
                case 'n': pname = "knight"; break;
                default:  pname = "queen";  break;
            }
            nb[r2][c2] = Piece(pname, p.color);
        }
    }

    return nb;
}

// Check if a move obeys the movement rules for the piece on (r1,c1).
// Does NOT test whether the move leaves the king in check — that is
// the caller's responsibility. Returns false for structurally invalid moves
// (wrong piece type motion, blocked path, moving opponent's piece, etc.).
//
// Arguments:
//   b         — current board
//   r1, c1    — square the piece is moving FROM
//   r2, c2    — square the piece is moving TO
//   color     — color of the side making the move ('w' or 'b');
//               used to determine pawn direction and castling row
//   ep_col    — column of the en-passant target (-1 if no ep available)
//   ep_row    — row the capturing pawn would land on (-1 if no ep available)
//   cr        — castling rights; checked before allowing a castle move
inline bool is_pseudo_legal(const Board& b, int r1, int c1, int r2, int c2,
                              char color, int ep_col, int ep_row,
                              const CastlingRights& cr) {
    if (!in_bounds(r1,c1) || !in_bounds(r2,c2)) return false;
    if (r1==r2 && c1==c2) return false;
    const Piece& p = b[r1][c1];
    if (p.color != color) return false;
    const Piece& dst = b[r2][c2];
    if (dst.color == color) return false; // can't capture own piece

    if (p.name == "pawn") {
        if (color == 'w') {
            // White pawn moves up (decreasing row)
            if (r2 == r1 - 1 && c2 == c1 && dst.is_space()) return true;
            if (r1 == 6 && r2 == r1 - 2 && c2 == c1 && dst.is_space() && b[r1-1][c1].is_space()) return true;
            if (r2 == r1 - 1 && std::abs(c2 - c1) == 1) {
                if (!dst.is_space() && dst.color == 'b') return true; // capture
                if (c2 == ep_col && r2 == ep_row) return true; // en passant
            }
        } else {
            // Black pawn moves down (increasing row)
            if (r2 == r1 + 1 && c2 == c1 && dst.is_space()) return true;
            if (r1 == 1 && r2 == r1 + 2 && c2 == c1 && dst.is_space() && b[r1+1][c1].is_space()) return true;
            if (r2 == r1 + 1 && std::abs(c2 - c1) == 1) {
                if (!dst.is_space() && dst.color == 'w') return true; // capture
                if (c2 == ep_col && r2 == ep_row) return true; // en passant
            }
        }
        return false;
    }

    if (p.name == "knight") {
        int dr = std::abs(r2 - r1), dc = std::abs(c2 - c1);
        return (dr == 2 && dc == 1) || (dr == 1 && dc == 2);
    }

    if (p.name == "rook") {
        if (r1 != r2 && c1 != c2) return false;
        // Check path clear
        int dr = (r2 == r1) ? 0 : (r2 > r1 ? 1 : -1);
        int dc = (c2 == c1) ? 0 : (c2 > c1 ? 1 : -1);
        int rr = r1 + dr, cc = c1 + dc;
        while (rr != r2 || cc != c2) {
            if (!b[rr][cc].is_space()) return false;
            rr += dr; cc += dc;
        }
        return true;
    }

    if (p.name == "bishop") {
        if (std::abs(r2-r1) != std::abs(c2-c1)) return false;
        int dr = (r2 > r1) ? 1 : -1;
        int dc = (c2 > c1) ? 1 : -1;
        int rr = r1 + dr, cc = c1 + dc;
        while (rr != r2 || cc != c2) {
            if (!b[rr][cc].is_space()) return false;
            rr += dr; cc += dc;
        }
        return true;
    }

    if (p.name == "queen") {
        bool straight = (r1 == r2 || c1 == c2);
        bool diagonal = (std::abs(r2-r1) == std::abs(c2-c1));
        if (!straight && !diagonal) return false;
        int dr = (r2 == r1) ? 0 : (r2 > r1 ? 1 : -1);
        int dc = (c2 == c1) ? 0 : (c2 > c1 ? 1 : -1);
        int rr = r1 + dr, cc = c1 + dc;
        while (rr != r2 || cc != c2) {
            if (!b[rr][cc].is_space()) return false;
            rr += dr; cc += dc;
        }
        return true;
    }

    if (p.name == "king") {
        int dr = std::abs(r2-r1), dc_abs = std::abs(c2-c1);
        // Normal one-step king move
        if (dr <= 1 && dc_abs <= 1) return true;
        // Castling: king moves exactly 2 columns, same row
        if (dr == 0 && dc_abs == 2) {
            char opp = (color == 'w') ? 'b' : 'w';
            int king_row = (color == 'w') ? 7 : 0;
            if (r1 != king_row || r2 != king_row) return false;
            if (c2 == c1 + 2) {
                // Kingside
                bool rights = (color == 'w') ? cr.w_k : cr.b_k;
                if (!rights) return false;
                if (!b[king_row][5].is_space() || !b[king_row][6].is_space()) return false;
                if (b[king_row][7].name != "rook" || b[king_row][7].color != color) return false;
                // King must not be in check, must not pass through check
                if (is_in_check(b, color)) return false;
                if (can_attack(b, opp, king_row, 5)) return false;
                return true;
            } else if (c2 == c1 - 2) {
                // Queenside
                bool rights = (color == 'w') ? cr.w_q : cr.b_q;
                if (!rights) return false;
                if (!b[king_row][1].is_space() || !b[king_row][2].is_space() || !b[king_row][3].is_space()) return false;
                if (b[king_row][0].name != "rook" || b[king_row][0].color != color) return false;
                if (is_in_check(b, color)) return false;
                if (can_attack(b, opp, king_row, 3)) return false;
                return true;
            }
        }
        return false;
    }

    return false;
}

} // namespace detail

// ============================================================
// has_legal_moves: does the side whose turn it is have any legal move?
//
// Arguments:
//   gs — full game state; gs.turn determines which side is tested,
//        gs.board/castling/ep_col/ep_row are passed to is_pseudo_legal
//        and apply_pseudo_move for each candidate move
//
// Used after every move to detect checkmate (no moves + in check)
// and stalemate (no moves + not in check). Iterates all 64x64 from-to
// combinations, filters by is_pseudo_legal, applies each on a temp board,
// and returns true the moment one move doesn't leave the king in check.
// ============================================================
static bool has_legal_moves(const GameState& gs) {
    char color = gs.turn;
    for (int r1 = 0; r1 < 8; ++r1) {
        for (int c1 = 0; c1 < 8; ++c1) {
            if (gs.board[r1][c1].color != color) continue;
            for (int r2 = 0; r2 < 8; ++r2) {
                for (int c2 = 0; c2 < 8; ++c2) {
                    if (!detail::is_pseudo_legal(gs.board, r1, c1, r2, c2,
                                                  color, gs.ep_col, gs.ep_row, gs.castling))
                        continue;
                    // Apply pseudo move and test for self-check
                    bool dummy1, dummy2;
                    Board nb = detail::apply_pseudo_move(gs.board, r1, c1, r2, c2,
                                                          'q', gs.ep_col, gs.ep_row,
                                                          gs.castling, dummy1, dummy2);
                    if (!is_in_check(nb, color)) return true;
                }
            }
        }
    }
    return false;
}

// ============================================================
// try_move: the main public entry point for making a move.
// Validates, applies, updates all game state, and detects game end.
//
// Arguments:
//   gs        — game state to mutate if the move is legal; untouched
//               if the move is rejected
//   r1, c1    — square the piece is moving FROM (0-indexed absolute coords)
//   r2, c2    — square the piece is moving TO
//   promotion — piece to promote to if a pawn reaches the back rank
//               ('q'=queen, 'r'=rook, 'b'=bishop, 'n'=knight);
//               ignored for any non-promotion move
//
// Returns a MoveOutcome. If out.valid == false, gs is unchanged and
// out.error describes why. If valid, gs has been fully updated:
// board, turn, castling rights, ep square, halfmove clock, fullmove
// number, and position history. out flags describe what happened
// (capture, castle, promotion, check, checkmate, stalemate, draws).
// ============================================================
static MoveOutcome try_move(GameState& gs, int r1, int c1, int r2, int c2, char promotion) {
    MoveOutcome out;
    char color = gs.turn;
    char opp   = (color == 'w') ? 'b' : 'w';

    // Basic pseudo-legality
    if (!detail::in_bounds(r1,c1) || !detail::in_bounds(r2,c2)) {
        out.error = "Out of bounds"; return out;
    }
    if (!detail::is_pseudo_legal(gs.board, r1, c1, r2, c2,
                                  color, gs.ep_col, gs.ep_row, gs.castling)) {
        out.error = "Illegal move"; return out;
    }

    // Apply move to a temporary board
    bool is_castle, is_ep;
    Board nb = detail::apply_pseudo_move(gs.board, r1, c1, r2, c2,
                                          promotion, gs.ep_col, gs.ep_row,
                                          gs.castling, is_castle, is_ep);

    // Must not leave own king in check
    if (is_in_check(nb, color)) {
        out.error = "Move leaves king in check"; return out;
    }

    // For castling, also verify the king's destination square is not attacked.
    // (The pass-through check is done in is_pseudo_legal;
    //  the destination check comes from is_in_check on nb.)
    // Already covered: after the castle move, if king is in check on nb, rejected above.

    // ---- Move is legal: apply it to gs ----
    out.valid = true;

    // Detect capture (before modifying)
    bool captured_piece = (!gs.board[r2][c2].is_space());
    out.capture   = captured_piece || is_ep;
    out.castle    = is_castle;
    out.en_passant = is_ep;

    // Detect promotion
    {
        const Piece& mp = gs.board[r1][c1];
        out.promotion = (mp.name == "pawn") &&
                        ((mp.color == 'w' && r2 == 0) || (mp.color == 'b' && r2 == 7));
    }

    // Update castling rights based on what moved FROM where
    {
        const Piece& mp = gs.board[r1][c1];
        if (mp.name == "king") {
            if (color == 'w') { gs.castling.w_k = false; gs.castling.w_q = false; }
            else              { gs.castling.b_k = false; gs.castling.b_q = false; }
        }
        if (mp.name == "rook") {
            if (color == 'w') {
                if (r1 == 7 && c1 == 7) gs.castling.w_k = false;
                if (r1 == 7 && c1 == 0) gs.castling.w_q = false;
            } else {
                if (r1 == 0 && c1 == 7) gs.castling.b_k = false;
                if (r1 == 0 && c1 == 0) gs.castling.b_q = false;
            }
        }
    }

    // Revoke castling rights when a rook is CAPTURED on its starting square
    if (captured_piece) {
        if (r2 == 7 && c2 == 7) gs.castling.w_k = false;
        if (r2 == 7 && c2 == 0) gs.castling.w_q = false;
        if (r2 == 0 && c2 == 7) gs.castling.b_k = false;
        if (r2 == 0 && c2 == 0) gs.castling.b_q = false;
    }

    // Set en-passant target for next move
    gs.ep_col = -1; gs.ep_row = -1;
    {
        const Piece& mp = gs.board[r1][c1];
        if (mp.name == "pawn") {
            if (mp.color == 'w' && r1 == 6 && r2 == 4) {
                // White double push: skipped row is r1-1 = 5
                gs.ep_col = c1;
                gs.ep_row = r1 - 1; // = 5
            } else if (mp.color == 'b' && r1 == 1 && r2 == 3) {
                // Black double push: skipped row is r1+1 = 2
                gs.ep_col = c1;
                gs.ep_row = r1 + 1; // = 2
            }
        }
    }

    // Halfmove clock
    if (out.capture || gs.board[r1][c1].name == "pawn")
        gs.halfmove_clock = 0;
    else
        ++gs.halfmove_clock;

    // Apply board
    gs.board = std::move(nb);

    // Switch turn
    gs.turn = opp;
    if (gs.turn == 'w') ++gs.fullmove_number;

    // Position hash (AFTER switch)
    std::string hash = detail::position_hash(gs.board, gs.turn, gs.castling, gs.ep_col, gs.ep_row);
    gs.position_history.push_back(hash);

    // --- Game-end detection ---

    out.gives_check = is_in_check(gs.board, opp);

    // Check for legal moves for the player whose turn it now is
    bool opp_has_moves = has_legal_moves(gs);

    if (!opp_has_moves) {
        if (is_in_check(gs.board, opp)) {
            out.is_checkmate = true;
            out.gives_check  = true;
        } else {
            out.is_stalemate = true;
        }
    }

    // 50-move rule (100 half-moves)
    if (gs.halfmove_clock >= 100) out.is_draw_fifty = true;

    // Threefold repetition
    {
        int count = 0;
        for (auto& h : gs.position_history)
            if (h == hash) ++count;
        if (count >= 3) out.is_draw_repetition = true;
    }

    // Insufficient material
    if (is_insufficient_material(gs.board)) out.is_draw_material = true;

    return out;
}

// ============================================================
// make_initial_state
// ============================================================
static GameState make_initial_state() {
    GameState gs;
    gs.board = Board(8, std::vector<Piece>(8));

    // Black back rank (row 0)
    gs.board[0] = {
        Piece("rook",'b'), Piece("knight",'b'), Piece("bishop",'b'), Piece("queen",'b'),
        Piece("king",'b'), Piece("bishop",'b'), Piece("knight",'b'), Piece("rook",'b')
    };
    // Black pawns (row 1)
    for (int c = 0; c < 8; ++c) gs.board[1][c] = Piece("pawn",'b');
    // Empty rows 2-5
    for (int r = 2; r <= 5; ++r)
        for (int c = 0; c < 8; ++c) gs.board[r][c] = Piece("space",'s');
    // White pawns (row 6)
    for (int c = 0; c < 8; ++c) gs.board[6][c] = Piece("pawn",'w');
    // White back rank (row 7)
    gs.board[7] = {
        Piece("rook",'w'), Piece("knight",'w'), Piece("bishop",'w'), Piece("queen",'w'),
        Piece("king",'w'), Piece("bishop",'w'), Piece("knight",'w'), Piece("rook",'w')
    };

    gs.turn = 'w';
    gs.castling = {true, true, true, true};
    gs.ep_col = gs.ep_row = -1;
    gs.halfmove_clock = 0;
    gs.fullmove_number = 1;

    // Record initial position hash
    std::string h = detail::position_hash(gs.board, gs.turn, gs.castling, gs.ep_col, gs.ep_row);
    gs.position_history.push_back(h);

    return gs;
}
