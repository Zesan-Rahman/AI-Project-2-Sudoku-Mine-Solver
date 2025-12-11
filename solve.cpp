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

Pair getCoordsOfTile(const Tile* tile, vector<vector<Tile*>> puzzle);

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
bool forwardChecking(int row, int col, vector<vector<Tile*>>& puzzle);

// Forward checks all the tiles in the row, column, and box. If forward check fails, all tiles domains stay the
// same. If forward check succeeds, tile domains get updated
bool forwardCheckRow(int row, int col, vector<vector<Tile*>>& puzzle);
bool forwardCheckCol(int row, int col, vector<vector<Tile*>>& puzzle);
bool forwardCheckBox(int row, int col, vector<vector<Tile*>>& puzzle);

// Checks if a specific numbered tile needs to update domains of the tiles around it
bool forwardCheckNumberedTile(int row, int col, vector<vector<Tile*>>& puzzle);
// Checks all numbered tiles around a newly assigned tile and forward checks each individual tile with function
// above.
bool forwardCheckNumberedTiles(int row, int col, vector<vector<Tile*>>& puzzle);

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
    // Forward check numbered tiles test
    // puzzle[0][1]->assignment = BOMB;
    // forwardCheckNumberedTiles(0, 1, puzzle);
    // cout << puzzle[1][0]->domain.canBeBomb << endl;
    // Forward check col test
    // puzzle[0][3]->assignment = BOMB;
    // forwardCheckCol(0, 3, puzzle);
    // puzzle[1][3]->assignment = BOMB;
    // forwardCheckCol(1, 3, puzzle);
    // puzzle[4][3]->assignment = BOMB;
    // forwardCheckCol(4, 3, puzzle);
    // // forwardcheck row test
    // puzzle[1][0]->assignment = BOMB;
    // cout << forwardCheckRow(1, 0, puzzle) << endl;
    // puzzle[1][2]->assignment = BOMB;
    // cout << forwardCheckRow(1, 2, puzzle) << endl;
    // puzzle[1][3]->assignment = BOMB;
    // cout << forwardCheckRow(1, 3, puzzle) << endl;
    // cout << puzzle[1][4]->domain.canBeBomb << endl;
    backtrackingSearch(puzzle);
    writeAnswer(puzzle);
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
    if (!chosenTile) return false;
    Pair chosenTileCoords = getCoordsOfTile(chosenTile, puzzle);
    cout << "Row:	Col:" << endl;
    cout << chosenTileCoords.row << "	" << chosenTileCoords.col << endl;
    vector<int> domain{0, 1};
    for (int value : domain) {
        // If value is consistent with assignment
        if (value == 0 && !chosenTile->domain.canBeEmpty) continue;
        if (value == 1 && !chosenTile->domain.canBeBomb) continue;
        // Add var = value to assignment
        value ? chosenTile->assignment = BOMB : chosenTile->assignment = EMPTY;
        vector<vector<Tile*>> copy = puzzle;
        bool inference = forwardChecking(chosenTileCoords.row, chosenTileCoords.col, puzzle);
        // inference != failure
        if (inference) {
            bool resultPassed = backtrackingSearch(puzzle);
            if (resultPassed) return resultPassed;
            // Remove inferences from csp
            puzzle = copy;
        }
        value ? chosenTile->domain.canBeBomb = false : chosenTile->domain.canBeEmpty = false;
    }
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

bool forwardCheckRow(int row, int col, vector<vector<Tile*>>& puzzle) {
    int numBombs = 0;
    int numEmptys = 0;
    for (size_t c = 0; c < puzzle[row].size(); ++c) {
        numBombs += (puzzle[row][c]->assignment == BOMB);
        numEmptys += (puzzle[row][c]->assignment == EMPTY);
    }
    // Check if we are third bomb. If we are, remove canBeBomb from every other unassigned tile's domain
    if (puzzle[row][col]->assignment == BOMB && numBombs == 3) {
        for (size_t c = 0; c < puzzle[row].size(); ++c) {
            if (c == col || puzzle[row][c]->num != 0 || puzzle[row][c]->assignment != UNASSIGNED) continue;
            puzzle[row][c]->domain.canBeBomb = false;
            if (!puzzle[row][c]->domain.canBeBomb && !puzzle[row][c]->domain.canBeEmpty) {
                return false;
            }
        }
    }
    // We are the 6th empty
    if (puzzle[row][col]->assignment == EMPTY && numEmptys == 6) {
        for (size_t c = 0; c < puzzle[row].size(); ++c) {
            if (c == col || puzzle[row][c]->num != 0 || puzzle[row][c]->assignment != UNASSIGNED) continue;
            puzzle[row][c]->domain.canBeEmpty = false;
            if (!puzzle[row][c]->domain.canBeBomb && !puzzle[row][c]->domain.canBeEmpty) {
                return false;
            }
        }
    }
    return true;
}

