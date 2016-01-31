#ifndef __BOARD_H__
#define __BOARD_H__

#include "types.h"

class Board {
public:
	Board();
	~Board();

	void doMove(Player p, Move m);
	MoveList getLegalMoves(Player p);

	void reset();
	void prettyPrint();

private:
	Stone *pieces;

    //Board(const Board &other);
    Board& operator=(const Board &other);

	void doCaptures(Player victim, Move seed);
	bool isSurrounded(Player victim, Player blocker, int x, int y);
};

#endif