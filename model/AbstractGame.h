#pragma once
#include "../utils/Type.h"
#include <memory>
#include <stack>
#include <string>

namespace chessgame::model {
class Board;
class GameRule;

class AbstractGame {
protected:
    std::shared_ptr<Board> board;
    std::shared_ptr<GameRule> rule;
    PieceType currentPlayer;
    GameType gameType;
    int passCount;
    std::stack<GameState> history;

public:
    AbstractGame(GameType type, int boardSize);
    virtual ~AbstractGame() = default;

    // 游戏初始化
    virtual void initGame();
    
    // 游戏运行主循环
    virtual void run() = 0;
    
    // 落子操作
    virtual bool makeMove(int x, int y);
    virtual bool makeMove(const Move& move);
    
    // 虚着操作
    virtual bool pass();
    
    // 认输操作
    virtual void resign();
    
    // 悔棋操作
    virtual bool undo();
    
    // 存档/读档
    virtual void saveGame(const std::string& filename);
    virtual bool loadGame(const std::string& filename);
    
    // 游戏状态查询
    PieceType getCurrentPlayer() const { return currentPlayer; }
    GameType getGameType() const { return gameType; }
    GameStatus getGameStatus() const;
    std::shared_ptr<Board> getBoard() const { return board; }
    
    // 保存当前状态: 备忘录模式
    void saveState();
    
    // 检查游戏是否结束
    virtual bool isGameOver() const = 0;
    
    // 获取获胜者
    virtual PieceType getWinner() const = 0;
};
}