bool forwardCheckCol(int row, int col, vector<vector<Tile*>>& puzzle) {
    int numBombs = 0;
    int numEmptys = 0;
    for (size_t r = 0; r < puzzle.size(); ++r) {
        numBombs += (puzzle[r][col]->assignment == BOMB);
        numEmptys += (puzzle[r][col]->assignment == EMPTY);
    }
    // Check if we are third bomb. If we are, remove canBeBomb from every other unassigned tile's domain
    if (puzzle[row][col]->assignment == BOMB && numBombs == 3) {
        for (size_t r = 0; r < puzzle[row].size(); ++r) {
            if (r == row || puzzle[r][col]->num != 0 || puzzle[r][col]->assignment != UNASSIGNED) continue;
            puzzle[r][col]->domain.canBeBomb = false;
            if (!puzzle[r][col]->domain.canBeBomb && !puzzle[r][col]->domain.canBeEmpty) {
                return false;
            }
        }
    }
    // We are the 6th empty
    if (puzzle[row][col]->assignment == EMPTY && numEmptys == 6) {
        for (size_t r = 0; r < puzzle[row].size(); ++r) {
            if (r == row || puzzle[r][col]->num != 0 || puzzle[r][col]->assignment != UNASSIGNED) continue;
            puzzle[r][col]->domain.canBeEmpty = false;
            if (!puzzle[r][col]->domain.canBeBomb && !puzzle[r][col]->domain.canBeEmpty) {
                return false;
            }
        }
    }
    return true;
}

