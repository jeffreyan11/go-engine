#ifndef __GTP_H__
#define __GTP_H__

#include <string>
#include "types.h"

using namespace std;


// State variables
extern int boardSize;
extern Board game;
extern int blackCaptures;
extern int whiteCaptures;
extern float komi;

// Constants
const string ENGINE_NAME = "Go Engine";
const string VERSION = "0.0";

const int NUM_KNOWN_COMMANDS = 12;
const string KNOWN_COMMANDS[NUM_KNOWN_COMMANDS] = {
	"play", "genmove",
	"boardsize", "clear_board", "komi",
	"protocol_version", "name", "version", "known_command", "list_commands",
	"showboard",
	"quit"
};

const string COLUMNS[25] = {
	"A", "B", "C", "D", "E", "F", "G", "H", "J", "K",
	"L", "M", "N", "O", "P", "Q", "R", "S", "T", "U",
	"V", "W", "X", "Y", "Z"
};

#endif