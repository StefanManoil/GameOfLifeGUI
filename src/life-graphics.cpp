/*
 * File: life-graphics.cpp
 * ----------------------------------------------------------
 * Implementation of the Life graphics support routines.  We have some
 * internal private helpers that do calculations for cell-size, 
 * colors, and "aging" the cells using shading. You are
 * welcome to read over this code, but you will not need to edit it unless 
 * you're attempting some sort of fancy extension.
 * 
 * This is based on a previous implementation by Julie Zelenski.
 * Multiple changes made by Stefan Manoil
 */

#include <sstream>  // for ostringstream
#include <iomanip>  // for setw, setfill
#include <ios>      // for hex stream manipulator
using namespace std;
#include "random.h" // for randomInteger
#include "strlib.h" // for integerToString
#include "error.h"  // for error
#include "gthread.h"

#include "life-constants.h"
#include "life-graphics.h"
const string LifeDisplay::kDefaultWindowTitle("Game of Life");
const double kWindowPadding = 5; // Margin from border of window to content area

LifeDisplay::LifeDisplay() {
    window = new GWindow(kDisplayWidth, kDisplayHeight);
    //gameGrid;
    initializeColors();
    window->setVisible(true);
    window->setWindowTitle(kDefaultWindowTitle);
    window->setRepaintImmediately(false);
    window->setAutoRepaint(false);
    window->setExitOnClose(true);
    window->setDisplay(this);
}

LifeDisplay::~LifeDisplay() {
    cells.clear();
    window->close();
    delete window;
}

void LifeDisplay::fillCellGrid() {
    cells.clear();
    cells.resize(numRows, numColumns);
    GThread::runOnQtGuiThread([&, this] {
        for (int r = 0; r < numRows; ++r) {
            for (int c = 0; c < numColumns; ++c) {
                cells[r][c] = new GOval(upperLeftX + c * cellDiameter + 1, upperLeftY + r * cellDiameter + 1,
                                        cellDiameter - 2, cellDiameter - 2);
                cells[r][c]->setVisible(false);
                window->add(cells[r][c]); // ownership of memory has been transferred over to window.
            }
        }
    });
}

void LifeDisplay::setDimensions(int numRows, int numColumns) {
    if (numRows <= 0 || numColumns <= 0) {
        error("LifeDisplay::setDimensions number of rows and columns must both be positive!");
    }
    
    this->numRows = numRows;
    this->numColumns = numColumns;
    ages.resize(numRows, numColumns);
    computeGeometry();
    window->clear();
    fillCellGrid();

    window->setColor("White");
    window->fillRect(0, 0, kDisplayWidth, kDisplayHeight);
    window->setColor("Black");
    window->drawRect(upperLeftX, upperLeftY,
                    numColumns * cellDiameter + 1, numRows * cellDiameter + 1);

}

void LifeDisplay::setTitle(const string& title) {
    window->setWindowTitle(title);
    windowTitle = title;
}

void LifeDisplay::drawCellAt(int row, int column, int age) {
    if (!coordinateInRange(row, column)) {
        error(string(__FUNCTION__) + " asked to draw location (" +
              integerToString(row) + ", " + integerToString(column) +
              ") which is outside the bounds of the current simulation.");
    }
    
    if (age < 0) {
        error(string(__FUNCTION__) + " specified a negative age when " +
              "drawing location (" + integerToString(row) + ", " + integerToString(column) + ").");
    }
    
    age = min(age, kMaxAge);
    if (age == 0) {
        cells[row][column]->setVisible(false);
    } else {
        cells[row][column]->setColor(colors[age]);
        cells[row][column]->setFillColor(colors[age]);
        cells[row][column]->setVisible(true);
    }
    ages[row][column] = age;
}

void LifeDisplay::repaint() {
    window->repaint();
}

int LifeDisplay::scalePrimaryColor(int baseContribution, int age) const {
    const int maxContribution = 220;
    int remaining = maxContribution - baseContribution;
    return static_cast<int>(baseContribution + static_cast<double>(age) * remaining / kMaxAge);
}

