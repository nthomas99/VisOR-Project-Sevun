#include <vector>
#include <string>
#include <fcntl.h>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <libv4l2.h>
#include <sys/stat.h>
#include <fmt/format.h>
#include <sys/sysmacros.h>
#include <linux/videodev2.h>
#include "device.h"

namespace sevun {

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
};