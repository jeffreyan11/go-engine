#ifndef __CHAIN_H__
#define __CHAIN_H__

#include "types.h"

// A node for the linked list chain structure
struct ChainNode {
	Move sq;
	ChainNode *next;

	ChainNode() {
		sq = MOVE_NULL;
		next = NULL;
	}
};

// Represents a chain, an orthogonally contiguous set of stones.
struct Chain {
	int id;
	Player color;
	ChainNode *head;
	ChainNode *tail;
	int size;
	int liberties;

	Chain(Player p);
	~Chain();

	void add(Move m);
};

#endif