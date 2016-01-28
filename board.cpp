#include <iostream>
#include "board.h"


int boardSize = 19;


// Returns an array index for the pieces array given the coordinates for a
// move (x, y).
inline int index(int x, int y) {
	return x + y * boardSize;
}


Board::Board() {
	pieces = new Stone[boardSize*boardSize];
	// Initialize the board to empty
	for (int i = 0; i < boardSize*boardSize; i++) {
		pieces[i] = EMPTY;
	}
}

Board::~Board() {
	delete pieces;
}


/*
 * Updates the board with a move. Assumes that the move is legal.
 */
void Board::doMove(Player p, Move m) {
	pieces[index(getX(m), getY(m))] = p;
}

/*
 * Returns a list of every possible legal move in the current board state.
 */
// TODO Account for ko
MoveList Board::getLegalMoves(Player p) {
	MoveList result;

	for (int j = 0; j < boardSize; j++) {
		for (int i = 0; i < boardSize; i++) {
			if (pieces[index(i, j)] == EMPTY)
				result.add(coordToMove(i, j));
		}
	}

	return result;
}


// Resets a board object completely.
void Board::reset() {
	delete pieces;
	pieces = new Stone[boardSize*boardSize];
	// Initialize the board to empty
	for (int i = 0; i < boardSize*boardSize; i++) {
		pieces[i] = EMPTY;
	}
}

// Prints a board state to terminal for debugging
void Board::prettyPrint() {
	// Since the y axis indexing is inverted
	for (int j = boardSize-1; j >= 0; j--) {
		if (j >= 9)
			std::cout << j+1 << " ";
		else
			std::cout << " " << j+1 << " ";

		for (int i = 0; i < boardSize; i++) {
			if (pieces[index(i, j)] == EMPTY)
				std::cout << ". ";
			else if (pieces[index(i, j)] == BLACK)
				std::cout << "B ";
			else
				std::cout << "W ";
		}

		if (j >= 9)
			std::cout << j+1 << " ";
		else
			std::cout << " " << j+1 << " ";
		std::cout << std::endl;
	}
}