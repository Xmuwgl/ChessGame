#include "GameManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <cstring>

using namespace chessgame::controller;

GameManager::GameManager() : gameMode(GameMode::PVP), running(false), isRecording(false), 
                              leaderboardManager(&accountManager), isHost(false), 
                              isNetworkGame(false), networkPlayerType(0) {
    gameFacade = std::make_unique<facade::GameFacade>();
    gameView = std::make_unique<view::ConsoleView>();
}

bool GameManager::initializeGame() {
    // 初始化游戏模式
    if (!initializeGameMode()) {
        return false;
    }
    
    // 选择游戏类型
    while (true) {
        std::string typeStr = gameView->getUserInput("请选择游戏类型 (1: 五子棋, 2: 围棋, 3: 黑白棋): ");
        if (typeStr == "1" || typeStr == "2" || typeStr == "3") {
            GameType type;
            if (typeStr == "1") {
                type = GOMOKU;
            } else if (typeStr == "2") {
                type = GO;
            } else {
                type = OTHELLO;
            }
            
            // 设置棋盘大小
            int boardSize = 0;
            while (true) {
                std::string sizeStr = gameView->getUserInput("请输入棋盘大小 (8-19): ");
                try {
                    boardSize = std::stoi(sizeStr);
                    if (boardSize >= 8 && boardSize <= 19) break;
                } catch(...) {
                    gameView->showError("输入无效, 请重新输入.");
                }
            }
            
            // 初始化游戏
            if (gameFacade->initGame(type, boardSize)) {
                // 初始化AI玩家
                initializeAIPlayers();
                
                // 询问是否开始录像
                std::string recordChoice = gameView->getUserInput("是否录像? (y/n): ");
                if (recordChoice == "y" || recordChoice == "Y") {
                    startRecording();
                }
                
                return true;
            } else {
                gameView->showError("游戏初始化失败!");
            }
        } else {
            gameView->showError("输入无效, 请输入 1、2 或 3.");
        }
    }
}

