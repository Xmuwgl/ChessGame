#include "GameManager.h"

using namespace chessgame::controller;

void GameManager::saveState(){
    // 保存当前状态到历史栈
    // 注意：这里保存的是深拷贝，为了简化代码直接存储 Board 对象
    // 优化建议：仅存储 Move 命令 (Command Pattern) 以减少内存 [cite: 51]
    // 但为了实现完整的 save/load 状态恢复，Snapshot 方式最直观
    GameState state(board->getSize());
    state.board = *board; // copy
    state.currentPlayer = currentPlayer;
    state.passCount = passCount;
    history.push(state);
}

GameManager::GameManager() : currentPlayer(BLACK), passCount(0), showHints(true) {
    view = unique_ptr<GameView>(new ConsoleView()); // 默认使用控制台视图
}

void GameManager::initGame() {
    while (true) {
        std::string typeStr = view->getUserInput("Please select the game type (1: Gomoku, 2: Go): ");
        if (typeStr == "1" || typeStr == "2") {
            gameType = (typeStr == "1") ? GOMOKU : GO;
            size = (gameType == GOMOKU) ? 19 : 8;
            break;
        }
    }

    

    board = unique_ptr<Board>(new Board(size));
    
    // 工厂模式体现：根据类型创建规则
    if (gameType == GOMOKU)
        rule = unique_ptr<GameRule>(new GomokuRule(board.get()));
    else
        rule = unique_ptr<GameRule>(new GoRule(board.get()));
    
    currentPlayer = BLACK;
    passCount = 0;
    while(!history.empty()) history.pop(); // 清空历史
}

bool GameManager::saveGame(std::string filename) { // 
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        view->displayBoard(*board, currentPlayer, "Failed to save game!");
        return false;
    }
    outfile << (int)gameType << " " << (int)currentPlayer << " " << passCount << std::endl;
    outfile << board->serialize();
    outfile.close();
    view->displayBoard(*board, currentPlayer, "Game saved to " + filename);
}

bool GameManager::loadGame(std::string filename) { // 
    ifstream infile(filename);
    if (!infile.is_open()) {
        return false;
    }
    int type, player;
    infile >> type >> player >> passCount;
    gameType = (GameType)type;
    currentPlayer = (PieceType)player;
    
    std::stringstream ss;
    ss << infile.rdbuf(); // 读取剩余内容
    
    // 重建对象
    board = unique_ptr<Board>(new Board(0)); // 临时
    board->deserialize(ss);
    
    if (gameType == GOMOKU)
        rule = unique_ptr<GameRule>(new GomokuRule(board.get()));
    else
        rule = unique_ptr<GameRule>(new GoRule(board.get()));
        
    return true;
}

void GameManager::startGame(GameType type, PieceType firstPlayer) {
    initGame();
    std::string message = "Game start! enter 'help' to check rules...";

    bool running = true;
    while (running) {
        view->displayBoard(*board, currentPlayer, message);
        message = "";

        std::string input = view->getUserInput("Enter command > ");
        
        if (input == "quit") {
            if(view->getUserInput("Are you sure to give up? (y/n): ") == "y") break;
            continue;
        }
        else if (input == "help") {
            view->showHelp();
            continue;
        }
        else if (input == "undo") { // 
            if (history.empty()) {
                message = "Cannot undo...";
            } else {
                GameState prev = history.top();
                history.pop();
                *board = prev.board;
                currentPlayer = prev.currentPlayer;
                passCount = prev.passCount;
                message = "Undo one step";
            }
            continue;
        }
        else if (input.substr(0, 4) == "save") {
            std::string fname = (input.length() > 5) ? input.substr(5) : "savegame.txt";
            saveGame(fname);
            continue;
        }
        else if (input.substr(0, 4) == "load") {
            std::string fname = (input.length() > 5) ? input.substr(5) : "savegame.txt";
            if(loadGame(fname)) message = "Load game success!";
            else message = "Load game failed or file not exist";
            continue;
        }

        // 处理落子逻辑
        int x = -1, y = -1;
        bool isPass = false;

        if (input == "pass") { // 
            if (gameType == GO) isPass = true;
            else {
                message = "Gomoku cannot pass...";
                continue;
            }
        } else {
            std::stringstream ss(input);
            if (!(ss >> x >> y)) {
                message = "Invalid command! Please enter 'x y'.";
                continue;
            }
            x--; y--; // 转换为0索引
        }

        // 逻辑判断
        if (isPass) {
            saveState(); // 记录状态以便悔棋
            passCount++;
            if (passCount >= 2) { // 围棋双方连续虚着: 平局
                view->displayBoard(*board, currentPlayer, "Go game end! Please count the score to decide the winner.");
                running = false;
            } else {
                currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
                message = "Player pass";
            }
        } else {
            if (rule->isValidMove(x, y, currentPlayer)) {
                saveState();
                rule->makeMove(x, y, currentPlayer);
                passCount = 0; // 落子则重置虚着计数
                
                // 胜负判断
                GameStatus status = rule->checkWin(x, y);
                if (status != PLAYING) {
                    view->displayBoard(*board, currentPlayer, "");
                    if (status == BLACK_WIN) view->displayBoard(*board, currentPlayer, "Black player win!");
                    else if (status == WHITE_WIN) view->displayBoard(*board, currentPlayer, "White player win!");
                    else view->displayBoard(*board, currentPlayer, "Draw!");
                    running = false;
                } else {
                    currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
                }
            } else {
                message = "Invalid move! Please try again.";
            }
        }
    }
}