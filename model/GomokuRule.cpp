#include "GomokuRule.h"
#include "Board.h"

using namespace chessgame::model;

bool GomokuRule::isValidMove(int x, int y, PieceType player) {
    return board->isValidBounds(x, y) && board->getPiece(x, y) == EMPTY;
}

void GomokuRule::makeMove(int x, int y, PieceType player) {
    board->setPiece(x, y, player);
}

chessgame::GameStatus GomokuRule::checkWin(int lastX, int lastY) {
    if (lastX == -1 && lastY == -1) return IN_PROGRESS; // 虚着或初始状态
    
    PieceType current = board->getPiece(lastX, lastY);
    if (current == EMPTY) return IN_PROGRESS;

    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}}; // 横、竖、正斜、反斜

    for (auto& dir : directions) {
        int count = 1;
        // 正向查找
        for (int i = 1; i < 5; ++i) {
            if (board->getPiece(lastX + i * dir[0], lastY + i * dir[1]) == current) count++;
            else break;
        }
        // 反向查找
        for (int i = 1; i < 5; ++i) {
            if (board->getPiece(lastX - i * dir[0], lastY - i * dir[1]) == current) count++;
            else break;
        }
        if (count >= 5) return (current == BLACK) ? BLACK_WIN : WHITE_WIN;
    }

    // 检查平局
    bool full = true;
    for(int i=0; i<board->getSize(); ++i)
        for(int j=0; j<board->getSize(); ++j)
            if(board->getPiece(i, j) == EMPTY) full = false;
    
    return full ? TIED : IN_PROGRESS;
}