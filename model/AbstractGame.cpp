#include "AbstractGame.h"
#include "Board.h"
#include "GameRule.h"
#include <fstream>
#include <sstream>

using namespace chessgame;

AbstractGame::AbstractGame(GameType type, int boardSize) 
    : gameType(type), currentPlayer(PieceType::BLACK), passCount(0) {
    board = std::make_shared<Board>(boardSize);
    
    // 根据游戏类型创建规则
    if (type == GameType::GOMOKU) {
        rule = std::make_shared<GomokuRule>(board.get());
    } else if (type == GameType::GO) {
        rule = std::make_shared<GoRule>(board.get());
    }
}

void AbstractGame::initGame() {
    board->clear();
    currentPlayer = PieceType::BLACK;
    passCount = 0;
    while (!history.empty()) history.pop();
}

bool AbstractGame::makeMove(int x, int y) {
    Move move(x, y, currentPlayer);
    return makeMove(move);
}

bool AbstractGame::makeMove(const Move& move) {
    if (move.isResign) {
        resign();
        return true;
    }
    
    if (move.isPass) {
        return pass();
    }
    
    if (!rule->isValidMove(move.x, move.y, currentPlayer)) {
        return false;
    }
    
    saveState();
    rule->makeMove(move.x, move.y, currentPlayer);
    currentPlayer = (currentPlayer == PieceType::BLACK) ? PieceType::WHITE : PieceType::BLACK;
    passCount = 0;
    
    return true;
}

bool AbstractGame::pass() {
    if (!rule->supportsPass()) {
        return false;
    }
    
    saveState();
    passCount++;
    currentPlayer = (currentPlayer == PieceType::BLACK) ? PieceType::WHITE : PieceType::BLACK;
    
    // 围棋连续两次虚着结束游戏
    if (passCount >= 2) {
        // 游戏结束逻辑由具体游戏类实现
    }
    
    return true;
}

void AbstractGame::resign() {
    saveState();
    // 认输逻辑由具体游戏类实现
}

bool AbstractGame::undo() {
    if (history.empty()) return false;
    
    GameState state = history.top();
    history.pop();
    
    // 恢复棋盘状态
    std::stringstream ss(state.boardState);
    board->deserialize(ss);
    
    currentPlayer = state.currentPlayer;
    passCount = state.passCount;
    
    return true;
}

void AbstractGame::saveGame(const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        return;
    }
    
    outfile << static_cast<int>(gameType) << " " 
            << static_cast<int>(currentPlayer) << " " 
            << passCount << std::endl;
    outfile << board->serialize();
    outfile.close();
}

bool AbstractGame::loadGame(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        return false;
    }
    
    int type, player;
    infile >> type >> player >> passCount;
    gameType = static_cast<GameType>(type);
    currentPlayer = static_cast<PieceType>(player);
    
    std::stringstream ss;
    ss << infile.rdbuf();
    
    board->deserialize(ss);
    
    // 重新创建规则对象
    if (gameType == GameType::GOMOKU) {
        rule = std::make_shared<GomokuRule>(board.get());
    } else if (gameType == GameType::GO) {
        rule = std::make_shared<GoRule>(board.get());
    }
    
    return true;
}

GameStatus AbstractGame::getGameStatus() const {
    // 检查是否有获胜者
    GameStatus status = rule->checkWin(-1, -1);
    if (status != GameStatus::IN_PROGRESS) {
        return status;
    }
    
    // 检查是否平局: 棋盘满
    bool full = true;
    for (int i = 0; i < board->getSize(); ++i) {
        for (int j = 0; j < board->getSize(); ++j) {
            if (board->getPiece(i, j) == PieceType::EMPTY) {
                full = false;
                break;
            }
        }
        if (!full) break;
    }
    
    return full ? GameStatus::DRAW : GameStatus::IN_PROGRESS;
}

void AbstractGame::saveState() {
    GameState state(board->getSize());
    state.boardState = board->serialize();
    state.currentPlayer = currentPlayer;
    state.passCount = passCount;
    history.push(state);
}