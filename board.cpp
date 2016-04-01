#include <iostream>
#include <random>
#include "board.h"


int boardSize = 19;
int arraySize = 21;


// Zobrist hashing table, initialized at startup
// 2 * 25 * 25 = 1250 entries, 2 per color for a max 25 x 25 board
static uint64_t zobristTable[1250];

void initZobristTable() {
    std::mt19937_64 rng (74623982906748ULL);
    for (int i = 0; i < 1250; i++)
        zobristTable[i] = rng();
}


// Returns an array index for the pieces array given the coordinates for a
// move (x, y).
inline int index(int x, int y) {
    return x + y * arraySize;
}

// Returns an indexing to the zobrist table.
inline int zobristIndex(Player p, int x, int y) {
    return (p-1) + (x-1) + (y-1) * boardSize;
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
    zobristKey = other.zobristKey;
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

    int x = getX(m);
    int y = getY(m);

    pieces[index(x, y)] = p;
    zobristKey ^= zobristTable[zobristIndex(p, x, y)];

    Player victim = otherPlayer(p);

    // Check if p captured any of the other player's stones with move m
    doCaptures<true>(victim, coordToMove(x+1, y));
    doCaptures<true>(victim, coordToMove(x-1, y));
    doCaptures<true>(victim, coordToMove(x, y+1));
    doCaptures<true>(victim, coordToMove(x, y-1));

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
 * This function does not account for suicides and ko rule.
 */
MoveList Board::getLegalMoves(Player p) {
    MoveList result;

    for (int j = 1; j <= boardSize; j++) {
        for (int i = 1; i <= boardSize; i++) {
            // All empty squares are legal moves
            if (pieces[index(i, j)] == EMPTY)
                result.add(coordToMove(i, j));
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
                zobristKey ^= zobristTable[zobristIndex(victim, getX(m), getY(m))];
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
    Stone *territory = new Stone[arraySize*arraySize];
    Stone *region = new Stone[arraySize*arraySize]; 

    // Count territory for both sides
    for (Player p = BLACK; p <= WHITE; p++) {
        // Reset the visited array
        for (int i = 0; i < arraySize*arraySize; i++)
            visited[i] = 0;

        // Main loop
        for (int j = 1; j <= boardSize; j++) {
            for (int i = 1; i <= boardSize; i++) {
                // Don't recount territory
                if (visited[index(i, j)])
                    continue;
                // Only use empty squares as seeds
                if (pieces[index(i, j)])
                    continue;
                
                for (int k = 0; k < arraySize*arraySize; k++)
                    territory[k] = 0;
                int territorySize = 0;
                int boundarySize = 0;

                getTerritory(p, i, j, visited, territory, territorySize, boundarySize);

                // Check if territory was actually sectioned off
                if (territorySize + boundarySize == boardSize*boardSize)
                    continue;

                // Detect life/death of internal stones
                // Initialize region to 0 if territory is 1, and vice versa
                // This acts as our "visited" array, so that we only explore areas
                // inside the territory
                for (int k = 0; k < arraySize*arraySize; k++)
                    region[k] = territory[k] ^ 1;
                int internalRegions = 0;

                for (int n = 1; n <= boardSize; n++) {
                    for (int m = 1; m <= boardSize; m++) {
                        if (region[index(m, n)])
                            continue;
                        if (pieces[index(m, n)])
                            continue;

                        MoveList eye;
                        if (isSurrounded(EMPTY, p, m, n, region, eye))
                            internalRegions++;
                    }
                }

                int territoryCount = 0;
                if (internalRegions == 0) {
                    territoryCount += territorySize;
                    // Score dead stones
                    for (int k = 0; k < arraySize*arraySize; k++)
                        if (territory[k] && pieces[k] == otherPlayer(p))
                            territoryCount++;
                }

                if (p == BLACK)
                    blackTerritory += territoryCount;
                else
                    whiteTerritory += territoryCount;
            }
        }
    }

    delete[] visited;
    delete[] territory;
    delete[] region;
}

// Given a seed square, determines whether the square is part of territory owned
// by color blocker.
void Board::getTerritory(Player blocker, int x, int y, Stone *visited,
    Stone *territory, int &territorySize, int &boundarySize) {
    visited[index(x, y)] = 1;

    // Record the boundary of the region we are flood filling
    if (pieces[index(x, y)] == blocker) {
        boundarySize++;
        return;
    }

    Stone east = pieces[index(x+1, y)];
    // Flood fill outwards
    if (east != -1 && visited[index(x+1, y)] == 0)
        getTerritory(blocker, x+1, y, visited, territory, territorySize, boundarySize);
    // Else we are on the edge of the board

    Stone west = pieces[index(x-1, y)];
    if (west != -1 && visited[index(x-1, y)] == 0)
        getTerritory(blocker, x-1, y, visited, territory, territorySize, boundarySize);

    Stone north = pieces[index(x, y+1)];
    if (north != -1 && visited[index(x, y+1)] == 0)
        getTerritory(blocker, x, y+1, visited, territory, territorySize, boundarySize);

    Stone south = pieces[index(x, y-1)];
    if (south != -1 && visited[index(x, y-1)] == 0)
        getTerritory(blocker, x, y-1, visited, territory, territorySize, boundarySize);

    territory[index(x, y)] = 1;
    territorySize++;
}


int Board::getCapturedStones(Player p) {
    return (p == BLACK) ? blackCaptures : whiteCaptures;
}


uint64_t Board::getZobristKey() {
    return zobristKey;
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
    zobristKey = 0;
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
