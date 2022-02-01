/**
 * File: life.cpp
 * --------------
 * Implements the Game of Life.
 */

#include <iostream> // for cout
#include <fstream> // for ifstream
#include <sstream> // for stringstream
#include <random> // for uniform_int_distribution
#include <utility> // for std::pair

#include "console.h" // required of all files that contain the main function
#include "simpio.h" // for getLine
#include "gevents.h" // for mouse event detection
#include "gtimer.h" // for GTimer
#include "strlib.h"

#include "life-constants.h"  // for kMaxAge
#include "life-graphics.h"   // for class LifeDisplay
#include "simulationgrid.h"

/**
 * Function: setupFromFile
 * -----------------
 * Populates a grid by reading the
 */
void setupGrid(const std::string& option, SimulationGrid& startGrid) {
    if (option == "f") {
        std::cout << "Enter the name of the configuration file as files/<filename>. Then press enter." << std::endl;
        std::string filename;
        std::getline(std::cin, filename);
        std::ifstream configFileStream;
        configFileStream.open(filename);
        while (!configFileStream) {
            std::cout << "This file seems to not exist, please enter a different one. Then press enter." << std::endl;
            configFileStream.close();
            std::getline(std::cin, filename);
            configFileStream.open(filename);
        }
        std::string currentLine;
        std::getline(configFileStream, currentLine);
        while (currentLine[0] == '#') {
            std::getline(configFileStream, currentLine);
        }
        int numRows, numCols;
        numRows = std::stoi(currentLine);
        std::getline(configFileStream, currentLine);
        numCols = std::stoi(currentLine);
        startGrid.setGridFieldsEmpty(numRows, numCols);
        for (int i = 0; i < numRows; i++) {
            std::getline(configFileStream, currentLine);
            for (int j = 0; j < numCols; j++) {
                if (currentLine[j] == '-') {
                    startGrid.getGrid()[i][j] = 0;
                }
                else if (currentLine[j] == 'X') {
                    startGrid.getGrid()[i][j] = 1;
                }
            }
        }
        configFileStream.close();
    }
    else { // randomize the grid
        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_int_distribution<> numRowsGenerator(40, 60);
        std::uniform_int_distribution<> numColsGenerator(40, 60);
        std::uniform_int_distribution<> unoccupiedOrOccupied(0, 1);
        std::uniform_int_distribution<> ageGenerator(1, kMaxAge);
        int numRows, numCols;
        numRows = numRowsGenerator(gen);
        numCols = numColsGenerator(gen);
        startGrid.setGridFieldsEmpty(numRows, numCols);
        for (int i = 0; i < numRows; i++) {
            for (int j = 0; j < numCols; j++) {
                if (unoccupiedOrOccupied(gen) == 1) {
                    startGrid.getGrid()[i][j] = ageGenerator(gen);
                }
                else {
                    startGrid.getGrid()[i][j] = 0;
                }
            }
        }
    }
}

/**
 * Function: welcome
 * -----------------
 * Introduces the user to the Game of Life and its rules.
 */
static void welcome(std::string& mode, SimulationGrid& startGrid) {
    std::cout << "Welcome to the game of Life, a simulation of the lifecycle of a bacteria colony." << std::endl;
    std::cout << "Cells live and die by the following rules:" << std::endl << std::endl;
    std::cout << "\tA cell with 1 or fewer neighbors dies of loneliness" << std::endl;
    std::cout << "\tLocations with 2 neighbors remain stable" << std::endl;
    std::cout << "\tLocations with 3 neighbors will spontaneously create life" << std::endl;
    std::cout << "\tLocations with 4 or more neighbors die of overcrowding" << std::endl << std::endl;
    std::cout << "In the animation, new cells are dark and fade to gray as they age." << std::endl << std::endl;
    std::cout << "Type f to choose a starting configuration from a file, or type r for a random one. Then hit enter." << std::endl << std::endl;
    std::cout << "Type m for manual mode, numbers for automatic mode: 1 for low speed, 2 for medium, 3 for high. Then hit enter" << std::endl << std::endl;
    // getLine("Hit [enter] to continue....   ");
    std::string startingOption, startingMode;
    std::getline(std::cin, startingOption);
    while (startingOption != "f" && startingOption != "r") {
        std::cout << "Type f to choose a starting configuration from a file, or type r for a random one. Then hit enter." << std::endl;
        std::getline(std::cin, startingOption);
    }
    std::getline(std::cin, startingMode);
    while (startingMode != "m" && startingMode != "1" && startingMode != "2" && startingMode != "3") {
        std::cout << "Type m for manual mode, numbers for automatic mode: 1 for low speed, 2 for medium, 3 for high. Then hit enter" << std::endl;
        std::getline(std::cin, startingMode);
    }
    mode = startingMode;
    setupGrid(startingOption, startGrid);
}

