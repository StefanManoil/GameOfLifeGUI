/**
 * File: life.cpp
 * --------------
 * Implements the Game of Life.
 * @author Stefan Manoil
 */

#include <iostream> // for cout
#include <fstream> // for ifstream
#include <sstream> // for stringstream
#include <random> // for uniform_int_distribution
#include <utility> // for std::pair

#include "console.h" // required of all files that contain the main function
#include "simpio.h" // for getLine
#include "gevents.h" // for event detection
#include "gbutton.h" // for GButton
#include "gslider.h" // for GSlider
#include "strlib.h"

#include "life-constants.h"  // for kMaxAge
#include "life-graphics.h"   // for class LifeDisplay

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
static void welcome(SimulationGrid& startGrid) {
    std::cout << "Welcome to the game of Life, a simulation of the lifecycle of a bacteria colony." << std::endl;
    std::cout << "Cells live and die by the following rules:" << std::endl << std::endl;
    std::cout << "\tA cell with 1 or fewer neighbors dies of loneliness" << std::endl;
    std::cout << "\tLocations with 2 neighbors remain stable" << std::endl;
    std::cout << "\tLocations with 3 neighbors will spontaneously create life" << std::endl;
    std::cout << "\tLocations with 4 or more neighbors die of overcrowding" << std::endl << std::endl;
    std::cout << "In the animation, new cells are dark and fade to gray as they age." << std::endl << std::endl;
    std::cout << "Type f to choose a starting configuration from a file, or type r for a random one. Then hit enter." << std::endl << std::endl;
    std::string startingOption;
    std::getline(std::cin, startingOption);
    while (startingOption != "f" && startingOption != "r") {
        std::cout << "Type f to choose a starting configuration from a file, or type r for a random one. Then hit enter." << std::endl;
        std::getline(std::cin, startingOption);
    }
    setupGrid(startingOption, startGrid);
}

void timerRing(GTimerEvent e) {
    std::cout << "Timer ringing" << std::endl;
    std::cout << e.getSource()->getType() << std::endl;
    LifeDisplay* display = e.getSource()->getWindow()->getDisplay();
    SimulationGrid* currentGrid = new SimulationGrid(display->getGrid());
    display->getUndoButtonStack().pushGrid(currentGrid);
    display->advanceBoard();
}

void advanceGenerationBtnPressed(GActionEvent e) {
    std::cout << e.getInteractor()->getType() << std::endl;
    LifeDisplay* display = e.getInteractor()->getWindow()->getDisplay();
    SimulationGrid* currentGrid = new SimulationGrid(display->getGrid());
    display->getUndoButtonStack().pushGrid(currentGrid);
    display->advanceBoard();
    for (GInteractor* interactor: e.getInteractor()->getContainer()->getInteractors()) {
        if (interactor->getName() == "<=") {
            interactor->setEnabled(true);
        }
    }
}

void reverseGenerationBtnPressed(GActionEvent e) {
    std::cout << e.getInteractor()->getType() << std::endl;
    LifeDisplay* display = e.getInteractor()->getWindow()->getDisplay();
    SimulationGrid* previousGrid = display->getUndoButtonStack().popGrid();
    if (display->getUndoButtonStack().getStackSize() == 0) e.getInteractor()->setEnabled(false);
    display->reverseBoard(*previousGrid);
    delete previousGrid;
}

void sliderSettingChanged(GActionEvent e) {
    std::cout << e.getInteractor()->getType() << std::endl;
    LifeDisplay* display = e.getInteractor()->getWindow()->getDisplay();
    GWindow* window = e.getInteractor()->getWindow();
    const GSlider* slider = e.getInteractor()->getSlider();
    if (display->getMode() != "m") {
        window->removeTimerListener(display->getTimerDelay());
    }
    int sliderValue = slider->getValue();
    std::string mode;
    if (sliderValue > 1) {
        //GEventListener timerRingListener = timerRing;
        if (sliderValue == 2) mode = "1";
        else if (sliderValue == 3) mode = "2";
        else if (sliderValue == 4) mode = "3";
        display->setMode(mode);
        window->setTimerListener(display->getTimerDelay(), timerRing);
    }
    else {
        mode = "m";
        display->setMode(mode);
    }
}

