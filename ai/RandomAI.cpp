#include "RandomAI.h"
#include "../model/Othello.h"
#include <cstdlib>
#include <ctime>

namespace chessgame::ai {

RandomAI::RandomAI(AIType aiType) : type(aiType) {
    // 使用随机种子初始化随机数生成器
    rng.seed(std::random_device{}());
}

chessgame::Move RandomAI::calculateMove(
    const std::shared_ptr<chessgame::model::Board>& board,
    chessgame::PieceType playerColor
) {
    // 获取所有合法移动
    std::vector<chessgame::Point> validMoves = getValidMoves(board, playerColor);
    
    // 如果没有合法移动，返回虚着
    if (validMoves.empty()) {
        return chessgame::Move(-1, -1, playerColor, true, false);
    }
    
    // 随机选择一个合法移动
    std::uniform_int_distribution<size_t> dist(0, validMoves.size() - 1);
    size_t index = dist(rng);
    chessgame::Point selectedMove = validMoves[index];
    
    return chessgame::Move(selectedMove.x, selectedMove.y, playerColor, false, false);
}

AILevel RandomAI::getLevel() const {
    return AILevel::LEVEL1;
}

AIType RandomAI::getType() const {
    return type;
}

std::vector<chessgame::Point> RandomAI::getValidMoves(
    const std::shared_ptr<chessgame::model::Board>& board,
    chessgame::PieceType playerColor
) {
    std::vector<chessgame::Point> validMoves;
    int size = board->getSize();
    
    if (type == AIType::GOMOKU) {
        // 五子棋：所有空位都是合法移动
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (isValidGomokuMove(board, i, j)) {
                    validMoves.push_back({i, j});
                }
            }
        }
    } else if (type == AIType::OTHELLO) {
        // 黑白棋：只有能翻转对手棋子的位置才是合法移动
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (isValidOthelloMove(board, i, j, playerColor)) {
                    validMoves.push_back({i, j});
                }
            }
        }
    }
    
    return validMoves;
}

bool RandomAI::isValidGomokuMove(
    const std::shared_ptr<chessgame::model::Board>& board,
    int x, int y
) {
    // 五子棋中，只要位置为空就是合法移动
    return board->getPiece(x, y) == chessgame::EMPTY;
}

bool RandomAI::isValidOthelloMove(
    const std::shared_ptr<chessgame::model::Board>& board,
    int x, int y,
    chessgame::PieceType playerColor
) {
    // 黑白棋的合法移动检查
    // 检查位置是否在棋盘内
    if (x < 0 || x >= board->getSize() || y < 0 || y >= board->getSize()) {
        return false;
    }
    
    // 检查位置是否为空
    if (board->getPiece(x, y) != chessgame::EMPTY) {
        return false;
    }
    
    // 检查是否能翻转对手的棋子
    int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[] = {1, 1, 0, -1, -1, -1, 0, 1};
    
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        bool foundOpponent = false;
        
        while (nx >= 0 && nx < board->getSize() && ny >= 0 && ny < board->getSize()) {
            chessgame::PieceType piece = board->getPiece(nx, ny);
            
            if (piece == chessgame::EMPTY) {
                break;
            }
            
            if (piece == playerColor) {
                if (foundOpponent) {
                    return true; // 找到可以翻转的棋子
                }
                break;
            }
            
            // 找到对手的棋子
            foundOpponent = true;
            nx += dx[i];
            ny += dy[i];
        }
    }
    
    return false;
}

}