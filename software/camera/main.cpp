#include <vector>
#include <string>
#include <cstdint>
#include <SDL2/SDL.h>
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

    auto window = SDL_CreateWindow(
            "Sevun OV7251 Test",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            640,
            480,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    auto renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    auto texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            320,
            240);

    auto surface = SDL_CreateRGBSurfaceWithFormat(
            0,
            320,
            240,
            32,
            SDL_PIXELFORMAT_RGB565);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

//    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
//    SDL_RenderSetLogicalSize(
//            renderer,
//            640,
//            480);

    video_device.capture_stream(
            result,
            "capture.raw",
            0,
            [&](uint8_t* data, size_t len) {
                SDL_Event e {};

                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        return false;
                    } else if (e.type == SDL_KEYDOWN) {
                        switch (e.key.keysym.sym) {
                            case SDLK_ESCAPE: {
                                return false;
                            }
                            default: {
                                break;
                            }
                        }
                    }
                }

                SDL_LockSurface(surface);
                memcpy(surface->pixels, data, len);
                SDL_UnlockSurface(surface);

                SDL_UpdateTexture(
                        texture,
                        nullptr,
                        surface->pixels,
                        surface->pitch);

                SDL_RenderCopy(
                        renderer,
                        texture,
                        nullptr,
                        nullptr);

                SDL_RenderPresent(renderer);

                return true;
            });


    return 0;
}
