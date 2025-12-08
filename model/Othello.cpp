#include "Othello.h"
#include "Board.h"
#include "Rule.h"
#include "../facade/GameFacade.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace chessgame::model {

Othello::Othello() {
    gameType = OTHELLO;
}

void Othello::initGame() {
    // 创建8x8棋盘
    board = std::make_shared<Board>(8);
    
    // 初始化黑白棋的起始位置
    // 棋盘中心4个位置初始为黑白相间
    board->setPiece(3, 3, WHITE);
    board->setPiece(3, 4, BLACK);
    board->setPiece(4, 3, BLACK);
    board->setPiece(4, 4, WHITE);
    
    currentPlayer = BLACK;
    passCount = 0;
    status = IN_PROGRESS;
    winner = EMPTY;
    
    // 保存初始状态
    saveState();
}

void Othello::run() {
    // 这个方法在当前架构中由GameManager处理
}

bool Othello::makeMove(int x, int y) {
    if (status != IN_PROGRESS) return false;
    
    // 检查移动是否合法
    if (!isValidMove(x, y, currentPlayer)) {
        return false;
    }
    
    // 获取将被翻转的棋子
    std::vector<Point> flipped = getFlippedPieces(x, y, currentPlayer);
    
    // 放置棋子
    board->setPiece(x, y, currentPlayer);
    
    // 翻转棋子
    flipPieces(flipped);
    
    // 切换玩家
    currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
    passCount = 0;
    
    // 保存状态
    saveState();
    
    // 更新游戏状态
    updateGameStatus();
    
    return true;
}

bool Othello::makeMove(const chessgame::Move& move) {
    if (move.isPass) return pass();
    if (move.isResign) {
        resign();
        return true;
    }
    return makeMove(move.x, move.y);
}

bool Othello::pass() {
    if (status != IN_PROGRESS) return false;
    
    // 检查当前玩家是否有合法移动
    if (hasValidMoves(currentPlayer)) {
        return false; // 有合法移动不能pass
    }
    
    // 切换玩家
    currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
    passCount++;
    
    // 保存状态
    saveState();
    
    // 更新游戏状态
    updateGameStatus();
    
    return true;
}

void Othello::resign() {
    if (status != IN_PROGRESS) return;
    
    // 当前玩家认输，对方获胜
    winner = (currentPlayer == BLACK) ? WHITE : BLACK;
    status = (winner == BLACK) ? BLACK_WIN : WHITE_WIN;
    
    // 保存状态
    saveState();
}

bool Othello::undo() {
    if (history.empty()) return false;
    
    // 恢复上一个状态
    history.pop(); // 移除当前状态
    
    if (history.empty()) {
        // 如果没有历史状态，重新初始化游戏
        initGame();
        return true;
    }
    
    // 恢复到上一个状态
    GameState prevState = history.top();
    
    // 恢复棋盘状态
    std::istringstream iss(prevState.boardState);
    std::string line;
    int row = 0;
    while (std::getline(iss, line) && row < board->getSize()) {
        for (int col = 0; col < board->getSize() && col < line.length(); col++) {
            char c = line[col];
            PieceType piece = EMPTY;
            if (c == 'B') piece = BLACK;
            else if (c == 'W') piece = WHITE;
            
            board->setPiece(row, col, piece);
        }
        row++;
    }
    
    // 恢复游戏状态
    currentPlayer = prevState.currentPlayer;
    passCount = prevState.passCount;
    status = prevState.status;
    
    // 重新评估游戏状态
    updateGameStatus();
    
    return true;
}

void Othello::saveGame(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    
    // 保存游戏类型
    file << "OTHELLO\n";
    
    // 保存棋盘状态
    int size = board->getSize();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            PieceType piece = board->getPiece(i, j);
            char c = '.';
            if (piece == BLACK) c = 'B';
            else if (piece == WHITE) c = 'W';
            file << c;
        }
        file << "\n";
    }
    
    // 保存当前玩家
    file << "CURRENT_PLAYER: " << (currentPlayer == BLACK ? "BLACK" : "WHITE") << "\n";
    
    // 保存游戏状态
    file << "STATUS: ";
    switch (status) {
        case IN_PROGRESS: file << "IN_PROGRESS"; break;
        case BLACK_WIN: file << "BLACK_WIN"; break;
        case WHITE_WIN: file << "WHITE_WIN"; break;
        case TIED: file << "TIED"; break;
    }
    file << "\n";
    
    file.close();
}

bool Othello::loadGame(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    
    std::string line;
    std::getline(file, line); // 游戏类型
    
    // 读取棋盘状态
    int size = board->getSize();
    for (int i = 0; i < size; i++) {
        if (!std::getline(file, line)) return false;
        for (int j = 0; j < size && j < line.length(); j++) {
            char c = line[j];
            PieceType piece = EMPTY;
            if (c == 'B') piece = BLACK;
            else if (c == 'W') piece = WHITE;
            
            board->setPiece(i, j, piece);
        }
    }
    
    // 读取当前玩家
    std::getline(file, line);
    if (line.find("BLACK") != std::string::npos) currentPlayer = BLACK;
    else if (line.find("WHITE") != std::string::npos) currentPlayer = WHITE;
    
    // 读取游戏状态
    std::getline(file, line);
    if (line.find("IN_PROGRESS") != std::string::npos) status = IN_PROGRESS;
    else if (line.find("BLACK_WIN") != std::string::npos) status = BLACK_WIN;
    else if (line.find("WHITE_WIN") != std::string::npos) status = WHITE_WIN;
    else if (line.find("TIED") != std::string::npos) status = TIED;
    
    file.close();
    
    // 保存加载的状态
    saveState();
    
    // 更新游戏状态
    updateGameStatus();
    
    return true;
}

