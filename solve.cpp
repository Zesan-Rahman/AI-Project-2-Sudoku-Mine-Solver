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
    bool canBeEmpty = true;
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

class NumberedTileHelper {
public:
    void addTile(int row, int col) {
        if (puzzle[row][col] != 1) {
            ++unassignedTiles;
            puzzle[row][col] = 1;
        }
    }
    int getUnassignedTiles() {
        return unassignedTiles;
    }

private:
    vector<vector<int>> puzzle = vector<vector<int>>(9, vector<int>(9, 0));
    int unassignedTiles = 0;
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

bool isPuzzleConsistent(vector<vector<Tile*>>& puzzle);

// checks row of tile and returns the domain
Domain getRowDomain(int row, int col, vector<vector<Tile*>>& puzzle);

// checks col of tile and returns the domain
Domain getColDomain(int row, int col, vector<vector<Tile*>>& puzzle);

// checks box of tile and returns the domain
Domain getBoxDomain(int row, int col, vector<vector<Tile*>>& puzzle);

// checks numbers adjacent to tile and returns the domain
Domain getNumbersDomain(int row, int col, vector<vector<Tile*>>& puzzle);

// checks a single numbered tile and returns the domain that the original tile should be in
Domain getSingleNumberDomain(int row, int col, int numRow, int numCol, vector<vector<Tile*>>& puzzle);

// combines a domain by doing && on both fields
void combineDomains(Domain& lhs, const Domain& rhs);

// checks if a numbered tile is consistent with its surrounding tiles
bool isNumberedTileConsistent(int row, int col, vector<vector<Tile*>>& puzzle);

vector<vector<Tile*>> readPuzzle(const string& filePath);

void writeAnswer(vector<vector<Tile*>>& puzzle);

bool backtrackingSearch(vector<vector<Tile*>>& puzzle);

bool isInRow(int row, int col, int newRow, int newCol);

bool isInCol(int row, int col, int newRow, int newCol);

bool isInBox(int row, int col, int newRow, int newCol);

// Inference functions
bool forwardChecking(vector<vector<Tile*>>& puzzle);

// chooses the next best tile
Tile* selectUnassignedVariable(vector<vector<Tile*>>& puzzle);

// sees if there are any unassigned tiles in the puzzle
bool isPuzzleComplete(vector<vector<Tile*>>& puzzle);

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
            // prevents recounting variables in the same row, column, and also the current tile
            if (newRow == row || newCol == col) continue;
            if (puzzle[newRow][col]->assignment == UNASSIGNED) {
                ++unassignedVariables;
            }
        }
    }

    // unassigned variables related to adjacent number tiles
    NumberedTileHelper helper;
    for (int rowOffset = -1; rowOffset < 2; ++rowOffset) {
        for (int colOffset = -1; colOffset < 2; ++colOffset) {
            if (rowOffset == 0 && colOffset == 0) continue;
            if (!isInGrid(row + rowOffset, col + colOffset)) {
                continue;
            }

            // get adjacent tiles and see if they are numbered
            Tile* tile = puzzle[row + rowOffset][col + colOffset];
            if (tile->num == 0) continue;

            // get the tiles adjacent to the numbered tile and see if they are unassigned
            for (int numRowOffset = -1; numRowOffset < 2; ++numRowOffset) {
                for (int numColOffset = -1; numColOffset < 2; ++numColOffset) {
                    // don't add dupes
                    if (numRowOffset == 0 && numColOffset == 0) continue;
                    int currRow = row + rowOffset + numRowOffset;
                    int currCol = col + colOffset + numColOffset;
                    if (!isInGrid(currRow, currCol)) continue;
                    if (currRow == row && currCol == col) continue;
                    if (isInRow(row, col, currRow, currCol)) continue;
                    if (isInCol(row, col, currRow, currCol)) continue;
                    if (isInBox(row, col, currRow, currCol)) continue;

                    Tile* tile = puzzle[row + numRowOffset][col + numColOffset];
                    if (tile->assignment == UNASSIGNED && tile->num == 0) {
                        helper.addTile(currRow, currCol);
                    }
                }
            }
        }
    }
    unassignedVariables += helper.getUnassignedTiles();

    return unassignedVariables;
}

