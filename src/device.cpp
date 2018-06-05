#include <vector>
#include <string>
#include <fcntl.h>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <libv4l2.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <fmt/format.h>
#include <sys/sysmacros.h>
#include <linux/videodev2.h>
#include "device.h"
#include "buffers.h"

namespace sevun {

    static std::string field2s(int val) {
        switch (val) {
            case V4L2_FIELD_ANY:
                return "Any";
            case V4L2_FIELD_NONE:
                return "None";
            case V4L2_FIELD_TOP:
                return "Top";
            case V4L2_FIELD_BOTTOM:
                return "Bottom";
            case V4L2_FIELD_INTERLACED:
                return "Interlaced";
            case V4L2_FIELD_SEQ_TB:
                return "Sequential Top-Bottom";
            case V4L2_FIELD_SEQ_BT:
                return "Sequential Bottom-Top";
            case V4L2_FIELD_ALTERNATE:
                return "Alternating";
            case V4L2_FIELD_INTERLACED_TB:
                return "Interlaced Top-Bottom";
            case V4L2_FIELD_INTERLACED_BT:
                return "Interlaced Bottom-Top";
            default:
                return fmt::format("Unknown ({})", val);
        }
    }

    static std::string frmtype2s(unsigned type) {
        static const char* types[] = {
            "Unknown",
            "Discrete",
            "Continuous",
            "Stepwise"
        };

        if (type > 3)
            type = 0;

        return types[type];
    }

    static std::string fract2sec(const struct v4l2_fract &f) {
        return fmt::format("{:.3}", (1.0 * f.numerator) / f.denominator);
    }

    static std::string fract2fps(const struct v4l2_fract &f) {
        return fmt::format("{:.3}", (1.0 * f.denominator) / f.numerator);
    }

