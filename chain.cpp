#include "chain.h"


Chain::Chain(Player p, int _id) {
	id = _id;
	color = p;
	size = 0;
	liberties = -1;
}

// Create a deep copy
Chain::Chain(const Chain &other) {
	id = other.id;
	color = other.color;
	size = other.size;
	liberties = other.liberties;
	for (int i = 0; i < size; i++)
		squares[i] = other.squares[i];
	for (int i = 0; i < liberties; i++)
		libertyList[i] = other.libertyList[i];
}

Chain::~Chain() {}

// Adds a square to the square list. Does not handle liberties.
void Chain::add(Move m) {
	squares[size] = m;
	size++;
}

int Chain::findLiberty(Move m) {
	int index = -1;
	for (int i = 0; i < liberties; i++) {
		if (libertyList[i] == m) {
			index = i;
			break;
		}
	}
	return index;
}

void Chain::removeLiberty(int index) {
	libertyList[index] = libertyList[liberties-1];
	liberties--;
}