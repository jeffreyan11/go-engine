#ifndef __BOARD_H__
#define __BOARD_H__

#include "types.h"

class Board {
public:
	Board();
	~Board() {};

	void doMove(Player p, Move m);

	void prettyPrint();

private:
	Stone pieces[BOARD_SIZE][BOARD_SIZE];
};

#endif