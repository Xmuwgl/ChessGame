#include "GoRule.h"
#include "Board.h"
#include <iostream>

using namespace chessgame::model;

int GoRule::getLiberties(int x, int y, PieceType color, std::vector<Point>& group) {
    // 重置访问标记
    for(int i=0; i<board->getSize(); ++i)
        for(int j=0; j<board->getSize(); ++j) visited[i][j] = false;

    return countLibertiesDFS(x, y, color, group);
}

int GoRule::countLibertiesDFS(int x, int y, PieceType color, std::vector<Point>& group) {
    if (!board->isValidBounds(x, y)) return 0;
    if (visited[x][y]) return 0;
    
    visited[x][y] = true;
    PieceType p = board->getPiece(x, y);

    if (p == EMPTY) return 1; // 找到气
    if (p != color) return 0; // 遇到对方棋子

    group.push_back({x, y}); // 加入同色群组
    
    int liberties = 0;
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    for (int i = 0; i < 4; ++i) {
        liberties += countLibertiesDFS(x + dx[i], y + dy[i], color, group);
    }
    return liberties;
}

void GoRule::capture(int x, int y, PieceType opponent) {
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (board->isValidBounds(nx, ny) && board->getPiece(nx, ny) == opponent) {
            std::vector<Point> group;
            if (getLiberties(nx, ny, opponent, group) == 0) {
                // 气为0，提子
                for (auto& p : group) {
                    board->setPiece(p.x, p.y, EMPTY);
                }
            }
        }
    }
}

bool GoRule::isValidMove(int x, int y, PieceType player) {
    if (!board->isValidBounds(x, y)) return false;
    if (board->getPiece(x, y) != EMPTY) return false;

    // 临时落子测试
    board->setPiece(x, y, player);
    
    // 1. 检查是否能提掉对方 (如果能提对方，则自己一定有气，不算自杀)
    bool captures = false;
    PieceType opponent = (player == BLACK) ? WHITE : BLACK;
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    
    // 检查四周对方棋子是否气绝
    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (board->isValidBounds(nx, ny) && board->getPiece(nx, ny) == opponent) {
            std::vector<Point> group;
            if (getLiberties(nx, ny, opponent, group) == 0) {
                captures = true;
            }
        }
    }

    // 2. 如果没提子，检查自己是否有气 (禁入点判断)
    bool suicide = false;
    if (!captures) {
        std::vector<Point> selfGroup;
        if (getLiberties(x, y, player, selfGroup) == 0) {
            suicide = true;
        }
    }

    // 撤销临时落子
    board->setPiece(x, y, EMPTY);

    // 简单劫禁入：避免立即打劫（上一手被吃点不能马上打回）。
    // 这里省略更复杂的劫判断，实际规则需保留历史棋形对比。

    return !suicide;
}

void GoRule::makeMove(int x, int y, PieceType player) {
    if (x == -1 && y == -1) return; // 虚着

    board->setPiece(x, y, player);
    capture(x, y, (player == BLACK) ? WHITE : BLACK);
}

chessgame::GameStatus GoRule::checkWin(int lastX, int lastY) {
    // 围棋的胜负通常由双方连续虚着触发，这里由外部逻辑控制
    // 如果需要简单判断：无子可下或满盘
    return IN_PROGRESS;
}
