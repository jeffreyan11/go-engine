#include <algorithm>
#include <cmath>
#include <ctime>
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


Board game;
float komi = 6.5;
int playouts = 1000;
uint64_t keyStack[4096];
int keyStackSize = 0;

HistoryTable raveTable;

std::default_random_engine rng(time(NULL));


void playRandomGame(Player p, Board &b);
void scoreGame(Player p, Board &b, float &myScore, float &oppScore);
void abSearch(int depth, Player p, Board &b, MoveList &legalMoves, ScoreList &scores);
int ab(int depth, Player p, Board &b, int alpha, int beta);


Move generateMove(Player p) {
    MoveList legalMoves = game.getLegalMoves(p);
    MCTree searchTree;

    // Add all first-level moves
    for (unsigned int n = 0; n < legalMoves.size(); n++) {
        Board copy = Board(game);
        Player genPlayer = p;

        Move next = legalMoves.get(n);
        // Check legality of moves (suicide)
        if (!copy.isMoveValid(genPlayer, next))
            continue;

        copy.doMove(genPlayer, next);

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

        // Add the new node to the tree
        leaf->children[leaf->size] = addition;
        leaf->size++;

        // Backpropagate the results
        searchTree.backPropagate(addition);
    }

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
    MoveList empties = b.getLegalMoves(p);
    int gameLength = 10 + empties.size() / 3;
    MoveList legalMoves = b.getLegalMoves(p);

    for (int i = 0; i < gameLength; i++) {
        if (legalMoves.size() <= 1)
            break;

        std::uniform_int_distribution<int> distribution(0, legalMoves.size()-1);
        int index = distribution(rng);
        b.doMove(p, legalMoves.get(index));
        legalMoves.removeFast(index);

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


//------------------------------------------------------------------------------
//-------------------------Standard Search Methods------------------------------
//------------------------------------------------------------------------------
void abSearch(int depth, Player p, Board &b, MoveList &legalMoves, ScoreList &scores) {
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Move m = legalMoves.get(i);
        Board copy = Board(b);
        copy.doMove(p, m);

        scores.add(-ab(depth-1, otherPlayer(p), copy, -65536, 65536));
    }
}

int ab(int depth, Player p, Board &b, int alpha, int beta) {
    // Score the game
    if (depth <= 0) {
        float myScore = 0.0, oppScore = 0.0;
        scoreGame(p, b, myScore, oppScore);
        return ((int) myScore) - ((int) oppScore);
    }

    MoveList legalMoves = b.getLegalMoves(p);
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Move m = legalMoves.get(i);
        Board copy = Board(b);
        if (!copy.isMoveValid(p, m))
            continue;

        copy.doMove(p, m);

        // Check for ko rule violation
        bool koViolation = false;
        if (m != MOVE_PASS) {
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

        int score = -ab(depth-1, otherPlayer(p), copy, -beta, -alpha);

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}


//------------------------------------------------------------------------------
//---------------------------Other Functions------------------------------------
//------------------------------------------------------------------------------
void resetSearchState() {
    raveTable.reset();
}
