#include "Rule.h"
#include <vector>

namespace chessgame::model {
class GoRule : public Rule {
private:
    mutable bool visited[19][19]; // 用于 dfs 计算棋子的气

    // 获取某个棋子所在群组的气
    int getLiberties(int x, int y, PieceType color, std::vector<Point>& group) const;
    
    // DFS计算气
    int countLibertiesDFS(int x, int y, PieceType color, std::vector<Point>& group) const;

    // 提子
    void capture(int x, int y, PieceType opponent);

public:
    GoRule(Board* b) : Rule(b) {}
    ~GoRule() = default;

    bool isValidMove(int x, int y, PieceType player) const override;

    void makeMove(int x, int y, PieceType player) override;

    GameStatus checkWin(int lastX, int lastY) override;
    
    bool supportsPass() const override { return true; }
};
}
