#ifndef __MCTREE_H__
#define __MCTREE_H__

#include "types.h"

struct MCNode {
    int numerator;
    int denominator;
    int64_t scoreDiff;
    Move m;
    int16_t size;
    MCNode *parent;
    MCNode **children;

    MCNode() {
        numerator = 0;
        denominator = 1;
        scoreDiff = 0;
        m = 0;
        size = 0;
        parent = NULL;
        children = new MCNode *[512];
    }

    // Memory management is done in cleanup()
    ~MCNode() {}

    void cleanup() {
        for (int i = 0; i < size; i++) {
            children[i]->cleanup();
            delete children[i];
        }
        delete[] children;
    }
};

struct MCTree {
    MCNode *root;

    MCTree() {
        root = new MCNode();
    }

    ~MCTree() {
        root->cleanup();

        delete root;
    }

    MCNode *findLeaf(Player &p, Board &b, int &depth);
    void backPropagate(MCNode *leaf);
};

#endif
