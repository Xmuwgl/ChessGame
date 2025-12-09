#include "OthelloRule.h"
#include "Board.h"
#include <algorithm>

using namespace chessgame::model;

// 方向数组：8个方向
const int OthelloRule::dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int OthelloRule::dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

OthelloRule::OthelloRule(Board* b) : Rule(b) {}

bool OthelloRule::isValidMove(int x, int y, PieceType player) const {
    // 检查边界
    if (!board->isValidBounds(x, y)) return false;
    
    // 检查位置是否为空
    if (board->getPiece(x, y) != EMPTY) return false;
    
    // 检查是否能翻转对手的棋子
    return !getFlippedPieces(x, y, player).empty();
}

std::vector<chessgame::Point> OthelloRule::getFlippedPieces(int x, int y, PieceType player) const {
    std::vector<Point> flipped;
    
    // 检查8个方向
    for (int i = 0; i < 8; ++i) {
        std::vector<Point> directionFlipped;
        int nx = x + dx[i];
        int ny = y + dy[i];
        
        // 寻找对手棋子
        bool foundOpponent = false;
        while (board->isValidBounds(nx, ny) && board->getPiece(nx, ny) == getOpponent(player)) {
            directionFlipped.push_back({nx, ny});
            foundOpponent = true;
            nx += dx[i];
            ny += dy[i];
        }
        
        // 检查是否以自己的棋子结束
        if (foundOpponent && board->isValidBounds(nx, ny) && board->getPiece(nx, ny) == player) {
            flipped.insert(flipped.end(), directionFlipped.begin(), directionFlipped.end());
        }
    }
    
    return flipped;
}

void OthelloRule::makeMove(int x, int y, PieceType player) {
    // 非法移动，不做任何操作
    if (!isValidMove(x, y, player)) return;
    
    // 放置棋子
    board->setPiece(x, y, player);
    
    // 翻转对手棋子
    for (int i = 0; i < 8; ++i) flipDirection(x, y, dx[i], dy[i], player);
}

bool OthelloRule::checkDirection(int x, int y, int dirX, int dirY, PieceType player) const {
    int nx = x + dirX;
    int ny = y + dirY;
    
    // 寻找对手棋子
    bool foundOpponent = false;
    while (board->isValidBounds(nx, ny) && board->getPiece(nx, ny) == getOpponent(player)) {
        foundOpponent = true;
        nx += dirX;
        ny += dirY;
    }
    
    // 检查是否以自己的棋子结束
    return foundOpponent && board->isValidBounds(nx, ny) && board->getPiece(nx, ny) == player;
}

void OthelloRule::flipDirection(int x, int y, int dirX, int dirY, PieceType player) {
    std::vector<Point> toFlip;
    int nx = x + dirX;
    int ny = y + dirY;
    
    // 收集需要翻转的对手棋子
    while (board->isValidBounds(nx, ny) && board->getPiece(nx, ny) == getOpponent(player)) {
        toFlip.push_back({nx, ny});
        nx += dirX;
        ny += dirY;
    }
    
    // 检查是否以自己的棋子结束
    if (!toFlip.empty() && board->isValidBounds(nx, ny) && board->getPiece(nx, ny) == player) {
        // 翻转棋子
        for (const auto& pos : toFlip) board->setPiece(pos.x, pos.y, player);
    }
}

chessgame::GameStatus OthelloRule::checkWin(int lastX, int lastY) {
    // 检查棋盘是否已满
    bool boardFull = true;
    int size = board->getSize();
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (board->getPiece(i, j) == EMPTY) {
                boardFull = false;
                break;
            }
        }
        if (!boardFull) break;
    }
    
    // 检查双方是否都无合法移动
    bool blackHasMove = hasValidMoves(BLACK);
    bool whiteHasMove = hasValidMoves(WHITE);
    
    // 游戏结束条件：棋盘已满或双方都无合法移动
    if (boardFull || (!blackHasMove && !whiteHasMove)) {
        auto [blackCount, whiteCount] = countPieces();
        if (blackCount > whiteCount) return BLACK_WIN;
        else if (whiteCount > blackCount) return WHITE_WIN;
        else return TIED;
    }
    
    return IN_PROGRESS;
}

std::vector<chessgame::Point> OthelloRule::getValidMoves(PieceType player) const {
    std::vector<Point> moves;
    int size = board->getSize();
    
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            if (isValidMove(i, j, player)) moves.push_back({i, j});
    
    return moves;
}

bool OthelloRule::hasValidMoves(PieceType player) const {
    int size = board->getSize();
    
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            if (isValidMove(i, j, player)) return true;
    
    return false;
}

std::pair<int, int> OthelloRule::countPieces() const {
    int blackCount = 0, whiteCount = 0;
    int size = board->getSize();
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            PieceType piece = board->getPiece(i, j);
            if (piece == BLACK) blackCount++;
            else if (piece == WHITE) whiteCount++;
        }
    }
    
    return std::make_pair(blackCount, whiteCount);
}

chessgame::PieceType OthelloRule::getOpponent(PieceType player) const {
    return (player == BLACK) ? WHITE : BLACK;
}