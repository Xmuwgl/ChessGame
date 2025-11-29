#pragma once
#include "AbstractGame.h"
#include "Board.h"
#include "GomokuRule.h"
#include <fstream>
#include <sstream>

namespace chessgame::model {
class Gomoku : public AbstractGame {
private:
    GameStatus status{IN_PROGRESS};
    int lastX{-1}, lastY{-1};

public:
    explicit Gomoku(int boardSize = 15) {
        board = std::make_shared<Board>(boardSize);
        rule = std::make_shared<GomokuRule>(board.get());
        gameType = GOMOKU;
        currentPlayer = BLACK;
        passCount = 0;
        status = IN_PROGRESS;
    }

    void initGame() override {
        board->clear();
        currentPlayer = BLACK;
        passCount = 0;
        status = IN_PROGRESS;
        while (!history.empty()) history.pop();
        saveState();
    }

    void run() override {}

    bool makeMove(int x, int y) override {
        if (status != IN_PROGRESS) return false;
        if (!rule->isValidMove(x, y, currentPlayer)) return false;
        saveState();
        rule->makeMove(x, y, currentPlayer);
        lastX = x; lastY = y;
        status = rule->checkWin(x, y);
        if (status == IN_PROGRESS) currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
        return true;
    }

    bool makeMove(const chessgame::Move& move) override {
        if (move.isResign) { resign(); return true; }
        if (move.isPass) { return pass(); }
        return makeMove(move.x, move.y);
    }

    bool pass() override { return false; }

    void resign() override {
        if (status != IN_PROGRESS) return;
        saveState();
        status = (currentPlayer == BLACK) ? WHITE_WIN : BLACK_WIN;
    }

    bool undo() override {
        if (history.empty()) return false;
        GameState st = history.top();
        history.pop();
        std::stringstream ss(st.boardState);
        board->deserialize(ss);
        currentPlayer = st.currentPlayer;
        passCount = st.passCount;
        status = st.status;
        return true;
    }

    void saveGame(const std::string& filename) override {
        std::ofstream out(filename);
        if (!out.is_open()) return;
        out << static_cast<int>(gameType) << " "
            << static_cast<int>(currentPlayer) << " "
            << passCount << " "
            << static_cast<int>(status) << "\n";
        out << board->serialize();
        out.close();
    }

    bool loadGame(const std::string& filename) override {
        std::ifstream in(filename);
        if (!in.is_open()) return false;
        int type, player, passes, st;
        in >> type >> player >> passes >> st;
        std::stringstream ss;
        ss << in.rdbuf();
        board->deserialize(ss);
        gameType = static_cast<GameType>(type);
        currentPlayer = static_cast<PieceType>(player);
        passCount = passes;
        status = static_cast<GameStatus>(st);
        rule = std::make_shared<GomokuRule>(board.get());
        while (!history.empty()) history.pop();
        saveState();
        return true;
    }

    GameStatus getGameStatus() const override { return status; }

    void saveState() override {
        GameState st;
        st.boardState = board->serialize();
        st.currentPlayer = currentPlayer;
        st.passCount = passCount;
        st.status = status;
        history.push(st);
    }

    bool isGameOver() const override { return status != IN_PROGRESS; }

    PieceType getWinner() const override {
        if (status == BLACK_WIN) return BLACK;
        if (status == WHITE_WIN) return WHITE;
        return EMPTY;
    }
};
}
