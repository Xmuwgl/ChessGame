#include "Rule.h"

namespace chessgame::model {
class GoRule : public Rule {
private:
    bool visited[19][19]; // 用于 dfs 计算棋子的气

//@brief 获取某个妻子的气
    void getLiberties(int x, int y, std::vector<std::pair<int, int>>& liberties);

    //@brief 提子
    void capture(int x, int y, PieceType player);

public:
    GoRule(Board* b) : Rule(b) {}
    ~GoRule() = default;

    bool isValidMove(int x, int y, PieceType player) override;

    void makeMove(int x, int y, PieceType player) override;

    GameStatus checkWin(int lastX, int lastY) override;
    
    bool supportsPass() const override { return true; }
};
}