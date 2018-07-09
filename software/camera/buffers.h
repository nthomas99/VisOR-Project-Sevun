#pragma once

#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <libv4l2.h>
#include <linux/videodev2.h>

namespace sevun {

    class buffers {
    public:
        buffers();

        virtual ~buffers();

        int expbufs(int fd, unsigned type);

        int reqbufs(int fd, unsigned buf_count);

    public:
        unsigned type;
        unsigned memory;
        bool is_mplane;
        unsigned bcount;
        unsigned num_planes;
        struct v4l2_plane planes[VIDEO_MAX_FRAME][VIDEO_MAX_PLANES];
        void *bufs[VIDEO_MAX_FRAME][VIDEO_MAX_PLANES];
        int fds[VIDEO_MAX_FRAME][VIDEO_MAX_PLANES];
        unsigned bpl[VIDEO_MAX_PLANES];
    };

}