#include <vector>
#include <string>
#include <cstdint>
#include <fmt/format.h>
#include "result.h"
#include "device.h"

int main(int argc, char** argv) {
    sevun::device video_device("/dev/video0");

    sevun::result result;
    if (!video_device.open(result)) {
        for (const auto& msg: result.messages()) {
            fmt::print("{}: {}\n", msg.code(), msg.message());
        }
        return 1;
    }

    auto info = video_device.info();

    fmt::print("      driver: {}\n", info.driver);
    fmt::print("        card: {}\n", info.card);
    fmt::print("    bus_info: {}\n", info.bus_info);
    fmt::print("     version: {}\n", info.version);
    fmt::print("\ncapability flags\n");
    fmt::print("name                       value\n");
    fmt::print("--------------------------------\n");
    fmt::print("capture                     {}\n", info.capabilities.capture);
    fmt::print("capture_mplane              {}\n", info.capabilities.capture_mplane);
    fmt::print("output                      {}\n", info.capabilities.output);
    fmt::print("output_mplane               {}\n", info.capabilities.output_mplane);
    fmt::print("video_m2m                   {}\n", info.capabilities.video_m2m);
    fmt::print("video_m2m_mplane            {}\n", info.capabilities.video_m2m_mplane);
    fmt::print("overlay                     {}\n", info.capabilities.overlay);
    fmt::print("vbi_capture                 {}\n", info.capabilities.vbi_capture);
    fmt::print("vbi_output                  {}\n", info.capabilities.vbi_output);
    fmt::print("sliced_vbi_capture          {}\n", info.capabilities.sliced_vbi_capture);
    fmt::print("sliced_vbi_output           {}\n", info.capabilities.sliced_vbi_output);
    fmt::print("rds_capture                 {}\n", info.capabilities.rds_capture);
    fmt::print("tuner                       {}\n", info.capabilities.tuner);
    fmt::print("audio                       {}\n", info.capabilities.audio);
    fmt::print("radio                       {}\n", info.capabilities.radio);
    fmt::print("read_write                  {}\n", info.capabilities.read_write);
    fmt::print("async_io                    {}\n", info.capabilities.async_io);
    fmt::print("streaming                   {}\n", info.capabilities.streaming);

    return 0;
}
