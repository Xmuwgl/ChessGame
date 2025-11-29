#pragma once
#include "../utils/Type.h"
#include <memory>
#include <stack>
#include <string>

/**
 * @brief 采用抽象工厂模式, 不同类型的游戏 {Gomoku, Go} 实现抽象接口.
 */

namespace chessgame::model {
class Board;
class Rule;

class AbstractGame {
protected:
    std::shared_ptr<Board> board{};
    std::shared_ptr<Rule> rule{};
    PieceType currentPlayer{BLACK};
    GameType gameType{GOMOKU};
    int passCount{0};
    std::stack<GameState> history{};

public:
    AbstractGame() = default;
    virtual ~AbstractGame() = default;

    // 游戏初始化
    virtual void initGame() = 0;
    
    // 游戏运行主循环
    virtual void run() = 0;
    
    // 落子操作
    virtual bool makeMove(int x, int y) = 0;
    virtual bool makeMove(const chessgame::Move& move) = 0;

    // 虚着操作
    virtual bool pass() = 0;
    
    // 认输操作
    virtual void resign() = 0;
    
    // 悔棋操作
    virtual bool undo() = 0;
    
    // 存档/读档
    virtual void saveGame(const std::string& filename) = 0;
    virtual bool loadGame(const std::string& filename) = 0;
    
    // 游戏状态查询
    PieceType getCurrentPlayer() const { return currentPlayer; }
    GameType getGameType() const { return gameType; }
    std::shared_ptr<Board> getBoard() const { return board; }
    virtual GameStatus getGameStatus() const = 0;
    
    // 保存当前状态: 备忘录模式
    virtual void saveState() = 0;
    
    // 检查游戏是否结束
    virtual bool isGameOver() const = 0;
    
    // 获取获胜者
    virtual PieceType getWinner() const = 0;
};
}