    static void print_frmsize(
            const struct v4l2_frmsizeenum &frmsize,
            const char *prefix) {
        fmt::print("{}\tSize: {} ", prefix, frmtype2s(frmsize.type));
        if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            fmt::print("{}x{}", frmsize.discrete.width, frmsize.discrete.height);
        } else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
            fmt::print("{}x{} - {}x{} with step {}/{}",
               frmsize.stepwise.min_width,
               frmsize.stepwise.min_height,
               frmsize.stepwise.max_width,
               frmsize.stepwise.max_height,
               frmsize.stepwise.step_width,
               frmsize.stepwise.step_height);
        }
        fmt::print("\n");
    }

    static void print_frmival(
            const struct v4l2_frmivalenum &frmival,
            const char *prefix) {
        fmt::print("{}\tInterval: {} ", prefix, frmtype2s(frmival.type));
        if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
            fmt::print("{}s ({} fps)\n", fract2sec(frmival.discrete),
                   fract2fps(frmival.discrete));
        } else if (frmival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
            fmt::print("{}s - {}s ({}-{} fps)\n",
                   fract2sec(frmival.stepwise.min),
                   fract2sec(frmival.stepwise.max),
                   fract2fps(frmival.stepwise.max),
                   fract2fps(frmival.stepwise.min));
        } else if (frmival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
            fmt::print("{}s - {}s with step {}s ({}-{} fps)\n",
                   fract2sec(frmival.stepwise.min),
                   fract2sec(frmival.stepwise.max),
                   fract2sec(frmival.stepwise.step),
                   fract2fps(frmival.stepwise.max),
                   fract2fps(frmival.stepwise.min));
        }
    }

    static std::string buftype2s(int type) {
        switch (type) {
            case 0:
                return "Invalid";
            case V4L2_BUF_TYPE_VIDEO_CAPTURE:
                return "Video Capture";
            case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
                return "Video Capture Multiplanar";
            case V4L2_BUF_TYPE_VIDEO_OUTPUT:
                return "Video Output";
            case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
                return "Video Output Multiplanar";
            case V4L2_BUF_TYPE_VIDEO_OVERLAY:
                return "Video Overlay";
            case V4L2_BUF_TYPE_VBI_CAPTURE:
                return "VBI Capture";
            case V4L2_BUF_TYPE_VBI_OUTPUT:
                return "VBI Output";
            case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
                return "Sliced VBI Capture";
            case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
                return "Sliced VBI Output";
            case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
                return "Video Output Overlay";
            case V4L2_BUF_TYPE_SDR_CAPTURE:
                return "SDR Capture";
            case V4L2_BUF_TYPE_SDR_OUTPUT:
                return "SDR Output";
//            case V4L2_BUF_TYPE_META_CAPTURE:
//                return "Metadata Capture";
            default:
                return fmt::format("Unknown ({})", type);
        }
    }

    static std::string fcc2s(unsigned int val) {
        std::string s;

        s += val & 0x7f;
        s += (val >> 8) & 0x7f;
        s += (val >> 16) & 0x7f;
        s += (val >> 24) & 0x7f;

        if (val & (1 << 31))
            s += "-BE";

        return s;
    }

    static std::string colorspace2s(int val) {
        switch (val) {
            case V4L2_COLORSPACE_DEFAULT:
                return "Default";
            case V4L2_COLORSPACE_SMPTE170M:
                return "SMPTE 170M";
            case V4L2_COLORSPACE_SMPTE240M:
                return "SMPTE 240M";
            case V4L2_COLORSPACE_REC709:
                return "Rec. 709";
            case V4L2_COLORSPACE_BT878:
                return "Broken Bt878";
            case V4L2_COLORSPACE_470_SYSTEM_M:
                return "470 System M";
            case V4L2_COLORSPACE_470_SYSTEM_BG:
                return "470 System BG";
            case V4L2_COLORSPACE_JPEG:
                return "JPEG";
            case V4L2_COLORSPACE_SRGB:
                return "sRGB";
            case V4L2_COLORSPACE_ADOBERGB:
                return "AdobeRGB";
            case V4L2_COLORSPACE_DCI_P3:
                return "DCI-P3";
            case V4L2_COLORSPACE_BT2020:
                return "BT.2020";
            case V4L2_COLORSPACE_RAW:
                return "Raw";
            default:
                return fmt::format("Unknown ({})", val);
        }
    }

    static std::string xfer_func2s(int val) {
        switch (val) {
            case V4L2_XFER_FUNC_DEFAULT:
                return "Default";
            case V4L2_XFER_FUNC_709:
                return "Rec. 709";
            case V4L2_XFER_FUNC_SRGB:
                return "sRGB";
            case V4L2_XFER_FUNC_ADOBERGB:
                return "AdobeRGB";
            case V4L2_XFER_FUNC_DCI_P3:
                return "DCI-P3";
            case V4L2_XFER_FUNC_SMPTE2084:
                return "SMPTE 2084";
            case V4L2_XFER_FUNC_SMPTE240M:
                return "SMPTE 240M";
            case V4L2_XFER_FUNC_NONE:
                return "None";
            default:
                return fmt::format("Unknown ({})", val);
        }
    }

    static std::string ycbcr_enc2s(int val) {
        switch (val) {
            case V4L2_YCBCR_ENC_DEFAULT:
                return "Default";
            case V4L2_YCBCR_ENC_601:
                return "ITU-R 601";
            case V4L2_YCBCR_ENC_709:
                return "Rec. 709";
            case V4L2_YCBCR_ENC_XV601:
                return "xvYCC 601";
            case V4L2_YCBCR_ENC_XV709:
                return "xvYCC 709";
            case V4L2_YCBCR_ENC_BT2020:
                return "BT.2020";
            case V4L2_YCBCR_ENC_BT2020_CONST_LUM:
                return "BT.2020 Constant Luminance";
            case V4L2_YCBCR_ENC_SMPTE240M:
                return "SMPTE 240M";
//            case V4L2_HSV_ENC_180:
//                return "HSV with Hue 0-179";
//            case V4L2_HSV_ENC_256:
//                return "HSV with Hue 0-255";
            default:
                return fmt::format("Unknown ({})", val);
        }
    }

    static std::string quantization2s(int val) {
        switch (val) {
            case V4L2_QUANTIZATION_DEFAULT:
                return "Default";
            case V4L2_QUANTIZATION_FULL_RANGE:
                return "Full Range";
            case V4L2_QUANTIZATION_LIM_RANGE:
                return "Limited Range";
            default:
                return fmt::format("Unknown ({})", val);
        }
    }

    static __u32 read_u32(FILE *f) {
        __u32 v;

        fread(&v, 1, sizeof(v), f);
        return ntohl(v);
    }

    static void write_u32(FILE *f, __u32 v) {
        v = htonl(v);
        fwrite(&v, 1, sizeof(v), f);
    }

    static int do_setup_cap_buffers(int fd, buffers &b) {
        for (unsigned i = 0; i < b.bcount; i++) {
            struct v4l2_plane planes[VIDEO_MAX_PLANES];
            struct v4l2_buffer buf {};

            memset(&buf, 0, sizeof(buf));
            memset(planes, 0, sizeof(planes));
            buf.type = b.type;
            buf.memory = b.memory;
            buf.index = i;
            if (b.is_mplane) {
                buf.m.planes = planes;
                buf.length = VIDEO_MAX_PLANES;
            }
            if (v4l2_ioctl(fd, VIDIOC_QUERYBUF, &buf))
                return -1;

            if (b.is_mplane) {
                for (unsigned j = 0; j < b.num_planes; j++) {
                    struct v4l2_plane &p = b.planes[i][j];

                    p.length = planes[j].length;
                    if (b.memory == V4L2_MEMORY_MMAP) {
                        b.bufs[i][j] = v4l2_mmap(
                            nullptr,
                            p.length,
                            PROT_READ | PROT_WRITE, MAP_SHARED,
                            fd,
                            planes[j].m.mem_offset);

                        if (b.bufs[i][j] == MAP_FAILED) {
                            fprintf(stderr, "mmap plane %u failed\n", j);
                            return -1;
                        }
                    } else {
                        b.bufs[i][j] = calloc(1, p.length);
                        planes[j].m.userptr = (unsigned long) b.bufs[i][j];
                    }
                }
            } else {
                struct v4l2_plane &p = b.planes[i][0];

                p.length = buf.length;
                if (b.memory == V4L2_MEMORY_MMAP) {
                    b.bufs[i][0] = v4l2_mmap(
                        nullptr,
                        p.length,
                        PROT_READ | PROT_WRITE, MAP_SHARED,
                        fd,
                        buf.m.offset);

                    if (b.bufs[i][0] == MAP_FAILED) {
                        fprintf(stderr, "mmap failed\n");
                        return -1;
                    }
                } else {
                    b.bufs[i][0] = calloc(1, p.length);
                    buf.m.userptr = (unsigned long) b.bufs[i][0];
                }
            }
            if (v4l2_ioctl(fd, VIDIOC_QBUF, &buf))
                return -1;
        }

        return 0;
    }

    int device::do_handle_cap(
            buffers &b,
            FILE *fout,
            int *index,
            unsigned &count,
            struct timespec &ts_last) {
        char ch = '<';
        int ret;
        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        struct v4l2_buffer buf {};
        static time_t last_sec;

        memset(&buf, 0, sizeof(buf));
        memset(planes, 0, sizeof(planes));

        buf.type = b.type;
        buf.memory = b.memory;
        if (b.is_mplane) {
            buf.m.planes = planes;
            buf.length = VIDEO_MAX_PLANES;
        }

        for (;;) {
            ret = v4l2_ioctl(_fd, VIDIOC_DQBUF, &buf);
            if (ret < 0 && errno == EAGAIN)
                return 0;
            if (ret < 0) {
                fprintf(stderr, "%s: failed: %s\n", "VIDIOC_DQBUF", strerror(errno));
                return -1;
            }
            if (!(buf.flags & V4L2_BUF_FLAG_ERROR))
                break;
            v4l2_ioctl(_fd, VIDIOC_QBUF, &buf);
        }
        if (fout && (!_stream_skip) && !(buf.flags & V4L2_BUF_FLAG_ERROR)) {
            for (unsigned j = 0; j < b.num_planes; j++) {
                __u32 used = b.is_mplane ? planes[j].bytesused : buf.bytesused;
                unsigned offset = b.is_mplane ? planes[j].data_offset : 0;
                unsigned sz;

                if (offset > used) {
                    // Should never happen
                    fprintf(stderr, "offset %d > used %d!\n",
                            offset, used);
                    offset = 0;
                }
                used -= offset;
                sz = static_cast<unsigned int>(fwrite(
                        (char *) b.bufs[buf.index][j] + offset,
                        1,
                        used,
                        fout));

                if (sz != used)
                    fprintf(stderr, "%u != %u\n", sz, used);
            }
        }
        if (buf.flags & V4L2_BUF_FLAG_KEYFRAME)
            ch = 'K';
        else if (buf.flags & V4L2_BUF_FLAG_PFRAME)
            ch = 'P';
        else if (buf.flags & V4L2_BUF_FLAG_BFRAME)
            ch = 'B';
        if (index == nullptr && v4l2_ioctl(_fd, VIDIOC_QBUF, &buf))
            return -1;
        if (index)
            *index = buf.index;

        if (count == 0) {
            clock_gettime(CLOCK_MONOTONIC, &ts_last);
            last_sec = 0;
        } else {
            struct timespec ts_cur {}, res {};

            clock_gettime(CLOCK_MONOTONIC, &ts_cur);
            res.tv_sec = ts_cur.tv_sec - ts_last.tv_sec;
            res.tv_nsec = ts_cur.tv_nsec - ts_last.tv_nsec;

            if (res.tv_nsec < 0) {
                res.tv_sec--;
                res.tv_nsec += 1000000000;
            }

            if (res.tv_sec > last_sec) {
                __u64 fps = 10000ULL * count;

                fps /= (__u64) res.tv_sec * 100ULL + (__u64) res.tv_nsec / 10000000ULL;
                last_sec = res.tv_sec;
                fmt::print(" {}.{} fps", fps / 100ULL, fps % 100ULL);
                fmt::print("\n");
            }
        }

        count++;

        if (_stream_skip) {
            _stream_skip--;
            return 0;
        }

        fmt::print("_stream_count = {}\n", _stream_count);

        if (_stream_count == 0)
            return 0;

        if (--_stream_count == 0)
            return -1;

        return 0;
    }

    static void do_release_buffers(buffers &b) {
        for (unsigned i = 0; i < b.bcount; i++) {
            for (unsigned j = 0; j < b.num_planes; j++) {
                if (b.memory == V4L2_MEMORY_USERPTR)
                    free(b.bufs[i][j]);
                else if (b.memory == V4L2_MEMORY_DMABUF)
                    munmap(b.bufs[i][j], b.planes[i][j].length);
                else
                    v4l2_munmap(b.bufs[i][j], b.planes[i][j].length);
            }
        }
    }

    void device::capture_stream(
            sevun::result &result,
            const std::string& output_path,
            uint32_t stream_count) {
        _stream_count = stream_count;
        _stream_skip = 0;

        struct v4l2_event_subscription sub {};
        int fd_flags = fcntl(_fd, F_GETFL);
        buffers b;
//        bool use_poll = false; //options[OptStreamPoll];
        unsigned count;
        struct timespec ts_last {};
        bool eos;
        bool source_change;
        FILE *fout = nullptr;

        memset(&sub, 0, sizeof(sub));
        sub.type = V4L2_EVENT_EOS;
        ioctl(_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
//        if (use_poll) {
//            sub.type = V4L2_EVENT_SOURCE_CHANGE;
//            ioctl(_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
//        }

        recover:
        eos = false;
        source_change = false;
        count = 0;

        struct v4l2_dv_timings new_dv_timings = {};
        v4l2_std_id new_std;
        struct v4l2_input in = {};

        if (!do_ioctl_name(result, VIDIOC_G_INPUT, &in.index, "VIDIOC_G_INPUT") &&
            !do_ioctl_name(result, VIDIOC_ENUMINPUT, &in, "VIDIOC_ENUMINPUT")) {
            if (in.capabilities & V4L2_IN_CAP_DV_TIMINGS) {
                while (do_ioctl_name(result, VIDIOC_QUERY_DV_TIMINGS, &new_dv_timings, "VIDIOC_QUERY_DV_TIMINGS"))
                    sleep(1);
                do_ioctl_name(result, VIDIOC_S_DV_TIMINGS, &new_dv_timings, "VIDIOC_S_DV_TIMINGS");
                fprintf(stderr, "New timings found\n");
            } else if (in.capabilities & V4L2_IN_CAP_STD) {
                if (!do_ioctl_name(result, VIDIOC_QUERYSTD, &new_std, "VIDIOC_QUERYSTD"))
                    do_ioctl_name(result, VIDIOC_S_STD, &new_std, "VIDIOC_S_STD");
            }
        }

        fout = fopen(output_path.c_str(), "w+");

        if (b.reqbufs(_fd, 3))
            goto done;

        if (do_setup_cap_buffers(_fd, b))
            goto done;

        if (do_ioctl_name(result, VIDIOC_STREAMON, &b.type, "VIDIOC_STREAMON"))
            goto done;

//        while (stream_sleep == 0)
//            sleep(100);

//        if (use_poll)
//            fcntl(_fd, F_SETFL, fd_flags | O_NONBLOCK);

        while (!eos && !source_change) {
            fd_set read_fds;
            fd_set exception_fds;
            struct timeval tv = {0, 0}; //{use_poll ? 2 : 0, 0};
            int r;

            FD_ZERO(&exception_fds);
            FD_SET(_fd, &exception_fds);
            FD_ZERO(&read_fds);
            FD_SET(_fd, &read_fds);
            r = select(
                    _fd + 1,
                    nullptr, //use_poll ? &read_fds : nullptr,
                    nullptr,
                    &exception_fds,
                    &tv);

            if (r == -1) {
                if (EINTR == errno)
                    continue;
                fprintf(stderr, "select error: %s\n",
                        strerror(errno));
                goto done;
            }
//            if (use_poll && r == 0) {
//                fprintf(stderr, "select timeout\n");
//                goto done;
//            }

            if (FD_ISSET(_fd, &exception_fds)) {
                struct v4l2_event ev {};

                while (!ioctl(_fd, VIDIOC_DQEVENT, &ev)) {
                    switch (ev.type) {
                        case V4L2_EVENT_SOURCE_CHANGE:
                            source_change = true;
                            fprintf(stderr, "\nSource changed");
                            break;
                        case V4L2_EVENT_EOS:
                            eos = true;
                            break;
                    }
                }
            }

            if (FD_ISSET(_fd, &read_fds)) {
                r = do_handle_cap(
                        b,
                        fout,
                        nullptr,
                        count,
                        ts_last);
                if (r == -1)
                    break;
            }

        }

        v4l2_ioctl(_fd, VIDIOC_STREAMOFF, &b.type);
        fcntl(_fd, F_SETFL, fd_flags);
        fmt::print("\n");

        do_release_buffers(b);
        if (source_change)
            goto recover;

        done:;
    }

    device::device(const std::string& path): _path(path) {
    }

    device::~device() {
        if (_fd != -1)
            v4l2_close(_fd);
    }

    bool device::open(sevun::result& result) {
        _fd = v4l2_open(_path.c_str(), O_RDWR);

        if (_fd < 0) {
            result.add_message(
                "V004",
                fmt::format("failed to open {}: {}\n", _path, strerror(errno)),
                true);
            return false;
        }

        struct v4l2_capability vcap {};
        memset(&vcap, 0, sizeof(vcap));

        if (!is_sub_device(result)
        &&   do_ioctl_name(result, VIDIOC_QUERYCAP, &vcap, "VIDIOC_QUERYCAP")) {
            result.add_message(
                "V005",
                fmt::format("{}: not a v4l2 node\n", _path),
                true);
            return false;
        }

//        V4L2_CAP_VIDEO_OUTPUT_OVERLAY	0x00000200	The device supports the Video Output Overlay (OSD) interface. Unlike the Video Overlay interface, this is a secondary function of video output devices and overlays an image onto an outgoing video signal. When the driver sets this flag, it must clear the V4L2_CAP_VIDEO_OVERLAY flag and vice versa. [1]
//        V4L2_CAP_HW_FREQ_SEEK	0x00000400	The device supports the ioctl VIDIOC_S_HW_FREQ_SEEK ioctl for hardware frequency seeking.
//        V4L2_CAP_RDS_OUTPUT	0x00000800	The device supports the RDS output interface.
//        V4L2_CAP_MODULATOR	0x00080000	The device has some sort of modulator to emit RF-modulated video/audio signals. For more information about modulator programming see Tuners and Modulators.
//        V4L2_CAP_SDR_CAPTURE	0x00100000	The device supports the SDR Capture interface.
//        V4L2_CAP_EXT_PIX_FORMAT	0x00200000	The device supports the struct v4l2_pix_format extended fields.
//        V4L2_CAP_SDR_OUTPUT	0x00400000	The device supports the SDR Output interface.
//        V4L2_CAP_META_CAPTURE	0x00800000	The device supports the Metadata Interface capture interface.
//        V4L2_CAP_TOUCH	0x10000000	This is a touch device.
//        V4L2_CAP_DEVICE_CAPS	0x80000000	The driver fills the device_caps field. This capability can only appear in the capabilities field and never in the device_caps field.

        _info.driver = std::string(reinterpret_cast<char*>(vcap.driver));
        _info.card = std::string(reinterpret_cast<char*>(vcap.card));
        _info.bus_info = std::string(reinterpret_cast<char*>(vcap.bus_info));
        _info.version = vcap.version;
        _info.capabilities.capture = (vcap.capabilities & V4L2_CAP_VIDEO_CAPTURE) != 0;
        _info.capabilities.capture_mplane = (vcap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) != 0;
        _info.capabilities.output = (vcap.capabilities & V4L2_CAP_VIDEO_OUTPUT) != 0;
        _info.capabilities.output_mplane = (vcap.capabilities & V4L2_CAP_VIDEO_OUTPUT_MPLANE) != 0;
        _info.capabilities.video_m2m = (vcap.capabilities & V4L2_CAP_VIDEO_M2M) != 0;
        _info.capabilities.video_m2m_mplane = (vcap.capabilities & V4L2_CAP_VIDEO_M2M_MPLANE) != 0;
        _info.capabilities.overlay = (vcap.capabilities & V4L2_CAP_VIDEO_OVERLAY) != 0;
        _info.capabilities.vbi_capture = (vcap.capabilities & V4L2_CAP_VBI_CAPTURE) != 0;
        _info.capabilities.vbi_output = (vcap.capabilities & V4L2_CAP_VBI_OUTPUT) != 0;
        _info.capabilities.sliced_vbi_capture = (vcap.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE) != 0;
        _info.capabilities.sliced_vbi_output = (vcap.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT) != 0;
        _info.capabilities.rds_capture = (vcap.capabilities & V4L2_CAP_RDS_CAPTURE) != 0;
        _info.capabilities.tuner = (vcap.capabilities & V4L2_CAP_TUNER) != 0;
        _info.capabilities.audio = (vcap.capabilities & V4L2_CAP_AUDIO) != 0;
        _info.capabilities.radio = (vcap.capabilities & V4L2_CAP_RADIO) != 0;
        _info.capabilities.read_write = (vcap.capabilities & V4L2_CAP_READWRITE) != 0;
        _info.capabilities.async_io = (vcap.capabilities & V4L2_CAP_ASYNCIO) != 0;
        _info.capabilities.streaming = (vcap.capabilities & V4L2_CAP_STREAMING) != 0;

        enumerate_video_formats(result, V4L2_BUF_TYPE_VIDEO_CAPTURE);
        get_capture_format(result);

        return true;
    }

    const device_info_t& device::info() const {
        return _info;
    }

    int device::do_ioctl_name(
            sevun::result& result,
            unsigned long int request,
            void* parm,
            const std::string& name) {
        auto rc = v4l2_ioctl(_fd, request, parm);

        if (rc < 0) {
            result.add_message(
                "V006",
                fmt::format("{}: failed: {}\n", name, strerror(errno)),
                true);
        }

        return rc;
    }

    bool device::is_sub_device(sevun::result& result) const {
        struct stat sb {};

        if (fstat(_fd, &sb) == -1) {
            result.add_message("V001", "failed to stat file.", true);
            return false;
        }

        auto uevent_path = fmt::format(
            "/sys/dev/char/{}:{}/uevent",
            major(sb.st_rdev),
            minor(sb.st_rdev));
        std::ifstream uevent_file(uevent_path);
        if (uevent_file.fail()) {
            result.add_message("V002", "failed to open file.", true);
            return false;
        }

        std::string line;
        while (std::getline(uevent_file, line)) {
            if (line.compare(0, 8, "DEVNAME="))
                continue;

            static const char* devnames[] = {
                "v4l-subdev",
                "video",
                "vbi",
                "radio",
                "swradio",
                "v4l-touch",
                nullptr
            };

            for (size_t i = 0; devnames[i]; i++) {
                size_t len = strlen(devnames[i]);

                if (!line.compare(8, len, devnames[i]) && isdigit(line[8+len])) {
                    uevent_file.close();
                    return i == 0;
                }
            }
        }

        uevent_file.close();

        result.add_message("V003", "unknown device name.", true);
        return false;
    }

    bool device::enumerate_video_formats(
            sevun::result &result,
            uint32_t type) {
        struct v4l2_fmtdesc fmt {};
        struct v4l2_frmsizeenum frmsize {};
        struct v4l2_frmivalenum frmival {};

        fmt.index = 0;
        fmt.type = type;
        while (v4l2_ioctl(_fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
            printf("\tIndex       : %d\n", fmt.index);
            printf("\tType        : %s\n", buftype2s(type).c_str());
            printf("\tPixel Format: '%s'", fcc2s(fmt.pixelformat).c_str());
//            if (fmt.flags)
//                printf(" (%s)", fmtdesc2s(fmt.flags).c_str());
            printf("\n");
            printf("\tName        : %s\n", fmt.description);
            frmsize.pixel_format = fmt.pixelformat;
            frmsize.index = 0;
            while (v4l2_ioctl(_fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
                print_frmsize(frmsize, "\t");
                if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                    frmival.index = 0;
                    frmival.pixel_format = fmt.pixelformat;
                    frmival.width = frmsize.discrete.width;
                    frmival.height = frmsize.discrete.height;
                    while (v4l2_ioctl(_fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0) {
                        print_frmival(frmival, "\t\t");
                        frmival.index++;
                    }
                }
                frmsize.index++;
            }
            printf("\n");
            fmt.index++;
        }
    }

    bool device::get_capture_format(sevun::result &result) {
        struct v4l2_format vfmt {};

        memset(&vfmt, 0, sizeof(vfmt));
        vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (do_ioctl_name(result, VIDIOC_G_FMT, &vfmt, "VIDIOC_G_FMT") == 0) {
            __u32 colsp = vfmt.fmt.pix.colorspace;
            __u32 ycbcr_enc = vfmt.fmt.pix.ycbcr_enc;

            fmt::print("Format Video Capture:\n");

            switch (vfmt.type) {
                case V4L2_BUF_TYPE_VIDEO_CAPTURE:
                    fmt::print("\tWidth/Height      : {}/{}\n", vfmt.fmt.pix.width, vfmt.fmt.pix.height);
                    fmt::print("\tPixel Format      : '{}'\n", fcc2s(vfmt.fmt.pix.pixelformat).c_str());
                    fmt::print("\tField             : {}\n", field2s(vfmt.fmt.pix.field).c_str());
                    fmt::print("\tBytes per Line    : {}\n", vfmt.fmt.pix.bytesperline);
                    fmt::print("\tSize Image        : {}\n", vfmt.fmt.pix.sizeimage);
                    fmt::print("\tColorspace        : {}\n", colorspace2s(colsp).c_str());
                    fmt::print("\tTransfer Function : {}", xfer_func2s(vfmt.fmt.pix.xfer_func).c_str());
                    if (vfmt.fmt.pix.xfer_func == V4L2_XFER_FUNC_DEFAULT)
                        fmt::print(" (maps to {})",
                                   xfer_func2s(V4L2_MAP_XFER_FUNC_DEFAULT(colsp)).c_str());
                    fmt::print("\n");
                    fmt::print("\tYCbCr/HSV Encoding: {}", ycbcr_enc2s(ycbcr_enc).c_str());
                    if (ycbcr_enc == V4L2_YCBCR_ENC_DEFAULT) {
                        ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(colsp);
                        fmt::print(" (maps to {})", ycbcr_enc2s(ycbcr_enc).c_str());
                    }
                    fmt::print("\n");
                    fmt::print("\tQuantization      : {}", quantization2s(vfmt.fmt.pix.quantization).c_str());
                    fmt::print("\n");
                    break;
                default:
                    fmt::print("unknown format\n");
                    break;
            }

            return true;
        }

        return false;
    }
};