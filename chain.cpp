#include "chain.h"


Chain::Chain(Player p) {
	id = -1;
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