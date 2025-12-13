
#pragma once
#include "../utils/Type.h"
#include "../model/Board.h"
#include "../model/Rule.h"
#include "../model/GameMemento.h"
#include <memory>
#include <string>
#include <utility>

namespace chessgame::facade {

// 游戏外观类 (Facade Pattern)
// 统一管理五子棋和围棋游戏，提供简化的接口
class GameFacade {
private:
    // 游戏状态
    std::unique_ptr<model::Board> board;
    PieceType currentPlayer;
    GameType gameType;
    GameStatus gameStatus;
    int passCount;
    
    // 规则策略
    std::unique_ptr<model::Rule> rule;
    
    // 备忘录管理者
    std::unique_ptr<model::GameCaretaker> caretaker;
    
    // 初始化游戏规则
    void initRule();
    
    // 保存当前状态到备忘录
    void saveStateToMemento();

    // 计算围棋双方分数 (简单地：地盘 + 棋子数)
    std::pair<int, int> computeGoScore() const;

public:
    GameFacade();
    ~GameFacade() = default;
    
    // 游戏初始化
    bool initGame(GameType type, int boardSize);
    
    // 落子操作
    bool makeMove(int x, int y, PieceType player);
    
    // 虚着操作（仅围棋）
    bool passMove(PieceType player);
    
    // 悔棋操作
    bool undoMove();
    
    // 认输操作
    bool resign(PieceType player);
    
    // 重新开始游戏
    bool restartGame();
    
    // 保存游戏
    bool saveGame(const std::string& filename);
    
    // 加载游戏
    bool loadGame(const std::string& filename);
    
    // 获取游戏状态
    GameStatus getGameStatus() const { return gameStatus; }
    
    // 获取当前玩家
    PieceType getCurrentPlayer() const { return currentPlayer; }
    
    // 获取游戏类型
    GameType getGameType() const { return gameType; }
    
    // 设置游戏类型
    void setGameType(GameType type) { gameType = type; }
    
    // 获取棋盘
    const model::Board& getBoard() const { return *board; }
    
    // 获取棋盘（非const版本）
    model::Board& getBoard() { return *board; }
    
    // 设置当前玩家
    void setCurrentPlayer(PieceType player) { currentPlayer = player; }
    
    // 减少虚着计数
    void decrementPassCount() { if (passCount > 0) passCount--; }
    
    // 设置游戏状态
    void setGameStatus(GameStatus status) { gameStatus = status; }
    
    // 检查是否支持虚着
    bool supportsPass() const;
    
    // 检查落子是否合法
    bool isValidMove(int x, int y, PieceType player) const;
    
    // 获取连续虚着次数
    int getPassCount() const { return passCount; }
    
    // 切换当前玩家
    void switchPlayer();
};

}