void GameManager::networkGameLoop() {
    while (isNetworkGame && gameFacade->getGameStatus() == IN_PROGRESS) {
        // 显示当前棋盘状态
        gameView->displayBoard(
            gameFacade->getBoard(),
            gameFacade->getCurrentPlayer(),
            gameFacade->getGameType(),
            "",
            getPlayerName(BLACK),
            getPlayerName(WHITE),
            getPlayerStats(BLACK),
            getPlayerStats(WHITE)
        );
        
        // 检查是否轮到当前玩家
        bool isMyTurn = false;
        if (networkPlayerType == 1 && gameFacade->getCurrentPlayer() == BLACK) {
            isMyTurn = true;
        } else if (networkPlayerType == 2 && gameFacade->getCurrentPlayer() == WHITE) {
            isMyTurn = true;
        }
        
        if (isMyTurn) {
            // 获取用户输入
            std::string input = gameView->getUserInput("请输入坐标 (格式: x,y) 或 'quit' 退出: ");
            
            // 清理输入：去除前后空白字符
            input.erase(0, input.find_first_not_of(" \t\r\n"));
            input.erase(input.find_last_not_of(" \t\r\n") + 1);
            
            if (input == "quit") {
                handleNetworkDisconnection();
                break;
            }
            
            // 解析移动命令
            std::unique_ptr<Command> command = parseCommand(input);
            if (command) {
                // 执行命令
                if (executeCommand(std::move(command))) {
                    // 发送移动信息到网络
                    // 这里需要从命令中提取坐标信息
                    // 简化处理：重新解析输入
                    std::istringstream iss(input);
                    std::string token;
                    int row = -1, col = -1;
                    
                    if (std::getline(iss, token, ',')) {
                        row = std::stoi(token);
                    }
                    if (std::getline(iss, token, ',')) {
                        col = std::stoi(token);
                    }
                    
                    if (row >= 0 && col >= 0) {
                        sendNetworkMove(row, col);
                    }
                }
            } else {
                gameView->showError("无效的输入格式! 请使用 x,y 格式");
            }
        } else {
            // 不是我的回合，等待网络消息
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 检查游戏状态
        GameStatus status = gameFacade->getGameStatus();
        if (status != IN_PROGRESS) {
            // 发送游戏结束消息
            network::NetworkMessage endMsg(network::MessageType::GAME_END, std::to_string(static_cast<int>(status)));
            
            if (isHost && networkServer) {
                networkServer->broadcastMessage(endMsg);
            } else if (!isHost && networkClient) {
                networkClient->sendMessage(endMsg);
            }
            
            handleGameEnd(status);
            break;
        }
    }
}

void GameManager::showAccountMenu() {
    while (true) {
        gameView->showMessage("\n===== 账户管理 =====");
        
        if (accountManager.isLoggedIn()) {
            gameView->showMessage("当前用户: " + accountManager.getCurrentUser());
            gameView->showMessage("1. 查看个人统计");
            gameView->showMessage("2. 修改密码");
            gameView->showMessage("3. 登出");
            gameView->showMessage("4. 删除账户");
            gameView->showMessage("5. 返回主菜单");
        } else {
            gameView->showMessage("1. 注册账户");
            gameView->showMessage("2. 登录");
            gameView->showMessage("3. 返回主菜单");
        }
        
        std::string choice = gameView->getUserInput("请选择: ");
        
        if (accountManager.isLoggedIn()) {
            if (choice == "1") {
                showUserStats();
            } else if (choice == "2") {
                changePassword();
            } else if (choice == "3") {
                logout();
            } else if (choice == "4") {
                deleteAccount();
            } else if (choice == "5") {
                break;
            } else {
                gameView->showError("无效的选择!");
            }
        } else {
            if (choice == "1") {
                registerAccount();
            } else if (choice == "2") {
                login();
            } else if (choice == "3") {
                break;
            } else {
                gameView->showError("无效的选择!");
            }
        }
    }
}

void GameManager::registerAccount() {
    gameView->showMessage("\n===== 注册账户 =====");
    
    std::string username = gameView->getUserInput("请输入用户名: ");
    if (username.empty()) {
        gameView->showError("用户名不能为空!");
        return;
    }
    
    if (accountManager.userExists(username)) {
        gameView->showError("用户名已存在!");
        return;
    }
    
    std::string password = gameView->getUserInput("请输入密码: ");
    if (password.empty()) {
        gameView->showError("密码不能为空!");
        return;
    }
    
    std::string confirmPassword = gameView->getUserInput("请确认密码: ");
    if (password != confirmPassword) {
        gameView->showError("两次输入的密码不一致!");
        return;
    }
    
    if (accountManager.registerAccount(username, password)) {
        gameView->showHint("注册成功!");
    } else {
        gameView->showError("注册失败!");
    }
}

void GameManager::login() {
    gameView->showMessage("\n===== 登录 =====");
    
    std::string username = gameView->getUserInput("请输入用户名: ");
    if (username.empty()) {
        gameView->showError("用户名不能为空!");
        return;
    }
    
    std::string password = gameView->getUserInput("请输入密码: ");
    if (password.empty()) {
        gameView->showError("密码不能为空!");
        return;
    }
    
    if (accountManager.login(username, password)) {
        gameView->showHint("登录成功!欢迎, " + username + "!");
    } else {
        gameView->showError("用户名或密码错误!");
    }
}

void GameManager::logout() {
    if (accountManager.isLoggedIn()) {
        std::string username = accountManager.getCurrentUser();
        accountManager.logout();
        gameView->showHint("已成功登出, " + username + "!");
    }
}

void GameManager::showUserStats() {
    if (!accountManager.isLoggedIn()) {
        gameView->showError("请先登录!");
        return;
    }
    
    auto* account = accountManager.getCurrentUserAccount();
    if (!account) {
        gameView->showError("获取用户信息失败!");
        return;
    }
    
    const auto& stats = account->getStats();
    
    // 显示最后游戏时间
    std::string lastPlayTimeStr = "";
    if (stats.lastPlayTime > 0) {
        std::tm* tm = std::localtime(&stats.lastPlayTime);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
        lastPlayTimeStr = "最后游戏时间: " + std::string(buffer);
    }
    
    // 显示排名
    leaderboardManager.updateLeaderboard();
    int rank = leaderboardManager.getUserRank(account->getUsername());
    
    // 使用GameView的新方法显示用户统计
    gameView->showUserStats(
        account->getUsername(),
        stats.totalGames,
        stats.wins,
        stats.losses,
        stats.draws,
        stats.winRate,
        stats.gomokuWins,
        stats.goWins,
        stats.othelloWins,
        stats.aiWins,
        stats.humanWins,
        rank
    );
    
    // 显示最后游戏时间
    if (!lastPlayTimeStr.empty()) {
        gameView->showMessage(lastPlayTimeStr);
    }
}

void GameManager::showLeaderboard() {
    leaderboardManager.updateLeaderboard();
    leaderboardManager.displayLeaderboard();
}

void GameManager::changePassword() {
    if (!accountManager.isLoggedIn()) {
        gameView->showError("请先登录!");
        return;
    }
    
    std::string username = accountManager.getCurrentUser();
    
    gameView->showMessage("\n===== 修改密码 =====");
    
    std::string oldPassword = gameView->getUserInput("请输入当前密码: ");
    if (oldPassword.empty()) {
        gameView->showError("密码不能为空!");
        return;
    }
    
    std::string newPassword = gameView->getUserInput("请输入新密码: ");
    if (newPassword.empty()) {
        gameView->showError("密码不能为空!");
        return;
    }
    
    std::string confirmPassword = gameView->getUserInput("请确认新密码: ");
    if (newPassword != confirmPassword) {
        gameView->showError("两次输入的密码不一致!");
        return;
    }
    
    if (accountManager.changePassword(username, oldPassword, newPassword)) {
        gameView->showHint("密码修改成功!");
    } else {
        gameView->showError("密码修改失败!请检查当前密码是否正确.");
    }
}

void GameManager::deleteAccount() {
    if (!accountManager.isLoggedIn()) {
        gameView->showError("请先登录!");
        return;
    }
    
    std::string username = accountManager.getCurrentUser();
    
    gameView->showMessage("\n===== 删除账户 =====");
    gameView->showMessage("警告：此操作不可恢复!");
    
    std::string confirm = gameView->getUserInput("确定要删除账户 '" + username + "' 吗？(输入 'yes' 确认): ");
    if (confirm != "yes") {
        gameView->showHint("已取消删除操作.");
        return;
    }
    
    std::string password = gameView->getUserInput("请输入密码以确认删除: ");
    if (password.empty()) {
        gameView->showError("密码不能为空!");
        return;
    }
    
    if (accountManager.deleteAccount(username, password)) {
        gameView->showHint("账户已成功删除!");
    } else {
        gameView->showError("删除失败!请检查密码是否正确.");
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
    gameView->showError("不合法的指令, 请输入 'help' 查看用法.");
    return nullptr;
}

bool GameManager::executeCommand(std::unique_ptr<Command> command) {
    if (!command) {
        return false;
    }
    
    bool success = command->execute();
    if (success) {
        // 如果正在录像, 记录这一步
        if (isRecording && currentRecord) {
            // 获取移动信息
            MoveCommand* moveCmd = dynamic_cast<MoveCommand*>(command.get());
            PassCommand* passCmd = dynamic_cast<PassCommand*>(command.get());
            ResignCommand* resignCmd = dynamic_cast<ResignCommand*>(command.get());
            
            if (moveCmd) {
                Move move;
                move.x = moveCmd->getX();
                move.y = moveCmd->getY();
                move.piece = moveCmd->getPlayer();
                move.isPass = false;
                move.isResign = false;
                
                currentRecord->addMove(move, "Move");
            } else if (passCmd) {
                Move move;
                move.x = -1;
                move.y = -1;
                move.piece = passCmd->getPlayer();
                move.isPass = true;
                move.isResign = false;
                
                currentRecord->addMove(move, "Pass");
            } else if (resignCmd) {
                Move move;
                move.x = -1;
                move.y = -1;
                move.piece = resignCmd->getPlayer();
                move.isPass = false;
                move.isResign = true;
                
                currentRecord->addMove(move, "Resign");
            }
        }
        
        // 将命令添加到历史记录
        commandHistory.push_back(std::move(command));
    } else {
        gameView->showError("命令执行失败!");
    }
    
    return success;
}

void GameManager::gameLoop() {
    running = true;
    std::string message = "游戏开始!输入 'help' 查看指令.";
    
    while (running) {
        // 显示当前游戏状态
        gameView->displayBoard(
            gameFacade->getBoard(), 
            gameFacade->getCurrentPlayer(),
            gameFacade->getGameType(),
            message,
            getPlayerName(BLACK),
            getPlayerName(WHITE),
            getPlayerStats(BLACK),
            getPlayerStats(WHITE)
        );
        
        // 检查游戏是否结束
        GameStatus status = gameFacade->getGameStatus();
        if (status != IN_PROGRESS) {
            gameView->showGameResult(status);
            handleGameEnd(status);
            break;
        }
        
        message = "";
        
        // 检查当前玩家是否是AI
        if (isCurrentPlayerAI()) {
            // AI回合
            aiTurn();
            
            // 在AI对AI模式下添加延迟, 使游戏过程可见
            if (gameMode == GameMode::AIVAI) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        } else {
            // 玩家回合
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
                message = "操作失败, 请重试.";
            }
        }
        
        // 检查游戏状态
        GameStatus newStatus = gameFacade->getGameStatus();
        if (newStatus != IN_PROGRESS) {
            gameView->displayBoard(
                gameFacade->getBoard(), 
                gameFacade->getCurrentPlayer(),
                gameFacade->getGameType(),
                "",
                getPlayerName(BLACK),
                getPlayerName(WHITE),
                getPlayerStats(BLACK),
                getPlayerStats(WHITE)
            );
            gameView->showGameResult(newStatus);
            handleGameEnd(newStatus);
            break;
        }
    }
}

void GameManager::startGame() {
    running = true;
    
    // 显示主菜单
    while (running) {
        gameView->showMessage("\n===== 棋类游戏系统 =====");
        gameView->showMessage("1. 新游戏");
        gameView->showMessage("2. 回放录像");
        gameView->showMessage("3. 账户管理");
        gameView->showMessage("4. 排行榜");
        gameView->showMessage("5. 网络对战");
        gameView->showMessage("6. 退出");
        
        std::string choice = gameView->getUserInput("请选择: ");
        
        // 清理输入：去除前后空白字符
        choice.erase(0, choice.find_first_not_of(" \t\r\n"));
        choice.erase(choice.find_last_not_of(" \t\r\n") + 1);
        
        if (choice == "1") {
            // 新游戏模式
            if (!initializeGame()) {
                continue;
            }
            
            // 游戏主循环
            gameLoop();
        } else if (choice == "2") {
            // 获取用户相关的录像列表
        std::vector<std::string> records = getUserRecordList();
            if (records.empty()) {
                gameView->showError("没有可用的录像文件!");
                continue;
            }
            
            // 显示录像列表
            gameView->showMessage("可用录像文件:");
            for (size_t i = 0; i < records.size(); ++i)
                gameView->showMessage(std::to_string(i + 1) + ". " + records[i]);
            
            // 选择录像
            std::string recordChoice = gameView->getUserInput("请选择录像文件编号: ");
            
            // 清理输入: 去除前后空白字符
            recordChoice.erase(0, recordChoice.find_first_not_of(" \t\r\n"));
            recordChoice.erase(recordChoice.find_last_not_of(" \t\r\n") + 1);
            
            std::string numberStr; // 在 try 外部声明以便在 catch 块中访问
            
            try {
                // 提取输入中的数字部分
                numberStr.clear();
                for (char c : recordChoice) {
                    // 直接检查ASCII数字字符范围
                    if (c >= '0' && c <= '9') {
                        numberStr += c;
                    } else if (!numberStr.empty()) {
                        break; // 遇到非数字字符且已有数字时停止
                    }
                }
                
                if (numberStr.empty()) {
                    gameView->showError("请输入有效的数字编号! 输入内容: '" + recordChoice + "'");
                    continue;
                }
                
                // 手动转换数字字符串为整数
                size_t recordIndex = 0;
                for (char c : numberStr) {
                    recordIndex = recordIndex * 10 + (c - '0');
                }
                
                // 转换为0基索引, 检查是否为0
                if (recordIndex == 0) {
                    gameView->showError("无效的选择! 编号从1开始");
                    continue;
                }
                recordIndex--; // 转换为0基索引
                
                if (recordIndex < records.size()) {
                    gameView->showMessage("调试: 准备回放录像 '" + records[recordIndex] + "', 索引=" + std::to_string(recordIndex));
                    try {
                        replayRecording(records[recordIndex]);
                        gameView->showMessage("调试: 录像回放成功");
                    } catch (const std::exception& e) {
                        gameView->showError("回放录像失败: " + std::string(e.what()));
                    } catch (...) {
                        gameView->showError("回放录像时发生未知错误");
                    }
                } else {
                    gameView->showError("无效的选择! 超出范围");
                }
            } catch (const std::exception& e) {
                gameView->showError("解析输入时发生错误: " + std::string(e.what()));
            } catch (...) {
                gameView->showError("未知错误: " + std::string(numberStr.empty() ? "空输入" : numberStr));
            }
        } else if (choice == "3") {
            // 账户管理
            showAccountMenu();
        } else if (choice == "4") {
            // 排行榜
            showLeaderboard();
        } else if (choice == "5") {
            // 网络对战
            showNetworkMenu();
        } else if (choice == "6") {
            // 退出
            quitGame();
        } else {
            gameView->showError("无效的选择!");
        }
    }
}

void GameManager::restartGame() {
    if (gameFacade->restartGame()) {
        commandHistory.clear();
        // 重新初始化AI玩家
        initializeAIPlayers();
        gameLoop();
    } else {
        gameView->showError("重新开始游戏失败!");
    }
}

void GameManager::quitGame() {
    running = false;
}

bool GameManager::initializeGameMode() {
    while (true) {
        std::string modeStr = gameView->getUserInput("请选择游戏模式 (1: 玩家对玩家, 2: 玩家对AI, 3: AI对AI): ");
        if (modeStr == "1" || modeStr == "2" || modeStr == "3") {
            if (modeStr == "1") {
                gameMode = GameMode::PVP;
            } else if (modeStr == "2") {
                gameMode = GameMode::PVAI;
            } else {
                gameMode = GameMode::AIVAI;
            }
            return true;
        } else {
            gameView->showError("输入无效, 请输入 1、2 或 3.");
        }
    }
}

void GameManager::initializeAIPlayers() {
    // 清除旧的AI玩家
    blackAI.reset();
    whiteAI.reset();
    
    // 根据游戏模式创建AI玩家
    if (gameMode == GameMode::PVAI || gameMode == GameMode::AIVAI) {
        // 获取游戏类型
        GameType gameType = gameFacade->getGameType();
        ai::AIType aiType;
        switch (gameType) {
            case GOMOKU: 
            case GO:
                aiType = ai::AIType::GOMOKU; // GO 使用 GOMOKU 的 AI
                break;
            case OTHELLO:
                aiType = ai::AIType::OTHELLO;
                break;
            default:
                aiType = ai::AIType::GOMOKU;
                break;
        }
        
        // 选择AI级别
        ai::AILevel aiLevel = ai::AILevel::LEVEL1; // 默认为一级AI
        
        if (gameMode == GameMode::PVAI) {
            // 玩家对AI模式：只有白棋是AI
            std::string levelStr = gameView->getUserInput("请选择AI级别 (1: 随机AI, 2: 评分AI): ");
            if (levelStr == "2") {
                aiLevel = ai::AILevel::LEVEL2;
            }
            
            whiteAI = ai::AIFactory::createAIPlayer(WHITE, aiType, aiLevel);
        } else {
            // AI对AI模式：两个AI
            std::string blackLevelStr = gameView->getUserInput("请选择黑棋AI级别 (1: 随机AI, 2: 评分AI): ");
            ai::AILevel blackLevel = (blackLevelStr == "2") ? ai::AILevel::LEVEL2 : ai::AILevel::LEVEL1;
            
            std::string whiteLevelStr = gameView->getUserInput("请选择白棋AI级别 (1: 随机AI, 2: 评分AI): ");
            ai::AILevel whiteLevel = (whiteLevelStr == "2") ? ai::AILevel::LEVEL2 : ai::AILevel::LEVEL1;
            
            blackAI = ai::AIFactory::createAIPlayer(BLACK, aiType, blackLevel);
            whiteAI = ai::AIFactory::createAIPlayer(WHITE, aiType, whiteLevel);
        }
    }
}

bool GameManager::isCurrentPlayerAI() const {
    PieceType currentPlayer = gameFacade->getCurrentPlayer();
    
    if (gameMode == GameMode::PVP) {
        return false; // 玩家对玩家模式, 没有AI
    } else if (gameMode == GameMode::PVAI) {
        return currentPlayer == WHITE; // 玩家对AI模式, 只有白棋是AI
    } else {
        return true; // AI对AI模式, 两个都是AI
    }
}

chessgame::ai::AIPlayer* GameManager::getCurrentAIPlayer() const {
    PieceType currentPlayer = gameFacade->getCurrentPlayer();
    
    if (currentPlayer == BLACK && blackAI) {
        return blackAI.get();
    } else if (currentPlayer == WHITE && whiteAI) {
        return whiteAI.get();
    }
    
    return nullptr;
}

std::string GameManager::getPlayerName(PieceType player) const {
    std::string playerName;
    
    if (gameMode == GameMode::PVP) {
        // 玩家对玩家模式
        if (accountManager.isLoggedIn()) {
            // 如果有登录用户，显示用户名
            if (player == BLACK) {
                playerName = accountManager.getCurrentUser() + " (黑棋)";
            } else {
                playerName = accountManager.getCurrentUser() + " (白棋)";
            }
        } else {
            // 游客模式
            if (player == BLACK) {
                playerName = "游客A (黑棋)";
            } else {
                playerName = "游客B (白棋)";
            }
        }
    } else if (gameMode == GameMode::PVAI) {
        // 玩家对AI模式
        if (player == BLACK) {
            if (accountManager.isLoggedIn()) {
                playerName = accountManager.getCurrentUser() + " (黑棋)";
            } else {
                playerName = "游客 (黑棋)";
            }
        } else {
            playerName = "AI (白棋)";
        }
    } else if (gameMode == GameMode::AIVAI) {
        // AI对AI模式
        if (player == BLACK) {
            playerName = "AI 1 (黑棋)";
        } else {
            playerName = "AI 2 (白棋)";
        }
    }
    
    return playerName;
}

std::string GameManager::getPlayerStats(PieceType player) const {
    if (gameMode == GameMode::AIVAI) {
        return ""; // AI不显示战绩
    }
    
    // 只有在PVP模式下的非AI玩家，或者PVA模式下的人类玩家才显示战绩
    if ((gameMode == GameMode::PVP) || 
        (gameMode == GameMode::PVAI && player == BLACK)) {
        
        if (accountManager.isLoggedIn()) {
            const account::UserAccount* account = accountManager.getCurrentUserAccount();
            if (account) {
                const account::GameStats& stats = account->getStats();
                std::ostringstream oss;
                oss << "战绩: " << stats.wins << "胜" << stats.losses << "负" 
                    << stats.draws << "平 (胜率: " << std::fixed << std::setprecision(1) 
                    << stats.winRate << "%)";
                return oss.str();
            }
        }
    }
    
    return ""; // 游客或AI不显示战绩
}

void GameManager::aiTurn() {
    ai::AIPlayer* currentAI = getCurrentAIPlayer();
    if (!currentAI) {
        return;
    }
    
    // 显示AI思考信息
    gameView->showHint("AI正在思考...");
    
    // 获取AI的移动
    auto boardPtr = std::shared_ptr<model::Board>(&gameFacade->getBoard(), [](model::Board*){});
    Move aiMove = currentAI->makeMove(boardPtr);
    
    // 执行AI的移动
    if (aiMove.isPass) {
        // AI选择虚着
        auto passCommand = std::make_unique<PassCommand>(gameFacade.get(), aiMove.piece);
        executeCommand(std::move(passCommand));
    } else {
        // AI选择落子
        auto moveCommand = std::make_unique<MoveCommand>(gameFacade.get(), aiMove.x, aiMove.y, aiMove.piece);
        executeCommand(std::move(moveCommand));
    }
}

void GameManager::startRecording() {
    if (isRecording) {
        gameView->showError("已经在录像中!");
        return;
    }
    
    // 创建新的录像
    GameType gameType = gameFacade->getGameType();
    int boardSize = gameFacade->getBoard().getSize();
    
    // 根据游戏模式设置玩家名
    std::string blackPlayer = "Black";
    std::string whitePlayer = "White";
    
    if (gameMode == GameMode::PVP) {
        blackPlayer = "Player 1 (Black)";
        whitePlayer = "Player 2 (White)";
    } else if (gameMode == GameMode::PVAI) {
        blackPlayer = "Human (Black)";
        whitePlayer = "AI (White)";
    } else if (gameMode == GameMode::AIVAI) {
        blackPlayer = "AI 1 (Black)";
        whitePlayer = "AI 2 (White)";
    }
    
    currentRecord = recordManager.createRecord(gameType, boardSize, blackPlayer, whitePlayer);
    isRecording = true;
    
    gameView->showHint("录像已开始");
}

void GameManager::stopRecording() {
    if (!isRecording) {
        gameView->showError("当前没有在录像!");
        return;
    }
    
    // 设置游戏结果
    GameStatus status = gameFacade->getGameStatus();
    currentRecord->setResult(status);
    
    isRecording = false;
    gameView->showHint("录像已停止");
    
    // 询问是否保存录像
    std::string saveChoice = gameView->getUserInput("是否保存录像? (y/n): ");
    if (saveChoice == "y" || saveChoice == "Y") {
        std::string filename = gameView->getUserInput("请输入录像文件名 (留空使用默认): ");
        if (saveRecording(filename)) {
            gameView->showHint("录像已保存");
        } else {
            gameView->showError("录像保存失败!");
        }
    }
}

bool GameManager::saveRecording(const std::string& filename) {
    if (!currentRecord) {
        gameView->showError("没有可保存的录像!");
        return false;
    }
    
    // 根据用户登录状态确定保存路径
    std::string savePath = filename;
    if (savePath.empty()) {
        // 如果用户已登录，保存到用户目录
        if (accountManager.isLoggedIn()) {
            std::string userDir = recordManager.getRecordDirectory() + "/" + 
                                 accountManager.getCurrentUser();
            // 确保用户目录存在
            if (!std::filesystem::exists(userDir)) {
                std::filesystem::create_directories(userDir);
            }
            // 生成默认文件名
            auto now = std::time(nullptr);
            auto tm = *std::localtime(&now);
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
            
            std::string gameTypeStr;
            switch (currentRecord->getGameType()) {
                case GOMOKU: gameTypeStr = "Gomoku"; break;
                case GO: gameTypeStr = "Go"; break;
                case OTHELLO: gameTypeStr = "Othello"; break;
                default: gameTypeStr = "Unknown"; break;
            }
            
            savePath = userDir + "/" + gameTypeStr + "_" + oss.str() + ".txt";
        }
    }
    
    bool success = recordManager.saveRecord(*currentRecord, savePath);
    
    // 如果保存成功且用户已登录，将录像添加到用户的保存列表
    if (success && accountManager.isLoggedIn()) {
        account::UserAccount* account = accountManager.getCurrentUserAccount();
        if (account) {
            // 提取文件名（不包含路径）
            std::filesystem::path filePath(savePath);
            account->addSavedGame(filePath.filename().string());
            
            // 保存账户信息
            std::string accountFile = accountManager.getAccountDirectory() + "/" + 
                                    account->getUsername() + ".txt";
            account->saveToFile(accountFile);
        }
    }
    
    return success;
}

std::vector<std::string> GameManager::getUserRecordList() const {
    std::vector<std::string> records;
    
    // 始终显示公共录像（包括游客的录像）
    std::vector<std::string> publicRecords = recordManager.getRecordList();
    records.insert(records.end(), publicRecords.begin(), publicRecords.end());
    
    // 如果用户已登录，也显示用户的个人录像
    if (accountManager.isLoggedIn()) {
        std::string userDir = recordManager.getRecordDirectory() + "/" + 
                             accountManager.getCurrentUser();
        
        if (std::filesystem::exists(userDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(userDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                    records.push_back(entry.path().string());
                }
            }
        }
    }
    
    return records;
}

bool GameManager::loadRecording(const std::string& filename) {
    auto record = recordManager.loadRecord(filename);
    if (!record) {
        gameView->showError("加载录像失败!");
        return false;
    }
    
    currentRecord = std::move(record);
    return true;
}

void GameManager::replayRecording(const std::string& filename) {
    if (!loadRecording(filename)) {
        return;
    }
    
    // 初始化游戏以匹配录像
    GameType gameType = currentRecord->getGameType();
    int boardSize = currentRecord->getBoardSize();
    
    if (!gameFacade->initGame(gameType, boardSize)) {
        gameView->showError("初始化游戏失败!");
        return;
    }
    
    // 开始回放
    recording::GameReplayer replayer;
    replayer.loadRecordObject(std::move(currentRecord));
    replayer.startReplay();
    
    // 回放循环
    while (replayer.getIsReplaying()) {
        // 显示当前棋盘状态
        gameView->displayBoard(
            gameFacade->getBoard(), 
            gameFacade->getCurrentPlayer(),
            gameFacade->getGameType(),
            "回放模式 - 步数: " + std::to_string(replayer.getCurrentMoveIndex()) + "/" + 
            std::to_string(replayer.getTotalMoves()),
            "", "", "", ""
        );
        
        // 获取用户输入
        std::string input = gameView->getUserInput("回放控制 (n: 下一步, p: 上一步, j: 跳转, q: 退出): ");
        
        if (input == "n") {
            // 下一步 - 先前进到下一步，然后获取并执行移动
            if (replayer.nextMove()) {
                const recording::GameRecordEntry* currentMove = replayer.getCurrentMove();
                if (currentMove) {
                    const Move& move = currentMove->move;
                    gameView->showMessage("调试: 执行移动 (" + std::to_string(move.x) + "," + std::to_string(move.y) + ") 玩家=" + std::to_string(move.piece));
                    if (move.isPass) {
                        gameFacade->passMove(move.piece);
                    } else if (move.isResign) {
                        gameFacade->resign(move.piece);
                    } else {
                        bool success = gameFacade->makeMove(move.x, move.y, move.piece);
                        gameView->showMessage("调试: makeMove返回=" + std::to_string(success));
                    }
                } else {
                    gameView->showMessage("调试: currentMove为空");
                }
            } else {
                gameView->showMessage("调试: nextMove失败，可能已到达最后一步");
            }
        } else if (input == "p") {
            // 上一步
            replayer.previousMove();
            // 重新构建棋盘状态到当前步数
            gameFacade->initGame(gameType, boardSize);
            for (size_t i = 0; i < replayer.getCurrentMoveIndex(); i++) {
                const recording::GameRecordEntry* entry = &replayer.getRecord()->getEntries()[i];
                const Move& move = entry->move;
                if (move.isPass) {
                    gameFacade->passMove(move.piece);
                } else if (move.isResign) {
                    gameFacade->resign(move.piece);
                } else {
                    gameFacade->makeMove(move.x, move.y, move.piece);
                }
            }
        } else if (input == "j") {
            // 跳转到指定步数
            std::string jumpStr = gameView->getUserInput("请输入要跳转的步数: ");
            try {
                size_t jumpTo = std::stoul(jumpStr);
                if (replayer.jumpToMove(jumpTo)) {
                    // 重新构建棋盘状态到指定步数
                    gameFacade->initGame(gameType, boardSize);
                    for (size_t i = 0; i < replayer.getCurrentMoveIndex(); i++) {
                        const recording::GameRecordEntry* entry = &replayer.getRecord()->getEntries()[i];
                        const Move& move = entry->move;
                        if (move.isPass) {
                            gameFacade->passMove(move.piece);
                        } else if (move.isResign) {
                            gameFacade->resign(move.piece);
                        } else {
                            gameFacade->makeMove(move.x, move.y, move.piece);
                        }
                    }
                } else {
                    gameView->showError("无效的步数!");
                }
            } catch (...) {
                gameView->showError("输入无效!");
            }
        } else if (input == "q") {
            // 退出回放
            replayer.resetReplay();
            break;
        }
    }
}

void GameManager::handleGameEnd(GameStatus status) {
    std::string message;
    
    switch (status) {
        case BLACK_WIN:
            message = "黑方获胜!";
            break;
        case WHITE_WIN:
            message = "白方获胜!";
            break;
        case TIED:
            message = "平局!";
            break;
        default:
            message = "游戏结束!";
            break;
    }
    
    gameView->showMessage(message);
    
    // 更新用户统计信息
    if (accountManager.isLoggedIn()) {
        auto* account = accountManager.getCurrentUserAccount();
        if (account) {
            GameType gameType = gameFacade->getGameType();
            bool isAI = (gameMode == GameMode::PVAI || gameMode == GameMode::AIVAI);
            
            // 判断当前用户是否获胜
            bool isWin = false;
            bool isDraw = (status == TIED);
            
            if (!isDraw) {
                if (gameMode == GameMode::PVP) {
                    // PVP模式, 根据当前玩家判断
                    PieceType currentPlayer = gameFacade->getCurrentPlayer();
                    // 游戏结束时, 当前玩家是输的一方
                    isWin = (status == BLACK_WIN && currentPlayer == WHITE) || 
                           (status == WHITE_WIN && currentPlayer == BLACK);
                } else if (gameMode == GameMode::PVAI) {
                    // PVAI模式, 人类是黑方
                    isWin = (status == BLACK_WIN);
                } else if (gameMode == GameMode::AIVAI) {
                    // AIVAI模式, 不更新统计
                    isWin = false;
                }
            }
            
            // 更新统计
            account->updateGameStats(isWin, isDraw, gameType, isAI);
            
            // 保存账户信息
            std::string filename = accountManager.getAccountDirectory() + "/" + 
                                  account->getUsername() + ".txt";
            account->saveToFile(filename);
        }
    }
    
    // 如果正在录像, 停止录像
    if (isRecording) {
        stopRecording();
    }
    
    // 询问是否重新开始
    std::string choice = gameView->getUserInput("是否重新开始游戏? (y/n): ");
    if (choice == "y" || choice == "Y") {
        restartGame();
    }
}

// 网络相关方法实现
void GameManager::startNetworkServer() {
    networkServer = std::make_unique<network::NetworkServer>();
    
    // 设置消息回调
    networkServer->setMessageCallback([this](int clientSocket, const network::NetworkMessage& message) {
        handleNetworkMessage(clientSocket, message);
    });
    
    networkServer->setConnectCallback([this](int clientSocket) {
        std::cout << "玩家已连接，准备开始游戏" << std::endl;
        // 分配连接的玩家为白棋
        networkPlayerType = 1; // 主机是黑棋
        
        // 初始化游戏（如果还没有初始化）
        if (!gameFacade) {
            gameFacade = std::make_unique<facade::GameFacade>();
        }
        
        // 发送游戏开始消息
        network::NetworkMessage startMsg(network::MessageType::GAME_START, "WHITE");
        networkServer->sendToClient(clientSocket, startMsg);
        
        // 发送当前游戏状态
        sendGameState();
    });
    
    networkServer->setDisconnectCallback([this](int clientSocket) {
        std::cout << "玩家断开连接" << std::endl;
        handleNetworkDisconnection();
    });
    
    if (networkServer->start()) {
        isHost = true;
        isNetworkGame = true;
        gameMode = GameMode::NETWORK;
        networkPlayerType = 1; // 主机是黑棋
        
        std::cout << "网络服务器已启动，等待玩家连接..." << std::endl;
        std::cout << "其他玩家可以通过以下IP地址连接：" << std::endl;
        std::cout << networkServer->getLocalIPAddress() << std::endl;
    } else {
        std::cerr << "启动网络服务器失败" << std::endl;
        networkServer.reset();
    }
}

void GameManager::connectToServer(const std::string& serverIP) {
    networkClient = std::make_unique<network::NetworkClient>();
    
    // 设置消息回调
    networkClient->setMessageCallback([this](const network::NetworkMessage& message) {
        handleClientMessage(message);
    });
    
    networkClient->setConnectCallback([this]() {
        std::cout << "成功连接到服务器" << std::endl;
    });
    
    networkClient->setDisconnectCallback([this]() {
        std::cout << "与服务器断开连接" << std::endl;
        handleNetworkDisconnection();
    });
    
    if (networkClient->connect(serverIP)) {
        isHost = false;
        isNetworkGame = true;
        gameMode = GameMode::NETWORK;
        networkPlayerType = 2; // 客户端是白棋
        
        std::cout << "已连接到服务器: " << serverIP << std::endl;
    } else {
        std::cerr << "连接服务器失败" << std::endl;
        networkClient.reset();
    }
}

void GameManager::handleNetworkMessage(int clientSocket, const network::NetworkMessage& message) {
    switch (message.type) {
        case network::MessageType::MOVE: {
            network::MoveInfo moveInfo = network::MoveInfo::deserialize(message.data);
            
            // 执行对手的移动
            if (gameFacade->makeMove(moveInfo.row, moveInfo.col, moveInfo.player)) {
                // 同步游戏状态
                sendGameState();
                
                // 显示更新后的棋盘
                gameView->displayBoard(
                    gameFacade->getBoard(),
                    gameFacade->getCurrentPlayer(),
                    gameFacade->getGameType(),
                    "",
                    getPlayerName(BLACK),
                    getPlayerName(WHITE),
                    getPlayerStats(BLACK),
                    getPlayerStats(WHITE)
                );
                
                // 检查游戏是否结束
                GameStatus status = gameFacade->getGameStatus();
                if (status != IN_PROGRESS) {
                    handleGameEnd(status);
                }
            }
            break;
        }
        
        case network::MessageType::DISCONNECT:
            std::cout << "对手断开连接" << std::endl;
            handleNetworkDisconnection();
            break;
            
        default:
            break;
    }
}

void GameManager::handleClientMessage(const network::NetworkMessage& message) {
    switch (message.type) {
        case network::MessageType::GAME_START: {
            // 确定自己的棋子颜色
            if (message.data == "WHITE") {
                networkPlayerType = 2; // 客户端是白棋
            } else {
                networkPlayerType = 1; // 客户端是黑棋
            }
            break;
        }
        
        case network::MessageType::BOARD_SYNC: {
            network::GameStateInfo stateInfo = network::GameStateInfo::deserialize(message.data);
            syncGameState(stateInfo);
            break;
        }
        
        case network::MessageType::GAME_END: {
            GameStatus status = static_cast<GameStatus>(std::stoi(message.data));
            handleGameEnd(status);
            break;
        }
        
        case network::MessageType::DISCONNECT:
            std::cout << "服务器断开连接" << std::endl;
            handleNetworkDisconnection();
            break;
            
        default:
            break;
    }
}

void GameManager::sendNetworkMove(int row, int col) {
    if (!isNetworkGame) return;
    
    network::MoveInfo moveInfo{row, col, gameFacade->getCurrentPlayer()};
    network::NetworkMessage moveMsg(network::MessageType::MOVE, moveInfo.serialize());
    
    if (isHost && networkServer) {
        networkServer->broadcastMessage(moveMsg);
    } else if (!isHost && networkClient) {
        networkClient->sendMessage(moveMsg);
    }
}

void GameManager::sendGameState() {
    if (!isNetworkGame) return;
    
    // 序列化棋盘状态
    std::ostringstream boardStream;
    const auto& board = gameFacade->getBoard();
    int size = board.getSize();
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            boardStream << static_cast<int>(board.getPiece(i, j));
            if (i < size - 1 || j < size - 1) {
                boardStream << ",";
            }
        }
    }
    
    network::GameStateInfo stateInfo{
        gameFacade->getGameType(),
        gameFacade->getCurrentPlayer(),
        gameFacade->getGameStatus(),
        boardStream.str()
    };
    
    network::NetworkMessage stateMsg(network::MessageType::BOARD_SYNC, stateInfo.serialize());
    
    if (isHost && networkServer) {
        networkServer->broadcastMessage(stateMsg);
    } else if (!isHost && networkClient) {
        networkClient->sendMessage(stateMsg);
    }
}

