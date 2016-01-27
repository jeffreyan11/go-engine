#ifndef __BOARD_H__
#define __BOARD_H__

#include "types.h"

class Board {
public:
	Board();
	~Board();

	void doMove(Player p, Move m);

	void reset();
	void prettyPrint();

private:
    //Board(const Board &other);
    Board& operator=(const Board &other);

	Stone *pieces;
};

#endif