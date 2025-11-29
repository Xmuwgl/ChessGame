#pragma once

#include "../utils/Type.h"
#include "../facade/GameFacade.h"
#include "../view/GameView.h"
#include "Command.h"
#include <memory>
#include <vector>

namespace chessgame::controller {

class GameManager {
private:
    std::unique_ptr<facade::GameFacade> gameFacade;
    std::unique_ptr<view::GameView> gameView;
    std::vector<std::unique_ptr<Command>> commandHistory;
    
    bool running;
    
    // 解析用户输入并创建命令
    std::unique_ptr<Command> parseCommand(const std::string& input);
    
    // 执行命令
    bool executeCommand(std::unique_ptr<Command> command);
    
    // 初始化游戏
    bool initializeGame();
    
    // 游戏主循环
    void gameLoop();

public:
    GameManager();
    ~GameManager() = default;
    
    // 开始游戏
    void startGame();
    
    // 重新开始游戏
    void restartGame();
    
    // 退出游戏
    void quitGame();
};

}