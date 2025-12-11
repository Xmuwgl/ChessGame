#pragma once
#include "NetworkProtocol.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <functional>

namespace chessgame::network {

class NetworkClient {
private:
    int clientSocket;
    std::atomic<bool> connected;
    std::atomic<bool> running;
    std::thread receiveThread;
    
    // 回调函数
    std::function<void(const NetworkMessage&)> messageCallback;
    std::function<void()> connectCallback;
    std::function<void()> disconnectCallback;
    
    // 私有方法
    void receiveMessages();
    bool sendMessageInternal(const NetworkMessage& message);
    NetworkMessage receiveMessage();

public:
    NetworkClient();
    ~NetworkClient();
    
    // 连接管理
    bool connect(const std::string& serverIP, int port = NetworkConfig::DEFAULT_PORT);
    void disconnect();
    bool isConnected() const { return connected.load(); }
    
    // 消息发送
    bool sendMessage(const NetworkMessage& message);
    
    // 回调设置
    void setMessageCallback(std::function<void(const NetworkMessage&)> callback) {
        messageCallback = callback;
    }
    
    void setConnectCallback(std::function<void()> callback) {
        connectCallback = callback;
    }
    
    void setDisconnectCallback(std::function<void()> callback) {
        disconnectCallback = callback;
    }
    
    // 连接状态检查
    void startHeartbeat();
    void stopHeartbeat();
    
private:
    std::thread heartbeatThread;
    std::atomic<bool> heartbeatRunning;
};

} // namespace chessgame::network