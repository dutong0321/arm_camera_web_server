#include "camera.h"


char* Camera::dev_name  = "/dev/video3";
int Camera::camera_fd = -1;
unsigned int Camera::n_buffers = 0;
int Camera::fd = 0;
struct buffer *Camera::buffers = NULL;

int Camera::read_frame (void)
{
	struct v4l2_buffer buf;
	int ret = -1;

	CLEAR (buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl (camera_fd, VIDIOC_DQBUF, &buf);
	if(ret < 0){
		printf("error: VIDIOC_DQBUF %s\n", __func__);
		goto err;
	}

	assert(buf.index < n_buffers);

	write(fd, buffers[buf.index].start, buffers[buf.index].length); 
	
	ret = ioctl (camera_fd, VIDIOC_QBUF, &buf); 
	if(ret < 0){
		printf("error: VIDIOC_QBUF %s\n", __func__);
		goto err;
	}

err:
	return 1;
}

int Camera::camera_init(int width, int height)
{
	struct v4l2_capability cap; 
	struct v4l2_format fmt;
	unsigned int i;
    enum v4l2_buf_type type;
	int ret;
	
	camera_fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
	ret = ioctl (camera_fd, VIDIOC_QUERYCAP, &cap);
	if(ret < 0){
		printf("<app> VIDIOC_QUERYCAP error!\n");
		return -1;
	}

	CLEAR (fmt);
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	fmt.fmt.pix.width       = width; 
	fmt.fmt.pix.height      = height;

	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;

	ret = ioctl (camera_fd, VIDIOC_S_FMT, &fmt);
	if(ret < 0){
		printf("<app> VIDIOC_S_FMT error!\n");
		return -1;
	}

	struct v4l2_requestbuffers req;
	CLEAR (req);
	req.count               = 1;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;
	
	ret = ioctl (camera_fd, VIDIOC_REQBUFS, &req); 
	if(ret < 0){
		printf("<app> VIDIOC_REQBUFS error!\n");
		return -1;
	}

	if (req.count < 1)
	   	printf("<app> Insufficient buffer memory\n");

    buffers =(struct buffer *) calloc (req.count, sizeof (*buffers));
	
	for (n_buffers = 0; n_buffers < req.count; ++n_buffers){
		struct v4l2_buffer buf;   
		CLEAR (buf);
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;
		  
		ret = ioctl (camera_fd, VIDIOC_QUERYBUF, &buf);
		if(ret < 0){
			printf("<app> VIDIOC_QUERYBUF error!\n");
			return -1;
		}

	   	buffers[n_buffers].length = buf.length;
	   	buffers[n_buffers].start =
	   	mmap (	NULL /* start anywhere */,    
	    		buf.length,
	    		PROT_READ | PROT_WRITE /* required */,
	    		MAP_SHARED /* recommended */,
	    		camera_fd, buf.m.offset);

	   	if (MAP_FAILED == buffers[n_buffers].start){
	   		printf ("<app> mmap failed\n");
			return -1;
		}
		memset(buffers[n_buffers].start, 0, buf.length); 
        	printf("buf.length = %d\n", buf.length);
	}

	for (i = 0; i < n_buffers; ++i){
	   	struct v4l2_buffer buf;
	   	CLEAR (buf);
	   	buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	   	buf.memory      = V4L2_MEMORY_MMAP;
	   	buf.index       = i;
	   	if (-1 == ioctl (camera_fd, VIDIOC_QBUF, &buf))
	    		printf ("<app> VIDIOC_QBUF failed\n");
	}
			
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (camera_fd, VIDIOC_STREAMON, &type))
	   	printf ("<app> VIDIOC_STREAMON failed\n");
}

void Camera::capture(void *filename)
{
	for (;;) 
	{
	   fd_set fds;
	   struct timeval tv;
	   int r;

	    fd = open((char*)filename, O_CREAT| O_RDWR, 0666);
          if(fd < 0)
          {
            printf("open video.jpg error!\n");    
            exit(0);
          }

	   FD_ZERO (&fds);
	   FD_SET (camera_fd, &fds);

	   /* Timeout. */
	   tv.tv_sec = 2;
	   tv.tv_usec = 0;

	   r = select (camera_fd + 1, &fds, NULL, NULL, &tv);

	   if (-1 == r){
	    	if(EINTR == errno)
	     		continue;
	    	printf ("select err\n");
	   }

	   if (0 == r){
	    	fprintf (stderr, "select timeout\n");
	    	exit (EXIT_FAILURE);
	   }

	    read_frame ();
        break;
	}
	
    	close(fd);

}

