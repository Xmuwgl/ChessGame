#include "Board.h"
#include <algorithm>
#include <sstream>
#include <vector>

using namespace chessgame::model;

Board::Board(int s) : size(s) {
    grid.resize(size, std::vector<PieceType>(size, PieceType::EMPTY));
}

// 拷贝构造函数: 用于备忘录模式保存状态
Board::Board(const Board& other) : size(other.size), grid(other.grid) {}

int Board::getSize() const { 
    return size; 
}
    
bool Board::isValidBounds(int x, int y) const {
    return x >= 0 && x < size && y >= 0 && y < size;
}

chessgame::PieceType Board::getPiece(int x, int y) const {
    if (!isValidBounds(x, y)) return PieceType::EMPTY;
    return grid[x][y];
}

void Board::setPiece(int x, int y, chessgame::PieceType p) {
    if (isValidBounds(x, y)) grid[x][y] = p;
}

void Board::clear() {
    for (auto& row : grid) std::fill(row.begin(), row.end(), chessgame::PieceType::EMPTY);
}

// 序列化用于存档
std::string Board::serialize() const {
    std::stringstream ss;
    ss << size << " ";
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            ss << static_cast<int>(grid[i][j]) << " ";
    return ss.str();
}

// 反序列化用于读档
void Board::deserialize(std::stringstream& ss) {
    ss >> size;
    grid.resize(size, std::vector<chessgame::PieceType>(size));
    int temp;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            ss >> temp;
            grid[i][j] = static_cast<chessgame::PieceType>(temp);
        }
    }
}

int Board::rows() const { 
    return size; 
}

int Board::cols() const { 
    return size; 
}