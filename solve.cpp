#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::abs;
using std::cout;
using std::endl;
using std::fstream;
using std::getline;
using std::iostream;
using std::ofstream;
using std::priority_queue;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

struct Domain {
    bool canBeBomb = true;
    bool cannotBeBomb = true;
};

// How we represent tiles and their information
struct Tile {
    int num = 0;
    Domain domain;
    bool hasBomb = false;
};

// For the prio queue, which applies MRV and DH on the tiles.
struct TileComp {
    bool operator()(const Tile* lhs, const Tile* rhs) const;
};

bool isInGrid(int x, int y);

// takes a pair of coordinates and
// returns how many non-number tiles adjacent to the coordinate
int degreeHeuristic(int x, int y, std::vector<std::vector<Tile>>& puzzle);

// takes a tile and returns number
// of possibilities that tile has (bomb or not bomb)
int mrv(Tile& tile);

vector<vector<Tile*>> readPuzzle(const string& filePath);

void writeAnswer(vector<vector<Tile*>> puzzle);

// Inference functions
bool forwardChecking(vector<vector<Tile*>> puzzle);

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

bool isInGrid(int x, int y) {
    return !(x < 0 || x > 8 || y < 0 || y > 8);
}

int degreeHeuristic(int x, int y, std::vector<std::vector<Tile>>& puzzle) {
    if (!isInGrid(x, y)) return -1;
    int adjZeros = 0;
    for (int xOffset = -1; xOffset < 2; ++xOffset) {
        for (int yOffset = -1; yOffset < 2; ++yOffset) {
            if (xOffset == 0 && yOffset == 0) continue;
            if (!isInGrid(x + xOffset, y + yOffset)) continue;

            Tile* tile = &puzzle[x + xOffset][y + yOffset];
            if (tile->num == 0) {
                ++adjZeros;
            }
        }
    }
    return adjZeros;
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
