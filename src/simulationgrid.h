#ifndef SIMULATIONGRID_H
#define SIMULATIONGRID_H

class SimulationGrid {

public:
    SimulationGrid();
    SimulationGrid(int numRows, int numCols);
    SimulationGrid(int numRows, int numCols, int** grid);
    SimulationGrid(const SimulationGrid& other);
    ~SimulationGrid();
    int getNumRows() const;
    int getNumCols() const;
    int** getGrid() const;
    void setGridFields(int numRows, int numCols, int** grid);
    void setGridFieldsEmpty(int numRows, int numCols);
    void operator=(const SimulationGrid& rhs);
private:
    int numRows;
    int numCols;
    int** grid;
};
#endif // SIMULATIONGRID_H
