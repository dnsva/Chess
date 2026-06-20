#!/usr/bin/env python3
"""
Drives the chess game with timed inputs to produce a clean demo recording.
Plays Scholar's Mate in 4 moves (7 half-moves).

Coordinate system (viewing_as='w'/'b'):
  board_row = 8 - user_R,  board_col = user_C - 1
Both orientations use the same formula.
"""

import os, pty, sys, time, select, struct, fcntl, termios, threading, subprocess

GAME = os.path.join(os.path.dirname(__file__), "build", "main")

# ── Move sequence ─────────────────────────────────────────────────────────────
# Each tuple: (string_to_send, seconds_to_wait_before_sending)
# Sequence: Scholar's Mate
#   1. e4   (white pawn e2→e4):  select (R=2,C=5), dest (R=4,C=5)
#   1…e5   (black pawn e7→e5):  select (R=7,C=5), dest (R=5,C=5)
#   2. Bc4  (white bishop f1→c4): select (R=1,C=6), dest (R=4,C=3)
#   2…Nc6  (black knight b8→c6): select (R=8,C=2), dest (R=6,C=3)
#   3. Qh5  (white queen d1→h5):  select (R=1,C=4), dest (R=5,C=8)
#   3…Nf6  (black knight g8→f6): select (R=8,C=7), dest (R=6,C=6)
#   4. Qxf7# (white queen h5xf7): select (R=5,C=8), dest (R=7,C=6)  ← CHECKMATE

INPUTS = [
    # title screen takes ~3 s
    # ── Move 1 – White: 1.e4 ──────────────────────────────────────────────────
    ("2\n",  4.2),   # row: select e2 pawn
    ("5\n",  0.7),   # col
    ("4\n",  0.7),   # row: move to e4
    ("5\n",  0.7),   # col
    # board flips, ~2 s delay (500ms move pause + 1s flip pause)
    # ── Move 2 – Black: 1…e5 ──────────────────────────────────────────────────
    ("7\n",  2.8),   # row: select e7 pawn (black's view)
    ("5\n",  0.7),   # col
    ("5\n",  0.7),   # row: move to e5
    ("5\n",  0.7),   # col
    # ── Move 3 – White: 2.Bc4 ─────────────────────────────────────────────────
    ("1\n",  2.8),   # row: select f1 bishop
    ("6\n",  0.7),   # col
    ("4\n",  0.7),   # row: move to c4
    ("3\n",  0.7),   # col
    # ── Move 4 – Black: 2…Nc6 ─────────────────────────────────────────────────
    ("8\n",  2.8),   # row: select b8 knight
    ("2\n",  0.7),   # col
    ("6\n",  0.7),   # row: move to c6
    ("3\n",  0.7),   # col
    # ── Move 5 – White: 3.Qh5 ─────────────────────────────────────────────────
    ("1\n",  2.8),   # row: select d1 queen
    ("4\n",  0.7),   # col
    ("5\n",  0.7),   # row: move to h5
    ("8\n",  0.7),   # col
    # ── Move 6 – Black: 3…Nf6?? ───────────────────────────────────────────────
    ("8\n",  2.8),   # row: select g8 knight
    ("7\n",  0.7),   # col
    ("6\n",  0.7),   # row: move to f6
    ("6\n",  0.7),   # col
    # ── Move 7 – White: 4.Qxf7# (CHECKMATE) ──────────────────────────────────
    ("5\n",  2.8),   # row: select h5 queen
    ("8\n",  0.7),   # col
    ("7\n",  0.7),   # row: move to f7 (captures pawn, checkmate)
    ("6\n",  0.7),   # col
    # let checkmate screen sit for a moment
    ("",     4.0),
]


def main():
    # Open a pseudo-terminal
    master_fd, slave_fd = pty.openpty()

    # Set a generous terminal size so ANSI art fits
    winsize = struct.pack("HHHH", 48, 120, 0, 0)  # rows, cols, xpix, ypix
    fcntl.ioctl(master_fd, termios.TIOCSWINSZ, winsize)
    fcntl.ioctl(slave_fd,  termios.TIOCSWINSZ, winsize)

    # Launch the game with slave as its controlling terminal
    proc = subprocess.Popen(
        [GAME],
        stdin=slave_fd,
        stdout=slave_fd,
        stderr=slave_fd,
        close_fds=True,
        preexec_fn=os.setsid,
    )
    os.close(slave_fd)

    stop_event = threading.Event()

    def relay_output():
        """Forward master PTY output to our stdout so asciinema captures it."""
        while not stop_event.is_set():
            try:
                r, _, _ = select.select([master_fd], [], [], 0.05)
                if r:
                    data = os.read(master_fd, 4096)
                    if data:
                        sys.stdout.buffer.write(data)
                        sys.stdout.buffer.flush()
            except OSError:
                break

    relay = threading.Thread(target=relay_output, daemon=True)
    relay.start()

    # Send inputs with timing delays
    for text, delay in INPUTS:
        time.sleep(delay)
        if text:
            os.write(master_fd, text.encode())

    proc.wait()
    stop_event.set()
    relay.join(timeout=1)


if __name__ == "__main__":
    main()
