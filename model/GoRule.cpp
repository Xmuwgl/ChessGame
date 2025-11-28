#include "GoRule.h"

using namespace chessgame::model;




GoRule::GoRule(Board* b) : Rule(b) {}

bool GoRule::isValidMove(int x, int y, PieceType player) override {
    if (!board->isValidBounds(x, y)) return false;
    if (board->getPiece(x, y) != EMPTY) return false;

    // 临时落子测试
    board->setPiece(x, y, player);
    
    // 1. 检查是否能提掉对方
    bool captures = false;
    PieceType opponent = (player == BLACK) ? WHITE : BLACK;
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    
    // 检查四周对方棋子是否气绝
    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (board->isValidBounds(nx, ny) && board->getPiece(nx, ny) == opponent) {
            vector<Point> group;
            if (getLiberties(nx, ny, opponent, group) == 0) {
                captures = true;
            }
        }
    }

    // 2. 禁入点判断
    bool suicide = false;
    if (!captures) {
        vector<Point> selfGroup;
        if (getLiberties(x, y, player, selfGroup) == 0) {
            suicide = true;
        }
    }

    // 撤销临时落子
    board->setPiece(x, y, EMPTY);

    return !suicide;
}

void GoRule::makeMove(int x, int y, PieceType player) override {
    if (x == -1 && y == -1) return; // 虚着

    board->setPiece(x, y, player);
    capture(x, y, (player == BLACK) ? WHITE : BLACK);
}

GameStatus GoRule::checkWin(int lastX, int lastY) override {
    // 无子可下或满盘则平局, 否则返回正在游戏
    bool full = true;
    for (int i = 0; i < board->getSize(); ++i) {
        for (int j = 0; j < board->getSize(); ++j) {
            if (board->getPiece(i, j) == EMPTY) {
                full = false;
                break;
            }
        }
        if (!full) break;
    }
    if (full) return TIED;
    return IN_PROGRESS;
}