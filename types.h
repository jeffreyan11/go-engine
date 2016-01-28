#ifndef __TYPES_H__
#define __TYPES_H__

#include <cstdint>

typedef int Player;
typedef int Stone;
const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;


/*
 * We use a 16-bit unsigned integer to store a move. The x coordinate of the
 * move (0-indexed) is in the upper 8 bits, and the y coordinate is in the
 * lower 8 bits.
 */
typedef uint16_t Move;

inline Move coordToMove(int x, int y) {
	return (Move) ((x << 8) | y);
}

inline int getX(Move m) {
	return (int) (m >> 8);
}

inline int getY(Move m) {
	return (int) (m & 0xFF);
}


/*
 * A basic arraylist implementation for storing movelists, etc.
 */
template <class T>
class GoArrayList {
public:
    T arrayList[512];
    unsigned int length;

    GoArrayList() {
        length = 0;
    }
    ~GoArrayList() {}

    unsigned int size() { return length; }

    void add(T o) {
        arrayList[length] = o;
        length++;
    }

    T get(int i) { return arrayList[i]; }

    void set(int i, T o) { arrayList[i] = o; }
/*
    T remove(int i) {
        T deleted = arrayList[i];
        for(unsigned int j = i; j < length-1; j++) {
            arrayList[j] = arrayList[j+1];
        }
        length--;
        return deleted;
    }

    void swap(int i, int j) {
        T temp = arrayList[i];
        arrayList[i] = arrayList[j];
        arrayList[j] = temp;
    }
*/
    void clear() {
        length = 0;
    }
};

typedef GoArrayList<Move> MoveList;

#endif