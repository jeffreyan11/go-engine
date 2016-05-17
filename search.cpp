#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <random>
#include "board.h"
#include "mctree.h"
#include "search.h"


struct HistoryTable {
    int data[23][23];
    HistoryTable() {
        for (int i = 0; i < 23; i++)
            for (int j = 0; j < 23; j++)
                data[i][j] = 0;
    }

    void inc(Move m, int depth) {
        if (m == MOVE_PASS)
            return;
        data[getX(m)][getY(m)] += std::max(0, 150 - 5*depth - depth*depth);
    }
    void dec(Move m, int depth) {
        if (m == MOVE_PASS)
            return;
        data[getX(m)][getY(m)] -= std::max(0, 150 - 5*depth - depth*depth);
    }
    void adjust(Move m, int val) {
        data[getX(m)][getY(m)] += val;
    }
    void age() {
        for (int i = 0; i < 23; i++)
            for (int j = 0; j < 23; j++)
                data[i][j] /= 2;
    }
    void reset() {
        for (int i = 0; i < 23; i++)
            for (int j = 0; j < 23; j++)
                data[i][j] = 0;
    }

    int score(Move m) {
        if (m == MOVE_PASS)
            return 0;
        return data[getX(m)][getY(m)];
    }
    int max() {
        int result = -(1 << 30);
        for (int i = 0; i < 23; i++)
            for (int j = 0; j < 23; j++)
                if (data[i][j] > result)
                    result = data[i][j];
        return result;
    }
};


extern int boardSize;
extern bool debugOutput;

Board game;
float komi = 6.5;
int playouts = 1000;
uint64_t keyStack[4096];
int keyStackSize = 0;

HistoryTable raveTable;

std::default_random_engine rng(time(NULL));


void playRandomGame(Player p, Board &b);
void scoreGame(Player p, Board &b, float &myScore, float &oppScore);


