#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "board.h"
#include "gtp.h"
#include "search.h"

using namespace std;


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


vector<string> split(const string &s, char d);


int main(int argc, char **argv) {
	string input;

	while (true) {
		getline(cin, input);
		// TODO tab can also be a delimiter
		vector<string> inputVector = split(input, ' ');
		cin.clear();

		// The command is the first word in the string
		string command = inputVector.at(0);


		// Gameplay commands
		if (command == "play") {

		}

		else if (command == "genmove") {

		}


		// Game setup commands
		// TODO changes board size
		else if (command == "boardsize") {
			int inputSize = stoi(inputVector.at(1));
			if (inputSize < 3 || inputSize > 21)
				cout << "? unacceptable size" << endl << endl;
			else {
				boardSize = inputSize;
				game.reset();
				cout << "= " << endl << endl;
			}
		}

		else if (command == "clear_board") {
			game.reset();
			blackCaptures = 0;
			whiteCaptures = 0;
			// TODO must also reset other state variables
			cout << "= " << endl << endl;
		}

		else if (command == "komi") {
			float inputKomi = stof(inputVector.at(1));
			komi = inputKomi;
			cout << "= " << endl << endl;
		}


		// Protocol / information commands
		else if (command == "protocol_version")
			cout << "= 2" << endl << endl;
		else if (command == "name")
			cout << "= " << ENGINE_NAME << endl << endl;
		else if (command == "version")
			cout << "= " << VERSION << endl << endl;
		// TODO implement this with a static list of all commands
		else if (command == "known_command") {

		}
		else if (command == "list_commands") {
			cout << "= " << endl;
			for (int i = 0; i < NUM_KNOWN_COMMANDS; i++) {
				cout << KNOWN_COMMANDS[i] << endl;
			}
			cout << endl;
		}


		// Debugging commands
		else if (command == "showboard") {
			cout << "= " << endl;
			game.prettyPrint();
		}


		else if (command == "quit") {
			cout << "= " << endl << endl;
			break;
		}


		// Otherwise, give an error message indicating an unknown command
		else {
			cout << "? unknown command" << endl << endl;
		}
	}

	return 0;
}


vector<string> split(const string &s, char d) {
    vector<string> v;
    stringstream ss(s);
    string item;
    while (getline(ss, item, d)) {
        v.push_back(item);
    }
    return v;
}