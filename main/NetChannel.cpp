#include "NetChannel.h"

#include <utility>
#include "utils.h"

NetChannel::NetChannel(asio::io_context* context)
        : context_(context), client_(*context, 1024 * 2) {
}

void NetChannel::start(std::string ip, std::string port)
{
    client_.open(std::move(ip), std::move(port));
}

void NetChannel::close()
{
    client_.close();
}

void NetChannel::sendData(std::string data) {
    if (!isOpen()) return;
    client_.send(std::move(data));
}

bool NetChannel::isOpen()
{
    return client_.is_open();
}
