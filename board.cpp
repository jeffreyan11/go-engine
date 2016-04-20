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
    nextID = other.nextID;
    chainID = new int[arraySize*arraySize];
    for (int i = 0; i < arraySize*arraySize; i++)
        chainID[i] = other.chainID[i];

    // Deep copy the linked list
    if (other.chainList == nullptr)
        chainList = nullptr;
    else {
        ChainListNode *onode = other.chainList;
        chainList = new ChainListNode();
        chainList->cargo = new Chain(*(onode->cargo));
        ChainListNode *node = chainList;
        onode = onode->next;

        while (onode != nullptr) {
            ChainListNode *toAdd = new ChainListNode();
            toAdd->cargo = new Chain(*(onode->cargo));
            node->next = toAdd;

            node = node->next;
            onode = onode->next;
        }
    }
}

Board::~Board() {
    deinit();
}



//------------------------------------------------------------------------------
//------------------------Move Generation and Handling--------------------------
//------------------------------------------------------------------------------

/*
 * Updates the board with a move. Assumes that the move is legal.
 */
void Board::doMove(Player p, Move m) {
    if (m == MOVE_PASS)
        return;

    int x = getX(m);
    int y = getY(m);

    assert(pieces[index(x, y)] == EMPTY);
    assert(chainID[index(x, y)] == 0);
    pieces[index(x, y)] = p;
    zobristKey ^= zobristTable[zobristIndex(p, x, y)];

    Player victim = otherPlayer(p);

    Stone east = pieces[index(x+1, y)];
    Stone west = pieces[index(x-1, y)];
    Stone north = pieces[index(x, y+1)];
    Stone south = pieces[index(x, y-1)];
    int connectionCount = (east == p) + (west == p) + (north == p) + (south == p);

    // If the stone placed is a new chain
    if (connectionCount == 0) {
        // Record which chain this square is a part of
        chainID[index(x, y)] = nextID;

        // Add this chain to the list of chains
        Chain *cargo = new Chain(p, nextID);
        cargo->add(m);
        cargo->liberties = 0;
        if (east == EMPTY)
            cargo->addLiberty(coordToMove(x+1, y));
        if (west == EMPTY)
            cargo->addLiberty(coordToMove(x-1, y));
        if (north == EMPTY)
            cargo->addLiberty(coordToMove(x, y+1));
        if (south == EMPTY)
            cargo->addLiberty(coordToMove(x, y-1));

        ChainListNode *node = new ChainListNode();
        node->cargo = cargo;

        if (chainList == nullptr) {
            chainList = node;
        }
        else {
            ChainListNode *temp = chainList;
            while (temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = node;
        }

        nextID++;
    }

    // If the stone placed is added to an existing chain
    else if (connectionCount == 1) {
        // Find the ID of the chain we are adding this stone to
        int thisID;
        if (east == p)
            thisID = chainID[index(x+1, y)];
        else if (west == p)
            thisID = chainID[index(x-1, y)];
        else if (north == p)
            thisID = chainID[index(x, y+1)];
        else
            thisID = chainID[index(x, y-1)];

        chainID[index(x, y)] = thisID;

        ChainListNode *node = chainList;
        searchChainsByID(node, thisID);

        node->cargo->add(m);

        // The new stone occupies a previous liberty, but adds on however many
        // liberties it itself has
        node->cargo->removeLiberty(node->cargo->findLiberty(m));
        updateLiberty(node, x, y);
    }

    // If the stone possibly connects two existing chains
    else {
        int eastID = (east == p) * chainID[index(x+1, y)];
        int westID = (west == p) * chainID[index(x-1, y)];
        int northID = (north == p) * chainID[index(x, y+1)];
        int southID = (south == p) * chainID[index(x, y-1)];
        ChainListNode *node = chainList;
        bool added = false;

        if (eastID) {
            chainID[index(x, y)] = eastID;
            searchChainsByID(node, eastID);

            node->cargo->add(m);
            node->cargo->removeLiberty(node->cargo->findLiberty(m));
            updateLiberty(node, x, y);
            added = true;
        }

        if (westID) {
            if (added) {
                // If two stones from the same chain are adjacent, do nothing
                // If they are from different chains, we need to combine...
                if (westID != eastID)
                    mergeChains(node, westID, m);
            }
            else {
                chainID[index(x, y)] = westID;
                searchChainsByID(node, westID);

                node->cargo->add(m);
                node->cargo->removeLiberty(node->cargo->findLiberty(m));
                updateLiberty(node, x, y);
                added = true;
            }
        }

        if (northID) {
            if (added) {
                if (northID != eastID && northID != westID)
                    mergeChains(node, northID, m);
            }
            else {
                chainID[index(x, y)] = northID;
                searchChainsByID(node, northID);

                node->cargo->add(m);
                node->cargo->removeLiberty(node->cargo->findLiberty(m));
                updateLiberty(node, x, y);
                added = true;
            }
        }

        if (southID) {
            if (added) {
                if (southID != eastID && southID != westID && southID != northID)
                    mergeChains(node, southID, m);
            }
            else {
                chainID[index(x, y)] = southID;
                searchChainsByID(node, southID);

                node->cargo->add(m);
                node->cargo->removeLiberty(node->cargo->findLiberty(m));
                updateLiberty(node, x, y);
                added = true;
            }
        }
    }


    // Update opponent liberties
    int eastID = (east == victim) * chainID[index(x+1, y)];
    int westID = (west == victim) * chainID[index(x-1, y)];
    int northID = (north == victim) * chainID[index(x, y+1)];
    int southID = (south == victim) * chainID[index(x, y-1)];

    if (eastID) {
        ChainListNode *prev = chainList;
        ChainListNode *node = chainList;
        searchChainsByID(node, prev, eastID);
        node->cargo->removeLiberty(node->cargo->findLiberty(m));

        if (node->cargo->liberties == 0) {
            captureChain(node, prev);
        }
    }
    if (westID && westID != eastID) {
        ChainListNode *prev = chainList;
        ChainListNode *node = chainList;
        searchChainsByID(node, prev, westID);
        node->cargo->removeLiberty(node->cargo->findLiberty(m));

        if (node->cargo->liberties == 0) {
            captureChain(node, prev);
        }
    }
    if (northID && northID != eastID && northID != westID) {
        ChainListNode *prev = chainList;
        ChainListNode *node = chainList;
        searchChainsByID(node, prev, northID);
        node->cargo->removeLiberty(node->cargo->findLiberty(m));

        if (node->cargo->liberties == 0) {
            captureChain(node, prev);
        }
    }
    if (southID && southID != eastID && southID != westID && southID != northID) {
        ChainListNode *prev = chainList;
        ChainListNode *node = chainList;
        searchChainsByID(node, prev, southID);
        node->cargo->removeLiberty(node->cargo->findLiberty(m));

        if (node->cargo->liberties == 0) {
            captureChain(node, prev);
        }
    }

    int selfID = chainID[index(x, y)];
    ChainListNode *prev = chainList;
    ChainListNode *node = chainList;
    searchChainsByID(node, prev, selfID);

    if (node->cargo->liberties == 0) {
        captureChain(node, prev);
    }

    assert(!checkChains());

    // Check if p captured any of the other player's stones with move m
    /*
    doCaptures<true>(victim, coordToMove(x+1, y));
    doCaptures<true>(victim, coordToMove(x-1, y));
    doCaptures<true>(victim, coordToMove(x, y+1));
    doCaptures<true>(victim, coordToMove(x, y-1));

    // Check if p suicided with move m
    doCaptures<true>(p, coordToMove(x, y));
    */
}

bool Board::isMoveValid(Player p, Move m) {
    if (m == MOVE_PASS)
        return true;

    int i = getX(m);
    int j = getY(m);
    assert(pieces[index(i, j)] == EMPTY);
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
//---------------------------Chain Update Algorithms----------------------------
//------------------------------------------------------------------------------
inline void Board::searchChainsByID(ChainListNode *&node, int id) {
    while (node->cargo->id != id)
        node = node->next;
}

inline void Board::searchChainsByID(ChainListNode *&node, ChainListNode *&prev, int id) {
    while (node->cargo->id != id) {
        prev = node;
        node = node->next;
    }
}

// Updates liberties of a chain after a single stone has been added to the chain
void Board::updateLiberty(ChainListNode *node, int x, int y) {
    Stone east = pieces[index(x+1, y)];
    Stone west = pieces[index(x-1, y)];
    Stone north = pieces[index(x, y+1)];
    Stone south = pieces[index(x, y-1)];

    if (east == EMPTY && node->cargo->findLiberty(coordToMove(x+1, y)) == -1)
        node->cargo->addLiberty(coordToMove(x+1, y));
    if (west == EMPTY && node->cargo->findLiberty(coordToMove(x-1, y)) == -1)
        node->cargo->addLiberty(coordToMove(x-1, y));
    if (north == EMPTY && node->cargo->findLiberty(coordToMove(x, y+1)) == -1)
        node->cargo->addLiberty(coordToMove(x, y+1));
    if (south == EMPTY && node->cargo->findLiberty(coordToMove(x, y-1)) == -1)
        node->cargo->addLiberty(coordToMove(x, y-1));
}

// Merges two chains
void Board::mergeChains(ChainListNode *node, int otherID, Move m) {
    // Find the chain to merge into the first chain
    ChainListNode *prev = chainList;
    ChainListNode *temp = chainList;
    searchChainsByID(temp, prev, otherID);

    // Update the chain id array when merging
    for (int i = 0; i < temp->cargo->size; i++) {
        Move idChange = temp->cargo->squares[i];
        chainID[index(getX(idChange), getY(idChange))] = node->cargo->id;

        node->cargo->add(idChange);
    }

    // Remove the move played from the other list of liberties
    temp->cargo->removeLiberty(temp->cargo->findLiberty(m));
    // And then merge the two lists
    for (int i = 0; i < temp->cargo->liberties; i++) {
        // If the liberty is not a repeat
        if (node->cargo->findLiberty(temp->cargo->libertyList[i]) == -1)
            node->cargo->addLiberty(temp->cargo->libertyList[i]);
    }

    // Delete the chain "temp" now since it has been fully merged in
    if (temp == chainList) {
        chainList = temp->next;
        delete temp;
    }
    else {
        prev->next = temp->next;
        delete temp;
    }
}

// Performs a capture on a chain and updates the board, adjacent liberties, etc.
// To be called on a chain that has no liberties
void Board::captureChain(ChainListNode *node, ChainListNode *prev) {
    Player victim = node->cargo->color;
    if (victim == BLACK)
        whiteCaptures += node->cargo->size;
    else
        blackCaptures += node->cargo->size;

    for (int i = 0; i < node->cargo->size; i++) {
        int rx = getX(node->cargo->squares[i]);
        int ry = getY(node->cargo->squares[i]);
        pieces[index(rx, ry)] = EMPTY;
        chainID[index(rx, ry)] = 0;
        zobristKey ^= zobristTable[zobristIndex(node->cargo->color, rx, ry)];

        // Add this square to adjacent chains' liberties
        int addID = chainID[index(rx+1, ry)];
        if (addID
         && addID != node->cargo->id) {
            ChainListNode *temp = chainList;
            searchChainsByID(temp, addID);

            if (temp->cargo->findLiberty(coordToMove(rx, ry)) == -1)
                temp->cargo->addLiberty(coordToMove(rx, ry));
        }

        addID = chainID[index(rx-1, ry)];
        if (addID
         && addID != node->cargo->id
         && addID != chainID[index(rx+1, ry)]) {
            ChainListNode *temp = chainList;
            searchChainsByID(temp, addID);

            if (temp->cargo->findLiberty(coordToMove(rx, ry)) == -1)
                temp->cargo->addLiberty(coordToMove(rx, ry));
        }

        addID = chainID[index(rx, ry+1)];
        if (addID
         && addID != node->cargo->id
         && addID != chainID[index(rx+1, ry)]
         && addID != chainID[index(rx-1, ry)]) {
            ChainListNode *temp = chainList;
            searchChainsByID(temp, addID);

            if (temp->cargo->findLiberty(coordToMove(rx, ry)) == -1)
                temp->cargo->addLiberty(coordToMove(rx, ry));
        }

        addID = chainID[index(rx, ry-1)];
        if (addID
         && addID != node->cargo->id
         && addID != chainID[index(rx+1, ry)]
         && addID != chainID[index(rx-1, ry)]
         && addID != chainID[index(rx, ry+1)]) {
            ChainListNode *temp = chainList;
            searchChainsByID(temp, addID);

            if (temp->cargo->findLiberty(coordToMove(rx, ry)) == -1)
                temp->cargo->addLiberty(coordToMove(rx, ry));
        }
    }

    // Remove this chain since it has been captured
    if (node == chainList) {
        chainList = node->next;
        delete node;
    }
    else {
        prev->next = node->next;
        delete node;
    }
}

// For debugging
bool Board::checkChains() {
    bool result = false;
    int *temp = new int[arraySize*arraySize];
    int *tempPieces = new int[arraySize*arraySize];
    for (int i = 0; i < arraySize*arraySize; i++)
        temp[i] = chainID[i];
    for (int i = 0; i < arraySize*arraySize; i++)
        tempPieces[i] = pieces[i];

    ChainListNode *node = chainList;
    while (node != NULL) {
        for (int i = 0; i < node->cargo->size; i++) {
            Move m = node->cargo->squares[i];

            if (temp[index(getX(m), getY(m))] == 0 || tempPieces[index(getX(m), getY(m))] == 0) {
                result = true;
                break;
            }

            temp[index(getX(m), getY(m))] = 0;
            tempPieces[index(getX(m), getY(m))] = 0;
        }

        node = node->next;
    }

    delete[] temp;
    delete[] tempPieces;
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

            // Record how many pieces were captured for scoring purposes
            if (victim == BLACK)
                whiteCaptures += captured.size();
            else
                blackCaptures += captured.size();
        }
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
    nextID = 1;
    chainID = new int[arraySize*arraySize];
    for (int i = 0; i < arraySize*arraySize; i++)
        chainID[i] = 0;
    chainList = nullptr;
}

void Board::deinit() {
    delete[] pieces;
    delete[] chainID;

    ChainListNode *node = chainList;
    while (node != nullptr) {
        ChainListNode *next = node->next;
        delete node;
        node = next;
    }
}

// Resets a board object completely.
void Board::reset() {
    deinit();
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