GEventListener timerRingListener;
GEventListener keyPressedListener;

// this will be what std::function<>
void mouseClickExitWindow(GMouseEvent e) {
    if (e.isLeftClick()) {
        if (e.getInteractor()->getType() == "GCanvas") {
            e.getInteractor()->getWindow()->close();
        }
    }
}

void timerRing(GTimerEvent e) {
    std::cout << "Timer ringing" << std::endl;
    std::cout << e.getSource()->getType() << std::endl;
    e.getSource()->getWindow()->getDisplay()->advanceBoard();
}

void keyPressed(GKeyEvent e) {
    std::cout << "Key Pressed" << std::endl;
    std::cout << e.getInteractor()->getType() << std::endl;
    std::cout << e.getKeyChar() << std::endl;
    if (e.getKeyChar() == 'm') {
        e.getInteractor()->getWindow()->removeTimerListener();
    }
    else if (e.getKeyChar() == '1') {
        e.getInteractor()->getWindow()->removeTimerListener();
        std::string mode = "1";
        e.getInteractor()->getWindow()->getDisplay()->setMode(mode);
        e.getInteractor()->getWindow()->setTimerListener(e.getInteractor()->getWindow()->getDisplay()->getTimerDelay(), timerRingListener);
    }
    else if (e.getKeyChar() == '2') {
        e.getInteractor()->getWindow()->removeTimerListener();
        std::string mode = "2";
        e.getInteractor()->getWindow()->getDisplay()->setMode(mode);
        e.getInteractor()->getWindow()->setTimerListener(e.getInteractor()->getWindow()->getDisplay()->getTimerDelay(), timerRingListener);
    }
    else if (e.getKeyChar() == '3') {
        e.getInteractor()->getWindow()->removeTimerListener();
        std::string mode = "3";
        e.getInteractor()->getWindow()->getDisplay()->setMode(mode);
        e.getInteractor()->getWindow()->setTimerListener(e.getInteractor()->getWindow()->getDisplay()->getTimerDelay(), timerRingListener);
    }
    else if (e.getInteractor()->getWindow()->getDisplay()->getMode() == "m" && (int)e.getKeyChar() == 32) {
        e.getInteractor()->getWindow()->getDisplay()->advanceBoard();
    }
}

/**
 * Function: main
 * --------------
 * Provides the entry point of the entire program.
 */
int main() {
    LifeDisplay display;
    display.setTitle("Game of Life");
    welcome(display.getMode(), display.getGrid());
    display.setMode(display.getMode());
    display.drawBoard();
    display.getWindow()->requestFocus();
    getLine("Hit [enter] to continue....   ");
    // event handling
    timerRingListener = timerRing;
    if (display.getTimerDelay() > 0) {
        display.getWindow()->setTimerListener(display.getTimerDelay(), timerRingListener);
    }
    // mouse left click on window
    // GEventListener mouseClickListener = mouseClickExitWindow;
    // display.getWindow()->setClickListener(mouseClickListener);
    // key pressed on window
    keyPressedListener = keyPressed;
    display.getWindow()->setKeyListener(keyPressedListener);
    getLine("Hit [enter] to continue....   ");
    getLine("Hit [enter] to continue....   ");
    getLine("Hit [enter] to continue....   ");
    return 0;
}
