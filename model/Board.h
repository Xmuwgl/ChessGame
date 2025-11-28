#pragma once
#include "../utils/Type.h"
#include <string>
#include <vector>

namespace chessgame::model {
class Board {
private:
    int size;
    std::vector<std::vector<PieceType>> grid;

public:
    Board(int s);
    // 拷贝构造函数: 用于备忘录模式保存状态
    Board(const Board& other);
    
    int getSize() const;
    bool isValidBounds(int x, int y) const;
    PieceType getPiece(int x, int y) const;
    void setPiece(int x, int y, PieceType p);
    void clear();
    
    // 序列化用于存档
    std::string serialize() const;
    
    // 反序列化用于读档
    void deserialize(std::stringstream& ss);
    
    int rows() const;
    int cols() const;
};
}
