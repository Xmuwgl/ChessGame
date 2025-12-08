#pragma once
#include "../utils/Type.h"
#include "../model/Board.h"
#include <vector>
#include <memory>

namespace chessgame::ai {

// using声明
using chessgame::model::Board;

// AI级别枚举
enum class AILevel {
    LEVEL1,  // 随机AI
    LEVEL2,  // 评分函数AI
    LEVEL3   // MCTS或强化学习AI（可选）
};

// AI玩家类型
enum class AIType {
    GOMOKU,
    OTHELLO
};

// AI接口 - 策略模式
class AIStrategy {
public:
    virtual ~AIStrategy() = default;
    
    // 根据当前棋盘状态和玩家颜色，计算下一步移动
    virtual chessgame::Move calculateMove(
        const std::shared_ptr<Board>& board,
        chessgame::PieceType playerColor
    ) = 0;
    
    // 获取AI级别
    virtual AILevel getLevel() const = 0;
    
    // 获取AI类型
    virtual AIType getType() const = 0;
};

// AI玩家类
class AIPlayer {
private:
    chessgame::PieceType color;
    std::unique_ptr<AIStrategy> strategy;
    
public:
    AIPlayer(chessgame::PieceType color, std::unique_ptr<AIStrategy> strategy);
    ~AIPlayer() = default;
    
    // 设置AI颜色
    void setColor(chessgame::PieceType newColor);
    
    // 获取AI颜色
    chessgame::PieceType getColor() const;
    
    // 设置AI策略
    void setStrategy(std::unique_ptr<AIStrategy> newStrategy);
    
    // 获取AI策略
    const AIStrategy* getStrategy() const;
    
    // 计算下一步移动
    chessgame::Move makeMove(const std::shared_ptr<Board>& board);
};

// AI工厂 - 抽象工厂模式
class AIFactory {
public:
    // 创建AI策略
    static std::unique_ptr<AIStrategy> createStrategy(AIType type, AILevel level);
    
    // 创建AI玩家
    static std::unique_ptr<AIPlayer> createAIPlayer(
        chessgame::PieceType color,
        AIType type,
        AILevel level
    );
};

}