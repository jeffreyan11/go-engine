#include <ctime>
#include <random>
#include "board.h"
#include "mctree.h"
#include "search.h"

Board game;
float komi = 6.5;

std::default_random_engine rng(time(NULL));


void playRandomGame(Player p, Board &b);


Move generateMove(Player p) {
	MoveList legalMoves = game.getLegalMoves(p);
	MCTree searchTree;

	// Expand the MC tree iteratively
	for (int n = 0; n < 1000; n++) {
		Board copy = Board(game);
		Player genPlayer = p;

		// Find a node in the tree to add a child to
		MCNode *leaf = searchTree.findLeaf(genPlayer, copy);

		MCNode *addition = new MCNode();
		addition->parent = leaf;
		MoveList candidates = copy.getLegalMoves(genPlayer);

		// Set up a permutation matrix
		int *permutation = new int[candidates.size()];
		for (unsigned int i = 0; i < candidates.size(); i++)
			permutation[i] = i;
		// Fisher-Yates shuffle
		for (int i = ((int) candidates.size())-1; i > 0; i--) {
			std::uniform_int_distribution<int> distribution(0, i);
			int j = distribution(rng);
			// Swap i and j
			int temp = permutation[i];
			permutation[i] = permutation[j];
			permutation[j] = temp;
		}

		// Find a random move that has not been explored yet
		Move next = 0;
		for (unsigned int i = 0; i < candidates.size(); i++) {
			next = candidates.get(permutation[i]);

			bool used = false;
			for (int j = 0; j < leaf->size; j++) {
				if (next == leaf->children[j]->m) {
					used = true;
					break;
				}
			}

			if (!used)
				break;
		}

		addition->m = next;
		copy.doMove(genPlayer, next);

		// Play out a random game. The final board state will be stored in copy.
		playRandomGame(otherPlayer(genPlayer), copy);

		// Score the game... somehow...
		int myScore = copy.getCapturedStones(genPlayer);
		int oppScore = copy.getCapturedStones(otherPlayer(genPlayer));
		if (myScore > oppScore)
			addition->numerator++;

		// Add the new node to the tree
		leaf->children[leaf->size] = addition;
		leaf->size++;

		// Backpropagate the results
		searchTree.backPropagate(addition);
	}


	// Find the highest scoring move
	Move bestMove = searchTree.root->children[0]->m;
	double bestScore = (double) searchTree.root->children[0]->numerator
					 / (double) searchTree.root->children[0]->denominator;
	for (int i = 0; i < searchTree.root->size; i++) {
		double candidateScore = (double) searchTree.root->children[i]->numerator
					 		  / (double) searchTree.root->children[i]->denominator;
		if (candidateScore > bestScore) {
			bestScore = candidateScore;
			bestMove = searchTree.root->children[i]->m;
		}
	}

	return bestMove;
}


void playRandomGame(Player p, Board &b) {
	for (int i = 0; i < 100; i++) {
		MoveList legalMoves = b.getLegalMoves(p);
		if (legalMoves.size() == 0)
			break;

		std::uniform_int_distribution<int> distribution(0, legalMoves.size()-1);
		b.doMove(p, legalMoves.get(distribution(rng)));

		p = otherPlayer(p);
	}
}