#include "GomokuRule.h"

using namespace chessgame::model;

GomokuRule::GomokuRule(Board* b) : GameRule(b) {}

bool GomokuRule::isValidMove(int x, int y, PieceType player) override {
    return board->isValidBounds(x, y) && board->getPiece(x, y) == EMPTY;
}

void GomokuRule::makeMove(int x, int y, PieceType player) override {
    board->setPiece(x, y, player);
}

GomokuRule::GameStatus checkWin(int x, int y) override {
    if (x == -1 && y == -1) return PLAYING; // 虚着或初始状态
    
    PieceType current = board->getPiece(x, y);
    if (current == EMPTY) return PLAYING;

    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}}; // 横、竖、正斜、反斜

    for (auto& dir : directions) {
        int count = 1;
        // 正向查找
        for (int i = 1; i < 5; ++i) {
            if (board->getPiece(x + i * dir[0], y + i * dir[1]) == current) count++;
            else break;
        }
        // 反向查找
        for (int i = 1; i < 5; ++i) {
            if (board->getPiece(x - i * dir[0], y - i * dir[1]) == current) count++;
            else break;
        }
        if (count >= 5) return (current == BLACK) ? BLACK_WIN : WHITE_WIN;
    }

    // 检查平局
    bool full = true;
    for(int i=0; i<board->getSize(); ++i)
        for(int j=0; j<board->getSize(); ++j)
            if(board->getPiece(i, j) == EMPTY) full = false;
    
    return full ? DRAW : PLAYING;
}