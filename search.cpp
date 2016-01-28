#include "board.h"
#include "search.h"

Board game;
int blackCaptures = 0;
int whiteCaptures = 0;
float komi = 6.5;


Move generateMove(Player p) {
	MoveList legalMoves = game.getLegalMoves(p);

	return legalMoves.get(0);
}