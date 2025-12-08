#pragma once
#include "AI.h"
#include "../model/Board.h"
#include <random>

namespace chessgame::ai {

// using声明
using model::Board;

// 一级AI - 随机落子
class RandomAI : public AIStrategy {
private:
    AIType type;
    std::mt19937 rng;
    
public:
    explicit RandomAI(AIType aiType);
    ~RandomAI() override = default;
    
    // 实现策略接口
    chessgame::Move calculateMove(
        const std::shared_ptr<Board>& board,
        chessgame::PieceType playerColor
    ) override;
    
    AILevel getLevel() const override;
    AIType getType() const override;
    
private:
    // 获取所有合法移动
    std::vector<chessgame::Point> getValidMoves(
        const std::shared_ptr<Board>& board,
        chessgame::PieceType playerColor
    );
    
    // 五子棋的合法移动检查
    bool isValidGomokuMove(
        const std::shared_ptr<Board>& board,
        int x, int y
    );
    
    // 黑白棋的合法移动检查
    bool isValidOthelloMove(
        const std::shared_ptr<Board>& board,
        int x, int y,
        chessgame::PieceType playerColor
    );
};

}