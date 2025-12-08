#pragma once
#include "AI.h"
#include "../model/Board.h"
#include <random>

namespace chessgame::ai {

// using声明
using model::Board;

// 二级AI - 基于评分函数
class HeuristicAI : public AIStrategy {
private:
    AIType type;
    std::mt19937 rng;
    
    // 黑白棋位置权重表（8x8）
    static const int othelloWeights[8][8];
    
public:
    explicit HeuristicAI(AIType aiType);
    ~HeuristicAI() override = default;
    
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
    
    // 五子棋评分函数
    int evaluateGomokuMove(
        const std::shared_ptr<Board>& board,
        int x, int y,
        chessgame::PieceType playerColor
    );
    
    // 黑白棋评分函数
    int evaluateOthelloMove(
        const std::shared_ptr<Board>& board,
        int x, int y,
        chessgame::PieceType playerColor
    );
    
    // 五子棋：检查某个方向的连续棋子数
    int countConsecutive(
        const std::shared_ptr<Board>& board,
        int x, int y, int dx, int dy,
        chessgame::PieceType playerColor
    );
    
    // 五子棋：检查某个方向是否两端被堵
    bool isBlocked(
        const std::shared_ptr<Board>& board,
        int x, int y, int dx, int dy,
        chessgame::PieceType playerColor
    );
    
    // 黑白棋：计算某个位置能翻转的棋子数
    int countFlippedPieces(
        const std::shared_ptr<Board>& board,
        int x, int y,
        chessgame::PieceType playerColor
    );
    
    // 黑白棋：检查是否为角落位置
    bool isCorner(int x, int y);
    
    // 黑白棋：检查是否为边缘位置
    bool isEdge(int x, int y);
};

}