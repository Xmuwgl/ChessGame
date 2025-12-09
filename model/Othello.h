#pragma once
#include "AbstractGame.h"
#include "../utils/Type.h"
#include <memory>

namespace chessgame::model {

class Othello : public AbstractGame {
private:
    GameStatus status{IN_PROGRESS};
    PieceType winner{EMPTY};
    
    // 黑白棋特有辅助方法
    bool checkDirection(int x, int y, int dx, int dy, PieceType player) const;
    std::vector<Point> getFlippedPieces(int x, int y, PieceType player) const;
    void flipPieces(const std::vector<Point>& pieces);
    bool hasValidMoves(PieceType player) const;
    void updateGameStatus();

public:
    Othello();
    ~Othello() override = default;
    
    // 实现抽象游戏接口
    void initGame() override;
    void run() override;
    bool makeMove(int x, int y) override;
    bool makeMove(const chessgame::Move& move) override;
    bool pass() override;
    void resign() override;
    bool undo() override;
    void saveGame(const std::string& filename) override;
    bool loadGame(const std::string& filename) override;
    GameStatus getGameStatus() const override;
    void saveState() override;
    bool isGameOver() const override;
    PieceType getWinner() const override;
    
    // 黑白棋特有方法
    bool isValidMove(int x, int y, PieceType player) const;
    std::vector<Point> getValidMoves(PieceType player) const;
    int countPieces(PieceType player) const;
};

}