GameStatus Othello::getGameStatus() const {
    return status;
}

void Othello::saveState() {
    GameState state(board->getSize());
    
    // 序列化棋盘状态
    std::ostringstream oss;
    int size = board->getSize();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            PieceType piece = board->getPiece(i, j);
            char c = '.';
            if (piece == BLACK) c = 'B';
            else if (piece == WHITE) c = 'W';
            oss << c;
        }
        oss << "\n";
    }
    state.boardState = oss.str();
    
    // 保存其他状态
    state.currentPlayer = currentPlayer;
    state.passCount = passCount;
    state.status = status;
    
    // 添加到历史栈
    history.push(state);
}

bool Othello::isGameOver() const {
    return status != IN_PROGRESS;
}

PieceType Othello::getWinner() const {
    return winner;
}

bool Othello::isValidMove(int x, int y, PieceType player) const {
    // 检查位置是否在棋盘内
    if (x < 0 || x >= board->getSize() || y < 0 || y >= board->getSize()) {
        return false;
    }
    
    // 检查位置是否为空
    if (board->getPiece(x, y) != EMPTY) {
        return false;
    }
    
    // 检查是否能翻转对手的棋子
    int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[] = {1, 1, 0, -1, -1, -1, 0, 1};
    
    for (int i = 0; i < 8; i++) {
        if (checkDirection(x, y, dx[i], dy[i], player)) {
            return true;
        }
    }
    
    return false;
}

std::vector<Point> Othello::getValidMoves(PieceType player) const {
    std::vector<Point> validMoves;
    int size = board->getSize();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (isValidMove(i, j, player)) {
                validMoves.push_back({i, j});
            }
        }
    }
    
    return validMoves;
}

int Othello::countPieces(PieceType player) const {
    int count = 0;
    int size = board->getSize();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board->getPiece(i, j) == player) {
                count++;
            }
        }
    }
    
    return count;
}

bool Othello::checkDirection(int x, int y, int dx, int dy, PieceType player) const {
    int nx = x + dx;
    int ny = y + dy;
    bool foundOpponent = false;
    
    while (nx >= 0 && nx < board->getSize() && ny >= 0 && ny < board->getSize()) {
        PieceType piece = board->getPiece(nx, ny);
        
        if (piece == EMPTY) {
            return false;
        }
        
        if (piece == player) {
            return foundOpponent;
        }
        
        // 找到对手的棋子
        foundOpponent = true;
        nx += dx;
        ny += dy;
    }
    
    return false;
}

std::vector<Point> Othello::getFlippedPieces(int x, int y, PieceType player) const {
    std::vector<Point> flippedPieces;
    int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[] = {1, 1, 0, -1, -1, -1, 0, 1};
    
    for (int i = 0; i < 8; i++) {
        std::vector<Point> directionFlips;
        int nx = x + dx[i];
        int ny = y + dy[i];
        
        while (nx >= 0 && nx < board->getSize() && ny >= 0 && ny < board->getSize()) {
            PieceType piece = board->getPiece(nx, ny);
            
            if (piece == EMPTY) {
                break;
            }
            
            if (piece == player) {
                // 找到自己的棋子，将路径上的对手棋子添加到翻转列表
                flippedPieces.insert(flippedPieces.end(), directionFlips.begin(), directionFlips.end());
                break;
            }
            
            // 对手的棋子
            directionFlips.push_back({nx, ny});
            nx += dx[i];
            ny += dy[i];
        }
    }
    
    return flippedPieces;
}

void Othello::flipPieces(const std::vector<Point>& pieces) {
    for (const auto& p : pieces) {
        board->setPiece(p.x, p.y, currentPlayer);
    }
}

bool Othello::hasValidMoves(PieceType player) const {
    return !getValidMoves(player).empty();
}

void Othello::updateGameStatus() {
    // 检查游戏是否结束
    bool blackHasMoves = hasValidMoves(BLACK);
    bool whiteHasMoves = hasValidMoves(WHITE);
    
    if (!blackHasMoves && !whiteHasMoves) {
        // 双方都没有合法移动，游戏结束
        int blackCount = countPieces(BLACK);
        int whiteCount = countPieces(WHITE);
        
        if (blackCount > whiteCount) {
            status = BLACK_WIN;
            winner = BLACK;
        } else if (whiteCount > blackCount) {
            status = WHITE_WIN;
            winner = WHITE;
        } else {
            status = TIED;
            winner = EMPTY;
        }
    } else if (passCount >= 2) {
        // 连续两次虚着，游戏结束
        int blackCount = countPieces(BLACK);
        int whiteCount = countPieces(WHITE);
        
        if (blackCount > whiteCount) {
            status = BLACK_WIN;
            winner = BLACK;
        } else if (whiteCount > blackCount) {
            status = WHITE_WIN;
            winner = WHITE;
        } else {
            status = TIED;
            winner = EMPTY;
        }
    }
}

}