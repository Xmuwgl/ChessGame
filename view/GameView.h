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
    
    virtual void showHint(const std::string& hint) = 0;
    
    virtual void setShowHints(bool show) = 0;
    
    virtual bool getShowHints() const = 0;
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
    
    void showHint(const std::string& hint) override;
    
    void setShowHints(bool show) override { showHints = show; }
    
    bool getShowHints() const override { return showHints; }
};

}