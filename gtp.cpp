#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>
#include "board.h"
#include "gtp.h"
#include "search.h"


Player stringToColor(string colorString);
vector<string> split(const string &s, char d);


bool debugOutput = false;


int main(int argc, char **argv) {
    // Parse command line arguments and flags with little error checking...
    if (argc == 2) {
        if (argv[1][0] == '-') {
            debugOutput = true;
        }
        else {
            playouts = stoi(string(argv[1]));
        }
    }
    else if (argc == 3) {
        debugOutput = true;

        if (argv[1][0] != '-') {
            playouts = stoi(string(argv[1]));
        }
        else {
            playouts = stoi(string(argv[2]));
        }
    }

    // Do necessary initializations
    initZobristTable();

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
            Player p = stringToColor(inputVector.at(1));

            if (p != EMPTY) {
                string moveString = inputVector.at(2);

                // We don't do anything to the board for passes
                if (moveString != "pass") {
                    char fileChar = moveString[0];
                    int file = 0;
                    if (fileChar >= 'A' && fileChar <= 'Z') {
                        // Board coordinates are 1-indexed
                        file = fileChar - 'A' + 1;
                        // The I character is skipped
                        if (fileChar > 'I')
                            file--;
                    }
                    else {
                        file = fileChar - 'a' + 1;
                        if (fileChar > 'i')
                            file--;
                    }

                    int rank = stoi(moveString.substr(1));
                    Move inputMove = coordToMove(file, rank);

                    keyStack[keyStackSize] = game.getZobristKey();
                    keyStackSize++;
                    game.doMove(p, inputMove);

                    if (debugOutput) {
                        cerr << endl << "   ";
                        for (int i = 1; i <= boardSize; i++)
                            cerr << COLUMNS[i] << " ";
                        cerr << endl;

                        game.errorPrint();

                        cerr << "   ";
                        for (int i = 1; i <= boardSize; i++)
                            cerr << COLUMNS[i] << " ";
                        cerr << endl;
                    }
                }

                cout << "= " << endl << endl;
            }
            else
                cout << "? invalid color" << endl << endl;
        }

        else if (command == "genmove") {
            Player p = stringToColor(inputVector.at(1));

            if (p != EMPTY) {
                auto startTime = std::chrono::high_resolution_clock::now();
                Move m = generateMove(p);
                if (debugOutput) {
                    auto endTime = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> timeSpan =
                            std::chrono::duration_cast<std::chrono::duration<double>>(
                            endTime-startTime);
                    cerr << "genmove took: " << timeSpan.count() << " sec" << endl;
                }

                if (m == MOVE_PASS)
                    cout << "= pass" << endl << endl;
                else {
                    keyStack[keyStackSize] = game.getZobristKey();
                    keyStackSize++;
                    game.doMove(p, m);

                    if (debugOutput) {
                        cerr << endl << "   ";
                        for (int i = 1; i <= boardSize; i++)
                            cerr << COLUMNS[i] << " ";
                        cerr << endl;

                        game.errorPrint();

                        cerr << "   ";
                        for (int i = 1; i <= boardSize; i++)
                            cerr << COLUMNS[i] << " ";
                        cerr << endl;
                    }

                    cout << "= " << COLUMNS[getX(m)] << getY(m) << endl << endl;
                }
            }
            else
                cout << "? invalid color" << endl << endl;
        }


        // Game setup commands
        else if (command == "boardsize") {
            int inputSize = stoi(inputVector.at(1));
            if (inputSize < 3 || inputSize > 21)
                cout << "? unacceptable size" << endl << endl;
            else {
                boardSize = inputSize;
                arraySize = inputSize + 2;
                game.reset();
                resetSearchState();
                cout << "= " << endl << endl;
            }
        }

        else if (command == "clear_board") {
            game.reset();
            resetSearchState();
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
        else if (command == "known_command") {
            bool isKnown = false;
            for (int i = 0; i < NUM_KNOWN_COMMANDS; i++) {
                if (KNOWN_COMMANDS[i] == inputVector.at(1)) {
                    isKnown = true;
                    break;
                }
            }

            if (isKnown)
                cout << "= true" << endl << endl;
            else
                cout << "= false" << endl << endl;
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
            cout << "   ";
            for (int i = 1; i <= boardSize; i++)
                cout << COLUMNS[i] << " ";
            cout << endl;

            game.prettyPrint();

            cout << "   ";
            for (int i = 1; i <= boardSize; i++)
                cout << COLUMNS[i] << " ";
            cout << endl << endl;
        }

        else if (command == "selfplay") {
            Player p = BLACK;
            Move last = 0;
            Move m = 0;

            while (last != MOVE_PASS || m != MOVE_PASS) {
                last = m;
                auto startTime = std::chrono::high_resolution_clock::now();
                m = generateMove(p);
                if (debugOutput) {
                    auto endTime = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> timeSpan =
                            std::chrono::duration_cast<std::chrono::duration<double>>(
                            endTime-startTime);
                    cerr << "genmove took: " << timeSpan.count() << " sec" << endl;
                }

                if (m != MOVE_PASS) {
                    keyStack[keyStackSize] = game.getZobristKey();
                    keyStackSize++;
                    game.doMove(p, m);
                }

                p = otherPlayer(p);
            }

            cout << "= " << endl << endl;
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


//------------------------------------------------------------------------------
//-----------------------------Parser Helpers-----------------------------------
//------------------------------------------------------------------------------

// Convert a GTP string input into a Player color
Player stringToColor(string colorString) {
    if (colorString == "black" || colorString == "b" || colorString == "B")
        return BLACK;
    else if (colorString == "white" || colorString == "w" || colorString == "W")
        return WHITE;
    else
        return EMPTY;
}


// Split string s based on delimiter d
vector<string> split(const string &s, char d) {
    vector<string> v;
    stringstream ss(s);
    string item;
    while (getline(ss, item, d)) {
        v.push_back(item);
    }
    return v;
}
