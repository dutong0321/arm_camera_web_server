#ifndef CAMERA_H
#define CAMERA_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <linux/fb.h>

#define CONTINUE 0
#define LITTLE   1
#define CLEAR(x) memset (&(x), 0, sizeof (x))
struct buffer {
    void *                  start;
    size_t                  length;
};

class Camera
{
public:
    int camera_init(int width, int height);
    void capture(void *filename);

private:
    FILE *file_fd;
    static int fd;
    static char * dev_name;
    static int  camera_fd;
    static struct buffer *buffers;
    static unsigned int  n_buffers;
    static int read_frame (void);
};
#endif // CAMERA_H
