#include "board.h"
#include "search.h"

Board game;
float komi = 6.5;


Move generateMove(Player p) {
	MoveList legalMoves = game.getLegalMoves(p);

	return legalMoves.get(0);
}