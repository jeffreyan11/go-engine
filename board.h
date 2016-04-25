#ifndef __BOARD_H__
#define __BOARD_H__

#include "chain.h"
#include "types.h"

void initZobristTable();

class Board {
public:
    Board();
    Board(const Board &other);
    ~Board();

    void doMove(Player p, Move m);
    bool isMoveValid(Player p, Move m);
    MoveList getLegalMoves(Player p);

    void countTerritory(int &whiteTerritory, int &blackTerritory);
    bool isEye(Player p, Move m);
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
    GoArrayList<Chain *> chainList;

    Board& operator=(const Board &other);

    // Chain update helpers
    int searchChainsByID(Chain *&node, int id);
    void updateLiberty(Chain *node, int x, int y);
    void mergeChains(Chain *node, int otherID, Move m);
    void captureChain(Chain *node, int nodeIndex);

    bool checkChains();

    // Region detection helpers
    template <bool updateBoard> int doCaptures(Player victim, Move seed);
    bool isSurrounded(Player victim, Player open, int x, int y,
        Stone *visited, MoveList &captured);
    void getTerritory(Player blocker, int x, int y, Stone *visited,
        Stone *territory, int &territorySize, int &boundarySize);

    void init();
    void deinit();
};

#endif
