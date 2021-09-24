#pragma once

#include "asio.hpp"
#include "base/noncopyable.hpp"

using asio::ip::tcp;

class TcpClient : noncopyable {
 public:
  explicit TcpClient(asio::io_context& context);

  void open(std::string ip, std::string port);

  bool isOpen();

  bool send(const void* data, size_t len);

  void close();

 private:
  void do_connect(const tcp::resolver::results_type& endpoints);

  void do_read();
  void on_read(std::error_code ec, std::size_t length);

  void do_close();

 public:
  std::function<void()> onOpen;
  std::function<void()> onClose;
  std::function<void(uint8_t* data, size_t len)> onData;

 private:
  tcp::socket socket_;
  const int BufferLen = 1024 * 4;
  std::string buffer_;
};
