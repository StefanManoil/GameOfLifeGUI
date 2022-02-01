#include "simulationgrid.h"

SimulationGrid::SimulationGrid():
    numRows(0), numCols(0), grid(nullptr) {
}

SimulationGrid::SimulationGrid(int numRows, int numCols):
    numRows(numRows), numCols(numCols) {
    int** newGrid = new int*[numRows];
    for (int i = 0; i < numRows; i++) {
        newGrid[i] = new int[numCols];
        for (int j = 0; j < numCols; j++) {
            newGrid[i][j] = 0;
        }
    }
    grid = newGrid;
}

SimulationGrid::SimulationGrid(int numRows, int numCols, int** grid) :
    numRows(numRows), numCols(numCols), grid(grid) {
}

SimulationGrid::SimulationGrid(const SimulationGrid& other):
    numRows(other.getNumRows()), numCols(other.getNumCols()) {
    int** otherGrid = new int*[numRows];
    for (int i = 0; i < numRows; i++) {
        otherGrid[i] = new int[numCols];
        for (int j = 0; j < numCols; j++) {
            otherGrid[i][j] = other.getGrid()[i][j];
        }
    }
    grid = otherGrid;

}

SimulationGrid::~SimulationGrid() {
    for (int i = 0; i < numRows; i++) {
        delete[] grid[i];
    }
    delete[] grid;
}

int SimulationGrid::getNumRows() const {
    return numRows;
}

int SimulationGrid::getNumCols() const {
    return numCols;
}

int** SimulationGrid::getGrid() const {
    return grid;
}

void SimulationGrid::setGridFields(int numRows, int numCols, int **grid) {
    if (this->grid != nullptr) {
        for (int i = 0; i < numRows; i++) {
            delete[] this->grid[i];
        }
        delete[] this->grid;
    }
    this->numRows = numRows;
    this->numCols = numCols;
    this->grid = grid;
}

void SimulationGrid::setGridFieldsEmpty(int numRows, int numCols) {
    if (this->grid != nullptr) {
        for (int i = 0; i < numRows; i++) {
            delete[] this->grid[i];
        }
        delete[] this->grid;
    }
    this->numRows = numRows;
    this->numCols = numCols;
    int** newGrid = new int*[numRows];
    for (int i = 0; i < numRows; i++) {
        newGrid[i] = new int[numCols];
        for (int j = 0; j < numCols; j++) {
            newGrid[i][j] = 0;
        }
    }
    grid = newGrid;
}

void SimulationGrid::operator =(const SimulationGrid& rhs) {
    if (this->grid != nullptr) {
        for (int i = 0; i < numRows; i++) {
            delete[] grid[i];
        }
        delete[] grid;
    }
    this->numRows = rhs.numRows;
    this->numCols = rhs.numCols;
    int** otherGrid = new int*[numRows];
    for (int i = 0; i < numRows; i++) {
        otherGrid[i] = new int[numCols];
        for (int j = 0; j < numCols; j++) {
            otherGrid[i][j] = rhs.getGrid()[i][j];
        }
    }
    grid = otherGrid;
}