Move generateMove(Player p, Move lastMove) {
    MoveList legalMoves = game.getLegalMoves(p);
    MoveList localMoves = game.getLocalMoves(lastMove);

    // Pass if every move is either into your own eye, a suicide, or places
    // a chain into atari
    bool playPass = true;
    for (unsigned int n = 0; n < legalMoves.size(); n++) {
        Board copy = Board(game);
        Move m = legalMoves.get(n);

        if (!copy.isMoveValid(otherPlayer(p), m) && copy.isEye(p, m))
            continue;

        if (!copy.isMoveValid(p, m))
            continue;

        copy.doMove(p, m);
        if (copy.isInAtari(m))
            continue;

        playPass = false;
        break;
    }

    if (playPass)
        return MOVE_PASS;


    MCTree searchTree;
    float komiAdjustment = 0.0;
    Move captureLastStone = game.getPotentialCapture(lastMove);
    Move potentialEscape = game.getPotentialEscape(p, lastMove);

    // Add all first-level moves
    for (unsigned int n = 0; n < legalMoves.size(); n++) {
        Board copy = Board(game);
        Player genPlayer = p;

        Move next = legalMoves.get(n);
        // Check legality of moves (suicide)
        if (!copy.isMoveValid(genPlayer, next))
            continue;

        copy.doMove(genPlayer, next);
        // Never place own chain in atari
        if (copy.isInAtari(next))
            continue;

        // Check for ko rule violation
        bool koViolation = false;
        if (next != MOVE_PASS) {
            uint64_t newKey = copy.getZobristKey();
            for (int i = keyStackSize-1; i >= 0; i--) {
                if (newKey == keyStack[i]) {
                    koViolation = true;
                    break;
                }
            }
        }
        if (koViolation)
            continue;

        // First level moves are added to the root
        MCNode *leaf = searchTree.root;
        MCNode *addition = new MCNode();
        addition->parent = leaf;
        addition->m = next;

        // Play out a random game. The final board state will be stored in copy.
        playRandomGame(otherPlayer(genPlayer), copy);

        // Score the game
        float myScore = 0.0, oppScore = 0.0;
        scoreGame(genPlayer, copy, myScore, oppScore);
        if (myScore > oppScore)
            addition->numerator++;
        addition->scoreDiff = ((int) myScore) - ((int) oppScore);
        komiAdjustment += myScore - oppScore;

        // Add the new node to the tree
        leaf->children[leaf->size] = addition;
        leaf->size++;

        // Backpropagate the results
        searchTree.backPropagate(addition);

        // Do priors, if any
        // Own eye and opening priors inspired by Pachi,
        // written by Petr Baudis and Jean-loup Gailly
        int basePrior = boardSize;
        // Discourage playing into own eyes
        if (game.isEye(genPlayer, next)) {
            addition->denominator += basePrior;
            // If this eye is not ko-related we almost certainly should not play
            // in it
            if (!game.isMoveValid(otherPlayer(genPlayer), next)) {
                addition->denominator += 10 * basePrior;
                addition->scoreDiff -= 10 * 360;
            }
        }

        // Discourage playing onto edges and encourage playing onto the 4th line
        // in 13x13 and 19x19 openings
        unsigned int openingMoves = boardSize * boardSize - boardSize;
        if ((boardSize == 13 || boardSize == 19) && legalMoves.size() > openingMoves) {
            int x = getX(next);
            int y = getY(next);
            if (x == 1 || x == 19 || y == 1 || y == 19) {
                addition->denominator += 2 * basePrior;
            }
            else {
                int taperedPrior = basePrior * (legalMoves.size() - openingMoves) / boardSize;
                if (x == 4 || x == boardSize-3) {
                    addition->numerator += 2 * taperedPrior;
                    addition->denominator += 2 * taperedPrior;
                }
                if (y == 4 || y == boardSize-3) {
                    addition->numerator += 2 * taperedPrior;
                    addition->denominator += 2 * taperedPrior;
                }
                if (x == 3 || x == boardSize-2 || y == 3 || y == boardSize-2) {
                    addition->numerator += taperedPrior;
                    addition->denominator += taperedPrior;
                }
            }
        }
        // And the same for 9x9
        else if (boardSize == 9 && legalMoves.size() > openingMoves) {
            int x = getX(next);
            int y = getY(next);
            if (x == 1 || x == boardSize || y == 1 || y == boardSize) {
                addition->denominator += 2 * basePrior;
            }
            else {
                int taperedPrior = basePrior * (legalMoves.size() - openingMoves) / boardSize;
                if (x == 3 || x == boardSize-2) {
                    addition->numerator += 2 * taperedPrior;
                    addition->denominator += 2 * taperedPrior;
                }
                if (y == 3 || y == boardSize-2) {
                    addition->numerator += 2 * taperedPrior;
                    addition->denominator += 2 * taperedPrior;
                }
            }
        }

        // Add a bonus for capturing a chain that the opponent placed
        // into atari on the previous move
        if (next == captureLastStone) {
            addition->numerator += 5 * basePrior;
            addition->denominator += 5 * basePrior;
        }

        // Add a bonus for escaping when the opponent's last move
        // placed our chain into atari
        if (next == potentialEscape) {
            addition->numerator += 5 * basePrior;
            addition->denominator += 5 * basePrior;
        }

        // Add a bonus to local moves
        if (legalMoves.size() < openingMoves) {
            int li = localMoves.find(next);
            if (li != -1) {
                localMoves.removeFast(li);
                addition->numerator += basePrior;
                addition->denominator += basePrior;
            }
            else {
                addition->numerator += basePrior;
                addition->denominator += 2 * basePrior;
            }
        }
    }

    // If we have no moves that are ko-legal, pass.
    if (searchTree.root->size == 0)
        return MOVE_PASS;

    // Calculate an estimate of a komi adjustment
    komiAdjustment /= legalMoves.size();


    // Expand the MC tree iteratively
    for (int n = 0; n < playouts; n++) {
        Board copy = Board(game);
        Player genPlayer = p;

        // Find a node in the tree to add a child to
        int depth = -1;
        MCNode *leaf = searchTree.findLeaf(genPlayer, copy, depth);

        MCNode *addition = new MCNode();
        addition->parent = leaf;
        MoveList candidates = copy.getLegalMoves(genPlayer);
        candidates.add(MOVE_PASS);

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
        myScore += (genPlayer == p) ? -komiAdjustment : komiAdjustment;

        if (myScore > oppScore) {
            addition->numerator++;
            // If the node is not a child of root
            if (depth)
                raveTable.inc(next, depth);
        }
        else {
            if (depth)
                raveTable.dec(next, depth);
        }
        addition->scoreDiff = ((int) myScore) - ((int) oppScore);

        // Add the new node to the tree
        leaf->children[leaf->size] = addition;
        leaf->size++;

        // Backpropagate the results
        searchTree.backPropagate(addition);
    }


    // Find the highest scoring move
    Move bestMove = searchTree.root->children[0]->m;
    double bestScore = 0.0;
    int64_t diff = -(1 << 30);
    int maxRAVE = raveTable.max();
    for (int i = 0; i < searchTree.root->size; i++) {
        double candidateScore = (double) searchTree.root->children[i]->numerator
                              / (double) searchTree.root->children[i]->denominator;
                            // +   (double) raveTable.score(searchTree.root->children[i]->m)
                            //   / ((double) maxRAVE)
                            //   / (16 + std::sqrt(searchTree.root->children[i]->denominator))
                            // +   (double) searchTree.root->children[i]->scoreDiff
                            //   / (double) (360 * 32);

        // if (candidateScore > bestScore) {
        //     bestScore = candidateScore;
        //     bestMove = searchTree.root->children[i]->m;
        // }

        if (debugOutput) {
            std::cerr << "(" << getX(searchTree.root->children[i]->m) << ", "
                      << getY(searchTree.root->children[i]->m) << "): "
                      << searchTree.root->children[i]->numerator << " / "
                      << searchTree.root->children[i]->denominator << std::endl;
        }

        if (candidateScore > bestScore
         || (candidateScore == bestScore && searchTree.root->children[i]->scoreDiff > diff)) {
            bestScore = candidateScore;
            bestMove = searchTree.root->children[i]->m;
            diff = searchTree.root->children[i]->scoreDiff;
        }
    }

    raveTable.age();

    return bestMove;
}


