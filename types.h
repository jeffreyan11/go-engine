#ifndef __TYPES_H__
#define __TYPES_H__

#include <cstdint>

typedef int Player;
typedef int Stone;
const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;

/*
 * We use a 32-bit unsigned integer to store a move. The x coordinate of the
 * move (0-indexed) is in the upper 16 bits, and the y coordinate is in the
 * lower 16 bits.
 */
typedef uint32_t Move;

inline Move coordToMove(int x, int y) {
	return (Move) ((x << 16) | y);
}

inline int getX(Move m) {
	return (int) (m >> 16);
}

inline int getY(Move m) {
	return (int) (m & 0xFFFF);
}

#endif