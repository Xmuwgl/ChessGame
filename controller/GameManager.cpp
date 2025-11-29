#include "GameManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace chessgame::controller;

GameManager::GameManager() : running(false) {
    gameFacade = std::make_unique<facade::GameFacade>();
    gameView = std::make_unique<view::ConsoleView>();
}

bool GameManager::initializeGame() {
    // 选择游戏类型
    while (true) {
        std::string typeStr = gameView->getUserInput("请选择游戏类型 (1: 五子棋, 2: 围棋): ");
        if (typeStr == "1" || typeStr == "2") {
            GameType type = (typeStr == "1") ? GOMOKU : GO;
            
            // 设置棋盘大小
            int boardSize = 0;
            while (true) {
                std::string sizeStr = gameView->getUserInput("请输入棋盘大小 (8-19): ");
                try {
                    boardSize = std::stoi(sizeStr);
                    if (boardSize >= 8 && boardSize <= 19) break;
                } catch(...) {
                    gameView->showError("输入无效，请重新输入。");
                }
            }
            
            // 初始化游戏
            if (gameFacade->initGame(type, boardSize)) {
                return true;
            } else {
                gameView->showError("游戏初始化失败！");
            }
        } else {
            gameView->showError("输入无效，请输入 1 或 2。");
        }
    }
}

std::unique_ptr<Command> GameManager::parseCommand(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;
    
    // 转换为小写
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    PieceType currentPlayer = gameFacade->getCurrentPlayer();
    
    if (cmd == "quit") {
        return std::make_unique<ResignCommand>(gameFacade.get(), currentPlayer);
    } else if (cmd == "help") {
        gameView->showHelp();
        return nullptr;
    } else if (cmd == "undo") {
        return std::make_unique<UndoCommand>(gameFacade.get());
    } else if (cmd == "pass") {
        return std::make_unique<PassCommand>(gameFacade.get(), currentPlayer);
    } else if (cmd == "resign") {
        return std::make_unique<ResignCommand>(gameFacade.get(), currentPlayer);
    } else if (cmd == "save") {
        std::string filename;
        iss >> filename;
        if (filename.empty()) {
            filename = "savegame.txt";
        }
        return std::make_unique<SaveCommand>(gameFacade.get(), filename);
    } else if (cmd == "load") {
        std::string filename;
        iss >> filename;
        if (filename.empty()) {
            filename = "savegame.txt";
        }
        return std::make_unique<LoadCommand>(gameFacade.get(), filename);
    } else if (cmd == "restart") {
        return std::make_unique<RestartCommand>(gameFacade.get());
    } else if (cmd == "hint") {
        // 切换提示显示
        bool showHints = gameView->getShowHints();
        gameView->setShowHints(!showHints);
        gameView->showHint(showHints ? "提示已隐藏" : "提示已显示");
        return nullptr;
    } else {
        // 尝试解析为坐标
        try {
            int x, y;
            std::istringstream coordStream(input);
            if (coordStream >> x >> y) {
                // 转换为0索引
                x--;
                y--;
                return std::make_unique<MoveCommand>(gameFacade.get(), x, y, currentPlayer);
            }
        } catch(...) {
            // 不是有效的坐标
        }
    }
    gameView->showError("不合法的指令，请输入 'help' 查看用法。");
    return nullptr;
}

bool GameManager::executeCommand(std::unique_ptr<Command> command) {
    if (!command) {
        return false;
    }
    
    bool success = command->execute();
    if (success) {
        // 将命令添加到历史记录
        commandHistory.push_back(std::move(command));
    } else {
        gameView->showError("命令执行失败！");
    }
    
    return success;
}

void GameManager::gameLoop() {
    running = true;
    std::string message = "游戏开始！输入 'help' 查看指令。";
    
    while (running) {
        // 显示当前游戏状态
        gameView->displayBoard(
            gameFacade->getBoard(), 
            gameFacade->getCurrentPlayer(),
            gameFacade->getGameType(),
            message
        );
        
        // 检查游戏是否结束
        GameStatus status = gameFacade->getGameStatus();
        if (status != IN_PROGRESS) {
            gameView->showGameResult(status);
            break;
        }
        
        message = "";
        
        // 获取用户输入
        std::string input = gameView->getUserInput("请输入指令 > ");
        
        // 解析并执行命令
        auto command = parseCommand(input);
        if (!command) {
            continue;  // 可能是help或hint等不需要执行的命令
        }
        
        // 检查是否是退出命令
        if (dynamic_cast<ResignCommand*>(command.get())) {
            std::string confirm = gameView->getUserInput("确认认输/退出吗? (y/n): ");
            if (confirm != "y" && confirm != "Y") {
                continue;
            }
        }
        
        // 执行命令
        if (!executeCommand(std::move(command))) {
            message = "操作失败，请重试。";
        } else {
            GameStatus status = gameFacade->getGameStatus();
            if (status != IN_PROGRESS) {
                gameView->showGameResult(status);
                break;
            }
        }
    }
}

void GameManager::startGame() {
    if (initializeGame()) {
        gameLoop();
    }
}

void GameManager::restartGame() {
    if (gameFacade->restartGame()) {
        commandHistory.clear();
        gameLoop();
    } else {
        gameView->showError("重新开始游戏失败！");
    }
}

void GameManager::quitGame() {
    running = false;
}
