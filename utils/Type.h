#pragma once
#include <string>
#include <vector>

namespace chessgame {

enum PieceType {EMPTY, BLACK, WHITE};

enum GameType {GOMOKU, GO};

enum GameStatus {IN_PROGRESS, BLACK_WIN, WHITE_WIN, TIED};

// 棋盘位置
struct Point {
    int x, y;
    bool operator<(const Point& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

// 移动结构体: 表示一次落子
struct Move {
    int x, y;
    PieceType piece;
    bool isPass;  // 是否为虚着
    bool isResign; // 是否为认输
    
    Move(int x = -1, int y = -1, PieceType p = EMPTY, bool pass = false, bool resign = false)
        : x(x), y(y), piece(p), isPass(pass), isResign(resign) {}
};

// 游戏状态结构体: 用于保存和恢复游戏状态
struct GameState {
    std::string boardState;  // 序列化的棋盘状态
    PieceType currentPlayer;
    int passCount;
    GameStatus status;
    
    GameState(int size = 0) 
        : currentPlayer(BLACK), 
          passCount(0), 
          status(IN_PROGRESS) {}
};

}