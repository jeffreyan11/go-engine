#include <iostream>
#include "board.h"


int boardSize = 19;
int arraySize = 21;


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

	blackCaptures = 0;
	whiteCaptures = 0;
}

Board::Board(const Board &other) {
	pieces = new Stone[arraySize*arraySize];
	for (int i = 0; i < arraySize*arraySize; i++) {
		pieces[i] = other.pieces[i];
	}

	blackCaptures = other.blackCaptures;
	whiteCaptures = other.whiteCaptures;
}

Board::~Board() {
	delete pieces;
}


//------------------------------------------------------------------------------
//------------------------------Move Generation---------------------------------
//------------------------------------------------------------------------------

/*
 * Updates the board with a move. Assumes that the move is legal.
 */
void Board::doMove(Player p, Move m) {
	pieces[index(getX(m), getY(m))] = p;

	Player victim = (p == BLACK) ? WHITE : BLACK;
	int x = getX(m);
	int y = getY(m);

	// Check if p captured any of the other player's stones with move m
	doCaptures(victim, coordToMove(x+1, y));
	doCaptures(victim, coordToMove(x-1, y));
	doCaptures(victim, coordToMove(x, y+1));
	doCaptures(victim, coordToMove(x, y-1));
	// Check if p suicided with move m
	doCaptures(p, coordToMove(x, y));
}

/*
 * Returns a list of every possible legal move in the current board state.
 */
// TODO Account for ko
MoveList Board::getLegalMoves(Player p) {
	MoveList result;

	for (int j = 1; j <= boardSize; j++) {
		for (int i = 1; i <= boardSize; i++) {
			// All empty squares are legal moves
			if (pieces[index(i, j)] == EMPTY)
				result.add(coordToMove(i, j));
		}
	}

	return result;
}


//------------------------------------------------------------------------------
//-------------------------Region Detection Algorithms--------------------------
//------------------------------------------------------------------------------

// Given a victim color and seed square, detects whether the square is part of
// a connected group of stones of victim color that are surrounded, and performs
// the capture if necessary.
void Board::doCaptures(Player victim, Move seed) {
	if (pieces[index(getX(seed), getY(seed))] != victim)
		return;

	Stone *visited = new Stone[arraySize*arraySize];
	for (int i = 0; i < arraySize*arraySize; i++) {
		visited[i] = 0;
	}
	MoveList captured;

	if (isSurrounded(victim, EMPTY, getX(seed), getY(seed), visited, captured)) {
		for (unsigned int i = 0; i < captured.size(); i++) {
			Move m = captured.get(i);
			pieces[index(getX(m), getY(m))] = EMPTY;
		}

		// Record how many pieces were captured for scoring purposes
		if (victim == BLACK)
			whiteCaptures += captured.size();
		else
			blackCaptures += captured.size();
	}
}

// Given a coordinate as a move, and a victim color, recursively determines
// whether the victim on this square is part of a surrounded chain
// Precondition: (x, y) is of color victim
bool Board::isSurrounded(Player victim, Player open, int x, int y,
	Stone *visited, MoveList &captured) {
	visited[index(x, y)] = 1;

	Stone east = pieces[index(x+1, y)];
	// If we are next to a non-blocker and non-victim, then we are not surrounded
	if (east == open)
		return false;
	// If we next to victim, we need to recursively see if the entire group
	// is surrounded
	else if (east == victim && visited[index(x+1, y)] == 0)
		if (!isSurrounded(victim, open, x+1, y, visited, captured))
			return false;
	// Else the piece is surrounded by a blocker or edge

	Stone west = pieces[index(x-1, y)];
	if (west == open)
		return false;
	else if (west == victim && visited[index(x-1, y)] == 0)
		if (!isSurrounded(victim, open, x-1, y, visited, captured))
			return false;

	Stone north = pieces[index(x, y+1)];
	if (north == open)
		return false;
	else if (north == victim && visited[index(x, y+1)] == 0)
		if (!isSurrounded(victim, open, x, y+1, visited, captured))
			return false;

	Stone south = pieces[index(x, y-1)];
	if (south == open)
		return false;
	else if (south == victim && visited[index(x, y-1)] == 0)
		if (!isSurrounded(victim, open, x, y-1, visited, captured))
			return false;

	// If we got here, we are surrounded on all four sides
	captured.add(coordToMove(x, y));
	return true;
}


int Board::getCapturedStones(Player p) {
	return (p == BLACK) ? blackCaptures : whiteCaptures;
}


//------------------------------------------------------------------------------
//---------------------------Misc Utility Functions-----------------------------
//------------------------------------------------------------------------------

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

	blackCaptures = 0;
	whiteCaptures = 0;
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