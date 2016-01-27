#ifndef __BOARD_H__
#define __BOARD_H__

#include "types.h"

class Board {
public:
	Board();
	~Board() {};
	
private:
	Stone pieces[BOARD_SIZE][BOARD_SIZE];
};

#endif