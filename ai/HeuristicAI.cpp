#include "HeuristicAI.h"
#include "../model/Othello.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace chessgame::ai {

// using声明
using model::Board;

// 黑白棋位置权重表（8x8）
// 角落位置权重最高，边缘次之，中心较低
const int HeuristicAI::othelloWeights[8][8] = {
    {100, -20, 10, 5, 5, 10, -20, 100},
    {-20, -50, -2, -2, -2, -2, -50, -20},
    {10, -2, -1, -1, -1, -1, -2, 10},
    {5, -2, -1, -1, -1, -1, -2, 5},
    {5, -2, -1, -1, -1, -1, -2, 5},
    {10, -2, -1, -1, -1, -1, -2, 10},
    {-20, -50, -2, -2, -2, -2, -50, -20},
    {100, -20, 10, 5, 5, 10, -20, 100}
};

HeuristicAI::HeuristicAI(AIType aiType) : type(aiType) {
    // 使用随机种子初始化随机数生成器
    rng.seed(std::random_device{}());
}

chessgame::Move HeuristicAI::calculateMove(
    const std::shared_ptr<Board>& board,
    chessgame::PieceType playerColor
) {
    // 获取所有合法移动
    std::vector<chessgame::Point> validMoves = getValidMoves(board, playerColor);
    
    // 如果没有合法移动，返回虚着
    if (validMoves.empty()) {
        return chessgame::Move(-1, -1, playerColor, true, false);
    }
    
    // 评估每个合法移动
    std::vector<std::pair<int, chessgame::Point>> evaluatedMoves;
    
    for (const auto& move : validMoves) {
        int score = 0;
        if (type == AIType::GOMOKU) {
            score = evaluateGomokuMove(board, move.x, move.y, playerColor);
        } else if (type == AIType::OTHELLO) {
            score = evaluateOthelloMove(board, move.x, move.y, playerColor);
        }
        evaluatedMoves.push_back({score, move});
    }
    
    // 按评分排序（降序）
    std::sort(evaluatedMoves.begin(), evaluatedMoves.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // 从评分最高的移动中随机选择一个（增加多样性）
    int topScore = evaluatedMoves[0].first;
    std::vector<chessgame::Point> topMoves;
    
    for (const auto& evaluatedMove : evaluatedMoves) {
        if (evaluatedMove.first == topScore) {
            topMoves.push_back(evaluatedMove.second);
        } else {
            break; // 评分已经降低，停止收集
        }
    }
    
    std::uniform_int_distribution<size_t> dist(0, topMoves.size() - 1);
    size_t index = dist(rng);
    chessgame::Point selectedMove = topMoves[index];
    
    return chessgame::Move(selectedMove.x, selectedMove.y, playerColor, false, false);
}

AILevel HeuristicAI::getLevel() const {
    return AILevel::LEVEL2;
}

AIType HeuristicAI::getType() const {
    return type;
}

std::vector<chessgame::Point> HeuristicAI::getValidMoves(
    const std::shared_ptr<Board>& board,
    chessgame::PieceType playerColor
) {
    std::vector<chessgame::Point> validMoves;
    int size = board->getSize();
    
    if (type == AIType::GOMOKU) {
        // 五子棋：所有空位都是合法移动
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board->getPiece(i, j) == chessgame::EMPTY) {
                    validMoves.push_back({i, j});
                }
            }
        }
    } else if (type == AIType::OTHELLO) {
        // 黑白棋：只有能翻转对手棋子的位置才是合法移动
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board->getPiece(i, j) == chessgame::EMPTY) {
                    // 检查是否能翻转对手的棋子
                    int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
                    int dy[] = {1, 1, 0, -1, -1, -1, 0, 1};
                    
                    for (int k = 0; k < 8; k++) {
                        int nx = i + dx[k];
                        int ny = j + dy[k];
                        bool foundOpponent = false;
                        
                        while (nx >= 0 && nx < size && ny >= 0 && ny < size) {
                            chessgame::PieceType piece = board->getPiece(nx, ny);
                            
                            if (piece == chessgame::EMPTY) {
                                break;
                            }
                            
                            if (piece == playerColor) {
                                if (foundOpponent) {
                                    validMoves.push_back({i, j});
                                    goto next_position; // 找到合法移动，跳出两层循环
                                }
                                break;
                            }
                            
                            // 找到对手的棋子
                            foundOpponent = true;
                            nx += dx[k];
                            ny += dy[k];
                        }
                    }
                    next_position:;
                }
            }
        }
    }
    
    return validMoves;
}