bool forwardCheckBox(int row, int col, vector<vector<Tile*>>& puzzle) {
    int numBombs = 0;
    int numEmptys = 0;
    // Check box
    Pair startCoords = getStartCoordsFromBox(getBoxFromCoords(row, col));
    for (size_t r = startCoords.row; r < startCoords.row + 3; ++r) {
        for (size_t c = startCoords.col; c < startCoords.col + 3; ++c) {
            // Tile has been checked by row and col
            numBombs += (puzzle[r][c]->assignment == BOMB);
            numEmptys += (puzzle[r][c]->assignment == EMPTY);
        }
    }

    if (puzzle[row][col]->assignment == BOMB && numBombs == 3) {
        for (size_t r = startCoords.row; r < startCoords.row + 3; ++r) {
            for (size_t c = startCoords.col; c < startCoords.col + 3; ++c) {
                if (r == row && c == col) continue;
                if (puzzle[r][c]->num != 0 || puzzle[r][c]->assignment != UNASSIGNED) continue;
                puzzle[r][c]->domain.canBeBomb = false;
                if (!puzzle[r][c]->domain.canBeBomb && !puzzle[r][c]->domain.canBeEmpty) {
                    return false;
                }
            }
        }
    }
    // We are the 6th empty
    if (puzzle[row][col]->assignment == EMPTY && numEmptys == 6) {
        for (size_t r = startCoords.row; r < startCoords.row + 3; ++r) {
            for (size_t c = startCoords.col; c < startCoords.col; ++c) {
                if (r == row && c == col) continue;
                if (puzzle[r][c]->num != 0 || puzzle[r][c]->assignment != UNASSIGNED) continue;
                puzzle[r][c]->domain.canBeBomb = false;
                if (!puzzle[r][c]->domain.canBeBomb && !puzzle[r][c]->domain.canBeEmpty) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool forwardCheckNumberedTile(int row, int col, vector<vector<Tile*>>& puzzle) {
    int numBombs = 0;
    int numEmptys = 0;
    int maxEmptys = 8;
    for (int rowOffset = -1; rowOffset < 2; ++rowOffset) {
        for (int colOffset = -1; colOffset < 2; ++colOffset) {
            if (rowOffset == 0 && colOffset == 0) continue;
            if (!isInGrid(row + rowOffset, col + colOffset)) {
                maxEmptys--;
                continue;
            }
            numBombs += puzzle[row + rowOffset][col + colOffset]->assignment == BOMB;
            numEmptys += puzzle[row + rowOffset][col + colOffset]->assignment == EMPTY;
        }
    }
    // Check if maxBombs allowed reached and if so, set everything around to cannotbebomb
    if (numBombs == puzzle[row][col]->num) {
        for (int rowOffset = -1; rowOffset < 2; ++rowOffset) {
            for (int colOffset = -1; colOffset < 2; ++colOffset) {
                if (rowOffset == 0 && colOffset == 0) continue;
                int r = row + rowOffset;
                int c = col + colOffset;
                if (!isInGrid(r, c)) continue;
                if (puzzle[r][c]->num != 0 || puzzle[r][c]->assignment != UNASSIGNED) continue;
                puzzle[r][c]->domain.canBeBomb = false;
                if (!puzzle[r][c]->domain.canBeBomb && !puzzle[r][c]->domain.canBeEmpty) {
                    return false;
                }
            }
        }
    }
    // Check if maxEmptys allowed reached:
    if (numEmptys == maxEmptys) {
        for (int rowOffset = -1; rowOffset < 2; ++rowOffset) {
            for (int colOffset = -1; colOffset < 2; ++colOffset) {
                if (rowOffset == 0 && colOffset == 0) continue;
                int r = row + rowOffset;
                int c = col + colOffset;
                if (!isInGrid(r, c)) continue;
                if (puzzle[r][c]->num != 0 || puzzle[r][c]->assignment != UNASSIGNED) continue;
                puzzle[r][c]->domain.canBeEmpty = false;
                if (!puzzle[r][c]->domain.canBeBomb && !puzzle[r][c]->domain.canBeEmpty) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool forwardCheckNumberedTiles(int row, int col, vector<vector<Tile*>>& puzzle) {
    for (int rowOffset = -1; rowOffset < 2; ++rowOffset) {
        for (int colOffset = -1; colOffset < 2; ++colOffset) {
            if (rowOffset == 0 && colOffset == 0) continue;
            if (!isInGrid(row + rowOffset, col + colOffset) || puzzle[row + rowOffset][col + colOffset]->num == 0)
                continue;
            // We are at a numbered tile
            if (!forwardCheckNumberedTile(row + rowOffset, col + colOffset, puzzle)) return false;
        }
    }
    return true;
}

bool forwardChecking(int row, int col, vector<vector<Tile*>>& puzzle) {
    // Fail-safe in case forwardChecking ends with a failure.
    vector<vector<Tile>> copy(puzzle.size(), vector<Tile>(puzzle[0].size()));
    for (size_t row = 0; row < puzzle.size(); ++row) {
        for (size_t col = 0; col < puzzle.size(); ++col) {
            copy[row][col] = *puzzle[row][col];
        }
    }
    if (forwardCheckRow(row, col, puzzle) && forwardCheckCol(row, col, puzzle) &&
        forwardCheckBox(row, col, puzzle) && forwardCheckNumberedTiles(row, col, puzzle))
        return true;
    for (size_t row = 0; row < puzzle.size(); ++row) {
        for (size_t col = 0; col < puzzle.size(); ++col) {
            *puzzle[row][col] = copy[row][col];
        }
    }
    return false;
}

Pair getCoordsOfTile(const Tile* tile, vector<vector<Tile*>> puzzle) {
    for (int row = 0; row < puzzle.size(); ++row) {
        for (int col = 0; col < puzzle.size(); ++col) {
            if (tile == puzzle[row][col]) return Pair{row, col};
        }
    }
    return Pair{0, 0};
}

void writeAnswer(vector<vector<Tile*>>& puzzle) {
    ofstream outputFile("output.txt");
    for (size_t row = 0; row < puzzle.size(); ++row) {
        for (size_t col = 0; col < puzzle[row].size(); ++col) {
            outputFile << (puzzle[row][col]->assignment == BOMB) << " ";
            delete puzzle[row][col];
        }
        outputFile << endl;
    }
}
