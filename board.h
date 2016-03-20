#ifndef __BOARD_H__
#define __BOARD_H__

#include "types.h"

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

    void reset();
    void prettyPrint();

private:
    Stone *pieces;
    int blackCaptures, whiteCaptures;
    // Records potential ko rule squares: the first element is the next move, and the
    // second element is the piece that cannot be captured (the last move)
    Move koRule[2];

    Board& operator=(const Board &other);

    template <bool updateBoard> int doCaptures(Player victim, Move seed);
    bool isSurrounded(Player victim, Player open, int x, int y,
        Stone *visited, MoveList &captured);
    void getTerritory(Player blocker, int x, int y, Stone *visited,
        Stone *territory, int &territorySize, Stone *boundary);

    void init();
};

#endif