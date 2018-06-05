#pragma once

#include "result.h"

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
        std::string driver;
        std::string card;
        std::string bus_info;
        uint32_t version;
        device_capabilities_t capabilities {};
    };

    class device {
    public:
        explicit device(const std::string& path);

        virtual ~device();

        bool open(sevun::result& result);

        const device_info_t& info() const;

    private:
        int do_ioctl_name(
            sevun::result& result,
            unsigned long int request,
            void* parm,
            const std::string& name);

        bool is_sub_device(sevun::result& result) const;

    private:
        int _fd;
        std::string _path;
        device_info_t _info {};
    };
};