void GameManager::syncGameState(const network::GameStateInfo& stateInfo) {
    // 恢复游戏类型
    gameFacade->setGameType(stateInfo.gameType);
    
    // 恢复棋盘状态
    std::istringstream boardStream(stateInfo.boardState);
    std::string cell;
    auto& board = gameFacade->getBoard();
    int size = board.getSize();
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (std::getline(boardStream, cell, ',')) {
                PieceType piece = static_cast<PieceType>(std::stoi(cell));
                board.setPiece(i, j, piece);
            }
        }
    }
    
    // 显示同步后的棋盘
    gameView->displayBoard(
        board,
        stateInfo.currentPlayer,
        stateInfo.gameType,
        "",
        getPlayerName(BLACK),
        getPlayerName(WHITE),
        getPlayerStats(BLACK),
        getPlayerStats(WHITE)
    );
}

void GameManager::handleNetworkDisconnection() {
    isNetworkGame = false;
    
    if (networkServer) {
        networkServer->stop();
        networkServer.reset();
    }
    
    if (networkClient) {
        networkClient->disconnect();
        networkClient.reset();
    }
    
    std::cout << "网络游戏已结束" << std::endl;
}

void GameManager::showNetworkMenu() {
    while (true) {
        gameView->showMessage("\n===== 网络对战 =====");
        gameView->showMessage("1. 创建游戏房间（主机）");
        gameView->showMessage("2. 加入游戏房间（客户端）");
        gameView->showMessage("3. 返回主菜单");
        
        std::string choice = gameView->getUserInput("请选择: ");
        
        // 清理输入：去除前后空白字符
        choice.erase(0, choice.find_first_not_of(" \t\r\n"));
        choice.erase(choice.find_last_not_of(" \t\r\n") + 1);
        
        if (choice == "1") {
            // 创建游戏房间
            startNetworkServer();
            if (isNetworkGame) {
                // 等待玩家连接
                gameView->showMessage("等待其他玩家连接...");
                
                // 简单的等待循环，实际应用中可以添加超时机制
                while (isNetworkGame && networkServer && networkServer->getConnectedClientCount() == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                
                if (isNetworkGame && networkServer && networkServer->getConnectedClientCount() > 0) {
                    // 玩家已连接，初始化游戏
                    if (!initializeGame()) {
                        handleNetworkDisconnection();
                        continue;
                    }
                    
                    // 网络游戏主循环
                    networkGameLoop();
                }
            }
        } else if (choice == "2") {
            // 加入游戏房间
            std::string serverIP = gameView->getUserInput("请输入服务器IP地址: ");
            if (!serverIP.empty()) {
                connectToServer(serverIP);
                if (isNetworkGame) {
                    // 等待游戏开始
                    gameView->showMessage("等待游戏开始...");
                    
                    // 等待主机发送游戏状态
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    
                    if (isNetworkGame) {
                        // 网络游戏主循环
                        networkGameLoop();
                    }
                }
            }
        } else if (choice == "3") {
            break;
        } else {
            gameView->showError("无效的选择!");
        }
    }
}
