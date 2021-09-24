#include "TcpClient.h"

TcpClient::TcpClient(asio::io_context& context)
    : socket_(context) {
  buffer_.reserve(BufferLen);
}

void TcpClient::open(std::string ip, std::string port)
{
    tcp::resolver resolver(socket_.get_executor());
    do_connect(resolver.resolve(std::move(ip), std::move(port)));
}

bool TcpClient::isOpen() { return socket_.is_open(); }

bool TcpClient::send(const void* data, size_t len) {
  asio::async_write(
      socket_, asio::buffer(data, len),
      [this, payloadSize = len](std::error_code ec, std::size_t length) {
        if (ec || length != payloadSize) {
          do_close();
        }
      });
  return true;
}

void TcpClient::close() { socket_.close(); }

void TcpClient::do_connect(const tcp::resolver::results_type& endpoints) {
  asio::async_connect(socket_, endpoints,
                      [this](std::error_code ec, const tcp::endpoint&) {
                        if (!ec) {
                          if (onOpen) onOpen();
                          do_read();
                        }
                      });
}

void TcpClient::do_read() {
  socket_.async_read_some(
      asio::buffer((void*)buffer_.data(), buffer_.capacity()),
      [this](std::error_code ec, std::size_t length) { on_read(ec, length); });
}

void TcpClient::on_read(std::error_code ec, std::size_t length) {
  if (!ec) {
    if (onData) {
      onData((uint8_t*)buffer_.data(), length);
    }
    do_read();
  } else {
    do_close();
  }
}

void TcpClient::do_close() {
  socket_.close();
  if (onClose) {
    onClose();
  }
}
