#include "GameMemento.h"
#include "Board.h"
#include <sstream>

using namespace chessgame::model;

GameMemento::GameMemento(const Board& board, PieceType player, int passes, 
                         GameStatus gameStatus, GameType type) 
    : currentPlayer(player), passCount(passes), status(gameStatus), 
      gameType(type), boardSize(board.getSize()) {
    // 序列化棋盘状态
    boardState = board.serialize();
}

void GameCaretaker::saveState(std::shared_ptr<GameMemento> memento) {
    history.push_back(memento);
    
    // 限制历史记录数量
    if (history.size() > maxHistorySize) {
        history.erase(history.begin());
    }
}

std::shared_ptr<GameMemento> GameCaretaker::getPreviousState() {
    if (history.empty()) return nullptr;
    
    auto memento = history.back();
    history.pop_back();
    return memento;
}

void GameCaretaker::clearHistory() {
    history.clear();
}