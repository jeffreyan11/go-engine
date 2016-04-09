#include "chain.h"


Chain::Chain(Player p, int _id) {
	id = _id;
	color = p;
	head = NULL;
	tail = NULL;
	size = 0;
	liberties = -1;
}

// Create a deep copy
Chain::Chain(const Chain &other) {
	id = other.id;
	color = other.color;
	size = other.size;
	liberties = other.liberties;
	for (int i = 0; i < size; i++) {
		libertyList[i] = other.libertyList[i];
	}

	// Deep copy linked list
    if (other.head == NULL) {
        head = NULL;
        tail = NULL;
    }
    else {
	    ChainNode *onode = other.head;

	    head = new ChainNode();
	    head->sq = onode->sq;

	    ChainNode *node = head;
	    onode = onode->next;

	    // Throughout this loop onode should stay one ahead of node in the list
	    while (onode != NULL) {
		    ChainNode *toAdd = new ChainNode();
		    toAdd->sq = onode->sq;
		    node->next = toAdd;

		    node = node->next;
		    onode = onode->next;
	    }

	    tail = node;
    }
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
