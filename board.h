#ifndef __BOARD_H__
#define __BOARD_H__

#include "types.h"

class Board {
public:
	Board();
	Board(const Board &other);
	~Board();

	void doMove(Player p, Move m);
	MoveList getLegalMoves(Player p);

	int getCapturedStones(Player p);

	void reset();
	void prettyPrint();

private:
	Stone *pieces;
	int blackCaptures, whiteCaptures;

    Board& operator=(const Board &other);

	void doCaptures(Player victim, Move seed);
	bool isSurrounded(Player victim, Player open, int x, int y,
		Stone *visited, MoveList &captured);
};

#endif