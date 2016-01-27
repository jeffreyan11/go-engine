#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "gtp.h"

using namespace std;


const string ENGINE_NAME = "Go Engine";
const string VERSION = "0.0";


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

		}
		else if (command == "clear_board") {

		}
		else if (command == "komi") {

		}

		// Protocol / information commands
		else if (command == "protocol_version") {
			cout << "= 2" << endl << endl;
		}
		else if (command == "name") {
			cout << "= " << ENGINE_NAME << endl << endl;
		}
		else if (command == "version") {
			cout << "= " << VERSION << endl << endl;
		}
		// TODO implement this with a static list of all commands
		else if (command == "known_command") {

		}
		// TODO implement this with a static list of all commands
		else if (command == "list_commands") {

		}

		else if (command == "quit") {
			cout << "= " << endl << endl;
			break;
		}

		// Otherwise, ignore unknown input?
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