//------------------------------------------------------------------------------
//-------------------------------MCTS Methods-----------------------------------
//------------------------------------------------------------------------------
void playRandomGame(Player p, Board &b) {
    int movesPlayed = 1;
    int i = 0;
    Move last = MOVE_PASS;

    // Regenerate the movelist up to 4 times
    while (movesPlayed > 0 && i < 4) {
        movesPlayed = 0;
        i++;
        MoveList legalMoves = b.getLegalMoves(p);
        int koCount = 0;

        // While we still have legal moves remaining
        while (legalMoves.size() > 0) {
            // Check if the last move put its own chain into atari
            // Do this at most 3 times in a row to prevent infinite ko recapture
            if (koCount <= 3) {
                Move cap = b.getPotentialCapture(last);
                if (cap != MOVE_PASS) {
                    int ci = legalMoves.find(cap);
                    if (ci != -1)
                        legalMoves.removeFast(ci);
                    
                    b.doMove(p, cap);
                    last = cap;
                    p = otherPlayer(p);
                    movesPlayed++;
                    koCount++;
                    continue;
                }
            }

            // Otherwise, pick a move at random
            std::uniform_int_distribution<int> distribution(0, legalMoves.size()-1);
            int index = distribution(rng);
            Move m = legalMoves.get(index);

            // Only play moves that are not into own eyes and not suicides
            if (!b.isEye(p, m) && b.isMoveValid(p, m)) {
                b.doMove(p, m);
                last = m;
                p = otherPlayer(p);
                movesPlayed++;
                koCount = 0;
            }

            legalMoves.removeFast(index);
        }
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


//------------------------------------------------------------------------------
//---------------------------Other Functions------------------------------------
//------------------------------------------------------------------------------
void resetSearchState() {
    raveTable.reset();
}
