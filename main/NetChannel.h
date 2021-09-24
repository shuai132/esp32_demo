#pragma once

#include "noncopyable.hpp"
#include "TcpClient.h"
#include "PacketProcessor.h"

class NetChannel : noncopyable {
public:
    explicit NetChannel(asio::io_context* context);
    void start(std::string ip, std::string port);
    void close();
    void sendData(void* data, size_t len);
    bool isOpen();

    std::function<void()>& onOpen = client_.onOpen;
    std::function<void()>& onClose = client_.onClose;
    std::function<void(void* data, size_t len)> onData;

private:
    asio::io_context* context_;
    TcpClient client_;
    PacketProcessor packetProcessor_;
};
