#pragma once

#include <cstdint>
#include <string>

namespace mprpc {

constexpr uint32_t kMaxFrameBodySize = 16 * 1024 * 1024;

enum class FrameDecodeResult {
    kNeedMoreData,
    kComplete,
    kInvalidSize,
};

bool EncodeFrame(const std::string& payload, std::string* frame);
FrameDecodeResult TryDecodeFrame(std::string* buffer, std::string* payload);

}  // namespace mprpc
