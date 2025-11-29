#include "controller/GameManager.h"
#include <iostream>

int main() {
    try {
        std::cout << "===== 棋类游戏系统 =====" << std::endl;
        std::cout << "支持五子棋和围棋" << std::endl;
        std::cout << "========================" << std::endl;
        
        chessgame::controller::GameManager gameManager;
        gameManager.startGame();
        
        std::cout << "感谢使用棋类游戏系统！" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "未知异常" << std::endl;
        return 1;
    }
}