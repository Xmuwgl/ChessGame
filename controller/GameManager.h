#pragma once

#include "../utils/Type.h"
#include <stack>

namespace chessgame::controller {
class GameManager {
private:
    unique_ptr<Board> board;
    unique_ptr<GameRule> rule;
    unique_ptr<GameView> view;
    
    PieceType currentPlayer;
    GameType gameType;
    int passCount; // 用于判断围棋终局: 连续两次虚着
    
    std::stack<GameState> history; // 备忘录模式: 历史记录用于悔棋
    bool showHints;

    void saveState();
public:
    GameManager();
    ~GameManager();
    
    void startGame(GameType type, PieceType firstPlayer);

    void initGame();

    bool saveGame(const std::string& filename);
    bool loadGame(const std::string& filename);

};
}