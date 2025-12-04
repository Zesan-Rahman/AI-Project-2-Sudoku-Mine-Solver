#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::fstream;
using std::iostream;
using std::ofstream;
using std::string;
using std::vector;

enum Assignment { UNASSIGNED, BOMB, EMPTY };

struct Domain {
    bool canBeBomb = true;
    bool cannotBeBomb = true;
};

// How we represent tiles and their information
struct Tile {
    int num = 0;
    Domain domain;
    Assignment assignment = UNASSIGNED;
};

struct Pair {
    int row = 0;
    int col = 0;
};

// For the prio queue, which applies MRV and DH on the tiles.
class TileComp {
public:
    bool operator()(const Tile* lhs, const Tile* rhs) const;

private:
    // needs this because of how degree heuristic works
    vector<vector<Tile*>>* puzzle;
};

// checks if a pair of coords is in the 9x9 grid
bool isInGrid(int row, int col);

// returns a box from 0-8 using coords
int getBoxFromCoords(int row, int col);

// returns a pair of coords to start from based on a box
Pair getStartCoordsFromBox(int box);

// takes a pair of coordinates and
// returns how many non-number tiles adjacent to the coordinate
int degreeHeuristic(int row, int col, vector<vector<Tile*>>& puzzle);

// takes a tile and returns number
// of possibilities that tile has (bomb or not bomb)
int mrv(Tile& tile);

vector<vector<Tile*>> readPuzzle(const string& filePath);

void writeAnswer(vector<vector<Tile*>> puzzle);

// Inference functions
bool forwardChecking(vector<vector<Tile*>>& puzzle);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Run ./solve.o fileName (e.g. ./solve.o input.txt)" << endl;
        return 1;
    }
    vector<vector<Tile*>> puzzle = readPuzzle(argv[1]);
    for (size_t row = 0; row < puzzle.size(); ++row) {
        for (size_t col = 0; col < puzzle[row].size(); ++col) {
            delete puzzle[row][col];
        }
    }
}

vector<vector<Tile*>> readPuzzle(const string& filePath) {
    fstream fd(filePath);
    vector<vector<Tile*>> result;
    if (!fd.is_open()) {
        cout << "Invalid file " << filePath << endl;
        return result;
    }
    char c;
    vector<Tile*> row;
    while (fd >> c) {
        if (c >= '0' && c <= '8') {
            Tile* newTile = new Tile{.num = c - '0'};
            row.push_back(newTile);
        }
        if (row.size() == 9) {
            result.push_back(row);
            row = {};
        }
    }
    return result;
}

bool isInGrid(int row, int col) {
    return !(row < 0 || row > 8 || col < 0 || col > 8);
}

int degreeHeuristic(int row, int col, vector<vector<Tile*>>& puzzle) {
    if (!isInGrid(row, col)) return -1;
    int unassignedVariables = 0;

    // unassigned variables in the row
    for (size_t newCol = 0; newCol < puzzle.size(); ++newCol) {
        if (newCol == col) continue;
        if (puzzle[row][newCol]->assignment == UNASSIGNED) {
            ++unassignedVariables;
        }
    }

    // unassigned variables in the column
    for (size_t newRow = 0; newRow < puzzle.size(); ++newRow) {
        if (newRow == row) continue;
        if (puzzle[newRow][col]->assignment == UNASSIGNED) {
            ++unassignedVariables;
        }
    }

    // unassigned variables in the box
    int box = getBoxFromCoords(row, col);
    Pair startCoords = getStartCoordsFromBox(box);
    for (size_t newRow = startCoords.row; newRow < startCoords.row + 3; ++newRow) {
        for (size_t newCol = startCoords.col; newCol < startCoords.col + 3; ++newCol) {
            if (newRow == row || newCol == col) continue;
            if (puzzle[newRow][col]->assignment == UNASSIGNED) {
                ++unassignedVariables;
            }
        }
    }

    return unassignedVariables;
}

int mrv(Tile& tile) {
    int possibilities = 0;
    if (tile.domain.canBeBomb == true) {
        ++possibilities;
    }
    if (tile.domain.cannotBeBomb == true) {
        ++possibilities;
    }
    return possibilities;
}

int getBoxFromCoords(int row, int col) {
    return row / 3 + (col / 3) * 3;
}

Pair getStartCoordsFromBox(int box) {
    return {.row = (box / 3) * 3, .col = (box % 3) * 3};
}