int HeuristicAI::evaluateGomokuMove(
    const std::shared_ptr<Board>& board,
    int x, int y,
    chessgame::PieceType playerColor
) {
    int score = 0;
    chessgame::PieceType opponentColor = (playerColor == chessgame::BLACK) ? chessgame::WHITE : chessgame::BLACK;
    
    // 检查四个方向：水平、垂直、两个对角线
    int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    
    for (int i = 0; i < 4; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];
        
        // 计算我方连续棋子数
        int myConsecutive = countConsecutive(board, x, y, dx, dy, playerColor) + 
                           countConsecutive(board, x, y, -dx, -dy, playerColor);
        
        // 计算对手连续棋子数
        int oppConsecutive = countConsecutive(board, x, y, dx, dy, opponentColor) + 
                            countConsecutive(board, x, y, -dx, -dy, opponentColor);
        
        // 检查两端是否被堵
        bool myBlocked = isBlocked(board, x, y, dx, dy, playerColor) && 
                         isBlocked(board, x, y, -dx, -dy, playerColor);
        
        bool oppBlocked = isBlocked(board, x, y, dx, dy, opponentColor) && 
                          isBlocked(board, x, y, -dx, -dy, opponentColor);
        
        // 根据连续棋子数和是否被堵来评分
        if (myConsecutive >= 4) {
            score += 10000; // 可以形成五连，最高优先级
        } else if (myConsecutive == 3 && !myBlocked) {
            score += 1000; // 活四，高优先级
        } else if (myConsecutive == 3 && myBlocked) {
            score += 100; // 冲四，中等优先级
        } else if (myConsecutive == 2 && !myBlocked) {
            score += 50; // 活三
        } else if (myConsecutive == 2 && myBlocked) {
            score += 10; // 眠三
        } else if (myConsecutive == 1 && !myBlocked) {
            score += 5; // 活二
        }
        
        // 防守评分
        if (oppConsecutive >= 4) {
            score += 5000; // 阻止对手五连
        } else if (oppConsecutive == 3 && !oppBlocked) {
            score += 500; // 阻止对手活四
        } else if (oppConsecutive == 3 && oppBlocked) {
            score += 50; // 阻止对手冲四
        }
    }
    
    return score;
}

int HeuristicAI::evaluateOthelloMove(
    const std::shared_ptr<Board>& board,
    int x, int y,
    chessgame::PieceType playerColor
) {
    int score = 0;
    
    // 基础分数：位置权重
    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
        score += othelloWeights[x][y];
    }
    
    // 翻转棋子数分数
    int flippedCount = countFlippedPieces(board, x, y, playerColor);
    score += flippedCount * 10;
    
    // 角落策略额外加分
    if (isCorner(x, y)) {
        score += 500;
    }
    
    // 边缘策略加分
    if (isEdge(x, y)) {
        score += 50;
    }
    
    // 避免给对手留下角落机会
    int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        
        if (isCorner(nx, ny) && board->getPiece(nx, ny) == chessgame::EMPTY) {
            score -= 200; // 靠近空角落的位置减分
        }
    }
    
    return score;
}

int HeuristicAI::countConsecutive(
    const std::shared_ptr<Board>& board,
    int x, int y, int dx, int dy,
    chessgame::PieceType playerColor
) {
    int count = 0;
    int nx = x + dx;
    int ny = y + dy;
    int size = board->getSize();
    
    while (nx >= 0 && nx < size && ny >= 0 && ny < size && 
           board->getPiece(nx, ny) == playerColor) {
        count++;
        nx += dx;
        ny += dy;
    }
    
    return count;
}

bool HeuristicAI::isBlocked(
    const std::shared_ptr<Board>& board,
    int x, int y, int dx, int dy,
    chessgame::PieceType playerColor
) {
    int nx = x + dx;
    int ny = y + dy;
    int size = board->getSize();
    
    // 跳过所有同色棋子
    while (nx >= 0 && nx < size && ny >= 0 && ny < size && 
           board->getPiece(nx, ny) == playerColor) {
        nx += dx;
        ny += dy;
    }
    
    // 检查是否到达边界或被对手棋子阻挡
    if (nx < 0 || nx >= size || ny < 0 || ny >= size) {
        return true; // 到达边界，被阻挡
    }
    
    return board->getPiece(nx, ny) != chessgame::EMPTY; // 被对手棋子阻挡
}

int HeuristicAI::countFlippedPieces(
    const std::shared_ptr<Board>& board,
    int x, int y,
    chessgame::PieceType playerColor
) {
    int count = 0;
    int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[] = {1, 1, 0, -1, -1, -1, 0, 1};
    int size = board->getSize();
    
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        std::vector<chessgame::Point> directionFlips;
        
        while (nx >= 0 && nx < size && ny >= 0 && ny < size) {
            chessgame::PieceType piece = board->getPiece(nx, ny);
            
            if (piece == chessgame::EMPTY) {
                break;
            }
            
            if (piece == playerColor) {
                // 找到自己的棋子，将路径上的对手棋子添加到翻转列表
                count += directionFlips.size();
                break;
            }
            
            // 对手的棋子
            directionFlips.push_back({nx, ny});
            nx += dx[i];
            ny += dy[i];
        }
    }
    
    return count;
}

bool HeuristicAI::isCorner(int x, int y) {
    return (x == 0 && y == 0) || (x == 0 && y == 7) || 
           (x == 7 && y == 0) || (x == 7 && y == 7);
}

bool HeuristicAI::isEdge(int x, int y) {
    int size = 8;
    return (x == 0 || x == size - 1 || y == 0 || y == size - 1) && !isCorner(x, y);
}

}