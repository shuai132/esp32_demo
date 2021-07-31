#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "MakeEvent.hpp"
#include "log.h"

using asio::ip::tcp;

class session
        : public std::enable_shared_from_this<session>
{
    MAKE_EVENT(Close);

public:
    session(tcp::socket socket)
            : socket_(std::move(socket))
    {
        LOGD("session");
    }

   ~session() {
        LOGD("~session");
    }

    void start()
    {
        do_read();
    }

    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(asio::buffer(data_, max_length),
                                [this, self](std::error_code ec, std::size_t length){
                                    if (ec) {
                                        onClose();
                                    }
        });
    }

    void write(const uint8_t* data, std::size_t length)
    {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer(data, length),
                          [self](std::error_code ec, std::size_t){});
    }

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

using Session = std::shared_ptr<session>;
using WSession = std::weak_ptr<session>;

class TcpServer
{
public:
    TcpServer(asio::io_context& io_context, short port)
            : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
              socket_(io_context)
    {
        do_accept();
    }

    void writeAll(const uint8_t* data, size_t size) {
        for(const auto& s : _sessions) {
            s->write(data, size);
        }
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(socket_,
                               [this](std::error_code ec)
                               {
                                   if (!ec)
                                   {
                                       auto s = std::make_shared<session>(std::move(socket_));
                                       s->start();
                                       _sessions.push_back(s);
                                       s->setCloseCb([this, ws = WSession(s)] {
                                           _sessions.erase(std::find(_sessions.cbegin(), _sessions.cend(), ws.lock()));
                                       });
                                   }

                                   do_accept();
                               });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;

    std::vector<std::shared_ptr<session>> _sessions;
};
