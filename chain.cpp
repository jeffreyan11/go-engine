#include "chain.h"


Chain::Chain(Player p, int _id) {
	id = _id;
	color = p;
	head = NULL;
	tail = NULL;
	size = 0;
	liberties = -1;
}

Chain::~Chain() {
	ChainNode *node = head;
	while (node != NULL) {
		ChainNode *next = node->next;
		delete node;
		node = next;
	}
}

// Adds a square to the linked list. Does not handle liberties.
void Chain::add(Move m) {
	ChainNode *node = new ChainNode();
	node->sq = m;

	if (head == NULL)
		head = node;
	else
		tail->next = node;

	tail = node;

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