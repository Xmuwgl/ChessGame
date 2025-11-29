#pragma once
#include "../utils/Type.h"
#include <memory>

namespace chessgame::model {
class Board;

// 游戏状态备忘录类 (Memento Pattern)
class GameMemento {
private:
    std::string boardState;      // 序列化的棋盘状态
    PieceType currentPlayer;      // 当前玩家
    int passCount;                // 连续虚着次数
    GameStatus status;            // 游戏状态
    GameType gameType;            // 游戏类型
    int boardSize;                // 棋盘大小

public:
    GameMemento(const Board& board, PieceType player, int passes, 
                GameStatus gameStatus, GameType type);
    
    // 获取保存的状态信息
    std::string getBoardState() const { return boardState; }
    PieceType getCurrentPlayer() const { return currentPlayer; }
    int getPassCount() const { return passCount; }
    GameStatus getStatus() const { return status; }
    GameType getGameType() const { return gameType; }
    int getBoardSize() const { return boardSize; }
};

// 备忘录管理者类
class GameCaretaker {
private:
    std::vector<std::shared_ptr<GameMemento>> history;
    int maxHistorySize;  // 最大历史记录数量

public:
    GameCaretaker(int maxSize = 50) : maxHistorySize(maxSize) {}
    
    // 保存状态
    void saveState(std::shared_ptr<GameMemento> memento);
    
    // 获取上一个状态
    std::shared_ptr<GameMemento> getPreviousState();
    
    // 检查是否有历史记录
    bool hasHistory() const { return !history.empty(); }
    
    // 清空历史记录
    void clearHistory();
    
    // 获取历史记录数量
    int getHistoryCount() const { return history.size(); }
};

}