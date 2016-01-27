#include <iostream>
#include "board.h"

Board::Board() {
	// Initialize the board to empty
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			pieces[i][j] = EMPTY;
		}
	}
}

/*
 * Updates the board with a move. Assumes that the move is legal.
 */
void Board::doMove(Player p, Move m) {
	pieces[getX(m)][getY(m)] = p;
}

// Prints a board state to terminal for debugging
void Board::prettyPrint() {
	std::cerr << std::endl;
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (pieces[i][j] == EMPTY)
				std::cerr << "O ";
			else if (pieces[i][j] == BLACK)
				std::cerr << "B ";
			else
				std::cerr << "W ";
		}
		std::cerr << std::endl;
	}
	std::cerr << std::endl;
}