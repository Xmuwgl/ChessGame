#pragma once
#include "NetworkProtocol.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>

namespace chessgame::network {

class NetworkServer {
private:
    int serverSocket;
    int clientSockets[NetworkConfig::MAX_CONNECTIONS];
    std::atomic<bool> running;
    std::thread acceptThread;
    std::thread clientThreads[NetworkConfig::MAX_CONNECTIONS];
    mutable std::mutex clientMutex;
    
    // 回调函数
    std::function<void(int, const NetworkMessage&)> messageCallback;
    std::function<void(int)> connectCallback;
    std::function<void(int)> disconnectCallback;
    
    // 私有方法
    void acceptConnections();
    void handleClient(int clientSocket);
    bool sendMessage(int clientSocket, const NetworkMessage& message);
    NetworkMessage receiveMessage(int clientSocket);
    void cleanupClient(int clientSocket);

public:
    NetworkServer();
    ~NetworkServer();
    
    // 服务器控制
    bool start(int port = NetworkConfig::DEFAULT_PORT);
    void stop();
    bool isRunning() const { return running.load(); }
    
    // 消息处理
    void broadcastMessage(const NetworkMessage& message);
    void sendToClient(int clientSocket, const NetworkMessage& message);
    
    // 回调设置
    void setMessageCallback(std::function<void(int, const NetworkMessage&)> callback) {
        messageCallback = callback;
    }
    
    void setConnectCallback(std::function<void(int)> callback) {
        connectCallback = callback;
    }
    
    void setDisconnectCallback(std::function<void(int)> callback) {
        disconnectCallback = callback;
    }
    
    // 客户端管理
    int getConnectedClientCount() const;
    std::vector<int> getConnectedClients() const;
    
    // 获取本地IP地址
    std::string getLocalIPAddress() const;
};

} // namespace chessgame::network