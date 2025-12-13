#include "GameFacade.h"
#include "../model/GomokuRule.h"
#include "../model/GoRule.h"
#include "../model/OthelloRule.h"
#include <fstream>
#include <sstream>
#include <queue>

using namespace chessgame::facade;
using namespace chessgame::model;

GameFacade::GameFacade() 
    : currentPlayer(BLACK), gameType(GOMOKU), gameStatus(IN_PROGRESS), 
      passCount(0), board(std::make_unique<Board>(15)), caretaker(std::make_unique<GameCaretaker>()) {
    initRule();
}

void GameFacade::initRule() {
    switch (gameType) {
        case GOMOKU:
            rule = std::make_unique<GomokuRule>(board.get());
            break;
        case GO:
            rule = std::make_unique<GoRule>(board.get());
            break;
        case OTHELLO:
            rule = std::make_unique<OthelloRule>(board.get());
            break;
        default:
            rule = std::make_unique<GomokuRule>(board.get());
            break;
    }
}

bool GameFacade::initGame(GameType type, int boardSize) {
    if (boardSize < 8 || boardSize > 19) {
        return false;
    }
    
    gameType = type;
    board = std::make_unique<Board>(boardSize);
    initRule();
    
    currentPlayer = BLACK;
    gameStatus = IN_PROGRESS;
    passCount = 0;
    
    // 为 Othello 设置初始棋子
    if (type == OTHELLO && boardSize == 8) {
        // 棋盘中心4个位置初始为黑白相间
        board->setPiece(3, 3, WHITE);
        board->setPiece(3, 4, BLACK);
        board->setPiece(4, 3, BLACK);
        board->setPiece(4, 4, WHITE);
    }
    
    // 清空历史记录
    caretaker->clearHistory();
    
    // 保存初始状态
    saveStateToMemento();
    
    return true;
}

bool GameFacade::makeMove(int x, int y, PieceType player) {
    if (gameStatus != IN_PROGRESS || player != currentPlayer) {
        return false;
    }
    
    if (!rule->isValidMove(x, y, player)) {
        return false;
    }
    
    // 保存当前状态
    saveStateToMemento();
    
    // 执行落子
    rule->makeMove(x, y, player);
    
    // 重置虚着计数
    passCount = 0;
    
    // 检查胜负
    gameStatus = rule->checkWin(x, y);
    
    // 切换玩家
    if (gameStatus == IN_PROGRESS) {
        switchPlayer();
    }
    
    return true;
}

bool GameFacade::passMove(PieceType player) {
    if (gameStatus != IN_PROGRESS || player != currentPlayer || !supportsPass()) {
        return false;
    }
    
    // 保存当前状态
    saveStateToMemento();
    
    // 增加虚着计数
    passCount++;
    
    // 检查是否双方连续虚着（围棋终局）
    if (passCount >= 2) {
        if (gameType == GO) {
            auto [blackScore, whiteScore] = computeGoScore();
            if (blackScore > whiteScore) gameStatus = BLACK_WIN;
            else if (whiteScore > blackScore) gameStatus = WHITE_WIN;
            else gameStatus = TIED;
        } else {
            gameStatus = TIED;
        }
    } else {
        switchPlayer();
    }
    
    return true;
}

bool GameFacade::undoMove() {
    auto memento = caretaker->getPreviousState();
    if (!memento) {
        return false;
    }
    
    // 恢复棋盘状态
    std::stringstream ss(memento->getBoardState());
    board->deserialize(ss);
    
    // 恢复游戏状态
    currentPlayer = memento->getCurrentPlayer();
    passCount = memento->getPassCount();
    gameStatus = memento->getStatus();
    
    return true;
}

bool GameFacade::resign(PieceType player) {
    if (gameStatus != IN_PROGRESS) {
        return false;
    }
    
    // 保存当前状态
    saveStateToMemento();
    
    // 设置游戏状态
    gameStatus = (player == BLACK) ? WHITE_WIN : BLACK_WIN;
    
    return true;
}

bool GameFacade::restartGame() {
    if (!board) {
        return false;
    }
    
    int boardSize = board->getSize();
    return initGame(gameType, boardSize);
}

bool GameFacade::saveGame(const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        return false;
    }
    
    // 保存游戏信息
    outfile << static_cast<int>(gameType) << " " 
            << static_cast<int>(currentPlayer) << " " 
            << passCount << " " 
            << static_cast<int>(gameStatus) << std::endl;
    
    // 保存棋盘状态
    outfile << board->serialize();
    
    outfile.close();
    return true;
}

bool GameFacade::loadGame(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        return false;
    }
    
    // 读取游戏信息
    int type, player, passes, status;
    infile >> type >> player >> passes >> status;
    
    // 读取棋盘状态
    std::stringstream ss;
    ss << infile.rdbuf();
    
    // 重建棋盘
    board = std::make_unique<Board>(0);  // 临时
    board->deserialize(ss);
    
    // 设置游戏状态
    gameType = static_cast<GameType>(type);
    currentPlayer = static_cast<PieceType>(player);
    passCount = passes;
    gameStatus = static_cast<GameStatus>(status);
    
    // 重新初始化规则
    initRule();
    
    // 清空历史记录
    caretaker->clearHistory();
    
    // 保存加载的状态
    saveStateToMemento();
    
    return true;
}

bool GameFacade::supportsPass() const {
    return rule && rule->supportsPass();
}

bool GameFacade::isValidMove(int x, int y, PieceType player) const {
    return rule && rule->isValidMove(x, y, player);
}

void GameFacade::switchPlayer() {
    currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
}

void GameFacade::saveStateToMemento() {
    if (board) {
        auto memento = std::make_shared<GameMemento>(
            *board, currentPlayer, passCount, gameStatus, gameType);
        caretaker->saveState(memento);
    }
}

std::pair<int, int> GameFacade::computeGoScore() const {
    int size = board->getSize();
    int blackStones = 0, whiteStones = 0;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            PieceType p = board->getPiece(i, j);
            if (p == BLACK) blackStones++;
            else if (p == WHITE) whiteStones++;
        }
    }

    std::vector<std::vector<bool>> visited(size, std::vector<bool>(size, false));
    auto inBounds = [&](int x, int y){ return x >= 0 && x < size && y >= 0 && y < size; };
    int dx[4] = {1,-1,0,0};
    int dy[4] = {0,0,1,-1};

    int blackTerritory = 0, whiteTerritory = 0;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (board->getPiece(i, j) != EMPTY || visited[i][j]) continue;
            std::queue<std::pair<int,int>> q;
            q.push({i,j});
            visited[i][j] = true;
            int area = 0;
            bool touchesBlack = false, touchesWhite = false;
            while (!q.empty()) {
                auto [x,y] = q.front(); q.pop();
                area++;
                for (int k = 0; k < 4; ++k) {
                    int nx = x + dx[k], ny = y + dy[k];
                    if (!inBounds(nx, ny)) continue;
                    PieceType p = board->getPiece(nx, ny);
                    if (p == EMPTY && !visited[nx][ny]) { visited[nx][ny] = true; q.push({nx,ny}); }
                    else if (p == BLACK) touchesBlack = true;
                    else if (p == WHITE) touchesWhite = true;
                }
            }
            if (touchesBlack && !touchesWhite) blackTerritory += area;
            else if (touchesWhite && !touchesBlack) whiteTerritory += area;
        }
    }
    return {blackStones + blackTerritory, whiteStones + whiteTerritory};
}
