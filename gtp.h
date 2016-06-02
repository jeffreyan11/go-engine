#ifndef __GTP_H__
#define __GTP_H__

#include <string>
#include "types.h"

using namespace std;


// State variables
extern int boardSize;
extern int arraySize;
extern Board game;
extern float komi;
extern int playouts;
extern uint64_t keyStack[4096];
extern int keyStackSize;

// Constants
const string ENGINE_NAME = "Go Engine";
const string VERSION = "0.0";

const int NUM_KNOWN_COMMANDS = 14;
const string KNOWN_COMMANDS[NUM_KNOWN_COMMANDS] = {
    "play", "genmove",
    "boardsize", "clear_board", "komi", "fixed_handicap",
    "protocol_version", "name", "version", "known_command", "list_commands",
    "showboard", "selfplay",
    "quit"
};

// Column enumeration: A-Z not including I
const string COLUMNS[26] = {"",
    "A", "B", "C", "D", "E", "F", "G", "H", "J", "K",
    "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U",
    "V", "W", "X", "Y", "Z"
};

#endif
