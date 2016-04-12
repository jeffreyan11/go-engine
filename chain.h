#ifndef __CHAIN_H__
#define __CHAIN_H__

#include "types.h"

// Represents a chain, an orthogonally contiguous set of stones.
struct Chain {
	int id;
	Player color;
	int size;
	int liberties;
	Move squares[512];
	Move libertyList[256];

	Chain(Player p, int _id);
	Chain(const Chain &other);
	~Chain();

	void add(Move m);
	int findLiberty(Move m);
	void removeLiberty(int index);
};

#endif