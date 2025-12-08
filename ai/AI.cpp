#include "AI.h"
#include "RandomAI.h"
#include "HeuristicAI.h"
#include <memory>

namespace chessgame::ai {

AIPlayer::AIPlayer(PieceType color, std::unique_ptr<AIStrategy> strategy) 
    : color(color), strategy(std::move(strategy)) {
}

void AIPlayer::setColor(PieceType newColor) {
    color = newColor;
}

PieceType AIPlayer::getColor() const {
    return color;
}

void AIPlayer::setStrategy(std::unique_ptr<AIStrategy> newStrategy) {
    strategy = std::move(newStrategy);
}

const AIStrategy* AIPlayer::getStrategy() const {
    return strategy.get();
}

Move AIPlayer::makeMove(const std::shared_ptr<Board>& board) {
    if (!strategy) {
        return Move(-1, -1, color, true, false); // 无策略时返回虚着
    }
    return strategy->calculateMove(board, color);
}

// AI工厂实现
std::unique_ptr<AIStrategy> AIFactory::createStrategy(AIType type, AILevel level) {
    switch (level) {
        case AILevel::LEVEL1:
            return std::make_unique<RandomAI>(type);
        case AILevel::LEVEL2:
            return std::make_unique<HeuristicAI>(type);
        case AILevel::LEVEL3:
            // TODO: 实现高级AI（MCTS或强化学习）
            // 目前暂时使用二级AI
            return std::make_unique<HeuristicAI>(type);
        default:
            return std::make_unique<RandomAI>(type);
    }
}

std::unique_ptr<AIPlayer> AIFactory::createAIPlayer(
    PieceType color,
    AIType type,
    AILevel level
) {
    auto strategy = createStrategy(type, level);
    return std::make_unique<AIPlayer>(color, std::move(strategy));
}

}