#include <iostream>
#include "board.h"


int boardSize = 19;
int arraySize = 21;
int blackCaptures = 0;
int whiteCaptures = 0;


// Returns an array index for the pieces array given the coordinates for a
// move (x, y).
inline int index(int x, int y) {
	return x + y * arraySize;
}


Board::Board() {
	pieces = new Stone[arraySize*arraySize];
	// Initialize the board to empty
	for (int i = 0; i < arraySize*arraySize; i++) {
		pieces[i] = EMPTY;
	}

	for (int i = 0; i < arraySize; i++) {
		pieces[index(0, i)] = -1;
		pieces[index(arraySize-1, i)] = -1;
		pieces[index(i, 0)] = -1;
		pieces[index(arraySize-1, i)] = -1;
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

	for (int j = 1; j <= boardSize; j++) {
		for (int i = 1; i <= boardSize; i++) {
			if (pieces[index(i, j)] == EMPTY)
				result.add(coordToMove(i, j));
		}
	}

	return result;
}


//-------------------------Region Detection Algorithms--------------------------
void Board::doCaptures(Player victim, Move seed) {
	if (pieces[index(getX(seed), getY(seed))] != victim)
		return;


}

// Given a coordinate as a move, and a victim color, recursively determines
// whether the victim on this square is part of a surrounded chain
bool Board::isSurrounded(Player victim, Player blocker, int x, int y) {
	return false;
}


// Resets a board object completely.
void Board::reset() {
	delete pieces;

	pieces = new Stone[arraySize*arraySize];
	for (int i = 0; i < arraySize*arraySize; i++) {
		pieces[i] = EMPTY;
	}

	for (int i = 0; i < arraySize; i++) {
		pieces[index(0, i)] = -1;
		pieces[index(arraySize-1, i)] = -1;
		pieces[index(i, 0)] = -1;
		pieces[index(arraySize-1, i)] = -1;
	}
}

// Prints a board state to terminal for debugging
void Board::prettyPrint() {
	// Since the y axis indexing is inverted
	for (int j = boardSize; j >= 1; j--) {
		if (j >= 10)
			std::cout << j << " ";
		else
			std::cout << " " << j << " ";

		for (int i = 1; i <= boardSize; i++) {
			if (pieces[index(i, j)] == EMPTY)
				std::cout << ". ";
			else if (pieces[index(i, j)] == BLACK)
				std::cout << "B ";
			else
				std::cout << "W ";
		}

		if (j >= 10)
			std::cout << j << " ";
		else
			std::cout << " " << j << " ";
		std::cout << std::endl;
	}
}