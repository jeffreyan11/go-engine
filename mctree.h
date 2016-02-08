#ifndef __MCTREE_H__
#define __MCTREE_H__

#include "types.h"

struct MCNode {
	int numerator;
	int denominator;
	Move m;
	int16_t size;
	MCNode *parent;
	MCNode **children;

	MCNode() {
		numerator = 0;
		denominator = 0;
		m = 0;
		size = 0;
		parent = NULL;
		children = new MCNode *[512];
	}

	~MCNode() {
		delete[] children;
	}
};

struct MCTree {
	MCNode *root;

	MCTree() {
		root = new MCNode();
	}

	~MCTree() {
		delete root;
	}

	MCNode *findLeaf();
	void backPropagate(MCNode *leaf);
};

#endif