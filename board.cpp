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
	init();
}

Board::Board(const Board &other) {
	pieces = new Stone[arraySize*arraySize];
	for (int i = 0; i < arraySize*arraySize; i++) {
		pieces[i] = other.pieces[i];
	}

	blackCaptures = other.blackCaptures;
	whiteCaptures = other.whiteCaptures;
	koRule[0] = other.koRule[0];
	koRule[1] = other.koRule[1];
}

Board::~Board() {
	delete[] pieces;
}


//------------------------------------------------------------------------------
//------------------------------Move Generation---------------------------------
//------------------------------------------------------------------------------

/*
 * Updates the board with a move. Assumes that the move is legal.
 */
void Board::doMove(Player p, Move m) {
	if (m == MOVE_PASS)
		return;

	pieces[index(getX(m), getY(m))] = p;

	Player victim = otherPlayer(p);
	int x = getX(m);
	int y = getY(m);

	// Check if p captured any of the other player's stones with move m
	int east = doCaptures<true>(victim, coordToMove(x+1, y));
	int west = doCaptures<true>(victim, coordToMove(x-1, y));
	int north = doCaptures<true>(victim, coordToMove(x, y+1));
	int south = doCaptures<true>(victim, coordToMove(x, y-1));

	// Detect potential ko rule
	if (east + west + north + south == 1) {
		if (east)
			koRule[0] = coordToMove(x+1, y);
		else if (west)
			koRule[0] = coordToMove(x-1, y);
		else if (north)
			koRule[0] = coordToMove(x, y+1);
		else
			koRule[0] = coordToMove(x, y-1);
		koRule[1] = coordToMove(x, y);
	}
	else {
		koRule[0] = MOVE_NULL;
		koRule[1] = MOVE_NULL;
	}

	// Check if p suicided with move m
	doCaptures<true>(p, coordToMove(x, y));
}

bool Board::isMoveValid(Player p, Move m) {
	int i = getX(m);
	int j = getY(m);
	pieces[index(i, j)] = p;

	// Suicides are illegal
	if (pieces[index(i+1, j)] && pieces[index(i-1, j)]
	 && pieces[index(i, j+1)] && pieces[index(i, j-1)]) {
		if (doCaptures<false>(p, coordToMove(i, j))) {
			pieces[index(i, j)] = EMPTY;
			return false;
		}
	}

	pieces[index(i, j)] = EMPTY;
	return true;
}

/*
 * Returns a list of every possible legal move in the current board state.
 */
MoveList Board::getLegalMoves(Player p) {
	MoveList result;

	for (int j = 1; j <= boardSize; j++) {
		for (int i = 1; i <= boardSize; i++) {
			// All empty squares are legal moves
			if (pieces[index(i, j)] == EMPTY) {
				pieces[index(i, j)] = p;

				if (i == getX(koRule[0]) && j == getY(koRule[0])) {
					if (doCaptures<false>(otherPlayer(p), koRule[1]) == 1) {
						pieces[index(i, j)] = EMPTY;
						continue;
					}
				}

				pieces[index(i, j)] = EMPTY;
				result.add(coordToMove(i, j));
			}
		}
	}

	result.add(MOVE_PASS);

	return result;
}


//------------------------------------------------------------------------------
//-------------------------Region Detection Algorithms--------------------------
//------------------------------------------------------------------------------

// Given a victim color and seed square, detects whether the square is part of
// a connected group of stones of victim color that are surrounded, and performs
// the capture if necessary.
// Returns the number of stones captured in this region.
template <bool updateBoard>
int Board::doCaptures(Player victim, Move seed) {
	if (pieces[index(getX(seed), getY(seed))] != victim)
		return 0;

	Stone *visited = new Stone[arraySize*arraySize];
	for (int i = 0; i < arraySize*arraySize; i++) {
		visited[i] = 0;
	}
	MoveList captured;

	if (isSurrounded(victim, EMPTY, getX(seed), getY(seed), visited, captured)) {
		if (updateBoard) {
			for (unsigned int i = 0; i < captured.size(); i++) {
				Move m = captured.get(i);
				pieces[index(getX(m), getY(m))] = EMPTY;
			}
		}

		// Record how many pieces were captured for scoring purposes
		if (victim == BLACK)
			whiteCaptures += captured.size();
		else
			blackCaptures += captured.size();
	}

	delete[] visited;
	return captured.size();
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

// Counts the territory each side owns
void Board::countTerritory(int &whiteTerritory, int &blackTerritory) {
	whiteTerritory = 0;
	blackTerritory = 0;
	Stone *visited = new Stone[arraySize*arraySize];
	for (int i = 0; i < arraySize*arraySize; i++)
		visited[i] = 0;

	// Count black territory
	for (int j = 1; j <= boardSize; j++) {
		for (int i = 1; i <= boardSize; i++) {
			// Don't recount territory
			if (visited[index(i, j)])
				continue;
			
			MoveList captured;
			if (isSurrounded(EMPTY, WHITE, i, j, visited, captured))
				blackTerritory += captured.size();
		}
	}

	// Reset the visited array
	delete[] visited;
	visited = new Stone[arraySize*arraySize];
	for (int i = 0; i < arraySize*arraySize; i++)
		visited[i] = 0;

	// And then count white territory
	for (int j = 1; j <= boardSize; j++) {
		for (int i = 1; i <= boardSize; i++) {
			// Don't recount territory
			if (visited[index(i, j)])
				continue;
			
			MoveList captured;
			if (isSurrounded(EMPTY, BLACK, i, j, visited, captured))
				whiteTerritory += captured.size();
		}
	}

	delete[] visited;
}


int Board::getCapturedStones(Player p) {
	return (p == BLACK) ? blackCaptures : whiteCaptures;
}


//------------------------------------------------------------------------------
//---------------------------Misc Utility Functions-----------------------------
//------------------------------------------------------------------------------

// Initializes a board to empty
void Board::init() {
	pieces = new Stone[arraySize*arraySize];
	// Initialize the board to empty
	for (int i = 0; i < arraySize*arraySize; i++)
		pieces[i] = EMPTY;

	// Initialize the edges to -1
	for (int i = 0; i < arraySize; i++) {
		pieces[index(0, i)] = -1;
		pieces[index(arraySize-1, i)] = -1;
		pieces[index(i, 0)] = -1;
		pieces[index(i, arraySize-1)] = -1;
	}

	blackCaptures = 0;
	whiteCaptures = 0;
	koRule[0] = MOVE_NULL;
	koRule[1] = MOVE_NULL;
}

// Resets a board object completely.
void Board::reset() {
	delete[] pieces;
	init();
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