int mrv(Tile& tile) {
    int possibilities = 0;
    if (tile.domain.canBeBomb == true) {
        ++possibilities;
    }
    if (tile.domain.canBeEmpty == true) {
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

bool isPuzzleConsistent(vector<vector<Tile*>>& puzzle) {
    for (size_t row = 0; row < puzzle.size(); ++row) {
        for (size_t col = 0; col < puzzle[row].size(); ++row) {
            Tile* currTile = puzzle[row][col];
            if (currTile->assignment != UNASSIGNED) {  // implies num == 0
                Domain updatedDomain;
                combineDomains(updatedDomain, getRowDomain(row, col, puzzle));
                combineDomains(updatedDomain, getColDomain(row, col, puzzle));
                combineDomains(updatedDomain, getBoxDomain(row, col, puzzle));
                combineDomains(updatedDomain, getNumbersDomain(row, col, puzzle));

                if (currTile->assignment == BOMB && updatedDomain.canBeBomb == false) {
                    return true;
                }
                if (currTile->assignment == EMPTY && updatedDomain.canBeEmpty == false) {
                    return true;
                }
                continue;
            }

            if (currTile->assignment == UNASSIGNED && currTile->num == 0) {
                Domain updatedDomain;
                combineDomains(updatedDomain, getRowDomain(row, col, puzzle));
                combineDomains(updatedDomain, getColDomain(row, col, puzzle));
                combineDomains(updatedDomain, getBoxDomain(row, col, puzzle));
                combineDomains(updatedDomain, getNumbersDomain(row, col, puzzle));
                if (!currTile->domain.canBeBomb && !currTile->domain.canBeEmpty) {
                    return false;
                }
                continue;
            }

            // if currTile->num != 0 then verify that adjacent tiles match up
            if (!isNumberedTileConsistent(row, col, puzzle)) {
                return false;
            }
        }
    }
    return true;
}

Domain getRowDomain(int row, int col, vector<vector<Tile*>>& puzzle) {
    Domain domain;
    int totalBombs = 3;
    int totalEmptys = 6;
    for (size_t newCol = 0; newCol < puzzle.size(); ++newCol) {
        if (newCol == col) continue;
        Tile* currTile = puzzle[row][newCol];
        if (currTile->assignment == BOMB) {
            --totalBombs;
        }
        if (currTile->assignment == EMPTY) {
            --totalEmptys;
        }
    }
    if (totalBombs == 0) {
        domain.canBeBomb = false;
    }
    if (totalEmptys == 0) {
        domain.canBeEmpty = false;
    }
    if (totalBombs < 0 || totalEmptys < 0) {
        domain.canBeBomb = false;
        domain.canBeEmpty = false;
    }
    return domain;
}

Domain getColDomain(int row, int col, vector<vector<Tile*>>& puzzle) {
    Domain domain;
    int totalBombs = 3;
    int totalEmptys = 6;
    for (size_t newRow = 0; newRow < puzzle.size(); ++newRow) {
        if (newRow == row) continue;
        Tile* currTile = puzzle[row][newRow];
        if (currTile->assignment == BOMB) {
            --totalBombs;
        }
        if (currTile->assignment == EMPTY) {
            --totalEmptys;
        }
    }
    if (totalBombs == 0) {
        domain.canBeBomb = false;
    }
    if (totalEmptys == 0) {
        domain.canBeEmpty = false;
    }
    if (totalBombs < 0 || totalEmptys < 0) {
        domain.canBeBomb = false;
        domain.canBeEmpty = false;
    }
    return domain;
}

Domain getBoxDomain(int row, int col, vector<vector<Tile*>>& puzzle) {
    Domain domain;
    int box = getBoxFromCoords(row, col);
    Pair startCoords = getStartCoordsFromBox(box);
    int totalBombs = 3;
    int totalEmptys = 6;
    for (size_t newRow = startCoords.row; newRow < startCoords.row + 3; ++newRow) {
        for (size_t newCol = startCoords.col; newCol < startCoords.col + 3; ++newCol) {
            if (newRow == row && newCol == col) continue;
            Tile* currTile = puzzle[row][newRow];
            if (currTile->assignment == BOMB) {
                --totalBombs;
            }
            if (currTile->assignment == EMPTY) {
                --totalEmptys;
            }
        }
    }
    if (totalBombs == 0) {
        domain.canBeBomb = false;
    }
    if (totalEmptys == 0) {
        domain.canBeEmpty = false;
    }
    if (totalBombs < 0 || totalEmptys < 0) {
        domain.canBeBomb = false;
        domain.canBeEmpty = false;
    }
    return domain;
}

Domain getNumbersDomain(int row, int col, vector<vector<Tile*>>& puzzle) {
    Domain domain;
    // get surrounding tiles and then get their domains and update the domain
    for (int rowOffset = -1; rowOffset < 2; ++rowOffset) {
        for (int colOffset = -1; colOffset < 2; ++colOffset) {
            if (rowOffset == 0 && colOffset == 0) continue;
            if (!isInGrid(row + rowOffset, col + colOffset)) {
                continue;
            }
            combineDomains(domain, getSingleNumberDomain(row, col, row + rowOffset, col + colOffset, puzzle));
        }
    }
    return domain;
}

Domain getSingleNumberDomain(int row, int col, int numRow, int numCol, vector<vector<Tile*>>& puzzle) {
    Domain domain;
    Tile* numTile = puzzle[numRow][numCol];
    if (numTile->num == 0) {
        return domain;
    }
    int numBombs = numTile->num;
    int numEmptys = 8 - numTile->num;
    for (int rowOffset = -1; rowOffset < 2; ++rowOffset) {
        for (int colOffset = -1; colOffset < 2; ++colOffset) {
            if (rowOffset == 0 && colOffset == 0) continue;
            if (!isInGrid(numRow + rowOffset, numCol + colOffset)) {
                --numEmptys;
                continue;
            }
            Tile* currTile = puzzle[row + rowOffset][col + colOffset];
            if (currTile->num != 0 || currTile->assignment == EMPTY) {
                --numEmptys;
            }
            if (currTile->assignment == BOMB) {
                --numBombs;
            }
        }
    }
    if (numBombs == 0) {
        domain.canBeBomb = false;
    }
    if (numEmptys == 0) {
        domain.canBeEmpty = false;
    }
    if (numBombs < 0 || numEmptys < 0) {
        domain.canBeBomb = false;
        domain.canBeEmpty = false;
    }
    // get a single tile's domain relative to current tile
    return domain;
}

void combineDomains(Domain& lhs, const Domain& rhs) {
    lhs.canBeBomb = lhs.canBeBomb && rhs.canBeBomb;
    lhs.canBeEmpty = lhs.canBeEmpty && rhs.canBeEmpty;
}

bool isNumberedTileConsistent(int row, int col, vector<vector<Tile*>>& puzzle) {
    Tile* currTile = puzzle[row][col];
    int adjBombs = currTile->num;
    int adjEmptys = 8 - adjBombs;
    for (int rowOffset = -1; rowOffset < 2; ++rowOffset) {
        for (int colOffset = -1; colOffset < 2; ++colOffset) {
            if (rowOffset == 0 && colOffset == 0) continue;
            if (!isInGrid(row + rowOffset, col + colOffset)) {
                --adjEmptys;
                continue;
            }

            Tile* tile = puzzle[row + rowOffset][col + colOffset];
            if (tile->num == 0 && tile->assignment == BOMB) {
                --adjBombs;
            } else if (tile->num == 0 && tile->assignment == EMPTY) {
                --adjEmptys;
            }
        }
    }
    return adjBombs < 0 || adjEmptys < 0;
}

bool isInRow(int row, int col, int newRow, int newCol) {
    return row == newRow;
}

bool isInCol(int row, int col, int newRow, int newCol) {
    return col == newCol;
}

bool isInBox(int row, int col, int newRow, int newCol) {
    int box = getBoxFromCoords(row, col);
    Pair startCoords = getStartCoordsFromBox(box);
    for (size_t startRow = startCoords.row; startRow < startCoords.row + 3; ++startRow) {
        for (size_t startCol = startCoords.col; startCol < startCoords.col + 3; ++startCol) {
            if (startRow == newRow && startCol == newCol) {
                return true;
            }
        }
    }
    return false;
}

bool backtrackingSearch(vector<vector<Tile*>>& puzzle) {
    // if assignment is complete return true
    bool isComplete = isPuzzleComplete(puzzle);
    // WARN: my intuition says this might cause problems during the
    // final recursive call with the isPuzzleConsistent call
    if (isComplete == true && isPuzzleConsistent(puzzle)) {
        return true;
    }

    Tile* chosenTile = selectUnassignedVariable(puzzle);

    chosenTile->assignment = EMPTY;
    // loop over puzzle and get best tile to change with mrv + dh
    // change tile value to empty, then bomb in second loop if empty fails
    // if is consistent then
    // run inference on csp and see if any fail
    // if none fail then recursively backtrack
    // if that succeeds return true
    // otherwise reset the tile's value
    // return false to upward call
    return false;
}

Tile* selectUnassignedVariable(vector<vector<Tile*>>& puzzle) {
    // mrv: lower = better
    int bestMrv = 2;
    // dh: higher = better
    int bestDH = 0;
    Tile* nextTile = nullptr;
    // one pass search for the best tile to assign next
    for (size_t row = 0; row < puzzle.size(); ++row) {
        for (size_t col = 0; col < puzzle[row].size(); ++col) {
            Tile* tile = puzzle[row][col];
            if (tile->num != 0 || tile->assignment != UNASSIGNED) continue;
            int mrvVal = mrv(*tile);
            if (mrvVal > bestMrv) {
                continue;
            }
            int dhVal = degreeHeuristic(row, col, puzzle);
            if (mrvVal < bestMrv || dhVal > bestDH) {
                bestMrv = mrvVal;
                bestDH = dhVal;
                nextTile = tile;
            }
        }
    }
    return nextTile;
}

bool isPuzzleComplete(vector<vector<Tile*>>& puzzle) {
    bool hasUnassigned = false;
    for (vector<Tile*>& row : puzzle) {
        for (Tile* tile : row) {
            if (tile->assignment != UNASSIGNED && tile->num == 0) {
                hasUnassigned = true;
            }
        }
    }
    return !hasUnassigned;
}
