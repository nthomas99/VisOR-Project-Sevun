#include "buffers.h"

sevun::buffers::buffers() {
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    memory = V4L2_MEMORY_MMAP;
    //memory = V4L2_MEMORY_USERPTR;
    //memory = V4L2_MEMORY_DMABUF;
    is_mplane = false;

    for (int i = 0; i < VIDEO_MAX_FRAME; i++)
        for (int p = 0; p < VIDEO_MAX_PLANES; p++)
            fds[i][p] = -1;
    num_planes = is_mplane ? 0 : 1;
}

sevun::buffers::~buffers() {
    for (int i = 0; i < VIDEO_MAX_FRAME; i++)
        for (int p = 0; p < VIDEO_MAX_PLANES; p++)
            if (fds[i][p] != -1)
                close(fds[i][p]);
}

int sevun::buffers::reqbufs(int fd, unsigned buf_count) {
    struct v4l2_requestbuffers reqbufs {};
    int err;

    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.count = buf_count;
    reqbufs.type = type;
    reqbufs.memory = memory;
    err = v4l2_ioctl(fd, VIDIOC_REQBUFS, &reqbufs);
    if (err >= 0)
        bcount = reqbufs.count;
//    if (is_mplane) {
//        struct v4l2_plane planes[VIDEO_MAX_PLANES];
//        struct v4l2_buffer buf {};
//
//        memset(&buf, 0, sizeof(buf));
//        memset(planes, 0, sizeof(planes));
//        buf.type = type;
//        buf.memory = memory;
//        buf.m.planes = planes;
//        buf.length = VIDEO_MAX_PLANES;
//        err = v4l2_ioctl(fd, VIDIOC_QUERYBUF, &buf);
//        if (err)
//            return err;
//        num_planes = buf.length;
//    }
    return err;
}

int sevun::buffers::expbufs(int fd, unsigned type) {
    struct v4l2_exportbuffer expbuf {};
    unsigned i, p;
    int err;

    memset(&expbuf, 0, sizeof(expbuf));
    for (i = 0; i < bcount; i++) {
        for (p = 0; p < num_planes; p++) {
            expbuf.type = type;
            expbuf.index = i;
            expbuf.plane = p;
            expbuf.flags = O_RDWR;
            err = v4l2_ioctl(fd, VIDIOC_EXPBUF, &expbuf);
            if (err < 0)
                return err;
            if (err >= 0)
                fds[i][p] = expbuf.fd;
        }
    }
    return 0;
}
