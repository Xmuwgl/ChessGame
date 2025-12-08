#include "GameView.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace chessgame::view;

void GameView::showGameResult(GameStatus status, PieceType winner) {
    std::cout << "\n================================" << std::endl;
    if (status == BLACK_WIN) {
        std::cout << "   BLACK WIN!" << std::endl;
    } else if (status == WHITE_WIN) {
        std::cout << "   WHITE WIN!" << std::endl;
    } else if (status == TIED) {
        std::cout << "   TIED!" << std::endl;
    }
    std::cout << "================================" << std::endl;
}

ConsoleView::ConsoleView() : showHints(true) {}

void ConsoleView::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

char ConsoleView::getPieceChar(PieceType piece) const {
    switch (piece) {
        case BLACK: return 'X';
        case WHITE: return 'O';
        default: return '+';
    }
}

std::string ConsoleView::getPlayerName(PieceType player) const {
    return (player == BLACK) ? "BLACK (X)" : "WHITE (O)";
}

std::string ConsoleView::getGameName(GameType gameType) const {
    return (gameType == GOMOKU) ? "GOMOKU" : "GO";
}

void ConsoleView::displayBoard(const model::Board& board, PieceType currentPlayer, 
                              GameType gameType, const std::string& message) {
    clearScreen();
    
    int size = board.getSize();
    
    // 显示标题
    std::cout << "===== " << getGameName(gameType) << " =====" << std::endl;
    
    // 显示列号
    std::cout << "   ";
    for (int i = 0; i < size; ++i) {
        std::cout << std::setw(2) << i + 1 << " ";
    }
    std::cout << std::endl;
    
    // 显示棋盘
    for (int i = 0; i < size; ++i) {
        std::cout << std::setw(2) << i + 1 << " ";
        for (int j = 0; j < size; ++j) {
            PieceType p = board.getPiece(i, j);
            std::cout << " " << getPieceChar(p) << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Current Player: " << getPlayerName(currentPlayer) << std::endl;
    
    if (!message.empty()) std::cout << "Tips: " << message << std::endl;
    // 显示操作提示
    if (showHints) std::cout << "Enter 'help' to show instructions" << std::endl;
}

std::string ConsoleView::getUserInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

void ConsoleView::showHelp() {
    clearScreen();
    std::cout << "===== 指令说明 =====" << std::endl;
    std::cout << "  x y      : 落子 (例如: 3 4)" << std::endl;
    std::cout << "  pass     : 虚着 (仅围棋)" << std::endl;
    std::cout << "  undo     : 悔棋" << std::endl;
    std::cout << "  resign   : 认输" << std::endl;
    std::cout << "  save [文件名] : 保存游戏" << std::endl;
    std::cout << "  load [文件名] : 加载游戏" << std::endl;
    std::cout << "  restart  : 重新开始" << std::endl;
    std::cout << "  hint     : 切换提示显示" << std::endl;
    std::cout << "  quit     : 退出游戏" << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << "按回车键继续...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

void ConsoleView::showError(const std::string& error) {
    std::cout << "错误: " << error << std::endl;
}

void ConsoleView::showMessage(const std::string& message) {
    std::cout << message << std::endl;
}

void ConsoleView::showHint(const std::string& hint) {
    std::cout << "提示: " << hint << std::endl;
}

void ConsoleView::showUserInfo(const std::string& username, const std::string& message) {
    std::cout << "\n===== 用户信息 =====" << std::endl;
    std::cout << "用户名: " << username << std::endl;
    if (!message.empty()) {
        std::cout << message << std::endl;
    }
    std::cout << "====================" << std::endl;
}

void ConsoleView::showUserStats(const std::string& username, int totalGames, int wins, 
                                int losses, int draws, double winRate, 
                                int gomokuWins, int goWins, int othelloWins,
                                int aiWins, int humanWins, int rank) {
    std::cout << "\n===== 个人战绩 =====" << std::endl;
    std::cout << "用户名: " << username << std::endl;
    std::cout << "总游戏数: " << totalGames << std::endl;
    std::cout << "胜利次数: " << wins << std::endl;
    std::cout << "失败次数: " << losses << std::endl;
    std::cout << "平局次数: " << draws << std::endl;
    std::cout << "胜率: " << std::fixed << std::setprecision(1) << winRate << "%" << std::endl;
    
    std::cout << "\n按游戏类型统计:" << std::endl;
    std::cout << "五子棋胜利: " << gomokuWins << std::endl;
    std::cout << "围棋胜利: " << goWins << std::endl;
    std::cout << "黑白棋胜利: " << othelloWins << std::endl;
    
    std::cout << "\n按对手类型统计:" << std::endl;
    std::cout << "对AI胜利: " << aiWins << std::endl;
    std::cout << "对人类胜利: " << humanWins << std::endl;
    
    if (rank > 0) {
        std::cout << "\n当前排名: 第" << rank << "名" << std::endl;
    }
    
    std::cout << "====================" << std::endl;
}