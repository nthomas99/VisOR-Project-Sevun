#pragma once

#include <functional>
#include "result.h"
#include "buffers.h"

namespace sevun {

    struct device_capabilities_t {
        bool capture = false;
        bool capture_mplane = false;
        bool output = false;
        bool output_mplane = false;
        bool video_m2m = false;
        bool video_m2m_mplane = false;
        bool overlay = false;
        bool vbi_capture = false;
        bool vbi_output = false;
        bool sliced_vbi_capture = false;
        bool sliced_vbi_output = false;
        bool rds_capture = false;
        bool tuner = false;
        bool audio = false;
        bool radio = false;
        bool read_write = false;
        bool async_io = false;
        bool streaming = false;
    };

    struct device_info_t {
        std::string card;
        std::string driver;
        std::string bus_info;
        uint32_t version = 0;
        device_capabilities_t capabilities {};
    };

    class device {
    public:
        using render_frame_callable = std::function<bool (uint8_t*, size_t)>;

        explicit device(const std::string& path);

        virtual ~device();

        bool open(sevun::result& result);

        const device_info_t& info() const;

        void capture_stream(
            sevun::result &result,
            const std::string& output_path,
            uint32_t stream_count,
            const render_frame_callable& callable);

    private:
        int do_handle_cap(
            sevun::buffers &b,
            FILE *fout,
            int *index,
            unsigned int &count,
            timespec &ts_last,
            const render_frame_callable& callable);

        int do_ioctl_name(
            sevun::result& result,
            unsigned long int request,
            void* parm,
            const std::string& name);

        bool enumerate_video_formats(
                sevun::result& result,
                uint32_t type);

        bool get_capture_format(sevun::result& result);

        bool is_sub_device(sevun::result& result) const;

    private:
        int _fd;
        std::string _path;
        device_info_t _info {};
        uint32_t _stream_skip = 0;
        uint32_t _stream_count = 0;
    };
};