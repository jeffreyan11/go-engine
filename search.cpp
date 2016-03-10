#include <ctime>
#include <random>
#include "board.h"
#include "mctree.h"
#include "search.h"

Board game;
float komi = 6.5;

std::default_random_engine rng(time(NULL));


void playRandomGame(Player p, Board &b);
void scoreGame(Player p, Board &b, float &myScore, float &oppScore);


Move generateMove(Player p) {
	MoveList legalMoves = game.getLegalMoves(p);
	MCTree searchTree;

	// Add pass move
	if (legalMoves.size() < 300) {
		Board copy = Board(game);
		Player genPlayer = p;

		MCNode *leaf = searchTree.root;
		Move next = MOVE_PASS;

		MCNode *addition = new MCNode();
		addition->parent = leaf;
		addition->m = next;
		copy.doMove(genPlayer, next);

		// Play out a random game. The final board state will be stored in copy.
		playRandomGame(otherPlayer(genPlayer), copy);

		// Score the game... somehow...
		float myScore = 0.0, oppScore = 0.0;
		scoreGame(genPlayer, copy, myScore, oppScore);
		if (myScore > oppScore)
			addition->numerator++;

		// Add the new node to the tree
		leaf->children[leaf->size] = addition;
		leaf->size++;

		// Backpropagate the results
		searchTree.backPropagate(addition);
	}

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
		// Fisher-Yates shuffle
		for (unsigned int i = 0; i < candidates.size(); i++) {
			std::uniform_int_distribution<int> distribution(0, i);
			int j = distribution(rng);
            permutation[i] = permutation[j];
            permutation[j] = i;
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

			if (!used && copy.isMoveValid(genPlayer, next))
				break;
		}

		delete[] permutation;

		addition->m = next;
		copy.doMove(genPlayer, next);

		// Play out a random game. The final board state will be stored in copy.
		playRandomGame(otherPlayer(genPlayer), copy);

		// Score the game... somehow...
		float myScore = 0.0, oppScore = 0.0;
		scoreGame(genPlayer, copy, myScore, oppScore);
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
	MoveList empties = b.getLegalMoves(p);
	int gameLength = 10 + empties.size() / 3;
    MoveList legalMoves = b.getLegalMoves(p);

	for (int i = 0; i < gameLength; i++) {
		if (legalMoves.size() <= 1)
			break;

		std::uniform_int_distribution<int> distribution(0, legalMoves.size()-1);
        int index = distribution(rng);
		b.doMove(p, legalMoves.get(index));
        legalMoves.remove(index);

		p = otherPlayer(p);
	}
}

void scoreGame(Player p, Board &b, float &myScore, float &oppScore) {
	int whiteTerritory = 0, blackTerritory = 0;
	b.countTerritory(whiteTerritory, blackTerritory);

	myScore = b.getCapturedStones(p)
		+ ((p == BLACK) ? blackTerritory : whiteTerritory);
	oppScore = b.getCapturedStones(otherPlayer(p))
		+ ((p == BLACK) ? whiteTerritory : blackTerritory);
	if (p == WHITE)
		myScore += komi;
	else
		oppScore += komi;
}
