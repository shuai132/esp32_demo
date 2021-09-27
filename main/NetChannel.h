#pragma once

#include "noncopyable.hpp"
#include "tcp_client.hpp"

using namespace asio_tcp;

class NetChannel : noncopyable {
public:
    explicit NetChannel(asio::io_context* context);
    void start(std::string ip, std::string port);
    void close();
    void sendData(std::string data);
    bool isOpen();

    std::function<void()>& onOpen = client_.onOpen;
    std::function<void()>& onClose = client_.onClose;
    std::function<void(std::string)>& onData = client_.onData;

private:
    asio::io_context* context_;
    tcp_client client_;
};
