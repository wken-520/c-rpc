#include "mprpcprotocol.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <cstring>

namespace mprpc {

bool EncodeFrame(const std::string& payload, std::string* frame)
{
    if (frame == nullptr || payload.size() > kMaxFrameBodySize)
    {
        return false;
    }

    const uint32_t network_size = htonl(static_cast<uint32_t>(payload.size()));
    frame->assign(reinterpret_cast<const char*>(&network_size),
                  sizeof(network_size));
    frame->append(payload);
    return true;
}

FrameDecodeResult TryDecodeFrame(std::string* buffer, std::string* payload)
{
    if (buffer == nullptr || payload == nullptr || buffer->size() < sizeof(uint32_t))
    {
        return FrameDecodeResult::kNeedMoreData;
    }

    uint32_t network_size = 0;
    std::memcpy(&network_size, buffer->data(), sizeof(network_size));
    const uint32_t payload_size = ntohl(network_size);
    if (payload_size > kMaxFrameBodySize)
    {
        return FrameDecodeResult::kInvalidSize;
    }

    const size_t frame_size = sizeof(uint32_t) + static_cast<size_t>(payload_size);
    if (buffer->size() < frame_size)
    {
        return FrameDecodeResult::kNeedMoreData;
    }

    payload->assign(buffer->data() + sizeof(uint32_t), payload_size);
    buffer->erase(0, frame_size);
    return FrameDecodeResult::kComplete;
}

}  // namespace mprpc
