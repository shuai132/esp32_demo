#include "NetChannel.h"

#include <utility>
#include "utils.h"

NetChannel::NetChannel(asio::io_context* context)
        : context_(context), client_(*context) {
    packetProcessor_.setMaxBufferSize(1024 * 2);
    packetProcessor_.setOnPacketHandle([this](uint8_t* data, size_t size) {
        if (onData) onData(data, size);
    });
    client_.onData = [this](uint8_t* data, size_t len) {
        FOR(i, len) {
            packetProcessor_.feed(data + i, 1);
        }
    };
}

void NetChannel::start(std::string ip, std::string port)
{
    client_.open(std::move(ip), std::move(port));
}

void NetChannel::close()
{
    client_.close();
}

void NetChannel::sendData(void* data, size_t len) {
    if (!client_.isOpen()) return;
    auto packet = packetProcessor_.pack(data, len);
    client_.send(packet.data(), packet.size());
}

bool NetChannel::isOpen()
{
    return client_.isOpen();
}