void manualOrAutoBtnPressed(GActionEvent e) {
    if (e.getInteractor()->getActionCommand() == ">") {
        std::string pauseText = "||";
        e.getInteractor()->setActionCommand(pauseText);
        GButton* button = e.getInteractor()->getButton();
        button->setText(pauseText);
        for (GInteractor* interactor: e.getInteractor()->getContainer()->getInteractors()) {
            if (interactor->getName() == "=>" || interactor->getName() == "<=") {
                interactor->setEnabled(false);
            }
            else if (interactor->getName() == "diffSpeeds") {
                interactor->setEnabled(true);
                const GSlider* slider = interactor->getSlider();
                int sliderValue = slider->getValue();
                if (sliderValue > 1) {
                    //GEventListener timerRingListener = timerRing;
                    std::string mode;
                    if (sliderValue == 2) mode = "1";
                    else if (sliderValue == 3) mode = "2";
                    else if (sliderValue == 4) mode = "3";
                    LifeDisplay* display = e.getInteractor()->getWindow()->getDisplay();
                    GWindow* window = e.getInteractor()->getWindow();
                    display->setMode(mode);
                    window->setTimerListener(display->getTimerDelay(), timerRing);
                }
            }
        }
    }
    else if (e.getInteractor()->getActionCommand() == "||") {
        LifeDisplay* display = e.getInteractor()->getWindow()->getDisplay();
        GWindow* window = e.getInteractor()->getWindow();
        std::string playText = ">";
        e.getInteractor()->setActionCommand(playText);
        GButton* button = e.getInteractor()->getButton();
        button->setText(playText);
        for (GInteractor* interactor: e.getInteractor()->getContainer()->getInteractors()) {
            if (interactor->getName() == "=>") {
                interactor->setEnabled(true);
            }
            else if (interactor->getName() == "<=") {
                if (display->getUndoButtonStack().getStackSize() > 0) {
                    interactor->setEnabled(true);
                }
                else {
                    interactor->setEnabled(false);
                }
            }
            else if (interactor->getName() == "diffSpeeds") {
                interactor->setEnabled(false);
            }
        }
        if (display->getMode() != "m") {
            window->removeTimerListener(display->getTimerDelay());
        }
        std::string mode = "m";
        display->setMode(mode);
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
    welcome(display.getGrid());

    std::string advanceGenerationText = "=>";
    GButton advanceGenerationBtn(advanceGenerationText);
    advanceGenerationBtn.setHeight(20.0);
    advanceGenerationBtn.setWidth(50.0);
    advanceGenerationBtn.setWindow(display.getWindow());
    GInteractor* interactorAdvanceGenerationBtn = &advanceGenerationBtn;
    interactorAdvanceGenerationBtn->setName(advanceGenerationText);
    std::string reverseGenerationText = "<=";
    GButton reverseGenerationBtn(reverseGenerationText);
    reverseGenerationBtn.setHeight(20.0);
    reverseGenerationBtn.setWidth(50.0);
    reverseGenerationBtn.setWindow(display.getWindow());
    GInteractor* interactorReverseGenerationBtn = &reverseGenerationBtn;
    interactorReverseGenerationBtn->setName(reverseGenerationText);
    std::string manualOrAutoModeString = ">";
    GButton manualOrAutoModeBtn(manualOrAutoModeString);
    manualOrAutoModeBtn.setHeight(20.0);
    manualOrAutoModeBtn.setWidth(50.0);
    manualOrAutoModeBtn.setWindow(display.getWindow());
    GInteractor* interactorManualOrAutoModeBtn = &manualOrAutoModeBtn;
    interactorManualOrAutoModeBtn->setName(manualOrAutoModeString);
    std::string diffAdvanceSpeedsString = "diffSpeeds";
    GSlider diffAdvanceSpeeds(1, 4, 1);
    diffAdvanceSpeeds.setHeight(50.0);
    diffAdvanceSpeeds.setWidth(50.0);
    diffAdvanceSpeeds.setWindow(display.getWindow());
    GInteractor* interactorDiffAdvanceSpeeds = &diffAdvanceSpeeds;
    interactorDiffAdvanceSpeeds->setName(diffAdvanceSpeedsString);

    display.getWindow()->addButton(interactorReverseGenerationBtn);
    display.getWindow()->addButton(interactorManualOrAutoModeBtn);
    display.getWindow()->addButton(interactorAdvanceGenerationBtn);
    display.getWindow()->addButton(interactorDiffAdvanceSpeeds);

    std::string mode = "m";
    display.setMode(mode);
    advanceGenerationBtn.setEnabled(true);
    reverseGenerationBtn.setEnabled(false);
    manualOrAutoModeBtn.setEnabled(true);
    diffAdvanceSpeeds.setEnabled(false);
    // GEventListener advanceGenerationBtnListener = advanceGenerationBtnPressed;
    // GEventListener manualOrAutoBtnListener = manualOrAutoBtnPressed;
    // GEventListener reverseGenerationBtnListener = reverseGenerationBtnPressed;
    // GEventListener sliderSettingChangedListener = sliderSettingChanged;
    advanceGenerationBtn.setActionListener(advanceGenerationBtnPressed);
    manualOrAutoModeBtn.setActionListener(manualOrAutoBtnPressed);
    reverseGenerationBtn.setActionListener(reverseGenerationBtnPressed);
    diffAdvanceSpeeds.setActionListener(sliderSettingChanged);
    // diffAdvanceSpeeds.setActionListener(sliderSettingChangedListener);

    display.drawBoard();
    display.getWindow()->requestFocus();
    getLine("Hit [enter] to continue....   ");
    return 0;
}

