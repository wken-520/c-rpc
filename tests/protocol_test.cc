#include "mprpcprotocol.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void Expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void TestPartialFrame()
{
    std::string frame;
    Expect(mprpc::EncodeFrame("hello", &frame), "frame should encode");

    std::string buffered(frame.data(), 3);
    std::string payload;
    Expect(mprpc::TryDecodeFrame(&buffered, &payload) ==
               mprpc::FrameDecodeResult::kNeedMoreData,
           "partial header should wait for more data");

    buffered.append(frame.data() + 3, frame.size() - 3);
    Expect(mprpc::TryDecodeFrame(&buffered, &payload) ==
               mprpc::FrameDecodeResult::kComplete,
           "complete frame should decode");
    Expect(payload == "hello", "decoded payload should match");
    Expect(buffered.empty(), "decoded frame should be consumed");
}

void TestCoalescedFrames()
{
    std::string first;
    std::string second;
    Expect(mprpc::EncodeFrame("one", &first), "first frame should encode");
    Expect(mprpc::EncodeFrame("two", &second), "second frame should encode");

    std::string buffered = first + second;
    std::string payload;
    Expect(mprpc::TryDecodeFrame(&buffered, &payload) ==
               mprpc::FrameDecodeResult::kComplete && payload == "one",
           "first coalesced frame should decode");
    Expect(mprpc::TryDecodeFrame(&buffered, &payload) ==
               mprpc::FrameDecodeResult::kComplete && payload == "two",
           "second coalesced frame should decode");
}

void TestOversizedFrame()
{
    const char oversized_header[] = {1, 0, 0, 1};
    std::string buffered(oversized_header, sizeof(oversized_header));
    std::string payload;
    Expect(mprpc::TryDecodeFrame(&buffered, &payload) ==
               mprpc::FrameDecodeResult::kInvalidSize,
           "oversized frame should be rejected");
}

}  // namespace

int main()
{
    TestPartialFrame();
    TestCoalescedFrames();
    TestOversizedFrame();
    return EXIT_SUCCESS;
}
