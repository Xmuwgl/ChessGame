#pragma once
#include "../utils/Type.h"

namespace chessgame::model {
class Board;

class Rule {
protected:
    Board* board;

public:
    Rule(Board* b) : board(b) {}
    virtual ~Rule() = default;

    // 判断落子是否合法
    virtual bool isValidMove(int x, int y, PieceType player) const = 0;
    
    // 执行落子: 有可能触发提子
    virtual void makeMove(int x, int y, PieceType player) = 0;
    
    // 判断胜负
    virtual GameStatus checkWin(int lastX, int lastY) = 0;
    
    // 虚着处理
    virtual bool supportsPass() const { return false; }
};

}