void LifeDisplay::initializeColors() {
    colors.add("White"); // colors[0] is used for age 0, and is always white
    int baseColor[] = {
        randomInteger(0, 192), randomInteger(0, 192), randomInteger(0, 192)
    };
    
    for (int age = 1; age <= kMaxAge; age++) {
        ostringstream oss;
        oss << "#";
        for (int primary = 0; primary < 3; primary++) {
            int contribution = scalePrimaryColor(baseColor[primary], age);
            oss << setw(2) << setfill('0') << hex << contribution;
        }
        colors.add(oss.str());
    }
}

void LifeDisplay::computeGeometry() {
    double width  = window->getCanvasWidth()  - 2 * kWindowPadding;
    double height = window->getCanvasHeight() - 2 * kWindowPadding;
    double hPixelsPerCell = height / numRows;
    double wPixelsPerCell = width  / numColumns;
    cellDiameter = min(wPixelsPerCell, hPixelsPerCell);
    upperLeftX = kWindowPadding + (width  - numColumns * cellDiameter) / 2;
    upperLeftY = kWindowPadding + (height - numRows    * cellDiameter) / 2;
}

bool LifeDisplay::coordinateInRange(int row, int column) const {
    return row >= 0 && row < numRows && column >= 0 && column < numColumns;
}

void LifeDisplay::printBoard() {
    cout << windowTitle << endl;
    for(int i = 0; i < numRows; ++i) {
        for(int j = 0; j < numColumns; ++j) {
            cout << setw(3) << setfill(' ') << ages[i][j];
        }
        cout << endl;
    }
}

void LifeDisplay::drawBoard() {
    setDimensions(gameGrid.getNumRows(), gameGrid.getNumCols());
    for (int i = 0; i < gameGrid.getNumRows(); i++) {
        for (int j = 0; j < gameGrid.getNumCols(); j++) {
            drawCellAt(i, j, gameGrid.getGrid()[i][j]);
        }
    }
    repaint();
}

void LifeDisplay::advanceBoard() {
    SimulationGrid tempGrid(gameGrid.getNumRows(), gameGrid.getNumCols());
    // Populate the temporary grid according to the rules and the cell positions of the game grid
    // The populating cells will wrap around the grid meaning that given the first row, if there is a cell in the first and last column,
    // they will be considered neighbours. This will be done using mod: rowPos = rowDirection % rowLength,
    // equivalently colPos = colDirection % colLength.
    for (int i = 0; i < gameGrid.getNumRows(); i++) {
        for (int j = 0; j < gameGrid.getNumCols(); j++) {
            int rowPos = 0, colPos = 0, countNeighbours = 0;
            for (const std::pair<int, int>& dir : directions) {
                rowPos = (i + dir.first + gameGrid.getNumRows()) % gameGrid.getNumRows();
                colPos = (j + dir.second + gameGrid.getNumCols()) % gameGrid.getNumCols();
                if (gameGrid.getGrid()[rowPos][colPos] != 0) countNeighbours++;
            }
            // now generate the cells according to rules
            if (countNeighbours <= 1) {
                tempGrid.getGrid()[i][j] = 0;
            }
            else if (countNeighbours == 2) {
                if (gameGrid.getGrid()[i][j] != 0) tempGrid.getGrid()[i][j]++;
            }
            else if (countNeighbours == 3) {
                if (gameGrid.getGrid()[i][j] == 0) {
                    tempGrid.getGrid()[i][j] = 1;
                }
                else {
                    tempGrid.getGrid()[i][j]++;
                }
            }
            else if (countNeighbours >= 4) {
                tempGrid.getGrid()[i][j] = 0;
            }
        }
    }
    gameGrid = tempGrid;
    drawBoard();
}

void LifeDisplay::setMode(const std::string& mode) {
    this->mode = mode;
    if (mode == "1") {
        timerDelay = 3000;
    }
    else if (mode == "2") {
        timerDelay = 1500;
    }
    else if (mode == "3") {
        timerDelay = 800;
    }
}

std::string& LifeDisplay::getMode() {
    return mode;
}

double LifeDisplay::getTimerDelay() const {
    return timerDelay;
}

GWindow* LifeDisplay::getWindow() {
    return window;
}

SimulationGrid& LifeDisplay::getGrid() {
    return gameGrid;
}
