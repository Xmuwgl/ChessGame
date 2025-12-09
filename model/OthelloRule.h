#pragma once
#include "Rule.h"
#include <vector>

namespace chessgame::model {

class OthelloRule : public Rule {
private:
    // 方向数组：8个方向
    static const int dx[8];
    static const int dy[8];
    
    // 检查某个方向是否可以翻转棋子
    bool checkDirection(int x, int y, int dirX, int dirY, PieceType player) const;
    
    // 翻转某个方向的棋子
    void flipDirection(int x, int y, int dirX, int dirY, PieceType player);
    
    // 获取所有被翻转的棋子位置
    std::vector<Point> getFlippedPieces(int x, int y, PieceType player) const;

public:
    OthelloRule(Board* b);
    ~OthelloRule() = default;
    
    // 判断落子是否合法
    bool isValidMove(int x, int y, PieceType player) const override;
    
    // 执行落子：翻转对手棋子
    void makeMove(int x, int y, PieceType player) override;
    
    // 判断胜负
    GameStatus checkWin(int lastX, int lastY) override;
    
    // 支持虚着
    bool supportsPass() const override { return true; }
    
    // 获取当前玩家的所有合法移动
    std::vector<Point> getValidMoves(PieceType player) const;
    
    // 检查玩家是否有合法移动
    bool hasValidMoves(PieceType player) const;
    
    // 计算双方棋子数量
    std::pair<int, int> countPieces() const;
    PieceType getOpponent(PieceType player) const;
};

}