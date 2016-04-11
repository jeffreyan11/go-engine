#ifndef __BOARD_H__
#define __BOARD_H__

#include "chain.h"
#include "types.h"

void initZobristTable();

struct ChainListNode {
    Chain *cargo;
    ChainListNode *next;

    ChainListNode() {
        cargo = NULL;
        next = NULL;
    }

    ~ChainListNode() {
        if (cargo != NULL)
            delete cargo;
    }
};

class Board {
public:
    Board();
    Board(const Board &other);
    ~Board();

    void doMove(Player p, Move m);
    bool isMoveValid(Player p, Move m);
    MoveList getLegalMoves(Player p);

    void countTerritory(int &whiteTerritory, int &blackTerritory);
    int getCapturedStones(Player p);

    uint64_t getZobristKey();

    void reset();
    void prettyPrint();

private:
    Stone *pieces;
    int blackCaptures, whiteCaptures;
    uint64_t zobristKey;
    int nextID;
    int *chainID;
    ChainListNode *chainList;

    Board& operator=(const Board &other);

    // Chain update helpers
    void updateLiberty(ChainListNode *node, int x, int y);
    void mergeChains(ChainListNode *node, ChainListNode *temp, ChainListNode *prev, Move m);
    void captureChain(ChainListNode *node, ChainListNode *prev);

    // Region detection helpers
    template <bool updateBoard> int doCaptures(Player victim, Move seed);
    bool isSurrounded(Player victim, Player open, int x, int y,
        Stone *visited, MoveList &captured);
    void getTerritory(Player blocker, int x, int y, Stone *visited,
        Stone *territory, int &territorySize, int &boundarySize);

    void init();
};

#endif
