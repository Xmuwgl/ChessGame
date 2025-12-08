#pragma once
#include "../utils/Type.h"
#include "../model/Board.h"
#include <string>

namespace chessgame::view {

class GameView {
public:
    virtual ~GameView() = default;
    
    virtual void displayBoard(const model::Board& board, PieceType currentPlayer, 
                             GameType gameType, const std::string& message = "") = 0;
    
    virtual std::string getUserInput(const std::string& prompt) = 0;
    
    virtual void showHelp() = 0;
    
    virtual void showGameResult(GameStatus status, PieceType winner = BLACK);
    
    virtual void showError(const std::string& error) = 0;
    
    virtual void showMessage(const std::string& message) = 0;
    
    virtual void showHint(const std::string& hint) = 0;
    
    virtual void setShowHints(bool show) = 0;
    
    virtual bool getShowHints() const = 0;
    
    // 显示用户信息
    virtual void showUserInfo(const std::string& username, const std::string& message = "") = 0;
    
    // 显示用户战绩
    virtual void showUserStats(const std::string& username, int totalGames, int wins, 
                              int losses, int draws, double winRate, 
                              int gomokuWins, int goWins, int othelloWins,
                              int aiWins, int humanWins, int rank) = 0;
};

class ConsoleView : public GameView {
private:
    bool showHints;
    
    void clearScreen();
    char getPieceChar(PieceType piece) const;
    std::string getPlayerName(PieceType player) const;
    std::string getGameName(GameType gameType) const;

public:
    ConsoleView();
    
    void displayBoard(const model::Board& board, PieceType currentPlayer, 
                     GameType gameType, const std::string& message = "") override;
    
    std::string getUserInput(const std::string& prompt) override;
    
    void showHelp() override;
    
    void showError(const std::string& error) override;
    
    void showMessage(const std::string& message) override;
    
    void showHint(const std::string& hint) override;
    
    void setShowHints(bool show) override { showHints = show; }
    
    bool getShowHints() const override { return showHints; }
    
    // 显示用户信息
    void showUserInfo(const std::string& username, const std::string& message = "") override;
    
    // 显示用户战绩
    void showUserStats(const std::string& username, int totalGames, int wins, 
                       int losses, int draws, double winRate, 
                       int gomokuWins, int goWins, int othelloWins,
                       int aiWins, int humanWins, int rank) override;
};

}