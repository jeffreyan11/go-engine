#include <random>
#include "board.h"
#include "mctree.h"


// Pseudo-randomize tree deterministically
std::default_random_engine mc_rng;
std::uniform_int_distribution<int> distribution(0, 50);


// Finds a node to attach a new branch to, and updates a board to the
// corresponding position
MCNode *MCTree::findLeaf(Player &p, Board &b) {
	MCNode *node = root;

	// Keep going until we either decide to split another child, or find a leaf
	while (node->size > 0) {
		// Create a new child for this node with some probability
		if (distribution(mc_rng) > node->size)
			break;

		// Otherwise, choose a child to follow
		// TODO currently fully random
		std::uniform_int_distribution<int> d(0, node->size-1);
		node = node->children[d(mc_rng)];
		b.doMove(p, node->m);
		p = otherPlayer(p);
	}

	return node;
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