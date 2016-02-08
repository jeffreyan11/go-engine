#include "mctree.h"


MCNode *MCTree::findLeaf() {
	return NULL;
}

void MCTree::backPropagate(MCNode *leaf) {
	int n = leaf->numerator;
	int d = leaf->denominator;

	MCNode *node = leaf->parent;
	// Flip whether the game was won or not since each level of the tree is
	// from the POV of the opposing player
	n ^= 1;
	while (node != NULL) {
		node->numerator += n;
		node->denominator += d;
		n ^= 1;
		node = node->parent;
	}
}