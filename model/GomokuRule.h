#include "Rule.h"

namespace chessgame::model {
class GomokuRule : public Rule {
public:
    GomokuRule(Board* b) : Rule(b) {}
    ~GomokuRule() = default;

    bool isValidMove(int x, int y, PieceType player) override;
    
    void makeMove(int x, int y, PieceType player) override;

    GameStatus checkWin(int lastX, int lastY) override;
};
}